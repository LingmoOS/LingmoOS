/*
 *
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
 *
 */
#ifndef NOTESEARCHPLUGIN_H
#define NOTESEARCHPLUGIN_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QAction>
#include <QDBusInterface>
#include <QThreadPool>
#include <QProcess>
#include <QDBusReply>
#include <typeinfo>
#include "search-plugin-iface.h"
#include "action-label.h"
#include "separation-line.h"
#include "libsearch_global.h"
#include "icon-loader.h"

namespace LingmoUISearch {

static size_t g_uniqueSymbol;
static QMutex g_mutex;

class NoteSearch;

class LIBSEARCH_EXPORT NoteSearchPlugin : public QObject, public SearchPluginIface
{
    Q_OBJECT
    friend class NoteSearch;
public:
    NoteSearchPlugin(QObject *parent = nullptr);
    PluginType pluginType() {return PluginType::SearchPlugin;}
    const QString name();
    const QString description();
    const QIcon icon() {return IconLoader::loadIconQt("folder");}
    void setEnable(bool enable) {m_enable = enable;}
    bool isEnable() {return m_enable;}
    QString getPluginName();

    void KeywordSearch(QString keyword,DataQueue<ResultInfo> *searchResult);
    void stopSearch();
    QList<SearchPluginIface::Actioninfo> getActioninfo(int type);
    void openAction(int actionkey, QString key, int type);
//    bool isPreviewEnable(QString key, int type);
//    QWidget *previewPage(QString key, int type, QWidget *parent);
    QWidget *detailPage(const ResultInfo &ri);

private:
    QDBusInterface *m_interFace = nullptr;
    void initDetailPage();
    QString m_currentActionKey;
    QWidget *m_detailPage = nullptr;
    QVBoxLayout *m_detailLyt = nullptr;
    QLabel *m_iconLabel = nullptr;
    QFrame *m_nameFrame = nullptr;
    QHBoxLayout *m_nameFrameLyt = nullptr;
    QLabel *m_nameLabel = nullptr;
    QLabel *m_pluginLabel = nullptr;
    SeparationLine *m_line_1 = nullptr;
    QFrame *m_descFrame = nullptr;
    QLabel *m_descLabel = nullptr;
    QVBoxLayout *m_descFrameLyt = nullptr;
    SeparationLine *m_line_2 = nullptr;
    QFrame *m_actionFrame = nullptr;
    QVBoxLayout *m_actionFrameLyt = nullptr;
    ActionLabel *m_actionLabel1 = nullptr;
    QVBoxLayout * m_actionLyt = nullptr;

    bool m_enable = true;
    QString m_keyword;
    QList<NoteSearchPlugin::Actioninfo> m_actionInfo;
    QThreadPool m_pool;
};

class NoteSearch : public QObject, public QRunnable {
    Q_OBJECT
public:
    NoteSearch(DataQueue<SearchPluginIface::ResultInfo> *searchResult, const QString& keyword, size_t uniqueSymbol);
    ~NoteSearch() = default;
protected:
    void run() override;
private:
    QString m_keyword;
    DataQueue<SearchPluginIface::ResultInfo> *m_searchResult = nullptr;
    size_t m_uniqueSymbol;
};
}

#endif // NOTESEARCHPLUGIN_H
