// src/main/java/com/example/fancontrol/service/AuthService.java
package com.example.fancontrol.service;

import com.example.fancontrol.dto.LoginRequest;
import com.example.fancontrol.dto.RegisterRequest;
import com.example.fancontrol.entity.User;
import com.example.fancontrol.repository.UserRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;

@Service
public class AuthService {

    @Autowired
    private UserRepository userRepository;

    @Autowired
    private PasswordEncoder passwordEncoder;

    public void register(RegisterRequest request) {
        if (userRepository.existsByUserName(request.getUserName())) {
            throw new RuntimeException("Username already exists");
        }

        User user = new User();
        user.setUserName(request.getUserName());
        user.setPassword(passwordEncoder.encode(request.getPassword()));
        userRepository.save(user);
    }

    public Object login(LoginRequest request) {
        User user = userRepository.findByUserName(request.getUserName())
                .orElseThrow(() -> new RuntimeException("User not found"));

        if (!passwordEncoder.matches(request.getPassword(), user.getPassword())) {
            throw new RuntimeException("Invalid credentials");
        }

        return Map.of("status", "success", "userId", user.getId());
    }
}