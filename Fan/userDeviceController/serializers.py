from rest_framework import serializers
from .models import UserDevice, Device


class DeviceSerializer(serializers.ModelSerializer):
    class Meta:
        model = Device
        fields = '__all__'


class UserDeviceSerializer(serializers.ModelSerializer):
    user_name = serializers.CharField(source='user.user_name')
    device_name = serializers.CharField(source='device.device_name')
    device_uuid = serializers.CharField(source='device.device_uuid')

    class Meta:
        model = UserDevice
        fields = ['id', 'user_name', 'device_name', 'device_uuid']    