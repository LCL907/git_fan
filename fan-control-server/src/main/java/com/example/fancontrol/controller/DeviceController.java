// src/main/java/com/example/fancontrol/controller/DeviceController.java
package com.example.fancontrol.controller;

import com.example.fancontrol.dto.UserDeviceDto;
import com.example.fancontrol.service.DeviceService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.messaging.handler.annotation.MessageMapping;
import org.springframework.messaging.handler.annotation.Payload;
import org.springframework.messaging.handler.annotation.SendTo;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/api/devices")
public class DeviceController {

    @Autowired
    private DeviceService deviceService;

    @PostMapping
    public ResponseEntity<?> bindDevice(@RequestBody UserDeviceDto dto) {
        return ResponseEntity.ok(deviceService.bindDevice(dto));
    }

    @MessageMapping("/control")
    @SendTo("/topic/deviceStatus")
    public String controlDevice(@Payload String message) {
        return "Device command processed: " + message;
    }
}