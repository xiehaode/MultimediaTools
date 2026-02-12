-- =============================================
-- 1. 创建数据库（指定字符集，避免乱码/注入风险）
-- =============================================
CREATE DATABASE IF NOT EXISTS media_tool_db 
DEFAULT CHARACTER SET utf8mb4 
DEFAULT COLLATE utf8mb4_unicode_ci;

-- 切换到目标数据库
USE media_tool_db;

-- =============================================
-- 2. 创建用户登录信息表（user_login）
-- 核心：密码加盐哈希存储，无明文，字段长度限制
-- =============================================
DROP TABLE IF EXISTS user_login;
CREATE TABLE user_login (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键ID',
    username VARCHAR(50) NOT NULL COMMENT '用户名（唯一，限制长度防超长注入）',
    password_hash CHAR(64) NOT NULL COMMENT '密码哈希（SHA256+盐值，固定长度）',
    salt CHAR(32) NOT NULL COMMENT '盐值（随机字符串，固定长度）',
    phone VARCHAR(20) DEFAULT NULL COMMENT '手机号（可选，限制长度）',
    email VARCHAR(100) DEFAULT NULL COMMENT '邮箱（可选，限制长度）',
    login_status TINYINT NOT NULL DEFAULT 0 COMMENT '登录状态：0-未登录，1-已登录',
    last_login_time DATETIME DEFAULT NULL COMMENT '最后登录时间',
    last_login_ip VARCHAR(45) DEFAULT NULL COMMENT '最后登录IP（IPv6兼容，最长45位）',
    create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    is_deleted TINYINT NOT NULL DEFAULT 0 COMMENT '逻辑删除：0-正常，1-删除',
    PRIMARY KEY (id),
    UNIQUE KEY uk_username (username) COMMENT '用户名唯一索引（防重复，加速查询）',
    KEY idx_create_time (create_time) COMMENT '创建时间索引',
    KEY idx_is_deleted (is_deleted) COMMENT '逻辑删除索引'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='用户登录信息表';

-- =============================================
-- 3. 创建FFmpeg命令行记录表（ffmpeg_command）
-- 核心：记录命令行、执行人、状态，防命令注入（应用层配合过滤）
-- =============================================
DROP TABLE IF EXISTS ffmpeg_command;
CREATE TABLE ffmpeg_command (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '主键ID',
    command_name VARCHAR(100) NOT NULL COMMENT '命令名称（如：视频转码、音频提取）',
    command_text TEXT NOT NULL COMMENT 'FFmpeg完整命令行（长文本存储）',
    user_id BIGINT UNSIGNED NOT NULL COMMENT '关联用户ID（外键关联user_login.id）',
    execute_status TINYINT NOT NULL DEFAULT 0 COMMENT '执行状态：0-待执行，1-执行中，2-成功，3-失败',
    execute_result TEXT DEFAULT NULL COMMENT '执行结果/错误信息',
    execute_start_time DATETIME DEFAULT NULL COMMENT '执行开始时间',
    execute_end_time DATETIME DEFAULT NULL COMMENT '执行结束时间',
    file_path VARCHAR(512) DEFAULT NULL COMMENT '处理的文件路径（限制长度防超长注入）',
    output_path VARCHAR(512) DEFAULT NULL COMMENT '输出文件路径',
    create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    is_deleted TINYINT NOT NULL DEFAULT 0 COMMENT '逻辑删除：0-正常，1-删除',
    PRIMARY KEY (id),
    KEY fk_user_id (user_id) COMMENT '用户ID外键索引',
    KEY idx_execute_status (execute_status) COMMENT '执行状态索引',
    KEY idx_create_time (create_time) COMMENT '创建时间索引',
    KEY idx_is_deleted (is_deleted) COMMENT '逻辑删除索引',
    -- 外键约束（保证数据完整性）
    CONSTRAINT fk_ffmpeg_command_user_id FOREIGN KEY (user_id) REFERENCES user_login (id) ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='FFmpeg命令行记录表';

-- =============================================
-- 4. 创建只读用户（最小权限，防注入/越权）
-- 核心：应用程序使用该用户连接，仅授予必要权限，禁止DROP/ALTER等高危操作
-- =============================================
-- 先删除旧用户（避免重复）
DROP USER IF EXISTS 'media_app'@'%';
DROP USER IF EXISTS 'media_app'@'localhost';

-- 创建应用用户（密码建议替换为强密码，如：Media@123456）
CREATE USER 'media_app'@'%' IDENTIFIED WITH mysql_native_password BY 'Media@123456';
CREATE USER 'media_app'@'localhost' IDENTIFIED WITH mysql_native_password BY 'Media@123456';

-- 授予最小权限（仅增删改查，无DDL权限）
GRANT SELECT, INSERT, UPDATE, DELETE ON media_tool_db.user_login TO 'media_app'@'%';
GRANT SELECT, INSERT, UPDATE, DELETE ON media_tool_db.ffmpeg_command TO 'media_app'@'%';
GRANT SELECT, INSERT, UPDATE, DELETE ON media_tool_db.user_login TO 'media_app'@'localhost';
GRANT SELECT, INSERT, UPDATE, DELETE ON media_tool_db.ffmpeg_command TO 'media_app'@'localhost';

-- 刷新权限
FLUSH PRIVILEGES;

-- =============================================
-- 5. 插入测试数据（可选，验证表结构）
-- =============================================
-- 测试用户（密码：123456，盐值：test_salt_123456，SHA256(123456+test_salt_123456)=e8d95a51f3af4a3b134bf6bb680a213a59355f74f98f794b97740d042890c888）
INSERT INTO user_login (username, password_hash, salt, phone, email) 
VALUES ('test_user', 'e8d95a51f3af4a3b134bf6bb680a213a59355f74f98f794b97740d042890c888', 'test_salt_123456', '13800138000', 'test@example.com')
ON DUPLICATE KEY UPDATE password_hash='e8d95a51f3af4a3b134bf6bb680a213a59355f74f98f794b97740d042890c888';

-- 测试FFmpeg命令记录
INSERT INTO ffmpeg_command (command_name, command_text, user_id, execute_status, file_path, output_path)
VALUES ('视频转码', 'ffmpeg -i input.mp4 -c:v h264 -c:a aac output.mp4', 1, 2, '/data/input.mp4', '/data/output.mp4')
ON DUPLICATE KEY UPDATE execute_status=2;

-- 输出创建成功提示
SELECT '数据库media_tool_db及表创建成功！' AS result;