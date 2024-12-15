// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "fcitx5configtool.h"

#include "ocean-control-center/oceanuifactory.h"

#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QtQml/QQmlEngine>

// #include "oceanuifactory.h"

namespace lingmo {
namespace fcitx5configtool {
Fcitx5ConfigToolWorker::Fcitx5ConfigToolWorker(QObject *parent)
    : QObject(parent)
{

}

DCC_FACTORY_CLASS(Fcitx5ConfigToolWorker)
} // namespace fcitx5configtool
} // namespace lingmo

#include "fcitx5configtool.moc"
