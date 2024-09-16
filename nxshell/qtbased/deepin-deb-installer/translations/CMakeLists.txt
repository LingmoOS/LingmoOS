# Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
#
# Author:     cuizhen <cuizhen@deepin.io>
#
# Maintainer: cuizhen <cuizhen@uniontech.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

file(GLOB APP_TS_FILES LIST_DIRECTORIES false *.ts)

qt5_add_translation(APP_QM_FILES ${APP_TS_FILES})

file(GLOB QM_FILES "${CMAKE_SOURCE_DIR}/*.qm")

install(FILES ${APP_QM_FILES} DESTINATION share/deepin-deb-installer/translations)

add_custom_target(QMFILES ALL DEPENDS ${APP_QM_FILES})

add_subdirectory(policy)
