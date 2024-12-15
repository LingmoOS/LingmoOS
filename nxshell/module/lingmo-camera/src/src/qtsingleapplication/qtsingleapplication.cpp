// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qtsingleapplication.h"
#include <QWidget>

/*!
    Creates a QtSingleApplication object. The application identifier
    will be QCoreApplication::applicationFilePath(). \a argc, \a
    argv, and \a GUIenabled are passed on to the QAppliation constructor.

    If you are creating a console application (i.e. setting \a
    GUIenabled to false), you may consider using
    QtSingleCoreApplication instead.
*/

QtSingleApplication::QtSingleApplication(int &argc, char **argv)
    : QObject(nullptr)
{
#if (DTK_VERSION < DTK_VERSION_CHECK(5, 4, 0, 0))
    _app = new DApplication(argc, argv);
#else
    _app = DApplication::globalApplication(argc, argv);
#endif
}

DApplication *QtSingleApplication::dApplication()
{
    return _app;
}


QtSingleApplication::~QtSingleApplication()
{
    if (_app != nullptr) {
        _app->deleteLater();
        _app = nullptr;
    }
}
