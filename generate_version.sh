#!/bin/bash

# 配置基本版本号
MAJOR=1
MINOR=0
PATCH=0

# 获取Git提交次数作为Build号，如果不是Git仓库则默认为0
if git rev-parse --is-inside-work-tree > /dev/null 2>&1; then
    BUILD=$(git rev-list --count HEAD)
    GIT_HASH=$(git rev-parse --short HEAD)
    GIT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
else
    BUILD=0
    GIT_HASH="unknown"
    GIT_BRANCH="unknown"
fi

# 获取当前时间
BUILD_TIME=$(date "+%Y-%m-%d %H:%M:%S")
PROJECT_NAME="MultiMediaTool"

# 定义文件路径
TEMPLATE_FILE="version_template.h"
OUTPUT_FILE="MultiMediaTool/src/version.h"

# 检查模板文件是否存在
if [ ! -f "$TEMPLATE_FILE" ]; then
    echo "Error: $TEMPLATE_FILE not found!"
    exit 1
fi

# 确保目标目录存在
mkdir -p $(dirname "$OUTPUT_FILE")

# 使用 sed 替换模板中的占位符
sed -e "s/@PROJECT_NAME@/$PROJECT_NAME/g" \
    -e "s/@VERSION_MAJOR@/$MAJOR/g" \
    -e "s/@VERSION_MINOR@/$MINOR/g" \
    -e "s/@VERSION_PATCH@/$PATCH/g" \
    -e "s/@VERSION_BUILD@/$BUILD/g" \
    -e "s/@GIT_HASH@/$GIT_HASH/g" \
    -e "s/@GIT_BRANCH@/$GIT_BRANCH/g" \
    -e "s/@BUILD_TIME@/$BUILD_TIME/g" \
    "$TEMPLATE_FILE" > "$OUTPUT_FILE"

echo "Version header generated at $OUTPUT_FILE"
echo "Version: $MAJOR.$MINOR.$PATCH.$BUILD ($GIT_HASH)"
