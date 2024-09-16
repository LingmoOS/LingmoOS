# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

option(ENABLE_PLUGIN_OOM "Build plugin-oom" ON)

if (ENABLE_PLUGIN_OOM)
    add_subdirectory("plugin-oom")
endif()

