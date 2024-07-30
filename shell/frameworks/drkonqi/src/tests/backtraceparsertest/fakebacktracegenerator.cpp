/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <kiagiadakis.george@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "fakebacktracegenerator.h"
#include <QFile>
#include <QTextStream>

void FakeBacktraceGenerator::sendData(const QString &filename)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream stream(&file);

    Q_EMIT starting();
    while (!stream.atEnd()) {
        Q_EMIT newLine(stream.readLine() + QLatin1Char('\n'));
    }
    Q_EMIT newLine(QString());
}

#include "moc_fakebacktracegenerator.cpp"
