-- src/main/resources/schema.sql
CREATE TABLE users (
    id INT PRIMARY KEY IDENTITY,
    user_name NVARCHAR(50) UNIQUE NOT NULL,
    password NVARCHAR(100) NOT NULL
);

CREATE TABLE devices (
    id INT PRIMARY KEY IDENTITY,
    device_uuid NVARCHAR(100) UNIQUE NOT NULL,
    device_name NVARCHAR(100) NOT NULL
);

CREATE TABLE user_devices (
    id INT PRIMARY KEY IDENTITY,
    user_id INT FOREIGN KEY REFERENCES users(id),
    device_id INT FOREIGN KEY REFERENCES devices(id)
);