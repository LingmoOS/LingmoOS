# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

find_program(pwplay_PATH pw-play REQUIRED)
find_program(pipewire_PATH pipewire REQUIRED)
ecm_add_test(mediamonitortest.cpp LINK_LIBRARIES Qt::Test Qt::Quick Qt::Qml)
