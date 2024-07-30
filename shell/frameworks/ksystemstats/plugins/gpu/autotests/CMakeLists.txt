# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2023 David Redondo <kde@david-redondo.de>

ecm_add_test(nvidia.cpp ../NvidiaSmiProcess.cpp
    TEST_NAME nvidiatest
    LINK_LIBRARIES Qt::Test
)
