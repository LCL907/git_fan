// src/main/java/com/example/fancontrol/entity/Device.java
package com.example.fancontrol.entity;

import jakarta.persistence.*;

@Entity
@Table(name = "devices")
public class Device {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Integer id;

    @Column(unique = true)
    private String deviceUUID;

    private String deviceName;

    // Getters and Setters
}