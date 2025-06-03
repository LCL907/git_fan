// src/main/java/com/example/fancontrol/controller/AuthController.java
package com.example.fancontrol.controller;

import com.example.fancontrol.dto.LoginRequest;
import com.example.fancontrol.dto.RegisterRequest;
import com.example.fancontrol.service.AuthService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/api/auth")
public class AuthController {

    @Autowired
    private AuthService authService;

    @PostMapping("/register")
    public ResponseEntity<?> register(@RequestBody RegisterRequest request) {
        authService.register(request);
        return ResponseEntity.ok().build();
    }

    @PostMapping("/login")
    public ResponseEntity<?> login(@RequestBody LoginRequest request) {
        return (ResponseEntity<?>) authService.login(request);
    }
}