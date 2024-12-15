TEMPLATE = subdirs

#QMAKE_CXX = ccache $$QMAKE_CXX //linux使用ccache加速c++编译速度

CONFIG  += ordered

###安全漏洞检测
#QMAKE_CXX += -g -fsanitize=undefined,address -O2
#QMAKE_CXXFLAGS += -g -fsanitize=undefined,address -O2
#QMAKE_LFLAGS += -g -fsanitize=undefined,address -O2

message("Build on host arch: $$QMAKE_HOST.arch")

SUBDIRS += 3rdparty/lingmo-pdfium

SUBDIRS += htmltopdf

SUBDIRS += reader

