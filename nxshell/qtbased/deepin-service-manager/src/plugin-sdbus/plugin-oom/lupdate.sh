#!/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: MIT

#判断文件夹是否存在，如果不存在，就新建文件夹

if [ ! -d "translations" ]; then
  mkdir translations
fi
cd translations
if [ -e "plugin-oom.ts" ]; then
  rm plugin-oom.ts
fi
touch plugin-oom.ts

echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>" >> ./plugin-oom.ts
echo "<!DOCTYPE TS>" >> ./plugin-oom.ts
echo "<TS version=\"2.1\">" >> ./plugin-oom.ts
echo "<context>" >> ./plugin-oom.ts
echo "    <name>oom</name>" >> ./plugin-oom.ts

# 读取json文件内容
json=$(cat "../earlyoom.service.json")

# 解析json内容，并拿到要执行的内容
value=$(echo "$json" | jq '.Exec[2]')
# 将字符串按照分号进行分割并循环遍历
echo "$value" | tr ';' '\n' | while read -r element; do
  result=$(echo $element | grep "getfromqm")
  if [ "$result" != "" ]
  then
	content=$(echo $result | awk -F "'" '{print $2}')
    echo "    <message>" >> ./plugin-oom.ts
    echo "        <source>$content</source>" >> ./plugin-oom.ts
  	echo "        <translation type=\"unfinished\"></translation>" >> ./plugin-oom.ts
    echo "    </message>" >> ./plugin-oom.ts
  fi
done

echo "</context>" >> ./plugin-oom.ts
echo "</TS>" >> ./plugin-oom.ts
cd ../
