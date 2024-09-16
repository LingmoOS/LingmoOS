#!/bin/bash

#下载lcov: apt source lcov （以源码包为例）
#解压lcov-x.xx.tar.gz，进入源码包，执行sudo make install
#在命令行执行lcov -v，正确输出版本号即安装成功

# 需要先安装lcov，打开./unittest/CMakeLists.txt 测试覆盖率的编译条件
# 将该脚本放置到dde-launcher-unittest二进制文件同级目录运行
lcov -c -i -d ./ -o init.info
./dde_launcher_unit_test
lcov -c -d ./ -o cover.info
lcov -a init.info -a cover.info -o total.info

#文件过滤,不统计以下目录覆盖率信息
lcov --remove total.info '*/usr/include/*' '*/usr/lib/*' '*/usr/lib64/*' '*/usr/local/include/*' '*/usr/local/lib/*' '*/usr/local/lib64/*' '*/third/*' 'testa.cpp' '*dbusinterface*' '*tests*' '*dbusservices*' -o final.info

# 生成报告
genhtml -o cover_report --legend --title "lcov"  --prefix=./ final.info

#打开报告
nohup x-www-browser ./cover_report/index.html &

exit 0
