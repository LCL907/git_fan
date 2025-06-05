from django.db import models

# Create your models here.
from django.db import models


class User(models.Model):
    user_name = models.CharField(max_length=100, unique=True)

    def __str__(self):
        return self.user_name


class Device(models.Model):
    device_uuid = models.CharField(max_length=100, unique=True)
    device_name = models.CharField(max_length=100)

    def __str__(self):
        return self.device_name


class UserDevice(models.Model):
    user = models.ForeignKey(User, on_delete=models.CASCADE)
    device = models.ForeignKey(Device, on_delete=models.CASCADE)

    class Meta:
        unique_together = ('user', 'device')

    def __str__(self):
        return f"{self.user.user_name} - {self.device.device_name}"