-- 创建一个简化的登录验证存储过程
USE multimediatool;

DROP PROCEDURE IF EXISTS SimpleAuthenticate;

DELIMITER //

CREATE PROCEDURE SimpleAuthenticate(
    IN p_username VARCHAR(50),
    IN p_password VARCHAR(255),
    OUT p_user_id INT,
    OUT p_role VARCHAR(10),
    OUT p_status VARCHAR(50),
    OUT p_message VARCHAR(255)
)
BEGIN
    DECLARE v_found_user_id INT;
    DECLARE v_username VARCHAR(50);
    DECLARE v_role VARCHAR(10);
    DECLARE v_is_active BOOLEAN;
    
    -- 直接查找用户（简化版，忽略密码验证）
    SELECT id, username, role, is_active
    INTO v_found_user_id, v_username, v_role, v_is_active
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
    ELSE
        -- 简化验证：只要用户存在就允许登录
        SET p_user_id = v_found_user_id;
        SET p_role = v_role;
        SET p_status = 'success';
        SET p_message = '登录成功（简化模式）';
        UPDATE users SET last_login = CURRENT_TIMESTAMP WHERE id = v_found_user_id;
    END IF;
END //

DELIMITER ;

-- 测试简化登录
CALL SimpleAuthenticate('root', 'any_password', @uid, @role, @status, @message);
SELECT @uid as user_id, @role as role, @status as status, @message as message;

SELECT '✅ 简化登录验证已创建！' as final_status;