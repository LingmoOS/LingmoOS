#!/bin/bash

# 检查文件是否存在
if [ ! -f repos.txt ]; then
    echo "Error: repos.txt file does not exist."
    exit 1
fi

# 读取文件中的每一行，并克隆仓库
while read -r line
do
    git clone http://github.com/LingmoOS/"$line"
done < repos.txt
