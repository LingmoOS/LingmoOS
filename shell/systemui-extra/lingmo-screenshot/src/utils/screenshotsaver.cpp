/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
    *           2020 KylinSoft Co., Ltd.
    * This file is part of Lingmo-Screenshot.
     * This program is free software: you can redistribute it and/or modify
     * it under the terms of the GNU General Public License as published by
     * the Free Software Foundation, either version 3 of the License, or
     * (at your option) any later version.
     *
     * This program is distributed in the hope that it will be useful,
     * but WITHOUT ANY WARRANTY; without even the implied warranty of
     * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     * GNU General Public License for more details.
     *
     * You should have received a copy of the GNU General Public License
     * along with this program.  If not, see <https://www.gnu.org/licenses/>.
    */
    #include "screenshotsaver.h"
    #include "src/utils/systemnotification.h"
    #include "src/utils/filenamehandler.h"
    #include "src/utils/confighandler.h"
    #include <QClipboard>
    #include <QApplication>
    #include <QMessageBox>
    #include <QImageWriter>
    #include <QStandardPaths>
    #include <QFileInfo>
    #include "mysavedialog.h"
    #include <QDebug>
#include <QDesktopWidget>
#include <windowmanager/windowmanager.h>
ScreenshotSaver::ScreenshotSaver()
{
    ScreenshotGsettings = new QGSettings("org.lingmo.screenshot");
    m_savePath = ScreenshotGsettings->get("screenshot-path").toString();
    m_saveName = ScreenshotGsettings->get("screenshot-name").toString();
    m_saveType = ScreenshotGsettings->get("screenshot-type").toString();
    if (m_savePath == "" || m_savePath.isNull()) {
        m_savePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0)
                     + QLatin1String("/");
        ScreenshotGsettings->set("screenshot-path", m_savePath);
    }
    if (m_saveType == "" || m_saveType.isNull()) {
        m_saveType = QLatin1String(".png");
        ScreenshotGsettings->set("screenshot-type", m_saveType);
    }
}

void ScreenshotSaver::saveToClipboard(const QPixmap &capture)
{
    SystemNotification().sendMessage(
        QObject::tr("Capture saved to clipboard"));
    QApplication::clipboard()->setPixmap(capture);
}

bool ScreenshotSaver::saveToFilesystem(const QPixmap &capture, const QString &path)
{
    QString completeName = ScreenshotGsettings->get("new-screenshot-name").toString();
    if (completeName == "" || completeName.isNull()) {
        completeName = FileNameHandler().parsedPattern();
    } else {
        ScreenshotGsettings->set("new-screenshot-name", "");
    }
    QString completePath;
    if (!path.endsWith('/')) {
        completePath = QString(path + QLatin1String("/")  + completeName);
        ScreenshotGsettings->set("screenshot-path", QString(path + QLatin1String("/")));
    } else {
        completePath = QString(path+ completeName);
        ScreenshotGsettings->set("screenshot-pth", path);
    }
    ScreenshotGsettings->set("screenshot-name", completeName);
    completePath = completePath + m_saveType;
    bool ok = capture.save(completePath);
    QString saveMessage;
    QString message;
    QString notificationPath = completePath;

    if (ok) {
        ConfigHandler().setSavePath(path);
        message = QObject::tr("Capture saved as ") + completePath;
        saveMessage = "<a href=\"" + completePath + "\">" + message + "</a>";
        QApplication::clipboard()->setPixmap(capture);
    } else {
        message = QObject::tr("Error trying to save as ") + completePath;
        saveMessage = "<a href=\"" + completePath + "\">" + message + "</a>";
        notificationPath = "";
    }

    SystemNotification().sendMessage(saveMessage, notificationPath);
    return ok;
}

bool ScreenshotSaver::saveToFilesystem(const QPixmap &capture, const QString &path,
                                       const QString &type)
{
    // QString completePath = FileNameHandler().generateAbsolutePath(path);
    QString completeName = ScreenshotGsettings->get("new-screenshot-name").toString();
    if (completeName == "" || completeName.isNull()) {
        completeName = FileNameHandler().parsedPattern();
    } else {
        ScreenshotGsettings->set("new-screenshot-name", "");
    }
    QString completePath;
    if (!path.endsWith('/')) {
        completePath = QString(path + QLatin1String("/")  + completeName);
        ScreenshotGsettings->set("screenshot-path", QString(path + QLatin1String("/")));
    } else {
        completePath = QString(path+ completeName);
        ScreenshotGsettings->set("screenshot-path", path);
    }
    ScreenshotGsettings->set("screenshot-name", completeName);
    completePath = completePath + type;
    bool ok = capture.save(completePath);
    QString saveMessage;
    QString message;
    QString notificationPath = completePath;

    if (ok) {
        ConfigHandler().setSavePath(path);
        // saveMessage = QObject::tr("Capture saved as ") + completePath;
        message = QObject::tr("Capture saved as ") + completePath;
        saveMessage = "<a href=\"" + completePath + "\">" + message + "</a>";
        QApplication::clipboard()->setPixmap(capture);
    } else {
        if(m_info.waylandDectected()){
            message = QObject::tr("In Wayland mode, screenshots of application windows are not supported ") + completePath;
            saveMessage = "<a href=\"" + completePath + "\">" + message + "</a>";
            notificationPath = "";

        }else{
        message = QObject::tr("Error trying to save as ") + completePath;
        saveMessage = "<a href=\"" + completePath + "\">" + message + "</a>";
        notificationPath = "";
        }
    }

    SystemNotification().sendMessage(saveMessage, notificationPath);
    return ok;
}

bool ScreenshotSaver::saveToFilesystemGUI(const QPixmap &capture)
{
    bool ok = false;
    while (!ok) {
        MySaveDialog *a = new  MySaveDialog(nullptr);
        a->move(((qApp->desktop()->geometry().width()
                            -a->width())/2),
                          ((qApp->desktop()->geometry().height()
                            -a->height())/2));
        kdk::WindowManager::setGeometry(a->windowHandle(),a->geometry());
        if (a->exec() == QFileDialog::Accepted) {
            QString savePath = a->selectedFiles().at(0);

            QFileInfo fileName = savePath;
            if (savePath.isNull()) {
                break;
            }

            if (fileName.suffix().isNull()) {
                savePath += QLatin1String(".png");
            }
            fileName = savePath;
            ScreenshotGsettings->set("screenshot-type", QLatin1String(".") +fileName.suffix());
            QString name = fileName.fileName();
            QString msg;
            QString message;
            if (!name.startsWith(QChar('.'), Qt::CaseInsensitive)) {
                ok = capture.save(savePath);
            }
            if (ok) {
                ScreenshotGsettings->set("screenshot-type", QLatin1String(".") +fileName.suffix());
                QString pathNoFile = savePath.left(savePath.lastIndexOf(QLatin1String("/")));
                ConfigHandler().setSavePath(pathNoFile);
                message = QObject::tr("Capture saved as ") + savePath;
                msg = "<a href=\"" + savePath + "\">" + message + "</a>";
                SystemNotification().sendMessage(msg, savePath);
                ScreenshotGsettings->set("screenshot-path", pathNoFile);
                ScreenshotGsettings->set("screenshot-name",
                                         name.left(name.lastIndexOf(QLatin1String("."))));
            } else {
                if (name.contains(QChar('/'))) {
                    msg = QObject::tr("file name can not contains '/'");
                } else if (name.startsWith(QChar('.'), Qt::CaseInsensitive)) {
                    msg = QObject::tr("can not save file as hide file");
                } else if (name.length() > 255) {
                    msg = QObject::tr("can not save  because filename too long");
                } else {
                    message = QObject::tr("Error trying to save as ") + savePath;
                    msg = "<a href=\"" + savePath + "\">" + message + "</a>";
                }
                QMessageBox saveErrBox(
                    QMessageBox::Warning,
                    QObject::tr("Save Error"),
                    msg);
                saveErrBox.exec();
            }
        } else {
            return ok;
        }
    }
    return ok;
}
