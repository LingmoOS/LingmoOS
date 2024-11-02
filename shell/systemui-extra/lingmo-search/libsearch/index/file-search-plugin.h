/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef FILESEARCHPLUGIN_H
#define FILESEARCHPLUGIN_H

#include <QObject>
#include <QThreadPool>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QAction>

#include "search-plugin-iface.h"
#include "common.h"
#include "action-label.h"
#include "separation-line.h"
#include "global-settings.h"
#include "icon-loader.h"

namespace LingmoUISearch {
//internal plugin
class LIBSEARCH_EXPORT FileSearchPlugin : public QObject, public SearchPluginIface
{
    Q_OBJECT
public:
    explicit FileSearchPlugin(QObject *parent = nullptr);
    PluginType pluginType() override {return PluginType::SearchPlugin;}
    const QString name() override;
    const QString description() override;
    const QIcon icon() override {return IconLoader::loadIconQt("folder");}
    void setEnable(bool enable) override {m_enable = enable;}
    bool isEnable() override {return m_enable;}
    QString getPluginName() override;

    void KeywordSearch(QString keyword,DataQueue<ResultInfo> *searchResult) override;
    void stopSearch() override;
    QList<SearchPluginIface::Actioninfo> getActioninfo(int type) override;
    void openAction(int actionkey, QString key, int type = 0) override;
//    bool isPreviewEnable(QString key, int type);
//    QWidget *previewPage(QString key, int type, QWidget *parent = nullptr);
    QWidget *detailPage(const ResultInfo &ri) override;

private:
    void initDetailPage();
    QString m_currentActionKey;
    QWidget *m_detailPage;
    QVBoxLayout *m_detailLyt = nullptr;
    QLabel *m_iconLabel = nullptr;
    QFrame *m_nameFrame = nullptr;
    QHBoxLayout *m_nameFrameLyt = nullptr;
    QLabel *m_nameLabel = nullptr;
    QLabel *m_pluginLabel = nullptr;
    SeparationLine *m_line_1 = nullptr;
    QFrame *m_pathFrame = nullptr;
    QLabel *m_pathLabel1 = nullptr;
    QLabel *m_pathLabel2 = nullptr;
    QHBoxLayout *m_pathFrameLyt = nullptr;
    QFrame *m_timeFrame = nullptr;
    QLabel *m_timeLabel1 = nullptr;
    QLabel *m_timeLabel2 = nullptr;
    QHBoxLayout *m_timeFrameLyt = nullptr;

    SeparationLine *m_line_2 = nullptr;
    QFrame *m_actionFrame = nullptr;
    QVBoxLayout *m_actionFrameLyt = nullptr;
    ActionLabel *m_actionLabel1 = nullptr;
    ActionLabel *m_actionLabel2 = nullptr;
    ActionLabel *m_actionLabel3 = nullptr;

    QVBoxLayout * m_actionLyt = nullptr;

    bool m_enable = true;
    QList<SearchPluginIface::Actioninfo> m_actionInfo;
    QThreadPool m_pool;
};

class LIBSEARCH_EXPORT DirSearchPlugin : public QObject, public SearchPluginIface
{
    Q_OBJECT
public:
    explicit DirSearchPlugin(QObject *parent = nullptr);
    PluginType pluginType() override {return PluginType::SearchPlugin;}
    const QString name() override;
    const QString description() override;
    const QIcon icon() override {return IconLoader::loadIconQt("folder");}
    void setEnable(bool enable) override {m_enable = enable;}
    bool isEnable() override {return m_enable;}
    QString getPluginName() override;

    void KeywordSearch(QString keyword,DataQueue<ResultInfo> *searchResult) override;
    void stopSearch() override;
    QList<SearchPluginIface::Actioninfo> getActioninfo(int type) override;
    void openAction(int actionkey, QString key, int type = 0) override;
//    bool isPreviewEnable(QString key, int type);
//    QWidget *previewPage(QString key, int type, QWidget *parent = nullptr);
    QWidget *detailPage(const ResultInfo &ri) override;
private:
    void initDetailPage();
    QWidget *m_detailPage = nullptr;
    QString m_currentActionKey;
    QVBoxLayout *m_detailLyt = nullptr;
    QLabel *m_iconLabel = nullptr;
    QFrame *m_nameFrame = nullptr;
    QHBoxLayout *m_nameFrameLyt = nullptr;
    QLabel *m_nameLabel = nullptr;
    QLabel *m_pluginLabel = nullptr;
    SeparationLine *m_line_1 = nullptr;
    QFrame *m_pathFrame = nullptr;
    QLabel *m_pathLabel1 = nullptr;
    QLabel *m_pathLabel2 = nullptr;
    QHBoxLayout *m_pathFrameLyt = nullptr;
    QFrame *m_timeFrame = nullptr;
    QLabel *m_timeLabel1 = nullptr;
    QLabel *m_timeLabel2 = nullptr;
    QHBoxLayout *m_timeFrameLyt = nullptr;

    SeparationLine *m_line_2 = nullptr;
    QFrame *m_actionFrame = nullptr;
    QVBoxLayout *m_actionFrameLyt = nullptr;
    ActionLabel *m_actionLabel1 = nullptr;
    ActionLabel *m_actionLabel2 = nullptr;
    ActionLabel *m_actionLabel3 = nullptr;

    QVBoxLayout * m_actionLyt = nullptr;

    bool m_enable = true;
    QList<SearchPluginIface::Actioninfo> m_actionInfo;
    QThreadPool m_pool;
};

class LIBSEARCH_EXPORT FileContentSearchPlugin : public QObject, public SearchPluginIface
{
    Q_OBJECT
public:
    explicit FileContentSearchPlugin(QObject *parent = nullptr);
    ~FileContentSearchPlugin();
    PluginType pluginType() override {return PluginType::SearchPlugin;}
    const QString name() override;
    const QString description() override;
    const QIcon icon() override {return IconLoader::loadIconQt("folder");}
    void setEnable(bool enable) override {m_enable = enable;}
    bool isEnable() override {return m_enable;}
    QString getPluginName() override;

    void KeywordSearch(QString keyword,DataQueue<ResultInfo> *searchResult) override;
    void stopSearch() override;
    QList<SearchPluginIface::Actioninfo> getActioninfo(int type) override;
    void openAction(int actionkey, QString key, int type = 0) override;
//    bool isPreviewEnable(QString key, int type);
//    QWidget *previewPage(QString key, int type, QWidget *parent = nullptr);
    QWidget *detailPage(const ResultInfo &ri) override;
    QString getHtmlText(const QString & text, const QString & keyword);
    QString wrapData(QLabel *p_label, const QString &text);
private:
    void initDetailPage();
    QWidget *m_detailPage = nullptr;
    QString m_currentActionKey;
    QVBoxLayout *m_detailLyt = nullptr;
    QLabel *m_iconLabel = nullptr;
    QFrame *m_nameFrame = nullptr;
    QHBoxLayout *m_nameFrameLyt = nullptr;
    QLabel *m_nameLabel = nullptr;
    QLabel *m_pluginLabel = nullptr;
    SeparationLine *m_line_1 = nullptr;
    QLabel *m_snippetLabel = nullptr;
    QFrame *m_pathFrame = nullptr;
    QLabel *m_pathLabel1 = nullptr;
    QLabel *m_pathLabel2 = nullptr;
    QHBoxLayout *m_pathFrameLyt = nullptr;
    QFrame *m_timeFrame = nullptr;
    QLabel *m_timeLabel1 = nullptr;
    QLabel *m_timeLabel2 = nullptr;
    QHBoxLayout *m_timeFrameLyt = nullptr;

    SeparationLine *m_line_2 = nullptr;
    QFrame *m_actionFrame = nullptr;
    QVBoxLayout *m_actionFrameLyt = nullptr;
    ActionLabel *m_actionLabel1 = nullptr;
    ActionLabel *m_actionLabel2 = nullptr;
    ActionLabel *m_actionLabel3 = nullptr;

    QVBoxLayout * m_actionLyt = nullptr;
    QString m_keyWord;
    bool m_enable = true;
    QList<SearchPluginIface::Actioninfo> m_actionInfo;
    QThreadPool m_pool;
    QThreadPool m_thumbnailPool;
};
}


#endif // FILESEARCHPLUGIN_H
