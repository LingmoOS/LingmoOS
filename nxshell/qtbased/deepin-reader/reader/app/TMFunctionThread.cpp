// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TMFunctionThread.h"

TMFunctionThread::TMFunctionThread(QObject *parent) : QThread(parent)
{

}

TMFunctionThread::~TMFunctionThread()
{
    this->wait();
}

void TMFunctionThread::run()
{
    result = func();
}
