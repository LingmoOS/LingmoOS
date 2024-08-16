#!/bin/bash

# 确保脚本在包含软件包的目录中运行

for pkg in *.lpk; do
    echo "正在签名: $pkg"
    # 使用你的 GPG 密钥对软件包进行签名
    gpg --detach-sign -o "$pkg".sig -a "$pkg"
    if [ $? -eq 0 ]; then
        echo "签名成功: $pkg"
    else
        echo "签名失败: $pkg"
    fi
done
