/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

//@@snippet_begin(sample-helloworld)
#include <ThreadWeaver/ThreadWeaver>

#include <QCoreApplication>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    using namespace ThreadWeaver;
    stream() << make_job([]() {
        qDebug() << "Hello World!";
    });
}
//@@snippet_end
