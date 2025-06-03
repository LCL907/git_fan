// src/main/java/com/example/fancontrol/entity/User.java
package com.example.fancontrol.entity;

import jakarta.persistence.*;
import java.util.List;

@Entity
@Table(name = "users")
public class User {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Integer id;

    @Column(nullable = false, unique = true)
    private String userName;

    @Column(nullable = false)
    private String password;

    @OneToMany(mappedBy = "user", cascade = CascadeType.ALL)
    private List<UserDevice> userDevices;

    // Getters and Setters
}