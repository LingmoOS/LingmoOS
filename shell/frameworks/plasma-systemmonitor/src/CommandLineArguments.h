/**
 * SPDX-FileCopyrightText: 2023 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QCommandLineParser>
#include <QObject>
#include <QPointer>
#include <qqmlregistration.h>

#include <KAboutData>

class CommandLineArguments : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString pageId READ pageId CONSTANT)
    Q_PROPERTY(QString pageName READ pageName CONSTANT)
    Q_PROPERTY(QVariant aboutData READ aboutData CONSTANT)
    QML_ELEMENT
    QML_SINGLETON

public:
    CommandLineArguments(QObject *parent = nullptr);

    QString pageId() const;
    QString pageName() const;
    QVariant aboutData() const;

    static void setCommandLineParser(std::shared_ptr<QCommandLineParser> parser);

private:
    QString m_pageId;
    QString m_pageName;

    inline static std::shared_ptr<QCommandLineParser> s_commandLineParser;
};
