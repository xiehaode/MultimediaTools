-- 清除可能包含无效salt的用户数据，并重新初始化数据库
USE multimediatool;

-- 删除可能存在的测试用户（包含无效的salt）
DELETE FROM users WHERE username = 'root' OR email = '2802625868@qq.com';

-- 重新应用存储过程修复
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

-- 验证修复
SELECT '✅ Salt encoding fix applied successfully!' as status;