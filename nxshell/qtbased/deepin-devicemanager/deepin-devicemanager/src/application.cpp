// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"

#include <QMetaType>

Application::Application(int &argc, char **argv)
    : DApplication(argc, argv)
{
    qRegisterMetaType<Application::TaskState>("Application::BackgroundTaskState");
}
