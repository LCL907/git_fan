/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   创建一个BLE服务器，一旦我们收到连接，将会周期性发送通知

   T使用步骤：
   1. 创建一个 BLE Server
   2. 创建一个 BLE Service
   3. 创建一个 BLE Characteristic
   4. 创建一个 BLE Descriptor
   5. 开始服务
   6. 开始广播


*/

// 引入所需的库文件
#include <Arduino.h>          // Arduino核心库
#include <BLEDevice.h>        // BLE设备库
#include <BLEServer.h>        // BLE服务器库
#include <BLEUtils.h>         // BLE工具库
#include <BLE2902.h>          // BLE 2902描述符库
#include <Robojax_L298N_DC_motor.h> // 电机驱动库
#include <ArduinoJson.h>      // JSON解析库
#include <WiFi.h>             // WiFi连接库
#include <WiFiMulti.h>        // 多WiFi连接库
#include <WiFiClientSecure.h> // 安全WiFi客户端库
#include <WebSocketsClient_Generic.h> // WebSocket客户端库
#include <DHT.h>              // DHT温湿度传感器库

// DHT11传感器定义
#define DHTPIN 33             // DHT11连接到GPIO33 (D33)
#define DHTTYPE DHT11         // 传感器类型为DHT11
DHT dht(DHTPIN, DHTTYPE);     // 创建DHT对象

// 全局变量定义
uint8_t txValue = 0;          // 发送值，用于BLE通知
BLEServer *pServer = NULL;    // BLEServer指针
BLECharacteristic *pTxCharacteristic = NULL; // 发送特征值指针
BLECharacteristic *reTxCharacteristic = NULL; // 读取特征值指针

bool deviceConnected = false;  // 当前BLE连接状态
bool oldDeviceConnected = false; // 上一次BLE连接状态

// 命令相关变量
String commandResult;         // 命令结果字符串
int command;                  // 命令编号
int speed;                    // 风扇速度
bool method;                  // 方法标志（开/关）
String WIFIName;              // WiFi名称
String PassWord;              // WiFi密码

// 湿度控制相关变量
float humidityThreshold = 60.0; // 湿度阈值，低于此值自动关闭风扇
bool fanAutoControl = false;    // 风扇是否处于自动控制状态
unsigned long lastDHTReadTime = 0; // 上次读取DHT11的时间
bool manualFanOff = false;      // 是否手动关闭风扇标志

//-------------------------------------------------------------------------------------------------
// 风扇马达定义
#define CHA 0                 // 通道0
#define ENA 17                // 使能引脚，必须是PWM引脚
#define IN1 25                // 电机控制引脚1
#define IN2 26                // 电机控制引脚2
#define motor1 1              // 电机编号

const int CCW = 2;            // 逆时针旋转
const int CW = 1;             // 顺时针旋转

// 创建电机对象，true表示启用串口调试
Robojax_L298N_DC_motor motor(IN1, IN2, ENA, CHA, true);
//-------------------------------------------------------------------------------------------------

// UUID定义，用于BLE服务和特征值
// 可以在 https://www.uuidgenerator.net/ 生成UUID
#define SERVICE_UUID "0b466f5e-1c11-413e-92cf-986e42af4861" // 服务UUID
#define CHARACTERISTIC_UUID_RX "19d8d9e0-7d45-430e-aa67-2f4169eb2e22" // 接收特征值UUID
#define CHARACTERISTIC_UUID_TX "7ee6c5c4-dbb2-4981-8e22-700846a4f83f" // 发送特征值UUID
#define CHARACTERISTIC_UUID_RETX "e1841616-4da8-4ea8-8407-ac1d2439ef51" // 读取特征值UUID
#define DEVICE_UUID "5A:1B:A6:3D:8A" // 设备唯一ID
//-------------------------------------------------------------------------------------------------

// BLE服务器回调类，处理连接和断开事件
class MyServerCallbacks : public BLEServerCallbacks
{
    // 当设备连接时触发
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    };

    // 当设备断开连接时触发
    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
    }
};
//-------------------------------------------------------------------------------------------------
// 定义WebSocket相关变量
#define USE_SERIAL Serial     // 使用Serial作为调试输出
#define DEBUG_ESP_PORT Serial // 调试端口
#define WS_SERVER "192.168.115.71" // WebSocket服务器地址
#define WS_PORT 5000          // WebSocket服务器端口
#define WS_SSID "mi"   // WiFi名称
#define WS_PASSWORD "2575963156A" // WiFi密码
#define USE_SSL false         // 是否使用SSL

// 创建WiFi和WebSocket对象
WiFiMulti wifiMulti;          // WiFi多连接对象
WebSocketsClient webSocket;   // WebSocket客户端对象

// 函数声明
void connectWIFI();           // 连接WiFi函数
void WIFI_Command();          // WiFi命令处理函数
//-------------------------------------------------------------------------------------------------
// 定义JSON对象，用于解析前端传来的命令
JsonObject inputValue;

// 解析JSON命令的函数
JsonObject get_jsoncomm(std::string rxvalue)
{
  // 设置json字符串的大小
  const size_t capacity = 2048;
  DynamicJsonDocument doc(capacity);
  
  // 将接收到的字符串转换为String类型
  String input = rxvalue.c_str();
  
  // 解析JSON数据
  deserializeJson(doc, input);

  // 获取JSON对象
  JsonObject tmp = doc.as<JsonObject>();
  
  // 解析各个字段
  commandResult = tmp[String("command")].as<String>();
  command = commandResult.toInt();
  speed = tmp[String("speed")].as<int>();
  method = tmp["method"];
  WIFIName = tmp[String("params")][String("wifiname")].as<String>();
  PassWord = tmp[String("params")][String("password")].as<String>();
  
  // 解析自动控制相关参数（如果存在）
  if (tmp.containsKey("autoControl")) {
    fanAutoControl = tmp["autoControl"];
  }
  if (tmp.containsKey("humidityThreshold")) {
    humidityThreshold = tmp["humidityThreshold"];
  }
  
  // 输出解析到的命令
  Serial.print("get_jsoncomm_command:");
  Serial.println(command);
  return tmp;
}
//-------------------------------------------------------------------------------------------------
// 扫描可用WiFi网络并通过BLE发送
void StartScanWIFI()
{
    // 扫描可用WiFi网络
    int n = WiFi.scanNetworks();
    int WIFI_SSID = 0;
    
    // 如果没有找到网络
    if (n==0)
    {
        Serial.println("no networks were found");
    }
    else
    {
        // 输出找到的网络数量
        Serial.print(n);
        Serial.println("networks were found");
        
        // 遍历所有找到的网络
        for (int i = 0; i < n; ++i)
        {
          // 串口输出WiFi的名字、信号强度、是否加密
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.print(WiFi.SSID(i));
          Serial.print(" (");
          Serial.print(WiFi.RSSI(i));
          Serial.print(")");
          Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");

          // 获取WiFi名称
          String ssid = WiFi.SSID(i);
          Serial.println(ssid);
          Serial.println(ssid.length());
          
          try
          {
            // 将String转换为const char*
            const char *a = ssid.c_str();

            Serial.println("before type change");
            // 将const char*转换为std::string
            std::string tmpContent = a;
            Serial.println("end type change");
            Serial.println(tmpContent.length());

            // 通过BLE发送WiFi名称
            pTxCharacteristic->setValue(tmpContent);
            pTxCharacteristic->notify();
            Serial.println("设备wifi通知已发出");
          }
          catch (std::exception e)
          {
            Serial.println(e.what());
          }
        }
    }
}
//-------------------------------------------------------------------------------------------------
// 将二进制数据转换为十六进制字符串并打印
void hexdump(const void *mem, uint32_t len, uint8_t cols = 16)
{
  const uint8_t *src = (const uint8_t *)mem;

  // 打印内存地址和长度信息
  Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);

  // 逐字节打印十六进制值
  for (uint32_t i = 0; i < len; i++)
  {
    // 每行开始时打印地址
    if (i % cols == 0)
    {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }

    // 打印字节的十六进制值
    Serial.printf("%02X ", *src);
    src++;
  }
  Serial.printf("\n");
}
//-------------------------------------------------------------------------------------------------
// 向WebSocket服务器注册设备
void registerDevice()
{
  // 创建JSON文档
  DynamicJsonDocument doc(1024);

  // 设置消息类型和参数
  doc["MessageType"] = "Register";
  doc["Params"][0] = DEVICE_UUID;
  
  // 将JSON转换为字符串
  String result;
  serializeJson(doc, result);
  
  // 发送注册消息
  webSocket.sendTXT(result);
  Serial.println("-------------------------------------");
}
//-------------------------------------------------------------------------------------------------
// WebSocket事件处理函数
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    Serial.println("webSocketEvent");
    Serial.println(type);

    // 根据事件类型处理
    switch (type)
    {
    // 断开连接
    case WStype_DISCONNECTED:
    {
        Serial.println("[WSc] Disconnected!");
      break;
    }
    // 连接成功
    case WStype_CONNECTED:
    {
      Serial.print("[WSc] Connected to url: ");
      Serial.println((char *)payload);
      // 连接成功后发送注册设备的请求
      registerDevice();
      Serial.print("already sent RegisterMessage!");
    }
    break;
    // 接收到文本消息
    case WStype_TEXT:
    {
      Serial.printf("[WSc] get text: %s\n", payload);
      // 解析JSON命令并执行
      get_jsoncomm(std::string((char *)payload));
      WIFI_Command();
    }
    break;
    // 接收到二进制数据
    case WStype_BIN:
    {
      Serial.printf("[WSc] get binary length: %u\n", length);
      hexdump(payload, length);

      // 发送二进制数据回服务器
      webSocket.sendBIN(payload, length);
    }
    break;
    // 接收到PING
    case WStype_PING:
      // 自动回应PONG
      Serial.printf("[WSc] get ping\n");
      break;
    // 接收到PONG
    case WStype_PONG:
      // 对我们发送的PING的回应
      Serial.printf("[WSc] get pong\n");
      break;
    // 发生错误
    case WStype_ERROR:
      Serial.printf("[WSc] get ERROR\n");
      break;
    // 文本片段开始
    case WStype_FRAGMENT_TEXT_START:
      Serial.printf("[WSc] get FRAGMENT_TEXT\n");
      break;
    // 二进制片段开始
    case WStype_FRAGMENT_BIN_START:
      Serial.printf("[WSc] get FRAGMENT_BIN\n");
      break;
    // 片段
    case WStype_FRAGMENT:
      Serial.printf("[WSc] get FRAGMENT\n");
      break;
    // 片段结束
    case WStype_FRAGMENT_FIN:
      Serial.printf("[WSc] get FRAGMENT_FIN\n");
      break;
    // 默认情况
    default:
      Serial.printf("[WSc] default\n");
      break;
    }
}
//-------------------------------------------------------------------------------------------------
// 处理通过WiFi/WebSocket接收到的命令
void WIFI_Command()
{
  Serial.println(command);
  Serial.println("\nWIFI");
  
  // 根据命令编号执行相应操作
  switch (command)
  {
    // 风扇开关控制（命令3）
    case 3:
    {
      if (method)
      {
        // 开启风扇
        Serial.println("fan has been on");
        motor.rotate(motor1, 10, CCW);
        fanAutoControl = true; // 开启风扇时自动启用湿度控制
        manualFanOff = false; // 重置手动关闭标志
        Serial.println("Humidity control enabled");
        /*开电扇*/
      }
      else
      {
        // 关闭风扇
        Serial.println("fan has been off");
        motor.brake(1);
        fanAutoControl = false; // 关闭风扇时禁用湿度控制
        manualFanOff = true; // 设置手动关闭标志
        /*关电扇*/
      }
    }
    break;
    // 风扇档速调节（命令4）
    case 4:
    {
      Serial.println("WIFI CONTROL");
      Serial.println(speed);
      
      // 根据速度设置风扇转速
      if (speed == 1)
      {
        // 1档速度
        Serial.println("1 Gear");
        motor.rotate(motor1, 40, CCW);
      }
      else if (speed == 2)
      {
        // 2档速度
        Serial.println("2 Gear");
        motor.rotate(motor1, 70, CCW);
      }
      else if (speed == 3)
      {
        // 3档速度
        Serial.println("3 Gear");
        motor.rotate(motor1, 100, CCW);
      }
      else
      {
        // 无效速度
        Serial.println("无效命令");
      }
    }
    break;
    // 其他命令
    default:
    {
      Serial.println("无效命令");
    }
    break;
  }
}
//-------------------------------------------------------------------------------------------------
// 读取DHT11传感器数据并根据湿度自动控制风扇
void checkHumidityAndControlFan()
{
  // 每2秒读取一次传感器数据
  unsigned long currentMillis = millis();
  if (currentMillis - lastDHTReadTime >= 2000) {
    lastDHTReadTime = currentMillis;
    
    // 读取湿度和温度
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    
    // 检查读取是否成功
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    
    // 打印当前温湿度
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("% | Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");
    
    // 只有在启用了自动控制且非手动关闭状态时才进行控制
    if (fanAutoControl && !manualFanOff) {
      // 根据湿度范围设置不同的风扇转速
      if (humidity <= humidityThreshold) {
        // 湿度低于阈值，关闭风扇
        if (method) {
          Serial.println("Auto: fan turned off (humidity below threshold)");
          method = false; // 更新风扇状态为关闭
          motor.brake(1); // 关闭风扇
        }
      } else if (humidity > humidityThreshold && humidity <= humidityThreshold + 10) {
        // 湿度略高于阈值，开启1档风扇
        Serial.println("Auto: fan set to speed 1 (low humidity)");
        method = true; // 更新风扇状态为开启
        motor.rotate(motor1, 30, CCW); // 1档速度
      } else if (humidity > humidityThreshold + 10 && humidity <= humidityThreshold + 20) {
        // 湿度中等，开启2档风扇
        Serial.println("Auto: fan set to speed 2 (medium humidity)");
        method = true; // 更新风扇状态为开启
        motor.rotate(motor1, 75, CCW); // 2档速度
      } else if (humidity > humidityThreshold + 20) {
        // 湿度很高，开启3档风扇
        Serial.println("Auto: fan set to speed 3 (high humidity)");
        method = true; // 更新风扇状态为开启
        motor.rotate(motor1, 100, CCW); // 3档速度
      }
    }
  }
}
//-------------------------------------------------------------------------------------------------
// 处理通过BLE接收到的命令
void BLE_command()
{
  Serial.println("");
  Serial.println("BLE");
  Serial.print("BLE_command:");
  Serial.println(command);

  // 根据命令编号执行相应操作
  switch (command)
  {
  // 扫描WiFi网络（命令1）
  case 1:
  {
    StartScanWIFI();
  }
  break;
  // 连接WiFi（命令2）
  case 2:
  {
    Serial.println("------------------------------------------------");
    Serial.println(WIFIName);
    Serial.println(PassWord);
    Serial.println("------------------------------------------------");
    
    // 接收从手机端传过来的WIFI名称密码
    // c_str()函数将String类型转换为const char*类型
    const char *name = WIFIName.c_str();
    const char *psw = PassWord.c_str();
    
    // 开始连接WiFi
    WiFi.begin(name,psw);
    
    // millis函数获取Arduino开机后运行的时间长度（毫秒）
    double start = millis();
    
    // isOverTime用来判断WIFI连接是否已经超时
    int isOverTime = 0;
    
    // 等待连接成功或超时
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WIFI CONNECTING...");
        delay(1000);
        if (millis() - start >10000) // 10秒超时
        {
            isOverTime = 1;
            break;
        }
    }
    
    // 连接结果处理
    if (!isOverTime) // 连接成功
    {
        pTxCharacteristic -> setValue("true");
        Serial.println("SUCCESS!");
        reTxCharacteristic -> setValue("WIFI CONNECTED!");
        connectWIFI(); // 连接WebSocket服务器
    }
    else // 连接失败
    {
        pTxCharacteristic -> setValue("false");
        Serial.println("Fail to connect!");
        reTxCharacteristic -> setValue("WIFI NOT CONNECTED!");
    }
    
    // 发送通知
    pTxCharacteristic -> notify();
  }
  break;
  // 风扇开关控制（命令3）
  case 3:
  {
    if (method)
    {
      // 开启风扇
      Serial.println("fan has been on");
      motor.rotate(motor1, 10, CCW);
      fanAutoControl = true; // 开启风扇时自动启用湿度控制
      manualFanOff = false; // 重置手动关闭标志
      Serial.println("Humidity control enabled");
      /*开电扇*/
    }
    else
    {
      // 关闭风扇
      Serial.println("fan has been off");
      motor.brake(1);
      fanAutoControl = false; // 关闭风扇时禁用湿度控制
      manualFanOff = true; // 设置手动关闭标志
      /*关电扇*/
    }
  }
  break;
  // 风扇档速调节（命令4）
  case 4:
  {
    // 根据速度设置风扇转速
    if (speed == 1)
    {
      // 1档速度
      Serial.println("1 gear");
      motor.rotate(motor1, 30, CCW);
    }
    else if (speed == 2)
    {
      // 2档速度
      Serial.println("2 gear");
      motor.rotate(motor1, 75, CCW);
    }
    else if (speed == 3)
    {
      // 3档速度
      Serial.println("3 gear");
      motor.rotate(motor1, 100, CCW);
    }
    else
    {
      // 无效速度
      Serial.println("无效命令");
    }
  }
  break;
 }
}
//-------------------------------------------------------------------------------------------------
// 连接WiFi并建立WebSocket连接
void connectWIFI()
{
    // 添加预设的WiFi接入点
    wifiMulti.addAP(WS_SSID, WS_PASSWORD);
    double start = millis();
    bool OverTime = false;

    // 等待连接成功或超时
    while (wifiMulti.run() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(1000);
        if (millis() - start > 10000) // 10秒超时
        {
            Serial.println("fail");
            return;
        }
    }
    
    // 输出客户端IP地址
    Serial.print("WebSockets Client Start at IP : ");
    Serial.print(WiFi.localIP());
    
    // 输出服务器地址和端口
    Serial.print("Connecting to Server at:");
    Serial.println(WS_SERVER);

    // 根据是否使用SSL选择连接方式
    #if USE_SSL
    webSocket.beginSSL(WS_SERVER,WS_PORT);
    #else
    webSocket.begin(WS_SERVER,WS_PORT,"/ws");
    #endif
    
    Serial.println("here we are");
    
    // 设置事件处理器
    webSocket.onEvent(webSocketEvent);

    Serial.println("hello!!");

    // 设置重连间隔为5秒
    webSocket.setReconnectInterval(5000);

    // 禁用心跳
    webSocket.disableHeartbeat();

    // 输出连接成功信息
    Serial.print("Connected to WebSockets Server at IP address: ");
    Serial.println(WS_SERVER);
}
//-------------------------------------------------------------------------------------------------

// BLE特征值回调类，处理接收到的数据
class MyCallbacks : public BLECharacteristicCallbacks
{
  // 当特征值被写入时触发
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    Serial.println("got Ble");
    // 获取接收到的数据
    std::string rxvalue = pCharacteristic->getValue();
    Serial.println("begin");
    // 解析JSON命令
    get_jsoncomm(rxvalue);
    Serial.print("onWriteCommand:");
    Serial.println(command);
    Serial.println("end");
    // 执行BLE命令
    BLE_command();
  }
};
//-------------------------------------------------------------------------------------------------

// 初始化设置
void setup()
{
    // 设置串口波特率
    Serial.begin(115200);

    // 初始化DHT11传感器
    dht.begin();
    Serial.println("DHT11 sensor initialized");

    // 创建BLE设备
    BLEDevice::init("莳山纯情男大");

    // 创建BLE服务器
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks()); // 设置回调
    
    // 创建BLE服务
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // 创建BLE特征值
    // 发送特征值（通知类型）
    pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());
    
    // 接收特征值（写入类型）
    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
    pRxCharacteristic->setCallbacks(new MyCallbacks()); // 设置回调
    
    // 读取特征值（读取类型）
    reTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RETX, BLECharacteristic::PROPERTY_READ);

    // 启动服务和广播
    pService->start();
    pServer->getAdvertising()->start();
    Serial.println(" Waiting... ");

    //------------------------------------------------------
    // 初始化风扇马达
    motor.begin();
    
    //------------------------------------------------------
    // 连接WiFi
    connectWIFI();
    
    // 设置WiFi连接状态
    if(wifiMulti.run() != WL_CONNECTED)
    {
        reTxCharacteristic->setValue("WIFI Not Connected");
    }else{
        reTxCharacteristic->setValue("WIFI Connected");
    }
}

// 主循环
void loop()
{
    // 处理BLE连接状态
    
    // 如果设备已连接
    if (deviceConnected)
    {
        // 发送通知
        pTxCharacteristic->setValue(&txValue, 1);
        pTxCharacteristic->notify();
        txValue++; // 值自增
        delay(500); // 延时防止蓝牙堵塞
    }

    // 如果设备刚断开连接
    if (!deviceConnected && oldDeviceConnected)
    {
        delay(500); // 给蓝牙缓冲时间
        pServer->startAdvertising(); // 重新开始广播
        Serial.println(" Start Advertising... ");
        oldDeviceConnected = deviceConnected;
    }

    // 如果设备刚连接
    if (deviceConnected && !oldDeviceConnected)
    {
        // 可以在这里添加连接时的处理代码
        oldDeviceConnected = deviceConnected;
    }

    // 检查湿度并根据需要自动控制风扇
    checkHumidityAndControlFan();

    // 处理WebSocket事件
    webSocket.loop();
}
