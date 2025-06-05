from django.urls import path
from .views import *

urlpatterns = [
    path('', get_user_devices, name='get_user_devices'),
    path('<int:id>/', get_user_device, name='get_user_device'),
    path('add/', post_user_device, name='post_user_device'),
    path('DeviceCommand/', device_command, name='device_command'),
    path('IsDeviceOnline/', is_device_online, name='is_device_online'),
]