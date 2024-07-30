# SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-clause

if (TARGET Qt6::Widgets)
    add_executable(spinbox spinbox.cpp)
    target_link_libraries(spinbox PRIVATE Qt6::Widgets KF6::I18n)
endif()
