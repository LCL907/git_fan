from django.shortcuts import render

# Create your views here.
from rest_framework.decorators import api_view, permission_classes
from rest_framework.permissions import AllowAny
from rest_framework.response import Response
from django.http import JsonResponse
from django.shortcuts import get_object_or_404
from .models import User, UserDevice
from .serializers import UserSerializer, CommonResultSerializer

@api_view(['GET'])
@permission_classes([AllowAny])
def get_users(request):
    users = User.objects.all()
    serializer = UserSerializer(users, many=True)
    return Response(serializer.data)

@api_view(['POST'])
@permission_classes([AllowAny])
def login(request):
    username = request.data.get('userName')
    password = request.data.get('Password')

    try:
        user = User.objects.get(username=username)
    except User.DoesNotExist:
        return Response({'status': False, 'message': '请注册！', 'deviceList': []})

    if user.password != password:
        return Response({'status': False, 'message': '密码错误！', 'deviceList': []})

    devices = UserDevice.objects.filter(user=user).values_list('device_id', flat=True)
    return Response({
        'status': True,
        'message': '登录成功！',
        'deviceList': list(devices)
    })

@api_view(['POST'])
@permission_classes([AllowAny])
def register(request):
    print(request.data)
    username = request.data.get('UserName')
    password = request.data.get('Password')

    if User.objects.filter(username=username).exists():
        return Response({'status': False, 'message': '用户已经存在'})

    user = User.objects.create(username=username, password=password)
    return Response({'status': True, 'message': '注册成功！'})

@api_view(['POST'])
@permission_classes([AllowAny])
def verify_user(request):
    username = request.data.get('UserName')

    if User.objects.filter(username=username).exists():
        return Response({'status': False, 'message': '用户已存在'})

    return Response({'status': True, 'message': '请输入密码！'})