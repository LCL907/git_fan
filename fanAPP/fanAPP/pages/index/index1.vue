<template>
	<view>
		<view class="content">
			<image class="logo" src="/static/ble.png"></image>
		</view>
		<view class="text-area">
			<text class="title">{{title}}</text>
		</view>
		<div class="list">
			<text class="title">{{title2}}</text>
			<!-- 注意事项: 不能使用 index 作为 key 的唯一标识 -->
			<view v-if="devicelist && devicelist.length > 0">
				<view v-for="(item, index) in devicelist" :key="item.id || item.name || item.mac"
					@click="wificonnected(item)" class="connectedlist">
					<text class="connected">{{item.name || item.deviceName || item}}</text>
				</view>
			</view>
			<view v-else class="empty-list">暂无在线设备</view>
		</div>
		<div class="list">
			<text class="title">{{title1}}</text>
			<!-- 注意事项: 不能使用 index 作为 key 的唯一标识 -->
			<view v-if="servicelist && servicelist.length > 0">
				<view v-for="(item, index) in servicelist" :key="item.id || item.name || item.deviceId"
					@click="onDeviceClick(item)" class="connectedlist">
					<text class="connected">{{item.name || '未命名设备'}}</text>
				</view>
			</view>
			<view v-else class="empty-list">暂无配对蓝牙</view>
		</div>
		<view class="uni-padding-wrap uni-common-mt">
			<view class="uni-btn-v">
				<button type="primary" @click="toBle">添加蓝牙</button>
				<button type="primary" @click="toWIFI">添加WIFI</button>
				<!-- <button type="primary" @click="controlFan">
					WIFI控制
				</button> -->
			</view>
		</view>
	</view>
</template>

<script>
import config from '@/config.js';

export default {
	data() {
		return {
			title: 'Hello,',
			title1: '已配对蓝牙列表',
			title2: 'WIFI在线设备',
			servicelist: [],
			devicelist: [],
			name: [],
			currentDeviceId: '' // 当前选中的设备ID
		}
	},
	onLoad() {
		var that = this; // 保留this上下文，兼容旧代码
		this.condition = null; // 设备状态条件
		
		// 获取已存储的设备ID
		var tmpDeviceId = uni.getStorageSync('deviceId');
		console.log('存储的设备ID:', tmpDeviceId);
		
		// 检查设备ID是否存在
		if (tmpDeviceId && tmpDeviceId.length > 0) {
			this.currentDeviceId = tmpDeviceId[0].deviceId;
			console.log('当前设备ID:', this.currentDeviceId);
		} else {
			console.warn('未找到存储的设备ID');
		}
		
		// 存储当前条件状态
		uni.setStorageSync("current", this.condition);
		
		// 初始化蓝牙模块
		uni.openBluetoothAdapter({
			success: (res) => { // 初始化成功
				uni.getBluetoothAdapterState({ // 蓝牙匹配状态
					success: (res1) => {
						console.log(res1, '"本机蓝牙已打开"')
					},
					fail() {
						uni.showToast({
							icon: 'none',
							title: '查看本机蓝牙是否打开'
						})
					}
				})
			},
			fail: (errMsg) => {
				console.log(errMsg);
			}
		});
		
		// 获取已配对蓝牙设备
		this.servicelist = uni.getStorageSync('device');
		this.servicelist.forEach((item, index) =>
			console.log(item.name, index)
		);
		
		// 获取wifi在线设备（使用箭头函数保留this上下文）
		setInterval(() => {
			// 使用组件data中的currentDeviceId，而不是闭包变量
			if (!this.currentDeviceId) {
				console.warn('设备ID为空，跳过在线设备查询');
				return;
			}
			
			uni.request({
				url: config.hostUrl + '/UserDevice/IsDeviceOnline/',
				method: "POST",
				data: {
					deviceId: this.currentDeviceId
				},
				success: (res) => {
					if (res.data && res.data.status === true) {
						// 确保返回的是数组
						this.devicelist = res.data.deviceList || [];
						this.devicelist.forEach((item, index) => 
							console.log(`在线设备[${index}]:`, item)
						);
						console.log(res.data.message);
					} else {
						this.devicelist = []; // 清空列表
						console.log('获取在线设备失败:', res.data.message || '未知错误');
					}
				},
				fail: (err) => {
					console.error('API请求失败:', err);
				}
			});
		}, 5000);
	},
	methods: {
		reNew() {
			// 刷新页面
			location.reload();
		},
		
		onDeviceClick(item) {
			console.log('连接蓝牙设备:', item.name);
			console.log('设备详情:', item);
			
			// 确保设备ID存在
			if (!item.deviceId) {
				uni.showToast({
					icon: 'none',
					title: '设备ID不存在'
				});
				return;
			}
			
			// 连接蓝牙设备
			uni.createBLEConnection({
				deviceId: item.deviceId,
				success: (res) => {
					console.log('蓝牙连接成功:', res);
					uni.showToast({
						icon: "success",
						duration: 1000
					});
					
					// 跳转到控制页面
					uni.navigateTo({
						url: '../control/control',
						success: () => {
							console.log('跳转成功');
						},
						fail: (res) => {
							console.log('跳转失败:', res);
						}
					});
				},
				fail: (res) => {
					console.log("蓝牙连接失败:", res);
					
					// 特殊错误码处理
					if (res.errCode == -1) {
						uni.navigateTo({
							url: '../control/control',
							success: () => {
								console.log('跳转成功');
							},
							fail: (res) => {
								console.log('跳转失败:', res);
							}
						});
					} else {
						uni.showToast({
							icon: 'none',
							title: `蓝牙连接失败: ${res.errMsg}`
						});
					}
				}
			});
		},
		
		toBle() {
			// 跳转到蓝牙添加页面
			uni.navigateTo({
				url: '../API/bluetooth',
				success: () => {
					console.log('跳转成功');
				},
				fail: (res) => {
					console.log('跳转失败:', res);
				}
			});
		},
		
		toWIFI() {
			// 跳转到WiFi添加页面
			uni.navigateTo({
				url: '../WIFI/wifi',
				success: () => {
					console.log('跳转成功');
				},
				fail: (res) => {
					console.log('跳转失败:', res);
				}
			});
		},
		
		wificonnected(item) {
			// 跳转到WiFi控制页面
			uni.navigateTo({
				url: '../control/wificontrol',
				success: () => {
					console.log('跳转成功');
				},
				fail: (res) => {
					console.log('跳转失败:', res);
				}
			});
		}
	}
}
</script>

<style>
/* 样式保持不变 */
.content {
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: center;
}

.logo {
	height: 200rpx;
	width: 200rpx;
	margin-top: 200rpx;
	margin-left: auto;
	margin-right: auto;
	margin-bottom: 50rpx;
}

.text-area {
	display: flex;
	justify-content: center;
}

.title {
	font-size: 36rpx;
	color: #8f8f94;
}

.connected {
	text-align: center;
	height: 40upx;
	line-height: 40upx;
	font-size: medium;
}

.connectedlist {
	float: left;
	border-bottom: #999999 solid 1px;
	height: 40upx;
	line-height: 40upx;
	padding: 3% 0 3% 2%;
	width: 70%;
	margin: 1% 15%;
	text-align: center;
}

.list {
	float: left;
	width: 100%;
	padding: 3% 0 3% 2%;
}

/* 新增空列表样式 */
.empty-list {
	text-align: center;
	color: #999;
	padding: 10rpx 0;
}
</style>