-- db.sql
CREATE DATABASE IF NOT EXISTS smarthome_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE smarthome_db;

-- Таблица пользователей
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    email VARCHAR(255) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL,
    phone VARCHAR(20) NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Таблица товаров
CREATE TABLE products (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    price DECIMAL(10,2) NOT NULL,
    image VARCHAR(255),
    description TEXT
);

-- Таблица корзины (привязана к пользователю)
CREATE TABLE cart (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    product_id INT NOT NULL,
    quantity INT DEFAULT 1,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE CASCADE,
    UNIQUE KEY unique_cart (user_id, product_id)
);

-- Вставка тестовых товаров
INSERT INTO products (name, price, image, description) VALUES
('Умная лампа', 2990.00, 'lamp.jpg', 'Управляйте освещением через приложение'),
('Умный термостат', 9990.00, 'thermostat.jpg', 'Автоматическая регулировка температуры'),
('Умная розетка', 1990.00, 'socket.jpg', 'Включайте устройства удаленно'),
('Датчик движения', 1001.00, 'sensor.jpg', 'Уведомления при движении');