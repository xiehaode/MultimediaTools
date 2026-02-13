-- MultiMediaTool 数据库脚本（适配 MySQL 8.0.45，Ubuntu 版本）
-- 解决 1295/1304 错误，兼容 MySQL 8.0 所有语法

-- 1. 创建/切换数据库
CREATE DATABASE IF NOT EXISTS multimediatool 
CHARACTER SET utf8mb4 
COLLATE utf8mb4_unicode_ci;
USE multimediatool;

-- 2. 手动删除旧存储过程（避免重复创建冲突）
DROP PROCEDURE IF EXISTS RegisterUser;
DROP PROCEDURE IF EXISTS AuthenticateUser;
DROP PROCEDURE IF EXISTS CreateUserSession;
DROP PROCEDURE IF EXISTS ValidateSession;
DROP PROCEDURE IF EXISTS SaveFFmpegCommand;
DROP PROCEDURE IF EXISTS UpdateCommandStatus;
DROP PROCEDURE IF EXISTS GetUserCommands;
DROP PROCEDURE IF EXISTS LogLoginAttempt;
DROP PROCEDURE IF EXISTS CreateMissingIndexes; -- 新增：删除临时索引创建存储过程

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

CREATE TABLE IF NOT EXISTS ffmpeg_commands (
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
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    -- 复合索引覆盖 "用户+状态" 核心查询
    INDEX idx_cmd_user_status (user_id, status),
    INDEX idx_cmd_created_at (created_at)
);

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
        SET p_status = 'error';
        SET p_message = '数据库操作失败';
    END;
    
    START TRANSACTION;
    IF EXISTS (SELECT 1 FROM users WHERE username = p_username) THEN
        SET p_status = 'exists';
        SET p_message = '用户名已存在';
    ELSEIF EXISTS (SELECT 1 FROM users WHERE email = p_email) THEN
        SET p_message = '邮箱已存在';
        SET p_status = 'exists';
    ELSE
        INSERT INTO users (username, email, password_hash, salt, role)
        VALUES (p_username, p_email, p_password_hash, p_salt, p_role);
        SET p_user_id = LAST_INSERT_ID();
        SET p_status = 'success';
        SET p_message = '用户注册成功';
    END IF;
    COMMIT;
END //

-- 4.2 用户登录验证
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
        SET p_message = '数据库操作失败';
    END;
    
    SELECT id, password_hash, salt, role, is_active
    INTO v_found_user_id, v_password_hash, v_salt, v_role, v_is_active
    FROM users WHERE username = p_username LIMIT 1;
    
    IF v_found_user_id IS NULL THEN
        SET p_status = 'not_found';
        SET p_message = '用户不存在';
        SET p_user_id = 0;
    ELSEIF NOT v_is_active THEN
        SET p_status = 'inactive';
        SET p_message = '账户已被禁用';
        SET p_user_id = 0;
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
    END IF;
END //

-- 4.3 创建用户会话
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

-- 4.4 验证会话
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

-- 4.5 保存FFmpeg命令
CREATE PROCEDURE SaveFFmpegCommand(
    IN p_user_id INT,
    IN p_command_name VARCHAR(100),
    IN p_input_file VARCHAR(500),
    IN p_output_file VARCHAR(500),
    IN p_command_line TEXT,
    IN p_parameters JSON,
    OUT p_command_id INT,
    OUT p_status VARCHAR(50)
)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        SET p_status = 'error';
    END;
    
    INSERT INTO ffmpeg_commands (
        user_id, command_name, input_file, output_file, command_line, parameters
    ) VALUES (
        p_user_id, p_command_name, p_input_file, p_output_file, p_command_line, p_parameters
    );
    SET p_command_id = LAST_INSERT_ID();
    SET p_status = 'success';
END //

-- 4.6 更新命令状态
CREATE PROCEDURE UpdateCommandStatus(
    IN p_command_id INT,
    IN p_status VARCHAR(20),
    IN p_progress INT,
    IN p_error_message TEXT,
    OUT p_result_status VARCHAR(50)
)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        SET p_result_status = 'error';
    END;
    
    UPDATE ffmpeg_commands 
    SET status = p_status,
        progress = p_progress,
        error_message = p_error_message,
        updated_at = CURRENT_TIMESTAMP,
        completed_at = CASE WHEN p_status IN ('completed', 'failed') THEN CURRENT_TIMESTAMP ELSE completed_at END
    WHERE id = p_command_id;
    SET p_result_status = 'success';
END //

-- 4.7 获取用户命令历史
CREATE PROCEDURE GetUserCommands(
    IN p_user_id INT,
    IN p_limit INT,
    IN p_offset INT
)
BEGIN
    SELECT 
        id, command_name, input_file, output_file, status,
        progress, error_message, created_at, updated_at, completed_at
    FROM ffmpeg_commands
    WHERE user_id = p_user_id
    ORDER BY created_at DESC
    LIMIT p_limit OFFSET p_offset;
END //

-- 4.8 记录登录日志
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

-- 4.9 临时存储过程：创建缺失的索引
CREATE PROCEDURE CreateMissingIndexes()
BEGIN
    -- 检查并创建 idx_login_username_status 索引
    DECLARE index_exists INT DEFAULT 0;
    SELECT COUNT(*) INTO index_exists 
    FROM INFORMATION_SCHEMA.STATISTICS 
    WHERE table_schema = DATABASE() 
      AND table_name = 'login_logs' 
      AND index_name = 'idx_login_username_status';
    
    IF index_exists = 0 THEN
        CREATE INDEX idx_login_username_status ON login_logs(username, status);
    END IF;

    -- 检查并创建 idx_cmd_completed_at 索引
    SET index_exists = 0;
    SELECT COUNT(*) INTO index_exists 
    FROM INFORMATION_SCHEMA.STATISTICS 
    WHERE table_schema = DATABASE() 
      AND table_name = 'ffmpeg_commands' 
      AND index_name = 'idx_cmd_completed_at';
    
    IF index_exists = 0 THEN
        CREATE INDEX idx_cmd_completed_at ON ffmpeg_commands(completed_at);
    END IF;
END //

DELIMITER ;

-- 5. 执行索引创建存储过程
CALL CreateMissingIndexes();
-- 删除临时存储过程
DROP PROCEDURE IF EXISTS CreateMissingIndexes;

-- 6. 初始化默认管理员
INSERT IGNORE INTO users (username, email, password_hash, salt, role) 
VALUES ('admin', 'admin@multimediatool.com', 'temp_hash', 'temp_salt', 'admin');

-- 7. 创建视图
CREATE OR REPLACE VIEW user_command_stats AS
SELECT 
    u.id as user_id,
    u.username,
    COUNT(fc.id) as total_commands,
    SUM(CASE WHEN fc.status = 'completed' THEN 1 ELSE 0 END) as completed_commands,
    SUM(CASE WHEN fc.status = 'failed' THEN 1 ELSE 0 END) as failed_commands,
    SUM(CASE WHEN fc.status = 'processing' THEN 1 ELSE 0 END) as processing_commands,
    AVG(CASE WHEN fc.status = 'completed' AND fc.completed_at IS NOT NULL 
            THEN TIMESTAMPDIFF(SECOND, fc.created_at, fc.completed_at) ELSE NULL END) as avg_completion_time
FROM users u
LEFT JOIN ffmpeg_commands fc ON u.id = fc.user_id
GROUP BY u.id, u.username;

-- 执行完成提示
SELECT '✅ Schema initialization completed successfully (MySQL 8.0.45 compatible!)' as status;