// src/main/java/com/example/fancontrol/util/DeviceControlUtil.java
package com.example.fancontrol.util;

import com.example.fancontrol.dto.MessageEntity;
import com.example.fancontrol.repository.UserDeviceRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Component;
import org.springframework.web.socket.WebSocketSession;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

@Component
public class DeviceControlUtil {

    private static final Map<String, WebSocketSession> sessions = new ConcurrentHashMap<>();

    @Autowired
    private SimpMessagingTemplate messagingTemplate;

    public void registerSession(String deviceId, WebSocketSession session) {
        sessions.put(deviceId, session);
    }

    public void sendMessage(String deviceId, MessageEntity message) {
        WebSocketSession session = sessions.get(deviceId);
        if (session != null && session.isOpen()) {
            messagingTemplate.convertAndSend(
                    "/topic/device/" + deviceId,
                    message
            );
        }
    }
}