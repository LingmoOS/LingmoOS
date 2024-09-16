// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QProcess>
#include "utils.h"
#include <QDebug>


namespace Utils {

QPixmap renderSVG(const QString &path, const QSize &size)
{
    QImageReader reader;
    QPixmap pixmap;
    reader.setFileName(path);
    if (reader.canRead()) {
        const qreal ratio = qApp->devicePixelRatio();
        reader.setScaledSize(size * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
    }
    else {
        pixmap.load(path);
    }
    return pixmap;
    }


QJsonObject QStringToJson(QString &jsonString)
{
    QJsonObject jsonObject;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonString.toLocal8Bit().data());
    if(jsonDocument.isNull()) {
        return  jsonObject;
    }
    jsonObject = jsonDocument.object();
    return jsonObject;

}

QString JsonToQString(QJsonObject &jsonObject)
{
     return QString(QJsonDocument(jsonObject).toJson());
}

const QPixmap hidpiPixmap(const QString &path, const QSize &sz)
{
    const auto ratio = qApp->devicePixelRatio();
    QPixmap iconPix = Utils::renderSVG(path, sz);
    iconPix.setDevicePixelRatio(ratio);

    return iconPix;
}

bool setDisablePluginList()
{
    QProcess process;
    process.setProgram("gsettings");
    process.setArguments(QStringList() << "get" << "com.deepin.dde.dock.disableplugins" << "disable-plugins-list");
    process.start();
    // Wait for process to finish without timeout.
    process.waitForFinished(-1);
    QString output = process.readAllStandardOutput();
    QString err = process.readAllStandardError();

    if (process.exitStatus() != QProcess::NormalExit && process.exitCode() == 0) {
        qCritical() << "gsettings get failed： " << err;
        return false;
    }

    output.remove("\'");
    int startPos = output.indexOf("[");
    int endPos = output.indexOf("]");

    if (endPos > startPos + 1) {
        QStringList list = output.mid(startPos + 1, endPos - startPos - 1).split(',');
        if (!list.contains("liboverlay-warning.so")) {
            list.append("liboverlay-warning.so");

            for (int i = 0; i < list.size(); i++) {
                list[i] = QString("\"%1\"").arg(list[i].trimmed());
            }

            QString disable_plugins_list = QString("[%1]").arg(list.join(','));
            disable_plugins_list = disable_plugins_list.trimmed();
            process.setArguments(
                    QStringList() << "set" << "com.deepin.dde.dock.disableplugins" << "disable-plugins-list" << disable_plugins_list);
            process.start();
            process.waitForFinished(-1);
            err = process.readAllStandardError();
            if (process.exitStatus() != QProcess::NormalExit && process.exitCode() == 0) {
                qWarning() << "gsettings set failed" << err;
                return false;
            }
        }
    } else if (endPos == startPos + 1){
        process.setArguments(
                QStringList() << "set" << "com.deepin.dde.dock.disableplugins" << "disable-plugins-list" << QString("[\"%1\"]").arg("liboverlay-warning.so"));
        process.start();
        process.waitForFinished(-1);
        err = process.readAllStandardError();
        if (process.exitStatus() != QProcess::NormalExit && process.exitCode() == 0) {
            qWarning() << "gsettings set failed" << err;
            return false;
        }
    }
    return true;
}

bool resetDisablePluginList()
{
    QProcess process;
    process.setProgram("gsettings");
    process.setArguments(QStringList() << "reset" << "com.deepin.dde.dock.disableplugins" << "disable-plugins-list");
    process.start();
    // Wait for process to finish without timeout.
    process.waitForFinished(-1);
    QString output = process.readAllStandardOutput();
    QString err = process.readAllStandardError();

    if (process.exitStatus() != QProcess::NormalExit && process.exitCode() == 0) {
        qCritical() << "gsettings get failed： " << err;
        return false;
    }
    return true;
}

}
