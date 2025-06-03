// src/main/java/com/example/fancontrol/repository/DeviceRepository.java
package com.example.fancontrol.repository;

import com.example.fancontrol.entity.Device;
import org.springframework.data.jpa.repository.JpaRepository;

import java.util.Optional;

public interface DeviceRepository extends JpaRepository<Device, Integer> {
    Optional<Device> findByDeviceUUID(String deviceUUID);
}