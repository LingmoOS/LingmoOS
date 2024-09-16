#!/bin/bash

# 定义源文件目录
SRC_DIR="./"

# 定义翻译文件目录
TS_DIR="translations"

# 定义需要生成的语言代码
LANGUAGES=("zh_CN" "es_ES")

# 生成通用翻译文件
COMMON_TS_FILE="${TS_DIR}/trayplugin-loader.ts"
lupdate ${SRC_DIR} -ts ${COMMON_TS_FILE}
echo "Generated ${COMMON_TS_FILE}"

# 循环生成翻译文件
for LANG in "${LANGUAGES[@]}"; do
    TS_FILE="${TS_DIR}/trayplugin-loader_${LANG}.ts"
    lupdate ${SRC_DIR} -ts ${TS_FILE}
    echo "Generated ${TS_FILE}"
done
