from rest_framework import serializers
from .models import User, UserDevice

class UserSerializer(serializers.ModelSerializer):
    class Meta:
        model = User
        fields = ['id', 'username', 'password']
        extra_kwargs = {'password': {'write_only': True}}

class UserDeviceSerializer(serializers.ModelSerializer):
    class Meta:
        model = UserDevice
        fields = ['device_id']

class CommonResultSerializer(serializers.Serializer):
    status = serializers.BooleanField()
    message = serializers.CharField()
    device_list = serializers.ListField(child=serializers.CharField(), required=False)