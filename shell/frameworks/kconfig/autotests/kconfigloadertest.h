/*
    SPDX-FileCopyrightText: 2010 Martin Blumenstingl <darklight.xdarklight@googlemail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGLOADERTEST_H
#define KCONFIGLOADERTEST_H

#include <QTest>

class KConfigLoader;

class QFile;

class ConfigLoaderTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void boolDefaultValue();
    void colorDefaultValue();
    void dateTimeDefaultValue();
    void enumDefaultValue();
    void enumDefaultValueString();
    void fontDefaultValue();
    void intDefaultValue();
    void passwordDefaultValue();
    void pathDefaultValue();
    void stringDefaultValue();
    void stringListDefaultValue();
    void uintDefaultValue();
    void urlDefaultValue();
    void doubleDefaultValue();
    void intListEmptyDefaultValue();
    void intListDefaultValue();
    void longLongDefaultValue();
    void pointDefaultValue();
    void pointFDefaultValue();
    void rectDefaultValue();
    void rectFDefaultValue();
    void sizeDefaultValue();
    void sizeFDefaultValue();
    void ulongLongDefaultValue();

private:
    KConfigLoader *cl;
    QFile *configFile;
};

#endif
