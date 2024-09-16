#!/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

make clean

rm *.gcno -r

rm *.gcda -r

rm *.o -r

rm moc_* -r

rm *Test -r

rm report -r

rm coverage -r

rm .qmake.stash

rm Makefile

rm coverage.info

rm test-deepin-reader

rm nohup.out

rm qrc_resources.cpp

rm target_wrapper.sh

rm copy.txt

rm clean

exit 0
