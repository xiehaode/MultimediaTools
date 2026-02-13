-- 完全兼容的数据库修复脚本
USE multimediatool;

-- 删除并重新创建RegisterUser存储过程
DROP PROCEDURE IF EXISTS RegisterUser;

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
    -- 声明变量
    DECLARE v_username VARCHAR(50);
    DECLARE v_email VARCHAR(100);
    DECLARE v_count INT DEFAULT 0;
    
    -- 转换输入参数避免字符集问题
    SET v_username = CONVERT(p_username USING utf8mb4);
    SET v_email = CONVERT(p_email USING utf8mb4);
    
    -- 检查用户名是否已存在（使用CONVERT确保一致的字符集）
    SELECT COUNT(*) INTO v_count FROM users WHERE CONVERT(username USING utf8mb4) = v_username;
    
    IF v_count > 0 THEN
        SET p_status = 'exists';
        SET p_message = '用户名已存在';
        SET p_user_id = NULL;
    ELSE
        -- 检查邮箱是否已存在
        SELECT COUNT(*) INTO v_count FROM users WHERE CONVERT(email USING utf8mb4) = v_email;
        
        IF v_count > 0 THEN
            SET p_status = 'exists';
            SET p_message = '邮箱已存在';
            SET p_user_id = NULL;
        ELSE
            -- 插入新用户
            INSERT INTO users (username, email, password_hash, salt, role)
            VALUES (v_username, v_email, p_password_hash, p_salt, p_role);
            
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
    END IF;
END //

DELIMITER ;

-- 清理测试数据
DELETE FROM users WHERE username IN ('testuser', 'testuser2');

-- 测试存储过程
CALL RegisterUser('testuser', 'test@example.com', 'test_hash_123', 'dGVzdHNhbHRlc3Q=', 'user', @user_id, @status, @message);

-- 查看测试结果
SELECT '=== Test Result ===' as info;
SELECT @user_id as user_id, @status as status, @message as message;

-- 验证用户是否成功创建
SELECT '=== Created User ===' as info;
SELECT id, username, email, salt, role, created_at FROM users WHERE username = 'testuser';

SELECT '✅ Compatible fix applied successfully!' as status;