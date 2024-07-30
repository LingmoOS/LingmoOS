/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <kiagiadakis.george@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FAKEBACKTRACEGENERATOR_H
#define FAKEBACKTRACEGENERATOR_H

#include <QObject>

class FakeBacktraceGenerator : public QObject
{
    Q_OBJECT
public:
    explicit FakeBacktraceGenerator(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    void sendData(const QString &filename);

Q_SIGNALS:
    void starting();
    void newLine(const QString &line);
};

#endif // FAKEBACKTRACEGENERATOR_H
