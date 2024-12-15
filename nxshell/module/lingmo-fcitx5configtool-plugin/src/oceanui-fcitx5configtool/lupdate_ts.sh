#!/bin/bash

# 设置项目名称和翻译文件目录
PROJECT_NAME="fcitx5configtool"
TRANS_DIR="translations"

# 定义支持的语言列表
LANGUAGES=(
    ""          # 默认翻译文件
    "az"        # 阿塞拜疆语
    "bo"        # 藏语
    "ca"        # 加泰罗尼亚语
    "es"        # 西班牙语
    "fi"        # 芬兰语
    "fr"        # 法语
    "hu"        # 匈牙利语
    "it"        # 意大利语
    "ja"        # 日语
    "ko"        # 韩语
    "nb_NO"     # 挪威语
    "pl"        # 波兰语
    "pt_BR"     # 巴西葡萄牙语
    "ru"        # 俄语
    "uk"        # 乌克兰语
    "zh_CN"     # 简体中文
    "zh_HK"     # 香港繁体
    "zh_TW"     # 台湾繁体
)

# 创建翻译目录（如果不存在）
mkdir -p ${TRANS_DIR}

# 生成翻译文件
for lang in "${LANGUAGES[@]}"; do
    if [ -z "$lang" ]; then
        # 生成默认翻译文件
        lupdate . -ts -no-obsolete "${TRANS_DIR}/${PROJECT_NAME}.ts"
    else
        # 生成特定语言的翻译文件
        lupdate . -ts -no-obsolete "${TRANS_DIR}/${PROJECT_NAME}_${lang}.ts"
    fi
done

echo "所有翻译文件已生成！" 