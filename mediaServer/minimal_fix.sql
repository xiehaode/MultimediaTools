-- 最小化修复 - 完全避开字符集问题
USE multimediatool;

-- 清理数据
DELETE FROM users WHERE username IN ('root', 'testuser', 'testuser2', 'testuser3', 'testfinal') 
   OR email IN ('2802625868@qq.com', 'test@example.com', 'test2@example.com', 'test3@example.com', 'test@final.com');

-- 删除存储过程
DROP PROCEDURE IF EXISTS RegisterUser;
DROP PROCEDURE IF EXISTS AuthenticateUser;

-- 创建最简单的存储过程
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
    DECLARE v_count INT;
    
    -- 简单的存在性检查（不使用字符串比较）
    SELECT COUNT(*) INTO v_count FROM users WHERE username = p_username LIMIT 1;
    
    IF v_count > 0 THEN
        SET p_status = 'exists';
        SET p_message = 'Username exists';
        SET p_user_id = 0;
    ELSE
        SELECT COUNT(*) INTO v_count FROM users WHERE email = p_email LIMIT 1;
        
        IF v_count > 0 THEN
            SET p_status = 'exists';
            SET p_message = 'Email exists';
            SET p_user_id = 0;
        ELSE
            -- 直接插入，不做字符集处理
            INSERT INTO users (username, email, password_hash, salt, role)
            VALUES (p_username, p_email, p_password_hash, p_salt, p_role);
            
            SET p_user_id = LAST_INSERT_ID();
            
            IF p_user_id IS NOT NULL AND p_user_id > 0 THEN
                SET p_status = 'success';
                SET p_message = 'Registration successful';
            ELSE
                SET p_status = 'error';
                SET p_message = 'Registration failed';
                SET p_user_id = 0;
            END IF;
        END IF;
    END IF;
END //

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
    
    SELECT id, password_hash, salt, role, is_active
    INTO v_found_user_id, v_password_hash, v_salt, v_role, v_is_active
    FROM users WHERE username = p_username LIMIT 1;
    
    IF v_found_user_id IS NULL THEN
        SET p_status = 'not_found';
        SET p_message = 'User not found';
        SET p_user_id = 0;
        SET p_role = '';
    ELSEIF v_is_active = 0 THEN
        SET p_status = 'inactive';
        SET p_message = 'Account disabled';
        SET p_user_id = 0;
        SET p_role = '';
    ELSEIF v_password_hash = p_password_hash THEN
        SET p_user_id = v_found_user_id;
        SET p_role = v_role;
        SET p_status = 'success';
        SET p_message = 'Login successful';
        UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE id = v_found_user_id;
    ELSE
        SET p_status = 'invalid_password';
        SET p_message = 'Invalid password';
        SET p_user_id = 0;
        SET p_role = '';
    END IF;
END //

DELIMITER ;

-- 测试
CALL RegisterUser('minitest', 'mini@test.com', 'hash123', 'dGVzdA==', 'user', @uid, @stat, @msg);

SELECT @uid as user_id, @stat as status, @msg as message;

-- 验证
SELECT id, username, email FROM users WHERE username = 'minitest';

-- 清理
DELETE FROM users WHERE username = 'minitest';

SELECT '✅ Minimal fix completed!' as status;