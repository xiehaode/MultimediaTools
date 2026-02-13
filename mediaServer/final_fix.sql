-- 最终修复方案 - 完全避免字符集冲突
USE multimediatool;

-- 1. 清理所有测试数据
DELETE FROM users WHERE username IN ('root', 'testuser', 'testuser2', 'testuser3') 
   OR email IN ('2802625868@qq.com', 'test@example.com', 'test2@example.com', 'test3@example.com');

-- 2. 强制设置会话字符集
SET NAMES utf8mb4 COLLATE utf8mb4_unicode_ci;
SET character_set_connection = utf8mb4;
SET collation_connection = utf8mb4_unicode_ci;

-- 3. 删除旧存储过程
DROP PROCEDURE IF EXISTS RegisterUser;
DROP PROCEDURE IF EXISTS AuthenticateUser;

-- 4. 使用BINARY比较避免字符集问题
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

DELIMITER ;

-- 5. 创建简化测试函数
DELIMITER //
CREATE FUNCTION safe_strcmp(str1 VARCHAR(255), str2 VARCHAR(255)) 
RETURNS INT
READS SQL DATA
DETERMINISTIC
BEGIN
    RETURN BINARY str1 = BINARY str2;
END //
DELIMITER ;

-- 6. 测试存储过程
SELECT '=== 开始测试 ===' as info;

CALL RegisterUser('testfinal', 'test@final.com', 'test_hash_final', 'dGVzdHNhbHRmaW5hbA==', 'user', @uid, @stat, @msg);

SELECT @uid as user_id, @stat as status, @msg as message;

-- 验证用户
SELECT id, username, email, salt, role FROM users WHERE username = 'testfinal';

-- 清理测试数据
DELETE FROM users WHERE username = 'testfinal';

SELECT '✅ 最终修复完成！' as status;