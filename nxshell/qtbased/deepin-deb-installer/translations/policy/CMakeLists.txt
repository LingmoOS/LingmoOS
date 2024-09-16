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

find_program(POLICY_TS_CONV deepin-policy-ts-convert REQUIRED)
add_custom_target(APP_POLICY_FILE
    ALL
    COMMAND ${POLICY_TS_CONV} ts2policy ${CMAKE_CURRENT_SOURCE_DIR}/${POLICY_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/ ${POLICY_FILE}.tmp
    COMMAND mv ${POLICY_FILE}.tmp ${POLICY_FILE}
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${POLICY_FILE} DESTINATION share/polkit-1/actions)
