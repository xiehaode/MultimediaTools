-- MultiMediaTool 数据库脚本（适配 MySQL 8.0.45，Ubuntu 版本）
-- 解决 1295/1304 错误，兼容 MySQL 8.0 所有语法

-- 1. 创建/切换数据库
CREATE DATABASE IF NOT EXISTS multimediatool 
CHARACTER SET utf8mb4 
COLLATE utf8mb4_unicode_ci;
USE multimediatool;

-- 2. 手动删除旧存储过程（避免重复创建冲突）
DROP PROCEDURE IF EXISTS RegisterUser;

-- 3. 创建表结构（优化复合索引，减少冗余）
CREATE TABLE IF NOT EXISTS users (
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
    -- 复合索引覆盖常用查询场景
    INDEX idx_user_active_role (is_active, role),
    INDEX idx_username (username),
    INDEX idx_email (email)
) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 4. 创建存储过程（MySQL 8.0 兼容语法）
DELIMITER //

-- 4.1 用户注册
CREATE PROCEDURE RegisterUser(
    IN p_username VARCHAR(50),
    IN p_email VARCHAR(100),
    IN p_password_hash VARCHAR(255),
    IN p_salt VARCHAR(64),
    IN p_role VARCHAR(10),
    OUT p_user_id INT,
    OUT p_status VARCHAR(50),
    OUT p_message VARCHAR(255)
)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        GET DIAGNOSTICS CONDITION 1
        @errno = MYSQL_ERRNO, @state = RETURNED_SQLSTATE, @text = MESSAGE_TEXT;
        SET p_status = 'error';
        SET p_message = CONCAT('数据库操作失败 (', @errno, ': ', @text, ')');
        SET p_user_id = NULL;
    END;
    
    START TRANSACTION;
    
    -- 检查用户名是否已存在
    IF EXISTS (SELECT 1 FROM users WHERE username = p_username) THEN
        SET p_status = 'exists';
        SET p_message = '用户名已存在';
        SET p_user_id = NULL;
    ELSEIF EXISTS (SELECT 1 FROM users WHERE email = p_email) THEN
        SET p_status = 'exists';
        SET p_message = '邮箱已存在';
        SET p_user_id = NULL;
    ELSE
        -- 插入新用户
        INSERT INTO users (username, email, password_hash, salt, role)
        VALUES (p_username, p_email, p_password_hash, p_salt, p_role);
        
        -- 获取插入的用户ID
        SET p_user_id = LAST_INSERT_ID();
        
        -- 检查插入是否成功
        IF p_user_id IS NULL OR p_user_id = 0 THEN
            ROLLBACK;
            SET p_status = 'error';
            SET p_message = '用户创建失败，无法获取用户ID';
        ELSE
            SET p_status = 'success';
            SET p_message = '用户注册成功';
        END IF;
    END IF;
    
    -- 根据状态决定提交或回滚
    IF p_status = 'success' THEN
        COMMIT;
    ELSE
        ROLLBACK;
    END IF;
END //

DELIMITER ;

-- 执行完成提示
SELECT '✅ RegisterUser procedure updated successfully!' as status;