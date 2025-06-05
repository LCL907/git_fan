from django.urls import path
from .views import get_users, login, register, verify_user

urlpatterns = [
    #path('', get_users, name='users-list'),
    path('Login/', login, name='users-login'),
    path('', register, name='users-register'),
    path('VerifyUser/', verify_user, name='users-verify'),
]