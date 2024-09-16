// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "signal_handler.h"
#include <QDebug>


signal_handler::signal_handler(QObject *parent) : QObject(parent)
{

}

void signal_handler::onOutputAdded(QString output)
{
    qDebug() << "cath signal: " << output;
}

void signal_handler::onOutputRemoved(QString output)
{
    qDebug() << "cath signal: " << output;
}

void signal_handler::onOutputChanged(QString output)
{
    qDebug() << "cath signal: " << output;
}
