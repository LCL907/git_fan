// src/main/java/com/example/fancontrol/service/DeviceService.java
package com.example.fancontrol.service;

import com.example.fancontrol.dto.UserDeviceDto;
import com.example.fancontrol.entity.Device;
import com.example.fancontrol.entity.UserDevice;
import com.example.fancontrol.repository.DeviceRepository;
import com.example.fancontrol.repository.UserDeviceRepository;
import com.example.fancontrol.repository.UserRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

@Service
@Transactional
public class DeviceService {

    @Autowired
    private DeviceRepository deviceRepository;

    @Autowired
    private UserDeviceRepository userDeviceRepository;

    @Autowired
    private UserRepository userRepository;

    @Autowired
    private SimpMessagingTemplate messagingTemplate;

    public Object bindDevice(UserDeviceDto dto) {
        Device device = deviceRepository.findByDeviceUUID(dto.getDeviceUUID())
                .orElseGet(() -> createNewDevice(dto));

        User user = userRepository.findByUserName(dto.getUserName())
                .orElseThrow(() -> new RuntimeException("User not found"));

        UserDevice userDevice = new UserDevice();
        userDevice.setUser(user);
        userDevice.setDevice(device);
        userDeviceRepository.save(userDevice);

        messagingTemplate.convertAndSend(
                "/topic/device/" + device.getDeviceUUID(),
                new MessageEntity(device.getDeviceUUID(), "BIND_SUCCESS", device.getDeviceName())
        );

        return Map.of("status", "success", "message", "Device bound successfully");
    }

    private Device createNewDevice(UserDeviceDto dto) {
        Device device = new Device();
        device.setDeviceUUID(dto.getDeviceUUID());
        device.setDeviceName(dto.getDeviceName());
        return deviceRepository.save(device);
    }
}