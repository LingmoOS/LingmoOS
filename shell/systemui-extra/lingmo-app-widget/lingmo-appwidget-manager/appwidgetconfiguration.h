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

#ifndef APPWIDGETCONFIGURATION_H
#define APPWIDGETCONFIGURATION_H

#include <QObject>
#include <QMap>

class QFileSystemWatcher;
namespace AppWidget{
class AppWidgetConfiguration : public QObject
{
    Q_OBJECT
public:
    explicit AppWidgetConfiguration(QObject *parent = nullptr);
    ~AppWidgetConfiguration() = default;
    QString getProviderNameByAppWidgetName(const QString &appwidgetname);
    //获取ui文件
    QString getAppWidgetUiFile(const QString &appwidgetname);
    //获取属性
    QString getAppWidgetConfig(const QString &appwidgetname, const QString &config);
    //获取所有属性
    QMap<QString, QVariant> getAppWidgetConfigs(const QString &appwidgetname, QString begianName);
    //各个属性的获取函数
    QString getConfigurationByKey(const QString &path, const QString &key);

signals:
    void appwidgetAdded(QString name);
    void appwidgetRemoved(QString name);

private slots:
    void onDirectoryChanged();

private:
    void initFileWatcher();
    QMap<QString, QString> getCurrentConfigInfo() const;
    QStringList getCurrentConfigFiles() const;
    QString getAppWidgetNameByFile(const QString& configFile) const;

    void onAppwidgetAdded(const QStringList& changedFiles);
    void onAppwidgetRemoved(const QStringList& changedFiles);

    //! @keyword config filename, @value AppWidget name
    QMap<QString, QString> m_configInfo;
};
} // namespace AppWidget
#endif // APPWIDGETCONFIGURATION_H
