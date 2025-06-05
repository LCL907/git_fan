# routing.py
from django.urls import re_path
from channels.routing import ProtocolTypeRouter, URLRouter
from .services import DeviceControlService

websocket_urlpatterns = [
    re_path(r'ws/device/$', DeviceControlService.handle_websocket.as_asgi()),
]

application = ProtocolTypeRouter({
    "websocket": URLRouter(websocket_urlpatterns),
})