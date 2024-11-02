#!/bin/bash

# 遍历当前目录下的所有 .po 文件
for file in *.po; do
    # 检查文件是否存在
    [ -e "$file" ] || continue

    # 提取文件名（不包含扩展名）
    base="${file%.po}"

    # 使用 msgfmt 转换 .po 文件为 .mo 文件
    msgfmt -o "$base.mo" "$file"

    # 输出转换状态
    echo "Converted $file to $base.mo"
done
