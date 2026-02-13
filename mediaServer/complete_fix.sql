-- 完整的数据库修复和清理脚本
USE multimediatool;

-- 1. 清理所有测试数据和无效数据
DELETE FROM users WHERE username IN ('root', 'testuser', 'testuser2', '2802625868@qq.com') 
   OR email IN ('2802625868@qq.com', 'test@example.com', 'test2@example.com');

-- 2. 删除并重新创建RegisterUser存储过程，确保使用最简单的逻辑
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
    DECLARE v_count INT DEFAULT 0;
    
    -- 开始事务
    START TRANSACTION;
    
    -- 检查用户名是否已存在
    SELECT COUNT(*) INTO v_count FROM users WHERE username = p_username;
    
    IF v_count > 0 THEN
        SET p_status = 'exists';
        SET p_message = '用户名已存在';
        SET p_user_id = 0;
        ROLLBACK;
    ELSE
        -- 检查邮箱是否已存在
        SELECT COUNT(*) INTO v_count FROM users WHERE email = p_email;
        
        IF v_count > 0 THEN
            SET p_status = 'exists';
            SET p_message = '邮箱已存在';
            SET p_user_id = 0;
            ROLLBACK;
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

DELIMITER ;

-- 3. 删除并重新创建AuthenticateUser存储过程
DROP PROCEDURE IF EXISTS AuthenticateUser;

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
    
    SELECT id, password_hash, salt, role, is_active
    INTO v_found_user_id, v_password_hash, v_salt, v_role, v_is_active
    FROM users WHERE username = p_username LIMIT 1;
    
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

-- 4. 验证存储过程创建
SELECT '✅ 存储过程创建完成' as status;

-- 5. 测试用安全的Base64 salt（模拟真实的salt值）
CALL RegisterUser('testuser3', 'test3@example.com', 'test_hash_123456', 'dGVzdHNhbHRlc3RzYWx0', 'user', @test_user_id, @test_status, @test_message);

-- 6. 查看测试结果
SELECT '=== 测试结果 ===' as info;
SELECT @test_user_id as user_id, @test_status as status, @test_message as message;

-- 7. 验证用户是否成功创建
SELECT '=== 创建的用户 ===' as info;
SELECT id, username, email, LEFT(salt, 20) as salt_prefix, role, created_at 
FROM users WHERE username = 'testuser3';

-- 8. 清理测试数据
DELETE FROM users WHERE username = 'testuser3';

SELECT '✅ 完整修复脚本执行完成！现在可以重新编译并测试服务器。' as final_status;