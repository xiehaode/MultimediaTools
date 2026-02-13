-- 检查数据库字符集设置的诊断脚本

-- 检查数据库默认字符集
SELECT 
    SCHEMA_NAME as 'Database',
    DEFAULT_CHARACTER_SET_NAME as 'Charset',
    DEFAULT_COLLATION_NAME as 'Collation'
FROM information_schema.SCHEMATA 
WHERE SCHEMA_NAME = 'multimediatool';

-- 检查表字符集
SELECT 
    TABLE_NAME as 'Table',
    TABLE_COLLATION as 'Collation'
FROM information_schema.TABLES 
WHERE TABLE_SCHEMA = 'multimediatool' 
AND TABLE_NAME = 'users';

-- 检查列字符集
SELECT 
    COLUMN_NAME as 'Column',
    DATA_TYPE as 'Type',
    CHARACTER_SET_NAME as 'Charset',
    COLLATION_NAME as 'Collation'
FROM information_schema.COLUMNS 
WHERE TABLE_SCHEMA = 'multimediatool' 
AND TABLE_NAME = 'users'
AND DATA_TYPE IN ('varchar', 'char', 'text');

-- 检查服务器字符集设置
SHOW VARIABLES LIKE 'character_set%';
SHOW VARIABLES LIKE 'collation%';

-- 检查MySQL版本
SELECT VERSION() as 'MySQL Version';