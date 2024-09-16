/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef QMLLOADER_H
#define QMLLOADER_H

#include <QObject>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QStandardPaths>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFile>
#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDebug>
#include <QPointer>
#include "dataconverter.h"
#include "adjunctaide.h"



#define DBUS_NAME "com.deepin.dde.UserFeedback"
#define DBUS_PATH "/com/deepin/dde/UserFeedback"
#define PROPERTY_IFCE "org.freedesktop.DBus.Properties"

class QmlLoaderDBus;
class QProcess;

struct Draft{
    DataConverter::FeedbackType feedbackType;
    QString title;
    QString content;
    QString email;
    bool helpDeepin;
    QStringList adjunctPathList;

    Draft(DataConverter::FeedbackType _feedbackType = DataConverter::DFeedback_Bug,
          QString _title="",
          QString _content = "",
          QString _email = "",
          bool _helpDeepin = true,
          QStringList _adjunctPathList = QStringList())
        :feedbackType(_feedbackType),title(_title),content(_content),email(_email),helpDeepin(_helpDeepin),adjunctPathList(_adjunctPathList){}
};

Q_DECLARE_METATYPE(DataConverter::FeedbackType)

class QmlLoader : public QObject
{
    Q_OBJECT
public:
    explicit QmlLoader(QObject *parent = 0);
    ~QmlLoader();

    QQmlEngine* engine;
    QQmlComponent* component;
    QQmlContext * rootContext;
    QObject * rootObject;

    void load(QUrl url);

    //call by main
    void reportBug();   //Unselected target
    void reportBug(const QString &target); //specity the target
    QStringList getSupportAppList();

public Q_SLOTS:
    //call by UI
    QString getHomeDir();
    bool saveDraft(const QString &targetApp,
                   const DataConverter::FeedbackType type,
                   const QString &title,
                   const QString &email,
                   const bool &helpDeepin,
                   const QString &content);
    void clearAllDraft();
    void clearDraft(const QString &targetApp);
    void clearSysAdjuncts(const QString &targetApp);
    QString addAdjunct(const QString &filePath, const QString &target);
    void removeAdjunct(const QString &filePath);
    bool draftTargetExist(const QString &target);
    bool draftNotEmpty(const QString &target);
    void updateUiDraftData(const QString &target);
    void getScreenShot(const QString &target);
    bool canAddAdjunct(const QString &target);
    qint64 getAdjunctSize(const QString &fileName);
    bool adjunctExist(const QString &filePath, const QString &target);
    void saveEmail(const QString &email);
    QStringList getEmails();
    QString getMatchEmailPart(const QString &text);

Q_SIGNALS:
    void getScreenshotFinish(QString fileName);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void init();

    void showManual();
    void parseJsonData(const QByteArray &byteArray, Draft * draft);
    void parseJsonArray(const QByteArray &byteArray, QStringList * emailsList);
    Draft getDraft(const QString &targetApp);
    qint64 getAdjunctsSize(const QString &target);
    QString getFileNameFromPath(const QString &filePath);
private:
    QPointer<QProcess> m_manualPro;
    AdjunctAide * adjunctAide;
    QmlLoaderDBus * mDbusProxyer;
};


class QmlLoaderDBus : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", DBUS_NAME)

public:
    QmlLoaderDBus(QmlLoader* parent);
    ~QmlLoaderDBus();

    Q_SLOT void switchProject(QString name);

private:
    QmlLoader* m_parent;
};
#endif // QMLLOADER_H
