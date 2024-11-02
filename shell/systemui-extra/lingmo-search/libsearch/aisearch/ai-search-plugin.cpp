/*
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: baijunjie <baijunjie@kylinos.cn>
 *
 */

#include "ai-search-plugin.h"
#include "file-utils.h"
#include "thumbnail-creator.h"
#include "file-indexer-config.h"
#include "data-collecter.h"
#include "file-utils.h"
#include "dir-watcher.h"
#include <unistd.h>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFileInfo>

using namespace LingmoUISearch;
size_t AiSearchPlugin::uniqueSymbolAi = 0;
QMutex AiSearchPlugin::mutex;
static const int THUMBNAIL_HEIGHT = 247;
static const int THUMBNAIL_WIDTH = 352;
bool AiSearch::s_failed = false;
DataManagementSession AiSearch::s_session = nullptr;
QMutex AiSearch::s_sessionMutex;

AiSearchPlugin::AiSearchPlugin(QObject *parent) : QObject(parent)
{
    SearchPluginIface::Actioninfo open { 0, tr("Open")};
    SearchPluginIface::Actioninfo OpenPath {1, tr("Open path")};
    SearchPluginIface::Actioninfo CopyPath { 2, tr("Copy Path")};
    m_actionInfo << open << OpenPath << CopyPath;
    m_pool.setMaxThreadCount(1);
    m_thumbnailPool.setMaxThreadCount(1);
    m_pool.setExpiryTimeout(1000);
    m_timer = new QTimer(this);
    m_timer->setInterval(500);
    connect(m_timer, &QTimer::timeout, this, [ = ] {
        if (FileIndexerConfig::getInstance()->isAiIndexEnable()) {
            auto aiSearch = new AiSearch(m_searchResult, uniqueSymbolAi, m_keyword);
            m_pool.start(aiSearch);
            m_timer->stop();
        }
    });
    initDetailPage();
}

const QString AiSearchPlugin::name()
{
    return "Ai Search";
}

const QString AiSearchPlugin::description()
{
    return tr("AI Search");
}

QString AiSearchPlugin::getPluginName()
{
    return tr("AI Search");
}

void AiSearchPlugin::KeywordSearch(QString keyword, DataQueue<ResultInfo> *searchResult)
{
    mutex.lock();
    uniqueSymbolAi++;
    mutex.unlock();
    m_keyword = keyword;
    m_searchResult = searchResult;
    m_timer->start();
}

void AiSearchPlugin::stopSearch()
{
    mutex.lock();
    ++uniqueSymbolAi;
    mutex.unlock();
    m_thumbnailPool.clear();
    m_pool.clear();
    m_timer->stop();
}

QList<SearchPluginIface::Actioninfo> AiSearchPlugin::getActioninfo(int type)
{
    Q_UNUSED(type)
    return m_actionInfo;
}

void AiSearchPlugin::openAction(int actionkey, QString key, int type)
{
    Q_UNUSED(type)
    //TODO add some return message here.
    switch (actionkey) {
        case 0:
            DataCollecter::collectLaunchEvent(QStringLiteral("AISearch"), QStringLiteral("openFile"));
            if(FileUtils::openFile(key) == -1) {
                QMessageBox msgBox(m_detailPage);
                msgBox.setModal(true);
                msgBox.addButton(tr("OK"), QMessageBox::YesRole);
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setText(tr("Can not get a default application for opening %1.").arg(key));
                msgBox.exec();
            }
            break;
        case 1:
            DataCollecter::collectLaunchEvent(QStringLiteral("AISearch"), QStringLiteral("openFileInDir"));
            FileUtils::openFile(key, true);
            break;
        case 2:
            DataCollecter::collectLaunchEvent(QStringLiteral("AISearch"), QStringLiteral("copyPath"));
            FileUtils::copyPath(key);
        default:
            break;
    }
}

QWidget *AiSearchPlugin::detailPage(const SearchPluginIface::ResultInfo &ri)
{
    if(ri.actionKey == m_currentActionKey) {
        return m_detailPage;
    }
    auto creator = new ThumbnailCreator(ri.actionKey, QSize(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT), m_detailPage->window()->devicePixelRatio());
    connect(creator, &ThumbnailCreator::ready, this, [&, ri](const QString& uri, const QImage &image){
        if(uri != m_currentActionKey) {
            return;
        }
        QPixmap pixmap = QPixmap::fromImage(image);
        if(!pixmap.isNull()) {
            m_iconLabel->setPixmap(pixmap);
            m_detailLyt->setContentsMargins(8, (THUMBNAIL_HEIGHT - pixmap.height()) / 2 + 8, 16, 0);
        } else {
            m_iconLabel->setPixmap(ri.icon.pixmap(120, 120));
        }
    }, Qt::QueuedConnection);
    m_thumbnailPool.start(creator);
    m_iconLabel->setPixmap(ri.icon.pixmap(120, 120));
    m_detailLyt->setContentsMargins(8, 50, 16, 0);
    m_currentActionKey = ri.actionKey;

    QFontMetrics fontMetrics = m_nameLabel->fontMetrics();
    QString showName = fontMetrics.elidedText(ri.name, Qt::ElideRight, 215); //当字体长度超过215时显示为省略号
    m_nameLabel->setText(FileUtils::setAllTextBold(showName));
    if(QString::compare(showName, ri.name)) {
        m_nameLabel->setToolTip(ri.name);
    } else {
        m_nameLabel->setToolTip("");
    }
    m_pluginLabel->setText(tr("File"));

    m_pathLabel2->setText(m_pathLabel2->fontMetrics().elidedText(m_currentActionKey, Qt::ElideRight, m_pathLabel2->width()));
    m_pathLabel2->setToolTip(m_currentActionKey);
    QString modTimeText = ri.description.at(1).value;
    m_timeLabel2->setText(modTimeText);
    return m_detailPage;
}

void AiSearchPlugin::initDetailPage()
{
    m_detailPage = new QWidget();
    m_detailPage->setFixedWidth(360);
    m_detailPage->setAttribute(Qt::WA_TranslucentBackground);
    m_detailLyt = new QVBoxLayout(m_detailPage);
    m_detailLyt->setContentsMargins(8, 0, 16, 0);
    m_iconLabel = new QLabel(m_detailPage);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setMinimumHeight(120);

    m_nameFrame = new QFrame(m_detailPage);
    m_nameFrameLyt = new QHBoxLayout(m_nameFrame);
    m_nameFrame->setLayout(m_nameFrameLyt);
    m_nameFrameLyt->setContentsMargins(8, 0, 0, 0);
    m_nameLabel = new QLabel(m_nameFrame);
    m_nameLabel->setMaximumWidth(280);
    m_pluginLabel = new QLabel(m_nameFrame);
    m_pluginLabel->setEnabled(false);
    m_nameFrameLyt->addWidget(m_nameLabel);
    m_nameFrameLyt->addStretch();
    m_nameFrameLyt->addWidget(m_pluginLabel);

    m_line_1 = new SeparationLine(m_detailPage);

    m_pathFrame = new QFrame(m_detailPage);
    m_pathFrameLyt = new QHBoxLayout(m_pathFrame);
    m_pathLabel1 = new QLabel(m_pathFrame);
    m_pathLabel2 = new QLabel(m_pathFrame);
    m_pathLabel1->setText(tr("Path"));
    m_pathLabel2->setFixedWidth(240);
    m_pathLabel2->setAlignment(Qt::AlignRight);
    m_pathFrameLyt->addWidget(m_pathLabel1);
    m_pathFrameLyt->addStretch();
    m_pathFrameLyt->addWidget(m_pathLabel2);

    m_timeFrame = new QFrame(m_detailPage);
    m_timeFrameLyt = new QHBoxLayout(m_timeFrame);
    m_timeLabel1 = new QLabel(m_timeFrame);
    m_timeLabel2 = new QLabel(m_timeFrame);
    m_timeLabel2->setAlignment(Qt::AlignRight);
    m_timeLabel1->setText(tr("Last time modified"));
    m_timeFrameLyt->addWidget(m_timeLabel1);
    m_timeFrameLyt->addStretch();
    m_timeFrameLyt->addWidget(m_timeLabel2);

    m_line_2 = new SeparationLine(m_detailPage);

    m_actionFrame = new QFrame(m_detailPage);
    m_actionFrameLyt = new QVBoxLayout(m_actionFrame);
    m_actionFrameLyt->setContentsMargins(8, 0, 0, 0);
    m_actionLabel1 = new ActionLabel(tr("Open"), m_currentActionKey, m_actionFrame);
    m_actionLabel2 = new ActionLabel(tr("Open path"), m_currentActionKey, m_actionFrame);
    m_actionLabel3 = new ActionLabel(tr("Copy path"), m_currentActionKey, m_actionFrame);

    m_actionFrameLyt->addWidget(m_actionLabel1);
    m_actionFrameLyt->addWidget(m_actionLabel2);
    m_actionFrameLyt->addWidget(m_actionLabel3);
    m_actionFrame->setLayout(m_actionFrameLyt);

    m_detailLyt->addWidget(m_iconLabel);
    m_detailLyt->addWidget(m_nameFrame);
    m_detailLyt->addWidget(m_line_1);
    m_detailLyt->addWidget(m_pathFrame);
    m_detailLyt->addWidget(m_timeFrame);
    m_detailLyt->addWidget(m_line_2);
    m_detailLyt->addWidget(m_actionFrame);
    m_detailPage->setLayout(m_detailLyt);
    m_detailLyt->addStretch();

    connect(m_actionLabel1, &ActionLabel::actionTriggered, [ & ](){
        DataCollecter::collectLaunchEvent(QStringLiteral("AISearch"), QStringLiteral("openFile"));
        if(FileUtils::openFile(m_currentActionKey) == -1) {
            QMessageBox msgBox(m_detailPage);
            msgBox.setModal(true);
            msgBox.addButton(tr("OK"), QMessageBox::YesRole);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(tr("Can not get a default application for opening %1.").arg(m_currentActionKey));
            msgBox.exec();
        }
    });
    connect(m_actionLabel2, &ActionLabel::actionTriggered, [ & ](){
        DataCollecter::collectLaunchEvent(QStringLiteral("AISearch"), QStringLiteral("openFileInDir"));
        FileUtils::openFile(m_currentActionKey, true);
    });
    connect(m_actionLabel3, &ActionLabel::actionTriggered, [ & ](){
        DataCollecter::collectLaunchEvent(QStringLiteral("AISearch"), QStringLiteral("copyPath"));
        FileUtils::copyPath(m_currentActionKey);
    });
}

AiSearchPlugin::~AiSearchPlugin()
{
    m_pool.clear();
    m_thumbnailPool.clear();
    m_thumbnailPool.waitForDone();
    m_pool.waitForDone();
    AiSearch::destroySession();
}

AiSearch::AiSearch(DataQueue<SearchPluginIface::ResultInfo> *searchResult, size_t uniqueSymbol, const QString &keyword)
{
    m_searchResult = searchResult;
    m_uniqueSymbol = uniqueSymbol;
    m_keyword = keyword;
}

void AiSearch::run()
{
    this->keywordSearch();
}

void AiSearch::keywordSearch()
{
    if (!AiSearch::createSession()) {
        return;
    }
    QJsonObject keyword;
    keyword.insert(QStringLiteral("text"), m_keyword);
    char* results = nullptr;
    DataManagementResult re = data_management_similarity_search(s_session, QJsonDocument(keyword).toJson().data(), &results);
    if(re == DATA_MANAGEMENT_SESSION_ERROR) {
        qWarning() << "AiSearch:DATA_MANAGEMENT_SESSION_ERROR occured, re Creating session";
        destroySession();
        if(!createSession()) {
            qWarning() << "AiSearch:Failed to create session";
            data_management_free_search_result(s_session, results);
            return;
        }
        qDebug() << "Creating session success.";
        data_management_similarity_search(s_session, QJsonDocument(keyword).toJson().data(), &results);
    }

    for (const auto &aResult : QJsonDocument::fromJson(results).array()) {
        if (aResult.isObject()) {
            bool isBlocked(false);
            QStringList blockList = DirWatcher::getDirWatcher()->getBlockDirsOfUser();
            QString filePath = aResult.toObject().value("filepath").toString();
            for (const auto& blockDir : blockList) {
                if (FileUtils::isOrUnder(filePath, blockDir)) {
                    isBlocked = true;
                    break;
                }
            }

            if (isBlocked) {
                continue;
            }

            SearchPluginIface::ResultInfo ri;
            if (createResultInfo(ri, aResult.toObject().value("filepath").toString())) {
                AiSearchPlugin::mutex.lock();
                if (m_uniqueSymbol == AiSearchPlugin::uniqueSymbolAi) {
                    m_searchResult->enqueue(ri);
                    AiSearchPlugin::mutex.unlock();
                } else {
                    AiSearchPlugin::mutex.unlock();
                    break;
                }
            }
        }
    }
    data_management_free_search_result(s_session, results);
}

bool AiSearch::createResultInfo(SearchPluginIface::ResultInfo &ri, const QString &path)
{
    QFileInfo info(path);
    if(!info.exists()) {
        return false;
    }
    ri.icon = FileUtils::getFileIcon(QUrl::fromLocalFile(path).toString(), false);
    ri.name = info.fileName().replace("\r", " ").replace("\n", " ");
    ri.toolTip = info.fileName();
    ri.resourceType = QStringLiteral("file");
    ri.description = QVector<SearchPluginIface::DescriptionInfo>() \
                    << SearchPluginIface::DescriptionInfo{"Path:", path} \
                    << SearchPluginIface::DescriptionInfo{"Modified time:", info.lastModified().toString("yyyy/MM/dd hh:mm:ss")};
    ri.actionKey = path;
    return true;
}

AiSearch::~AiSearch() {}

bool AiSearch::createSession() {
    QMutexLocker locker(&s_sessionMutex);
    if (!s_session || s_failed) {
        if (data_management_create_session(&s_session, DataManagementType::SYS_SEARCH, getuid()) != DataManagementResult::DATA_MANAGEMENT_SUCCESS) {
            s_failed = true;
            qWarning() << "===Create data management session failed!===" << s_session;
            return false;
        }
    }
    return true;
}

void AiSearch::destroySession() {
    QMutexLocker locker(&s_sessionMutex);
    if (data_management_destroy_session(s_session) != DataManagementResult::DATA_MANAGEMENT_SUCCESS) {
        qWarning() << "===fail to destroy session===";
    }
    s_session = nullptr;
}
