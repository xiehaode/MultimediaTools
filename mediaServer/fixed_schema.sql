-- 简化修复版本的数据库初始化脚本
USE multimediatool;

-- 确保使用一致的字符集排序规则
SET NAMES utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 删除并重新创建RegisterUser存储过程
DROP PROCEDURE IF EXISTS RegisterUser;

DELIMITER //

CREATE PROCEDURE RegisterUser(
    IN p_username VARCHAR(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_email VARCHAR(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_password_hash VARCHAR(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_salt VARCHAR(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    IN p_role VARCHAR(10) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    OUT p_user_id INT,
    OUT p_status VARCHAR(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
    OUT p_message VARCHAR(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci
)
BEGIN
    -- 声明变量
    DECLARE user_exists INT DEFAULT 0;
    DECLARE email_exists INT DEFAULT 0;
    
    -- 使用COLLATE指定排序规则来避免冲突
    -- 检查用户名是否已存在
    SELECT COUNT(*) INTO user_exists 
    FROM users 
    WHERE username COLLATE utf8mb4_unicode_ci = p_username COLLATE utf8mb4_unicode_ci;
    
    -- 检查邮箱是否已存在
    SELECT COUNT(*) INTO email_exists 
    FROM users 
    WHERE email COLLATE utf8mb4_unicode_ci = p_email COLLATE utf8mb4_unicode_ci;
    
    -- 根据检查结果设置返回值
    IF user_exists > 0 THEN
        SET p_status = 'exists';
        SET p_message = '用户名已存在';
        SET p_user_id = NULL;
    ELSEIF email_exists > 0 THEN
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
        IF p_user_id > 0 THEN
            SET p_status = 'success';
            SET p_message = '用户注册成功';
        ELSE
            SET p_status = 'error';
            SET p_message = '用户创建失败';
            SET p_user_id = NULL;
        END IF;
    END IF;
END //

DELIMITER ;

-- 测试存储过程
CALL RegisterUser('testuser2', 'test2@example.com', 'test_hash', 'dGVzdHNhbHQ=', 'user', @user_id, @status, @message);

-- 查看测试结果
SELECT 'Test Result:' as info;
SELECT @user_id as user_id, @status as status, @message as message;

-- 验证用户是否成功创建
SELECT id, username, email, salt FROM users WHERE username = 'testuser2';

SELECT '✅ Fixed schema applied successfully!' as status;