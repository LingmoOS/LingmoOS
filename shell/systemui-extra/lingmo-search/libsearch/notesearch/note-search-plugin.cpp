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
#include "note-search-plugin.h"
#include <QWidget>
#include <QLabel>
#include <QDebug>
#include "file-utils.h"
#include "chinese-segmentation.h"
#include "data-collecter.h"
using namespace LingmoUISearch;

NoteSearchPlugin::NoteSearchPlugin(QObject *parent)
{
    Q_UNUSED(parent)
    g_uniqueSymbol = 0;
    SearchPluginIface::Actioninfo open { 0, tr("Open")};
    m_actionInfo << open;
    m_pool.setMaxThreadCount(1);
    m_pool.setExpiryTimeout(1000);
    initDetailPage();
}

const QString NoteSearchPlugin::name()
{
    return "Note Search";
}

const QString NoteSearchPlugin::description()
{
    return tr("Note Search.");
}

QString NoteSearchPlugin::getPluginName()
{
    return tr("Note Search");
}

void NoteSearchPlugin::KeywordSearch(QString keyword, DataQueue<SearchPluginIface::ResultInfo> *searchResult)
{
    g_mutex.lock();
    ++g_uniqueSymbol;
    g_mutex.unlock();
    m_keyword = keyword;
    NoteSearch *ns = new NoteSearch(searchResult, keyword, g_uniqueSymbol);
    m_pool.start(ns);
}

void NoteSearchPlugin::stopSearch()
{
    g_mutex.lock();
    ++g_uniqueSymbol;
    g_mutex.unlock();
}

QList<SearchPluginIface::Actioninfo> NoteSearchPlugin::getActioninfo(int type)
{
    Q_UNUSED(type)
    return m_actionInfo;
}

void NoteSearchPlugin::openAction(int actionkey, QString key, int type)
{
    Q_UNUSED(type)
    switch (actionkey) {
    case 0:
    {
        DataCollecter::collectLaunchEvent(QStringLiteral("noteSearch"), QStringLiteral("open"));
        bool res(false);
        QDBusInterface * appLaunchInterface = new QDBusInterface(QStringLiteral("com.lingmo.ProcessManager"),
                                                                 QStringLiteral("/com/lingmo/ProcessManager/AppLauncher"),
                                                                 QStringLiteral("com.lingmo.ProcessManager.AppLauncher"),
                                                                 QDBusConnection::sessionBus());
        if(!appLaunchInterface->isValid()) {
            qWarning() << qPrintable(QDBusConnection::sessionBus().lastError().message());
            res = false;
        } else {
            appLaunchInterface->setTimeout(10000);
            QDBusReply<void> reply = appLaunchInterface->call("LaunchAppWithArguments", "/usr/share/applications/lingmo-notebook.desktop", QStringList() << "--show" << key);
            if(reply.isValid()) {
                res = true;
            } else {
                qWarning() << "ProcessManager dbus called failed!" << reply.error();
                res = false;
            }
        }
        if(appLaunchInterface) {
            delete appLaunchInterface;
        }
        appLaunchInterface = nullptr;
        if (res)
            break;
        QProcess process;
        process.startDetached("lingmo-notebook", {"--show", key});
        break;
    }
    default:
        break;
    }
}

QWidget *NoteSearchPlugin::detailPage(const SearchPluginIface::ResultInfo &ri)
{
    m_currentActionKey = ri.actionKey;
    m_iconLabel->setPixmap(ri.icon.pixmap(120, 120));
    QFontMetrics fontMetrics = m_nameLabel->fontMetrics();
    QString showname = fontMetrics.elidedText(ri.name, Qt::ElideRight, 215); //当字体长度超过215时显示为省略号
    m_nameLabel->setText(FileUtils::setAllTextBold(showname));
    if(QString::compare(showname, ri.name)) {
        m_nameLabel->setToolTip(ri.name);
    } else {
        m_nameLabel->setToolTip("");
    }
    m_pluginLabel->setText(tr("Application"));
    QString showDesc = fontMetrics.elidedText(/*ri.description.at(0).key + " " + */ri.description.at(0).value, Qt::ElideRight, m_descLabel->width() * 2); //当字体长度超过215时显示为省略号
    m_descLabel->setText(FileUtils::getHtmlText(FileUtils::wrapData(m_descLabel, showDesc), m_keyword));
    m_descFrame->show();
    m_line_2->show();
    return m_detailPage;
}

void NoteSearchPlugin::initDetailPage()
{
    m_detailPage = new QWidget();
    m_detailPage->setFixedWidth(360);
    m_detailPage->setAttribute(Qt::WA_TranslucentBackground);
    m_detailLyt = new QVBoxLayout(m_detailPage);
    m_detailLyt->setContentsMargins(8, 0, 16, 0);
    m_iconLabel = new QLabel(m_detailPage);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedHeight(128);

    m_nameFrame = new QFrame(m_detailPage);
    m_nameFrameLyt = new QHBoxLayout(m_nameFrame);
    m_nameFrame->setLayout(m_nameFrameLyt);
    m_nameFrameLyt->setContentsMargins(8, 0, 0, 0);
    m_nameLabel = new QLabel(m_nameFrame);
    m_nameLabel->setMaximumWidth(280);
    m_pluginLabel = new QLabel(m_nameFrame);
    m_pluginLabel->setText(tr("Note Search"));
    m_pluginLabel->setEnabled(false);
    m_nameFrameLyt->addWidget(m_nameLabel);
    m_nameFrameLyt->addStretch();
    m_nameFrameLyt->addWidget(m_pluginLabel);

    m_line_1 = new SeparationLine(m_detailPage);

    m_descFrame = new QFrame(m_detailPage);
    m_descFrameLyt = new QVBoxLayout(m_descFrame);
    m_descLabel = new QLabel(m_descFrame);
    m_descLabel->setTextFormat(Qt::AutoText);
    m_descLabel->setWordWrap(true);
    m_descFrameLyt->addWidget(m_descLabel);
    m_descFrame->setLayout(m_descFrameLyt);
    m_descFrameLyt->setContentsMargins(8, 0, 0, 0);

    m_line_2 = new SeparationLine(m_detailPage);

    m_actionFrame = new QFrame(m_detailPage);
    m_actionFrameLyt = new QVBoxLayout(m_actionFrame);
    m_actionFrameLyt->setContentsMargins(8, 0, 0, 0);
    m_actionLabel1 = new ActionLabel(tr("Open"), m_currentActionKey, m_actionFrame);

    m_actionFrameLyt->addWidget(m_actionLabel1);;
    m_actionFrame->setLayout(m_actionFrameLyt);

    m_detailLyt->addSpacing(50);
    m_detailLyt->addWidget(m_iconLabel);
    m_detailLyt->addWidget(m_nameFrame);
    m_detailLyt->addWidget(m_line_1);
    m_detailLyt->addWidget(m_descFrame);
    m_detailLyt->addWidget(m_line_2);
    m_detailLyt->addWidget(m_actionFrame);
    m_detailPage->setLayout(m_detailLyt);
    m_detailLyt->addStretch();

    connect(m_actionLabel1, &ActionLabel::actionTriggered, [ & ](){
        openAction(0, m_currentActionKey, 0);
    });
}


NoteSearch::NoteSearch(DataQueue<SearchPluginIface::ResultInfo> *searchResult, const QString &keyword, size_t uniqueSymbol) {
    this->setAutoDelete(true);
    this->m_searchResult = searchResult;
    this->m_keyword = keyword;
    this->m_uniqueSymbol = uniqueSymbol;
}

void NoteSearch::run() {
    std::vector<KeyWord> sKeyWordVec = ChineseSegmentation::getInstance()->callSegment(m_keyword);
    QStringList keywordList;
    for (KeyWord sKeyWord : sKeyWordVec) {
        keywordList.append(QString::fromStdString(sKeyWord.word));
    }
    QDBusInterface qi("org.lingmo.note", "/org/lingmo/note", "org.lingmo.note.interface", QDBusConnection::sessionBus());
    QDBusReply<QVariantMap> reply = qi.call("keywordMatch", keywordList);

    if(reply.isValid()) {
        if (m_uniqueSymbol ^ g_uniqueSymbol) {
            qDebug() << m_uniqueSymbol << g_uniqueSymbol;
            return;
        } else {
            qDebug() << m_uniqueSymbol << g_uniqueSymbol;

            for (std::pair<QString, QVariant> it : reply.value().toStdMap()) {
                qDebug() << it.first;
                qDebug() << it.second;
                qDebug() << it.second.value<QDBusArgument>().currentType();
                QDBusArgument dbusArgs = it.second.value<QDBusArgument>();
                QStringList str;
                dbusArgs.beginArray();
                while (!dbusArgs.atEnd()) {
                    QVariant tmp;
                    dbusArgs >> tmp;
                    str.append(tmp.toString());
                }
                dbusArgs.endArray();
                qDebug() << str;
                QString name = str.length() > 2? str.at(1) : "";
                SearchPluginIface::ResultInfo ri(
                        IconLoader::loadIconQt("lingmo-notebook", QIcon(":/res/icons/desktop.png")),
                        name.replace("\r", " ").replace("\n", " "),
                        name,
                        QStringLiteral("lingmo-note"),
                        QVector<SearchPluginIface::DescriptionInfo>() << SearchPluginIface::DescriptionInfo {
                                key : QString(tr("Note Description:")),
                                value : str.at(0)
                        },
                        it.first
                );
                if (m_uniqueSymbol ^ g_uniqueSymbol) {
                    qDebug() << m_uniqueSymbol << g_uniqueSymbol;
                    return;
                } else {
                    m_searchResult->enqueue(ri);
                }
            }
        }
    } else {
        qWarning() << "Note dbus called failed!" << qi.lastError();
    }
}

