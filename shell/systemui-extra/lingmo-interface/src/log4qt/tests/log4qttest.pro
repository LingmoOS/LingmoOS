# *******************************************************************************
#
# package:     Log4Qt
# file:        log4qttest.pro
# created:     September 2007
# author:      Martin Heinrich
#
# 
# Copyright 2007 Martin Heinrich
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 
# *******************************************************************************

TEMPLATE = app

isEqual(QT_MAJOR_VERSION, 5) {
    QT += testlib
} else { # if not Qt5
    CONFIG += qtestlib
}

DEFINES -= \
    QT_NO_CAST_FROM_ASCII \
	QT_NO_CAST_TO_ASCII

include(../src/log4qt/log4qt.pri)

HEADERS += log4qttest.h
SOURCES += log4qttest.cpp


