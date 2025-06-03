// src/main/java/com/example/fancontrol/FanControlApplication.java
package com.example.fancontrol;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.scheduling.annotation.EnableScheduling;

@SpringBootApplication
@EnableScheduling
public class FanControlApplication {
    public static void main(String[] args) {
        SpringApplication.run(FanControlApplication.class, args);
    }
}