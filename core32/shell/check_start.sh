#!/bin/sh

# 最大重试次数
RETRIES=50
# 每次重试之间的间隔（秒）
INTERVAL=0.5

for i in $(seq 1 $RETRIES); do
    if pgrep -f "lingmo-filemanager" > /dev/null; then
        exit 0
    fi
    sleep $INTERVAL
done

exit 1