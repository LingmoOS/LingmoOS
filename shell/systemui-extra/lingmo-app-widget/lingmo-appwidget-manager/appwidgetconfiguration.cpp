/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
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
 *
 * Authors: wangyan <wangyan@kylinos.cn>
 *
 */

#include "appwidgetconfiguration.h"
#include <QDebug>
#include <QSettings>
#include <QTextCodec>
#include <QFileSystemWatcher>
#include <QDir>

static const char* CONFIG_FILE_PATH = "/usr/share/appwidget/config/";

namespace AppWidget {
AppWidgetConfiguration::AppWidgetConfiguration(QObject *parent)
    : QObject(parent)
{
    m_configInfo = getCurrentConfigInfo();
    initFileWatcher();
}

QString AppWidgetConfiguration::getProviderNameByAppWidgetName(const QString &appwidgetname)
{
    if(appwidgetname.isEmpty()) {
        qWarning() << __FILE__ << __FUNCTION__ << "parameter is empty";
        return QString();
    }
    QString path = QString(CONFIG_FILE_PATH) + QString("%1.conf").arg(appwidgetname);
    QSettings setting(path, QSettings::IniFormat);
    setting.beginGroup("AppWidget");
    QString providerName = setting.value("providerName").toString();
    qDebug() << __FILE__ << __FUNCTION__ << appwidgetname << " `s appwidgetname is :" <<providerName;
    setting.endGroup();
    setting.sync();
    return providerName;
}
QString AppWidgetConfiguration::getAppWidgetUiFile(const QString &appwidgetname)
{
    if(appwidgetname.isEmpty()) {
        qWarning() << "parameter is empty";
        return QString();
    }
    QString path = QString(CONFIG_FILE_PATH) + QString("%1.conf").arg(appwidgetname);
    QSettings setting(path, QSettings::IniFormat);
    setting.beginGroup("AppWidget");
    QString qmlFile = setting.value("qmlFile").toString();
    qDebug() << __FILE__ << __FUNCTION__ << appwidgetname << " `s qmlFile is :" <<qmlFile;
    setting.endGroup();
    setting.sync();
    return qmlFile;
}
//获取ui文件
QString AppWidgetConfiguration::getAppWidgetConfig(const QString &appwidgetname, const QString &config)
{
    if(appwidgetname.isEmpty() || config.isEmpty()) {
        qWarning() << __FILE__ << __FUNCTION__ << "parameter is empty";
        return QString();
    }
    QString path = QString(CONFIG_FILE_PATH) + QString("%1.conf").arg(appwidgetname);
    return getConfigurationByKey(path, config);
}

//获取属性
QMap<QString, QVariant> AppWidgetConfiguration::getAppWidgetConfigs(const QString &appwidgetname, QString begianName)
{
    if(appwidgetname.isEmpty()) {
        qWarning() << __FILE__ << __FUNCTION__ << "parameter is empty";
        return QMap<QString, QVariant>();
    }
    QMap<QString, QVariant> configs;
    QString path = QString(CONFIG_FILE_PATH) + QString("%1.conf").arg(appwidgetname);
    QSettings setting(path, QSettings::IniFormat);
    setting.beginGroup(begianName);
    QStringList keylist = setting.allKeys();
    for (int i = 0; i < keylist.count(); i++) {
        configs.insert(keylist.at(i),setting.value(keylist.at(i)));
    }
    setting.endGroup();
    setting.sync();
    return configs;
}
QString AppWidgetConfiguration::getConfigurationByKey(const QString &path, const QString &key)
{
    if(path.isEmpty() || key.isEmpty()) {
        qWarning() << __FILE__ << __FUNCTION__ << "parameter is empty";
        return QString();
    }
    QSettings setting(path, QSettings::IniFormat);
    setting.setIniCodec(QTextCodec::codecForName("utf-8"));
    setting.beginGroup("AppWidget");
    QString value = setting.value(key).toString();
    setting.endGroup();
    setting.sync();
    return value;
}

void AppWidgetConfiguration::initFileWatcher()
{
    QFileSystemWatcher* fileWatcher = new QFileSystemWatcher(this);
    fileWatcher->addPath(QString(CONFIG_FILE_PATH));
    connect(fileWatcher, &QFileSystemWatcher::directoryChanged, this, &AppWidgetConfiguration::onDirectoryChanged);
}

QMap<QString, QString> AppWidgetConfiguration::getCurrentConfigInfo() const
{
    QMap<QString, QString> files;
    QStringList configFiles = getCurrentConfigFiles();

    for (const QString& file : configFiles) {
        QString appWidgetName = getAppWidgetNameByFile(file);
        files.insert(file, appWidgetName);
    }

    return files;
}

QStringList AppWidgetConfiguration::getCurrentConfigFiles() const
{
    QDir dirinfo = QDir(QString(CONFIG_FILE_PATH));
    if (!dirinfo.exists()) {
        return QStringList();
    }

    QStringList filename = {"*.conf"};
    QStringList configFiles = dirinfo.entryList(filename, QDir::Files, QDir::Name);

    return configFiles;
}

QString AppWidgetConfiguration::getAppWidgetNameByFile(const QString& configFile) const
{
    QString path = QString(CONFIG_FILE_PATH) + QString("%1").arg(configFile);
    QSettings setting(path, QSettings::IniFormat);

    setting.beginGroup("AppWidget");
    QString appWidgetName = setting.value("name").toString();
    setting.endGroup();
    setting.sync();

    return appWidgetName;

}

/* 分别获得目录下的当前文件与目录变化之前的文件，移除相同部分；
 * 若当前文件非空，则判断为文件新增；
 * 若目录变化之前的文件非空，则判断为文件删除；
 * 若文件重命名，则表现为旧名文件的删除和新名文件的新增；
 */
void AppWidgetConfiguration::onDirectoryChanged()
{
    QStringList currentFiles = getCurrentConfigFiles();
    QStringList oldCofigFiles = m_configInfo.keys();

    for (const auto& file : currentFiles) {
        if (!oldCofigFiles.contains(file)) {
            continue;
        }

        oldCofigFiles.removeOne(file);
        currentFiles.removeOne(file);
    }

    if (currentFiles.count() != 0) {
        onAppwidgetAdded(currentFiles);
    }

    if (oldCofigFiles.count() != 0) {
        onAppwidgetRemoved(oldCofigFiles);
    }

    m_configInfo = getCurrentConfigInfo();
}



void AppWidgetConfiguration::onAppwidgetAdded(const QStringList& changedFiles)
{
    for (const QString& file : changedFiles) {
        QString appWidgetName = getAppWidgetNameByFile(file);
        qDebug() << __FILE__ << __FUNCTION__ << "Appwidgt added: " << appWidgetName;
        emit appwidgetAdded(appWidgetName);
    }
}

void AppWidgetConfiguration::onAppwidgetRemoved(const QStringList& changedFiles)
{
    for (const QString& file : changedFiles) {
        QString appWidgetName = m_configInfo.value(file);
        qDebug() << __FILE__ << __FUNCTION__ << "Appwidgt removed: " << appWidgetName;
        emit appwidgetRemoved(appWidgetName);
    }
}

} // namespace AppWidget
