/*
 *  SPDX-FileCopyrightText: 2021 Felipe Kinoshita <kinofhek@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <KAboutData>
#include <QJSValue>
#include <QObject>
#include <QVersionNumber>
#include <qqmlregistration.h>

class Controller : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    Controller();

    Q_INVOKABLE static void launchApp(const QString &program);
    Q_INVOKABLE void runCommand(const QString &command, QJSValue callback = QJSValue());
    Q_INVOKABLE static void copyToClipboard(const QString &text);
    Q_INVOKABLE bool networkAlreadyConnected();
    Q_INVOKABLE bool userFeedbackAvailable();
    Q_INVOKABLE QStringList distroPages();
    Q_INVOKABLE QString distroName();
    Q_INVOKABLE QString distroIcon();
    Q_INVOKABLE QString distroUrl();
    Q_INVOKABLE bool isDistroSnapOnly();
    Q_INVOKABLE QString distroBugReportUrl();
    Q_INVOKABLE QString installPrefix();

    Q_PROPERTY(KAboutData aboutData READ aboutData CONSTANT)
    Q_PROPERTY(Mode mode MEMBER m_mode CONSTANT)
    Q_PROPERTY(QString shownVersion MEMBER m_shownVersion CONSTANT)
    Q_PROPERTY(int patchVersion MEMBER m_patchVersion CONSTANT)
    Q_PROPERTY(QString releaseUrl MEMBER m_releaseUrl CONSTANT)
    Q_PROPERTY(QString customIntroText MEMBER m_customIntroText CONSTANT)
    Q_PROPERTY(QString customIntroIcon MEMBER m_customIntroIcon CONSTANT)
    Q_PROPERTY(QString customIntroIconLink MEMBER m_customIntroIconLink CONSTANT)
    Q_PROPERTY(QString customIntroIconCaption MEMBER m_customIntroIconCaption CONSTANT)

    enum Mode { Update, Beta, Live, Welcome };
    Q_ENUM(Mode)

    KAboutData aboutData() const;
    void setMode(Mode mode);

Q_SIGNALS:
    void modeChanged();

private:
    Mode m_mode = Mode::Welcome;
    QVersionNumber m_version;
    QString m_shownVersion;
    int m_patchVersion;
    QString m_releaseUrl;
    QString m_customIntroText;
    QString m_customIntroIcon;
    QString m_customIntroIconLink;
    QString m_customIntroIconCaption;
};
