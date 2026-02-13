-- 简化的数据库修复脚本
-- 解决字符编码问题的最简单方法

-- 1. 删除现有数据库（如果有）
DROP DATABASE IF EXISTS multimediatool;

-- 2. 重新创建数据库（使用UTF8MB4）
CREATE DATABASE multimediatool 
CHARACTER SET utf8mb4 
COLLATE utf8mb4_unicode_ci;

-- 3. 使用数据库
USE multimediatool;

-- 4. 创建用户表
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    salt VARCHAR(64) NOT NULL,
    role ENUM('admin', 'user') DEFAULT 'user',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    last_login TIMESTAMP NULL,
    is_active BOOLEAN DEFAULT TRUE,
    -- 索引
    INDEX idx_user_active_role (is_active, role),
    INDEX idx_username (username),
    INDEX idx_email (email)
) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 5. 创建其他必要的表
CREATE TABLE ffmpeg_commands (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    command_name VARCHAR(100) NOT NULL,
    input_file VARCHAR(500) NOT NULL,
    output_file VARCHAR(500) NOT NULL,
    command_line TEXT NOT NULL,
    parameters JSON,
    status ENUM('pending', 'processing', 'completed', 'failed') DEFAULT 'pending',
    progress INT DEFAULT 0,
    error_message TEXT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    completed_at TIMESTAMP NULL,
    -- 索引
    INDEX idx_user_status (user_id, status),
    INDEX idx_created_at (created_at),
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 6. 创建存储过程
DELIMITER //

-- 用户注册存储过程
CREATE PROCEDURE RegisterUser(
    IN p_username VARCHAR(50),
    IN p_email VARCHAR(100),
    IN p_password_hash VARCHAR(255),
    IN p_salt VARCHAR(64)
)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        RESIGNAL;
    END;
    
    START TRANSACTION;
    
    INSERT INTO users (username, email, password_hash, salt)
    VALUES (p_username, p_email, p_password_hash, p_salt);
    
    SELECT LAST_INSERT_ID() as user_id;
    
    COMMIT;
END //

-- 用户认证存储过程
CREATE PROCEDURE AuthenticateUser(
    IN p_username VARCHAR(50),
    IN p_password_hash VARCHAR(255)
)
BEGIN
    SELECT id, username, email, role, password_hash, salt, is_active
    FROM users 
    WHERE username = p_username AND is_active = TRUE;
END //

DELIMITER ;

-- 7. 验证创建结果
SHOW CREATE TABLE users;
SHOW CREATE TABLE ffmpeg_commands;

SELECT 
    table_schema,
    table_name,
    table_collation
FROM information_schema.tables 
WHERE table_schema = 'multimediatool';

-- 8. 插入测试数据（可选）
-- INSERT INTO users (username, email, password_hash, salt)
-- VALUES ('admin', 'admin@example.com', 'hashed_password', 'test_salt_value');

SELECT 'Database setup completed successfully!' as status;