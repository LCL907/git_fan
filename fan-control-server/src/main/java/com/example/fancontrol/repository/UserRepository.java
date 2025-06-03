// src/main/java/com/example/fancontrol/repository/UserRepository.java
package com.example.fancontrol.repository;

import com.example.fancontrol.entity.User;
import org.springframework.data.jpa.repository.JpaRepository;

import java.util.Optional;

public interface UserRepository extends JpaRepository<User, Integer> {
    Optional<User> findByUserName(String userName);
}