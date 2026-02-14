-- MultiMediaTool 数据库脚本（适配 MySQL 8.0.45，Ubuntu 版本）
-- 解决 1295/1304 错误，兼容 MySQL 8.0 所有语法

-- 1. 创建/切换数据库
CREATE DATABASE IF NOT EXISTS multimediatool 
CHARACTER SET utf8mb4 
COLLATE utf8mb4_unicode_ci;
USE multimediatool;

-- 2. 强制设置会话字符集
SET NAMES utf8mb4 COLLATE utf8mb4_unicode_ci;
SET character_set_connection = utf8mb4;
SET collation_connection = utf8mb4_unicode_ci;

-- 3. 手动删除旧存储过程（避免重复创建冲突）
DROP PROCEDURE IF EXISTS RegisterUser;
DROP PROCEDURE IF EXISTS AuthenticateUser;
DROP PROCEDURE IF EXISTS CreateUserSession;
DROP PROCEDURE IF EXISTS ValidateSession;
DROP PROCEDURE IF EXISTS LogLoginAttempt;

-- 4. 创建表结构（优化复合索引，减少冗余）
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

CREATE TABLE IF NOT EXISTS user_sessions (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    session_token VARCHAR(255) UNIQUE NOT NULL,
    expires_at TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ip_address VARCHAR(45) NULL,
    user_agent TEXT NULL,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    -- 复合索引覆盖会话验证查询
    INDEX idx_session_token_user (session_token, user_id, expires_at),
    INDEX idx_session_expires (expires_at)
);

CREATE TABLE IF NOT EXISTS login_logs (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT,
    username VARCHAR(50),
    ip_address VARCHAR(45),
    user_agent TEXT,
    login_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    status ENUM('success', 'failed') NOT NULL,
    failure_reason VARCHAR(255) NULL,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE SET NULL,
    INDEX idx_login_user_status (user_id, status),
    INDEX idx_login_time (login_time)
);

-- 5. 创建存储过程（MySQL 8.0 兼容语法，使用BINARY比较避免字符集问题）
DELIMITER //

-- 5.1 用户注册
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
    DECLARE v_count INT DEFAULT 0;
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        GET DIAGNOSTICS CONDITION 1
        @errno = MYSQL_ERRNO, @state = RETURNED_SQLSTATE, @text = MESSAGE_TEXT;
        SET p_status = 'error';
        SET p_message = CONCAT('数据库错误: ', @text);
        SET p_user_id = 0;
    END;
    
    START TRANSACTION;
    
    -- 使用BINARY比较避免字符集问题
    SELECT COUNT(*) INTO v_count FROM users WHERE BINARY username = BINARY p_username;
    
    IF v_count > 0 THEN
        SET p_status = 'exists';
        SET p_message = '用户名已存在';
        SET p_user_id = 0;
        ROLLBACK;
    ELSE
        SELECT COUNT(*) INTO v_count FROM users WHERE BINARY email = BINARY p_email;
        
        IF v_count > 0 THEN
            SET p_status = 'exists';
            SET p_message = '邮箱已存在';
            SET p_user_id = 0;
            ROLLBACK;
        ELSE
            -- 插入用户，使用CAST确保字符集一致
            INSERT INTO users (username, email, password_hash, salt, role)
            VALUES (
                CAST(p_username AS CHAR(50) CHARACTER SET utf8mb4),
                CAST(p_email AS CHAR(100) CHARACTER SET utf8mb4),
                CAST(p_password_hash AS CHAR(255) CHARACTER SET utf8mb4),
                CAST(p_salt AS CHAR(64) CHARACTER SET utf8mb4),
                CAST(p_role AS CHAR(10) CHARACTER SET utf8mb4)
            );
            
            SET p_user_id = LAST_INSERT_ID();
            
            IF p_user_id > 0 THEN
                SET p_status = 'success';
                SET p_message = '用户注册成功';
                COMMIT;
            ELSE
                SET p_status = 'error';
                SET p_message = '用户创建失败';
                SET p_user_id = 0;
                ROLLBACK;
            END IF;
        END IF;
    END IF;
END //

-- 5.2 用户登录验证
CREATE PROCEDURE AuthenticateUser(
    IN p_username VARCHAR(50),
    IN p_password_hash VARCHAR(255),
    OUT p_user_id INT,
    OUT p_role VARCHAR(10),
    OUT p_status VARCHAR(50),
    OUT p_message VARCHAR(255)
)
BEGIN
    DECLARE v_found_user_id INT;
    DECLARE v_password_hash VARCHAR(255);
    DECLARE v_salt VARCHAR(64);
    DECLARE v_role VARCHAR(10);
    DECLARE v_is_active BOOLEAN;
    
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        SET p_status = 'error';
        SET p_message = '认证失败';
        SET p_user_id = 0;
        SET p_role = '';
    END;
    
    -- 使用BINARY比较
    SELECT id, password_hash, salt, role, is_active
    INTO v_found_user_id, v_password_hash, v_salt, v_role, v_is_active
    FROM users WHERE BINARY username = BINARY p_username LIMIT 1;
    
    IF v_found_user_id IS NULL THEN
        SET p_status = 'not_found';
        SET p_message = '用户不存在';
        SET p_user_id = 0;
        SET p_role = '';
    ELSEIF NOT v_is_active THEN
        SET p_status = 'inactive';
        SET p_message = '账户已被禁用';
        SET p_user_id = 0;
        SET p_role = '';
    ELSEIF v_password_hash = p_password_hash THEN
        SET p_user_id = v_found_user_id;
        SET p_role = v_role;
        SET p_status = 'success';
        SET p_message = '登录成功';
        UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE id = v_found_user_id;
    ELSE
        SET p_status = 'invalid_password';
        SET p_message = '密码错误';
        SET p_user_id = 0;
        SET p_role = '';
    END IF;
END //

-- 5.3 创建用户会话
CREATE PROCEDURE CreateUserSession(
    IN p_user_id INT,
    IN p_session_token VARCHAR(255),
    IN p_expires_at TIMESTAMP,
    IN p_ip_address VARCHAR(45),
    IN p_user_agent TEXT,
    OUT p_session_id INT,
    OUT p_status VARCHAR(50)
)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        SET p_status = 'error';
    END;
    
    DELETE FROM user_sessions WHERE expires_at < CURRENT_TIMESTAMP;
    INSERT INTO user_sessions (user_id, session_token, expires_at, ip_address, user_agent)
    VALUES (p_user_id, p_session_token, p_expires_at, p_ip_address, p_user_agent);
    SET p_session_id = LAST_INSERT_ID();
    SET p_status = 'success';
END //

-- 5.4 验证会话
CREATE PROCEDURE ValidateSession(
    IN p_session_token VARCHAR(255),
    OUT p_user_id INT,
    OUT p_username VARCHAR(50),
    OUT p_role VARCHAR(10),
    OUT p_status VARCHAR(50)
)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        SET p_status = 'error';
    END;
    
    DELETE FROM user_sessions WHERE expires_at < CURRENT_TIMESTAMP;
    SELECT s.user_id, u.username, u.role
    INTO p_user_id, p_username, p_role
    FROM user_sessions s
    JOIN users u ON s.user_id = u.id
    WHERE s.session_token = p_session_token 
    AND s.expires_at > CURRENT_TIMESTAMP
    AND u.is_active = TRUE
    LIMIT 1;
    
    IF p_user_id IS NOT NULL THEN
        SET p_status = 'success';
    ELSE
        SET p_user_id = 0;
        SET p_status = 'invalid';
    END IF;
END //

-- 5.5 记录登录日志
CREATE PROCEDURE LogLoginAttempt(
    IN p_user_id INT,
    IN p_username VARCHAR(50),
    IN p_ip_address VARCHAR(45),
    IN p_user_agent TEXT,
    IN p_status VARCHAR(10),
    IN p_failure_reason VARCHAR(255)
)
BEGIN
    INSERT INTO login_logs (
        user_id, username, ip_address, user_agent, status, failure_reason
    ) VALUES (
        p_user_id, p_username, p_ip_address, p_user_agent, p_status, p_failure_reason
    );
END //

-- 5.9 创建简化测试函数
CREATE FUNCTION safe_strcmp(str1 VARCHAR(255), str2 VARCHAR(255)) 
RETURNS INT
READS SQL DATA
DETERMINISTIC
BEGIN
    RETURN BINARY str1 = BINARY str2;
END //

DELIMITER ;

-- 6. 初始化默认管理员
INSERT IGNORE INTO users (username, email, password_hash, salt, role) 
VALUES ('admin', 'admin@multimediatool.com', 'temp_hash', 'temp_salt', 'admin');

-- 7. 清理测试数据
DELETE FROM users WHERE username IN ('root', 'testuser', 'testuser2', 'testuser3', 'testfinal') 
   OR email IN ('2802625868@qq.com', 'test@example.com', 'test2@example.com', 'test3@example.com', 'test@final.com');

-- 执行完成提示
SELECT '✅ Schema initialization completed successfully (MySQL 8.0.45 compatible!)' as status;