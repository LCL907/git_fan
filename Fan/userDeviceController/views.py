from django.shortcuts import render

# Create your views here.
from django.http import JsonResponse
from django.views.decorators.http import require_http_methods
from django.views.decorators.csrf import csrf_exempt
from django.db.models import Q
import json

from .models import User, Device, UserDevice
from .serializers import UserDeviceSerializer, DeviceSerializer
from .services import DeviceControlService


@csrf_exempt
@require_http_methods(["GET"])
def get_user_devices(request):
    user_devices = UserDevice.objects.all()
    serializer = UserDeviceSerializer(user_devices, many=True)
    return JsonResponse(serializer.data, safe=False)


@csrf_exempt
@require_http_methods(["GET"])
def get_user_device(request, id):
    try:
        user_device = UserDevice.objects.get(id=id)
        serializer = UserDeviceSerializer(user_device)
        return JsonResponse(serializer.data)
    except UserDevice.DoesNotExist:
        return JsonResponse({'error': 'UserDevice not found'}, status=404)


@csrf_exempt
@require_http_methods(["POST"])
def post_user_device(request):
    data = json.loads(request.body)

    # 处理设备
    device, created = Device.objects.get_or_create(
        device_uuid=data.get('device_uuid'),
        defaults={'device_name': data.get('device_name')}
    )

    # 获取用户
    try:
        user = User.objects.get(user_name=data.get('user_name'))
    except User.DoesNotExist:
        return JsonResponse({'error': 'User not found'}, status=400)

    # 检查是否已绑定
    user_device_exists = UserDevice.objects.filter(
        user_id=user.id,
        device_id=device.id
    ).exists()

    if user_device_exists:
        return JsonResponse({
            'status': False,
            'message': '请勿重复绑定'
        })

    # 检查用户是否已绑定其他设备
    user_has_other_devices = UserDevice.objects.filter(
        user_id=user.id
    ).exists()

    if user_has_other_devices:
        # 绑定新设备
        UserDevice.objects.create(user_id=user.id, device_id=device.id)
        return JsonResponse({
            'status': True,
            'message': '绑定新设备成功'
        })
    else:
        # 首次绑定
        UserDevice.objects.create(user_id=user.id, device_id=device.id)
        return JsonResponse({
            'status': True,
            'message': '注册绑定成功'
        })


@csrf_exempt
@require_http_methods(["POST"])
def device_command(request):
    data = json.loads(request.body)
    device_id = data.get('device_id')
    message = data.get('message')

    try:
        DeviceControlService.send_message(device_id, message)
        return JsonResponse({'status': True})
    except Exception as e:
        return JsonResponse({'status': False, 'error': str(e)}, status=500)


@csrf_exempt
@require_http_methods(["POST"])
def is_device_online(request):
    data = json.loads(request.body)
    device_id = data.get('device_id')

    # 获取设备名称列表
    device_list = list(Device.objects.filter(
        device_uuid=device_id
    ).values_list('device_name', flat=True))

    is_online = DeviceControlService.is_device_online(device_id)

    if is_online:
        return JsonResponse({
            'status': True,
            'message': '设备在线！',
            'device_list': device_list
        })
    else:
        return JsonResponse({
            'status': False,
            'message': '设备不在线!'
        })
