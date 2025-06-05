# services.py
import json
from django.http import HttpResponse
from django.core.cache import cache
from channels.generic.websocket import AsyncWebsocketConsumer
from asgiref.sync import async_to_sync
from typing import List, Dict, Any

# 设备连接管理（使用Django缓存模拟内存存储，支持异步操作）
DEVICE_SOCKETS_CACHE_KEY = "device_sockets"


class DeviceControlService:
    @staticmethod
    def _get_sockets() -> Dict[str, Any]:
        """获取设备连接字典（从缓存中读取或初始化）"""
        sockets = cache.get(DEVICE_SOCKETS_CACHE_KEY)
        if sockets is None:
            sockets = {}
            cache.set(DEVICE_SOCKETS_CACHE_KEY, sockets, None)  # 永久缓存
        return sockets

    @staticmethod
    def _save_sockets(sockets: Dict[str, Any]):
        """保存设备连接字典到缓存"""
        cache.set(DEVICE_SOCKETS_CACHE_KEY, sockets, None)

    @staticmethod
    async def handle_websocket(scope, receive, send):
        """处理WebSocket连接（类似C#的Invoke方法）"""
        if scope["type"] != "websocket":
            return

        # 接受WebSocket连接
        await send({
            "type": "websocket.accept",
        })

        try:
            while True:
                event = await receive()
                if event["type"] == "websocket.receive":
                    # 解析消息（假设消息格式为JSON，包含"device_id"和"message"字段）
                    try:
                        message = json.loads(event["text"])
                        device_id = message.get("device_id")
                        if device_id:
                            # 注册设备连接
                            DeviceControlService.regist_device(device_id, send)
                    except json.JSONDecodeError:
                        pass
                elif event["type"] == "websocket.disconnect":
                    # 连接断开时移除设备
                    DeviceControlService.remove_device_by_socket(send)
                    break
        except Exception as e:
            print(f"WebSocket error: {e}")
            await send({
                "type": "websocket.close",
                "code": 1000,
                "reason": "Server error"
            })

    @staticmethod
    def regist_device(device_id: str, send_func):
        """注册设备连接（类似C#的RegistDevice方法）"""
        sockets = DeviceControlService._get_sockets()
        sockets[device_id] = send_func  # 存储发送函数用于消息推送
        DeviceControlService._save_sockets(sockets)

    @staticmethod
    def remove_device_by_socket(send_func):
        """根据连接移除设备"""
        sockets = DeviceControlService._get_sockets()
        # 反向查找socket对应的device_id（可能存在多个相同send_func，此处简化处理）
        device_id = next((k for k, v in sockets.items() if v == send_func), None)
        if device_id:
            del sockets[device_id]
            DeviceControlService._save_sockets(sockets)

    @staticmethod
    async def send_message(device_id: str, message: str):
        """发送消息到指定设备（类似C#的SendMessage方法）"""
        sockets = DeviceControlService._get_sockets()
        if device_id not in sockets:
            print(f"设备 {device_id} 不在线")
            return False

        send_func = sockets[device_id]
        try:
            # 发送文本消息（Django Channels需要包装为指定格式）
            await send_func({
                "type": "websocket.send",
                "text": message
            })
            return True
        except Exception as e:
            print(f"发送消息失败: {e}")
            return False

    @staticmethod
    def is_device_online(device_id: str) -> bool:
        """检查设备在线状态（类似C#的IsDeviceOnline方法）"""
        return device_id in DeviceControlService._get_sockets()