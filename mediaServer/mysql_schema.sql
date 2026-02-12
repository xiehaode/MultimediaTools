-- MultiMediaTool 数据库预处理语句和存储过程
-- 兼容 MySQL 5.7+ / 8.0+，避免重复创建对象

-- 切换到目标数据库（不存在则创建）
CREATE DATABASE IF NOT EXISTS multimediatool;
USE multimediatool;

-- ===================== 表结构（保留 IF NOT EXISTS，避免重复创建） =====================
-- 用户表结构
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
    INDEX idx_username (username),
    INDEX idx_email (email)
);

-- FFmpeg命令行存储表
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
    INDEX idx_user_id (user_id),
    INDEX idx_status (status),
    INDEX idx_created_at (created_at)
);

-- 用户会话表
CREATE TABLE IF NOT EXISTS user_sessions (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    session_token VARCHAR(255) UNIQUE NOT NULL,
    expires_at TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ip_address VARCHAR(45) NULL,
    user_agent TEXT NULL,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    INDEX idx_session_token (session_token),
    INDEX idx_user_id (user_id),
    INDEX idx_expires_at (expires_at)
);

-- 登录记录表
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
    INDEX idx_user_id (user_id),
    INDEX idx_login_time (login_time),
    INDEX idx_status (status)
);

-- ===================== 存储过程（先删后建，兼容所有MySQL版本） =====================
-- 定义通用删除存储过程的函数（避免重复代码）
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS DropProcedureIfExists(IN p_proc_name VARCHAR(64))
BEGIN
    DECLARE v_count INT;
    SELECT COUNT(*) INTO v_count FROM INFORMATION_SCHEMA.ROUTINES 
    WHERE ROUTINE_NAME = p_proc_name AND ROUTINE_TYPE = 'PROCEDURE' AND ROUTINE_SCHEMA = DATABASE();
    IF v_count > 0 THEN
        SET @drop_sql = CONCAT('DROP PROCEDURE ', p_proc_name);
        PREPARE stmt FROM @drop_sql;
        EXECUTE stmt;
        DEALLOCATE PREPARE stmt;
    END IF;
END //
DELIMITER ;

-- 1. 用户注册存储过程（先删后建）
CALL DropProcedureIfExists('RegisterUser');
DELIMITER //
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
    
    -- 检查用户名是否已存在
    IF EXISTS (SELECT 1 FROM users WHERE username = p_username) THEN
        SET p_status = 'exists';
        SET p_message = '用户名已存在';
    ELSEIF EXISTS (SELECT 1 FROM users WHERE email = p_email) THEN
        SET p_status = 'exists';
        SET p_message = '邮箱已存在';
    ELSE
        -- 插入新用户
        INSERT INTO users (username, email, password_hash, salt, role)
        VALUES (p_username, p_email, p_password_hash, p_salt, p_role);
        
        SET p_user_id = LAST_INSERT_ID();
        SET p_status = 'success';
        SET p_message = '用户注册成功';
    END IF;
    
    COMMIT;
END //
DELIMITER ;

-- 2. 用户登录验证存储过程
CALL DropProcedureIfExists('AuthenticateUser');
DELIMITER //
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
    
    -- 查找用户
    SELECT id, password_hash, salt, role, is_active
    INTO v_found_user_id, v_password_hash, v_salt, v_role, v_is_active
    FROM users
    WHERE username = p_username
    LIMIT 1;
    
    IF v_found_user_id IS NULL THEN
        SET p_status = 'not_found';
        SET p_message = '用户不存在';
        SET p_user_id = 0;
    ELSEIF NOT v_is_active THEN
        SET p_status = 'inactive';
        SET p_message = '账户已被禁用';
        SET p_user_id = 0;
    ELSE
        -- 验证密码（这里应该使用客户端发送的经过salt处理后的hash）
        IF v_password_hash = p_password_hash THEN
            SET p_user_id = v_found_user_id;
            SET p_role = v_role;
            SET p_status = 'success';
            SET p_message = '登录成功';
            
            -- 更新最后登录时间
            UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE id = v_found_user_id;
        ELSE
            SET p_status = 'invalid_password';
            SET p_message = '密码错误';
            SET p_user_id = 0;
        END IF;
    END IF;
END //
DELIMITER ;

-- 3. 创建用户会话存储过程
CALL DropProcedureIfExists('CreateUserSession');
DELIMITER //
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
    
    -- 清理过期会话
    DELETE FROM user_sessions WHERE expires_at < CURRENT_TIMESTAMP;
    
    -- 插入新会话
    INSERT INTO user_sessions (user_id, session_token, expires_at, ip_address, user_agent)
    VALUES (p_user_id, p_session_token, p_expires_at, p_ip_address, p_user_agent);
    
    SET p_session_id = LAST_INSERT_ID();
    SET p_status = 'success';
END //
DELIMITER ;

-- 4. 验证会话存储过程
CALL DropProcedureIfExists('ValidateSession');
DELIMITER //
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
    
    -- 清理过期会话
    DELETE FROM user_sessions WHERE expires_at < CURRENT_TIMESTAMP;
    
    -- 查找有效会话
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
DELIMITER ;

-- 5. 保存FFmpeg命令存储过程
CALL DropProcedureIfExists('SaveFFmpegCommand');
DELIMITER //
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
DELIMITER ;

-- 6. 更新命令状态存储过程
CALL DropProcedureIfExists('UpdateCommandStatus');
DELIMITER //
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
DELIMITER ;

-- 7. 获取用户命令历史存储过程
CALL DropProcedureIfExists('GetUserCommands');
DELIMITER //
CREATE PROCEDURE GetUserCommands(
    IN p_user_id INT,
    IN p_limit INT,
    IN p_offset INT
)
BEGIN
    SELECT 
        id,
        command_name,
        input_file,
        output_file,
        status,
        progress,
        error_message,
        created_at,
        updated_at,
        completed_at
    FROM ffmpeg_commands
    WHERE user_id = p_user_id
    ORDER BY created_at DESC
    LIMIT p_limit OFFSET p_offset;
END //
DELIMITER ;

-- 8. 记录登录日志存储过程
CALL DropProcedureIfExists('LogLoginAttempt');
DELIMITER //
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
DELIMITER ;

-- 删除临时的存储过程删除函数
CALL DropProcedureIfExists('DropProcedureIfExists');

-- ===================== 索引创建（兼容低版本MySQL） =====================
-- 方式1：先检查索引是否存在，不存在再创建（兼容MySQL 5.7+）
DELIMITER //
CREATE PROCEDURE IF NOT EXISTS CreateIndexIfNotExists(
    IN p_table_name VARCHAR(64),
    IN p_index_name VARCHAR(64),
    IN p_index_columns VARCHAR(255)
)
BEGIN
    DECLARE v_count INT;
    SELECT COUNT(*) INTO v_count FROM INFORMATION_SCHEMA.STATISTICS 
    WHERE TABLE_NAME = p_table_name AND INDEX_NAME = p_index_name AND TABLE_SCHEMA = DATABASE();
    IF v_count = 0 THEN
        SET @create_sql = CONCAT('CREATE INDEX ', p_index_name, ' ON ', p_table_name, ' (', p_index_columns, ')');
        PREPARE stmt FROM @create_sql;
        EXECUTE stmt;
        DEALLOCATE PREPARE stmt;
    END IF;
END //
DELIMITER ;

-- 创建索引
CALL CreateIndexIfNotExists('users', 'idx_users_active', 'is_active');
CALL CreateIndexIfNotExists('ffmpeg_commands', 'idx_commands_status_user', 'status, user_id');
CALL CreateIndexIfNotExists('user_sessions', 'idx_sessions_token_user', 'session_token, user_id');

-- 删除临时函数
CALL DropProcedureIfExists('CreateIndexIfNotExists');

-- ===================== 初始化数据 =====================
-- 创建默认管理员用户（密码: admin123，需要在应用中进行加密处理）
INSERT IGNORE INTO users (username, email, password_hash, salt, role) 
VALUES ('admin', 'admin@multimediatool.com', 'temp_hash', 'temp_salt', 'admin');

-- 创建视图：用户命令统计（CREATE OR REPLACE 兼容所有版本）
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
SELECT 'Schema initialization completed successfully!' as status;