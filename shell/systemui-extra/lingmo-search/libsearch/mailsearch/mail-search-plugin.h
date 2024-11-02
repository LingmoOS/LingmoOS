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
#ifndef MAILSEARCHPLUGIN_H
#define MAILSEARCHPLUGIN_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
//#include <QTextEdit>
//#include <QLineEdit>
#include <QTextBrowser>
//#include <QListWidget>
#include <QAction>
#include <QScrollArea>
#include <QScrollBar>
#include <QDBusInterface>
#include <QDBusReply>
#include <QThreadPool>
#include <QProcess>
#include <qt5xdg/xdgicon.h>
#include "search-plugin-iface.h"
#include "action-label.h"
#include "separation-line.h"
#include "libsearch_global.h"
namespace LingmoUISearch {


class MailDescArea : public QScrollArea {
    Q_OBJECT
public:
    MailDescArea(QWidget *parent = nullptr);
    ~MailDescArea() = default;
private:
    void initUi();
};

class LIBSEARCH_EXPORT MailSearchPlugin : public QObject , public SearchPluginIface
{
    Q_OBJECT
    friend class MailSearch;
public:
    MailSearchPlugin(QObject *parent = nullptr);
    const QString name();
    const QString description();
    PluginType pluginType() {return PluginType::SearchPlugin;}
    const QIcon icon() {return XdgIcon::fromTheme("evolution");}
    void setEnable(bool enable) {m_enable = enable;}
    bool isEnable() {return m_enable;}

    QString getPluginName();
    void KeywordSearch(QString keyword, DataQueue<ResultInfo> *searchResult);
    void stopSearch();
    QList<SearchPluginIface::Actioninfo> getActioninfo(int type);
    void openAction(int actionkey, QString key, int type);
    QWidget *detailPage(const ResultInfo &ri);
private:
    void initDetailPage();
    void resizeTextEdit(QTextEdit *textEdit = nullptr);
    bool m_enable = true;
    static size_t uniqueSymbol;
    static QMutex m_mutex;
    QList<SearchPluginIface::Actioninfo> m_actionInfo;
    QThreadPool m_pool;
    QString m_keyword;

    QString m_currentActionKey;
    QWidget *m_detailPage = nullptr;
    QVBoxLayout *m_detailLyt = nullptr;
    QLabel *m_iconLabel = nullptr;
    QFrame *m_nameFrame = nullptr;
    QHBoxLayout *m_nameFrameLyt = nullptr;
    QLabel *m_nameLabel = nullptr;
    QLabel *m_pluginLabel = nullptr;
    SeparationLine *m_line_1 = nullptr;

    MailDescArea *m_descListArea = nullptr;
    QWidget *m_descPage = nullptr;
    QFormLayout *m_descPageLyt = nullptr;
    QLabel *m_senderLabel = nullptr;
    QLabel *m_timeLabel = nullptr;
    QLabel *m_recipientsLabel = nullptr;
    QLabel *m_ccLabel = nullptr;
    QLabel *m_attachmentLabel = nullptr;
    QLabel *m_senderFieldsLabel = nullptr;
    QLabel *m_timeFieldsLabel = nullptr;
//    QListWidget *m_recipientsFieldsLabel = nullptr;
    QTextBrowser *m_recipientsFieldsLabel = nullptr;
//    QTextEdit *m_recipientsFieldsLabel = nullptr;
//    QLabel *m_recipientsFieldsLabel = nullptr;
//    QLabel *m_ccFieldsLabel = nullptr;
    QTextBrowser *m_ccFieldsLabel = nullptr;
    QLabel *m_attachmentFieldsLabel = nullptr;

    SeparationLine *m_line_2 = nullptr;
    QFrame *m_actionFrame = nullptr;
    QVBoxLayout *m_actionFrameLyt = nullptr;
    ActionLabel *m_actionLabel1 = nullptr;
    QVBoxLayout * m_actionLyt = nullptr;
private Q_SLOTS:
    void resetHeight();
};

class MailSearch : public QObject, public QRunnable {
    Q_OBJECT
public:
    MailSearch(DataQueue<SearchPluginIface::ResultInfo> *searchResult, const QString &keyword, size_t uniqueSymbol);
    ~MailSearch() = default;
protected:
    void run() override;
    void createResultInfo(SearchPluginIface::ResultInfo &ri, QStringList &resultList);
private:
    DataQueue<SearchPluginIface::ResultInfo> *m_searchResult = nullptr;
    QString m_keyword;
    size_t m_uniqueSymbol;
};
}

#endif // MAILSEARCHPLUGIN_H
