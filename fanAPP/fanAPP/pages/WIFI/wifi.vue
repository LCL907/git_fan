<template>
	<view class="uni-content">
		<text class="title title-box">WIFI连接</text>
		<uni-forms>
			<uni-forms-item name="username">
				<uni-easyinput 
					:focus="focusUsername" 
					@blur="focusUsername = false" 
					class="input-box"
					:inputBorder="false" 
					v-model="username" 
					placeholder="请输入WIFI名称"
				></uni-easyinput>
			</uni-forms-item>
			<uni-forms-item name="password">
				<uni-easyinput 
					:focus="focusPassword" 
					@blur="focusPassword = false" 
					class="input-box" 
					clearable
					type="password" 
					:inputBorder="false" 
					v-model="password" 
					placeholder="请输入WIFI密码"
				></uni-easyinput>
			</uni-forms-item>
		</uni-forms>
		<button class="uni-btn" type="primary" @click="pwdWIFI">登录</button>
		<button class="uni-btn" type="primary" @click="navigateBack">返回</button>
	</view>
</template>

<script>
import config from '@/config.js';

export default {
	data() {
		return {
			password: "",
			username: "",
			focusUsername: false,
			focusPassword: false
		}
	},
	methods: {
		controlFan() {
			uni.navigateTo({
				url: '../control/wificontrol',
				success: () => {
					console.log('跳转成功');
				},
				fail: (res) => {
					console.log('跳转失败:', res);
				}
			});
		},

		// 兼容多环境的字符串转 ArrayBuffer 方法（优先使用 UTF-8 编码）
		str2ab(str) {
			// 检查是否支持 TextEncoder（现代浏览器/小程序）
			if (typeof TextEncoder !== 'undefined') {
				return new TextEncoder().encode(str); // 返回 Uint8Array（自动兼容 ArrayBuffer）
			}
			// 旧环境兼容方案（仅适用于 ASCII 字符，中文需额外处理）
			const buffer = new ArrayBuffer(str.length);
			const uint8Array = new Uint8Array(buffer);
			for (let i = 0; i < str.length; i++) {
				uint8Array[i] = str.charCodeAt(i) & 0xFF; // 截断为单字节（避免高字节错误）
			}
			return uint8Array;
		},

		pwdWIFI() {
			// 输入校验
			if (!this.password.length) {
				this.focusPassword = true;
				return uni.showToast({ title: '请输入密码', icon: 'none' });
			}
			if (!this.username.length) {
				this.focusUsername = true;
				return uni.showToast({ title: '请输入WIFI名称', icon: 'none' });
			}

			// 获取蓝牙设备信息
			const deviceInfo = uni.getStorageSync('deviceId') || [];
			const serviceInfo = uni.getStorageSync('serviceId') || [];
			const characteristicInfo = uni.getStorageSync('characteristicId') || [];

			if (!deviceInfo.length || !serviceInfo.length || !characteristicInfo.length) {
				return uni.showToast({
					title: '未获取到蓝牙设备连接信息，请先完成蓝牙配对',
					icon: 'none'
				});
			}

			const deviceId = deviceInfo[0].deviceId;
			const serviceId = serviceInfo[0].uuid;
			const characteristicId = characteristicInfo[0].uuid;

			// 构建请求数据（保持 command 为字符串，符合原始 JSON 格式）
			const requestData = {
				command: "2", // 保持字符串类型（与设备端协议一致）
				params: {
					wifiname: this.username,
					password: this.password
				}
			};
			const jsonstr1 = JSON.stringify(requestData);
			console.log('蓝牙发送数据:', jsonstr1);

			// 转换为 ArrayBuffer（兼容多环境）
			const buffer = this.str2ab(jsonstr1);

			uni.writeBLECharacteristicValue({
				deviceId,
				serviceId,
				characteristicId,
				value: buffer, // 直接传递 Uint8Array（自动转换为 ArrayBuffer）
				success: (res) => {
					console.log('写入成功:', res.errMsg);
					uni.showToast({ icon: 'success', duration: 1000 });
					this.controlFan();
				},
				fail: (res) => {
					console.log('写入失败:', res.errCode, res.errMsg);
					uni.showToast({ title: '蓝牙写入失败，请重试', icon: 'none' });
					setTimeout(() => {
						this.pwdWIFI(); // 重试写入
					}, 500);
				}
			});
		},

		navigateBack() {
			uni.navigateBack();
		}
	}
}
</script>

<style lang="scss" scoped>
	@import "@/uni_modules/uni-id-pages/common/login-page.scss";

	.forget {
		font-size: 12px;
		color: #8a8f8b;
	}

	.link-box {
		/* #ifndef APP-NVUE */
		display: flex;
		/* #endif */
		flex-direction: row;
		justify-content: space-between;
		margin-top: 20px;
	}

	.link {
		font-size: 12px;
	}
</style>