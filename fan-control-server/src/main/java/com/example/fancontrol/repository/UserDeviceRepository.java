// src/main/java/com/example/fancontrol/repository/UserDeviceRepository.java
package com.example.fancontrol.repository;

import com.example.fancontrol.entity.UserDevice;
import org.springframework.data.jpa.repository.JpaRepository;

public interface UserDeviceRepository extends JpaRepository<UserDevice, Integer> {
}