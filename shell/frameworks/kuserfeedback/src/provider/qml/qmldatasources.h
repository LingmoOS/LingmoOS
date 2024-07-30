/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_QMLDATASOURCES_H
#define KUSERFEEDBACK_QMLDATASOURCES_H

#include "qmlabstractdatasource.h"

namespace KUserFeedback {

#define MAKE_BASIC_QML_WRAPPER(x) \
    class Qml ## x : public QmlAbstractDataSource { \
        Q_OBJECT \
    public: \
        explicit Qml ## x (QObject *parent = nullptr);\
    };

MAKE_BASIC_QML_WRAPPER(ApplicationVersionSource)
MAKE_BASIC_QML_WRAPPER(CompilerInfoSource)
MAKE_BASIC_QML_WRAPPER(CpuInfoSource)
MAKE_BASIC_QML_WRAPPER(LocaleInfoSource)
MAKE_BASIC_QML_WRAPPER(OpenGLInfoSource)
MAKE_BASIC_QML_WRAPPER(PlatformInfoSource)
MAKE_BASIC_QML_WRAPPER(QPAInfoSource)
MAKE_BASIC_QML_WRAPPER(QtVersionSource)
MAKE_BASIC_QML_WRAPPER(ScreenInfoSource)
MAKE_BASIC_QML_WRAPPER(StartCountSource)
MAKE_BASIC_QML_WRAPPER(UsageTimeSource)

#undef MAKE_BASIC_QML_WRAPPER
}

#endif // KUSERFEEDBACK_QMLDATASOURCES_H
