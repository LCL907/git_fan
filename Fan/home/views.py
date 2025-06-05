from django.shortcuts import render
from django.http import HttpResponse
from django.views.decorators.cache import never_cache
import uuid

def index(request):
    """主页视图"""
    return render(request, 'home/index.html')

def privacy(request):
    """隐私政策视图"""
    return render(request, 'home/privacy.html')

@never_cache
def error(request):
    """错误处理视图"""
    # 生成或获取请求ID
    request_id = request.META.get('HTTP_X_REQUEST_ID', str(uuid.uuid4()))
    context = {
        'request_id': request_id
    }
    return render(request, 'home/error.html', context)