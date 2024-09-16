// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dservicemanager.h"
#include "ddatabasemanagement.h"
#include "commondef.h"
#include <DLog>

#include <QDBusConnection>
#include <QDBusError>
#include <QTranslator>
#include <QCoreApplication>


bool loadTranslator(QCoreApplication *app, QList<QLocale> localeFallback = QList<QLocale>() << QLocale::system())
{
    bool bsuccess = false;
    QString CalendarServiceTranslationsDir = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                  		    QString("dde-calendar/translations"),
								    QStandardPaths::LocateDirectory);
    for (auto &locale : localeFallback) {
        QString translateFilename = QString("%1_%2").arg(app->applicationName()).arg(locale.name());
        QString translatePath = QString("%1/%2.qm").arg(CalendarServiceTranslationsDir).arg(translateFilename);
        if (QFile(translatePath).exists()) {
            QTranslator *translator = new QTranslator(app);
            translator->load(translatePath);
            app->installTranslator(translator);
            bsuccess = true;
        }
        QStringList parseLocalNameList = locale.name().split("_", QString::SkipEmptyParts);
        if (parseLocalNameList.length() > 0 && !bsuccess) {
            translateFilename = QString("%1_%2").arg(app->applicationName()).arg(parseLocalNameList.at(0));
            QString parseTranslatePath = QString("%1/%2.qm").arg(CalendarServiceTranslationsDir).arg(translateFilename);
            if (QFile::exists(parseTranslatePath)) {
                QTranslator *translator = new QTranslator(app);
                translator->load(parseTranslatePath);
                app->installTranslator(translator);
                bsuccess = true;
            }
        }
    }
    return bsuccess;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setOrganizationName("deepin");
    a.setApplicationName("dde-calendar-service");

    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerFileAppender();
    Dtk::Core::DLogManager::registerJournalAppender();

    //加载翻译
    if (!loadTranslator(&a)) {
        qCDebug(ServiceLogger) << "loadtranslator failed";
    }

    DDataBaseManagement dbManagement;

    DServiceManager serviceManager;

    //如果存在迁移，则更新提醒
    if(dbManagement.hasTransfer()){
        //延迟处理
        QTimer::singleShot(0,[&](){
          serviceManager.updateRemindJob();
        });
    }
    qCDebug(ServiceLogger) << "dde-calendar-service start";
    return a.exec();
}
