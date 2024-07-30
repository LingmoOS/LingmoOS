/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "qmldatasources.h"

#include <KUserFeedback/ApplicationVersionSource>
#include <KUserFeedback/CompilerInfoSource>
#include <KUserFeedback/CpuInfoSource>
#include <KUserFeedback/LocaleInfoSource>
#include <KUserFeedback/OpenGLInfoSource>
#include <KUserFeedback/PlatformInfoSource>
#include <KUserFeedback/QPAInfoSource>
#include <KUserFeedback/QtVersionSource>
#include <KUserFeedback/ScreenInfoSource>
#include <KUserFeedback/StartCountSource>
#include <KUserFeedback/UsageTimeSource>

using namespace KUserFeedback;

#define MAKE_BASIC_QML_WRAPPER(x) Qml ## x::Qml ## x(QObject *parent) : QmlAbstractDataSource(new x, parent) {}

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

#undef MAKE_DEFAULT_SOURCE

#include "moc_qmldatasources.cpp"
