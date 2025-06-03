// src/main/java/com/example/fancontrol/entity/UserDevice.java
package com.example.fancontrol.entity;

import jakarta.persistence.*;

@Entity
@Table(name = "user_devices")
public class UserDevice {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Integer id;

    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "user_id", nullable = false)
    private User user;

    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "device_id", nullable = false)
    private Device device;

    // Getters and Setters
}