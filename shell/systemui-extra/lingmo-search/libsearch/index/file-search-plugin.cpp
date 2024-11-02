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
#include "file-search-plugin.h"
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDBusInterface>
#include <QMutexLocker>
#include "search-manager.h"
#include "file-indexer-config.h"
#include "libkydate.h"
#include "data-collecter.h"
#include "file-reader.h"
#include "thumbnail-creator.h"

#define OCR_ICONLABLE_WITH 352
#define OCR_ICONLABLE_HEIGHT 247
using namespace LingmoUISearch;

static QMutex g_formatMutex;
static const QString DEFAULT_DATETIME_FORMAT = "yyyy/MM/dd hh:mm:ss";

class DateFormat : public QObject
{
    Q_OBJECT

public:
    static DateFormat& getInstance() {
        static DateFormat instance;
        return instance;
    }

    QString format() {
        QMutexLocker locker(&g_formatMutex);
        if (m_dateFormat == DEFAULT_DATETIME_FORMAT) {
            return QString();
        }
        return m_dateFormat;
    }

public Q_SLOTS:
    void updateDateFormat(QString format) {
        QMutexLocker locker(&g_formatMutex);
        m_dateFormat = format + " hh:mm:ss";
    }
private:
    explicit DateFormat(QObject *parent = nullptr) : QObject(parent)
    {
        char* shortFormat = kdk_system_get_shortformat();
        m_dateFormat = shortFormat;
        free(shortFormat);
        if (m_dateFormat.isEmpty()) {
            m_dateFormat = DEFAULT_DATETIME_FORMAT;
        } else {
            m_dateFormat += " hh:mm:ss";
        }

        qDebug() << "connect sdk date signal" << QDBusConnection::sessionBus().connect("com.lingmo.lingmosdk.DateServer",
                                                                                       "/com/lingmo/lingmosdk/Date",
                                                                                       "com.lingmo.lingmosdk.DateInterface",
                                                                                       "ShortDateSignal",
                                                                                       this, SLOT(updateDateFormat(QString)));
    }
    DateFormat(const DateFormat&) = delete;
    DateFormat& operator = (const DateFormat&) = delete;
    ~DateFormat() = default;

    QString m_dateFormat;
};

FileSearchPlugin::FileSearchPlugin(QObject *parent) : QObject(parent)
{
    SearchPluginIface::Actioninfo open { 0, tr("Open")};
    SearchPluginIface::Actioninfo Openpath { 1, tr("Open path")};
    SearchPluginIface::Actioninfo CopyPath { 2, tr("Copy Path")};
    m_actionInfo << open << Openpath << CopyPath;
    m_pool.setMaxThreadCount(1);
    m_pool.setExpiryTimeout(1000);
    initDetailPage();
}

const QString FileSearchPlugin::name()
{
    return "File Search";
}

const QString FileSearchPlugin::description()
{
    return tr("File search.");
}

QString FileSearchPlugin::getPluginName()
{
    return tr("File Search");
}

void LingmoUISearch::FileSearchPlugin::KeywordSearch(QString keyword, DataQueue<ResultInfo> *searchResult)
{
    SearchManager::m_mutexFile.lock();
    ++SearchManager::uniqueSymbolFile;
    SearchManager::m_mutexFile.unlock();

    if(FileIndexerConfig::getInstance()->isFileIndexEnable()) {
        FileSearch *filesearch;
        filesearch = new FileSearch(searchResult, SearchManager::uniqueSymbolFile, keyword, FILE_SEARCH_VALUE, 1, 0, 5);
        m_pool.start(filesearch);
    } else{
        DirectSearch *directSearch;
        directSearch = new DirectSearch(keyword, searchResult, FILE_SEARCH_VALUE, SearchManager::uniqueSymbolFile);
        m_pool.start(directSearch);
    }
}

void FileSearchPlugin::stopSearch()
{
    SearchManager::m_mutexFile.lock();
    ++SearchManager::uniqueSymbolFile;
    SearchManager::m_mutexFile.unlock();
}

QList<SearchPluginIface::Actioninfo> FileSearchPlugin::getActioninfo(int type)
{
    Q_UNUSED(type)
    return m_actionInfo;
}

void FileSearchPlugin::openAction(int actionkey, QString key, int type)
{
    Q_UNUSED(type)
    //TODO add some return message here.
    switch (actionkey) {
        case 0:
            DataCollecter::collectLaunchEvent(QStringLiteral("fileSearch"), QStringLiteral("openFile"));
            if(FileUtils::openFile(key) == -1) {
                QMessageBox msgBox(m_detailPage);
                msgBox.setModal(true);
                msgBox.setWindowTitle(tr("lingmo-search"));
                msgBox.addButton(tr("OK"), QMessageBox::YesRole);
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setText(tr("Can not get a default application for opening %1.").arg(key));
                msgBox.exec();
            }
            break;
        case 1:
            DataCollecter::collectLaunchEvent(QStringLiteral("fileSearch"), QStringLiteral("openFileInDir"));
            FileUtils::openFile(key, true);
            break;
        case 2:
            DataCollecter::collectLaunchEvent(QStringLiteral("fileSearch"), QStringLiteral("copyPath"));
            FileUtils::copyPath(key);
        default:
            break;
    }
}

QWidget *FileSearchPlugin::detailPage(const ResultInfo &ri)
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
    m_pluginLabel->setText(tr("File"));

    m_pathLabel2->setText(m_pathLabel2->fontMetrics().elidedText(m_currentActionKey, Qt::ElideRight, m_pathLabel2->width()));
    m_pathLabel2->setToolTip(m_currentActionKey);
    QString modTimeText = ri.description.at(1).value;
    if (!DateFormat::getInstance().format().isEmpty()) {
        QDateTime modTime = QDateTime::fromString(modTimeText, DEFAULT_DATETIME_FORMAT);
        modTimeText = modTime.toString(DateFormat::getInstance().format());
    }
    m_timeLabel2->setText(modTimeText);
    return m_detailPage;
}

void FileSearchPlugin::initDetailPage()
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

    m_detailLyt->addSpacing(50);
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
        DataCollecter::collectLaunchEvent(QStringLiteral("fileSearch"), QStringLiteral("openFile"));
        if(FileUtils::openFile(m_currentActionKey) == -1) {
            QMessageBox msgBox(m_detailPage);
            msgBox.setModal(true);
            msgBox.setWindowTitle(tr("lingmo-search"));
            msgBox.addButton(tr("OK"), QMessageBox::YesRole);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText(tr("Can not get a default application for opening %1.").arg(m_currentActionKey));
            msgBox.exec();
        }
    });
    connect(m_actionLabel2, &ActionLabel::actionTriggered, [ & ](){
        DataCollecter::collectLaunchEvent(QStringLiteral("fileSearch"), QStringLiteral("openFileInDir"));
        FileUtils::openFile(m_currentActionKey, true);
    });
    connect(m_actionLabel3, &ActionLabel::actionTriggered, [ & ](){
        DataCollecter::collectLaunchEvent(QStringLiteral("fileSearch"), QStringLiteral("copyPath"));
        FileUtils::copyPath(m_currentActionKey);
    });
}

//bool FileSearchPlugin::isPreviewEnable(QString key, int type)
//{
//    return true;
//}

//QWidget *FileSearchPlugin::previewPage(QString key, int type, QWidget *parent)
//{
//    QWidget *previewPage = new QWidget(parent);
//    QHBoxLayout * previewLyt = new QHBoxLayout(previewPage);
//    previewLyt->setContentsMargins(0, 0, 0, 0);
//    QLabel *label = new QLabel(previewPage);
//    previewLyt->addWidget(label);
//    label->setFixedHeight(120);
//    previewPage->setFixedSize(120,120);
//    previewLyt->setAlignment(Qt::AlignCenter);
//    label->setPixmap(FileUtils::getFileIcon(QUrl::fromLocalFile(key).toString()).pixmap(120,120));
//    return previewPage;
//}

DirSearchPlugin::DirSearchPlugin(QObject *parent) : QObject(parent)
{
    SearchPluginIface::Actioninfo open { 0, tr("Open")};
    SearchPluginIface::Actioninfo Openpath { 1, tr("Open path")};
    SearchPluginIface::Actioninfo CopyPath { 2, tr("Copy Path")};
    m_actionInfo << open << Openpath << CopyPath;
    m_pool.setMaxThreadCount(1);
    m_pool.setExpiryTimeout(1000);
    initDetailPage();
}

const QString DirSearchPlugin::name()
{
    return "Dir Search";
}

const QString DirSearchPlugin::description()
{
    return tr("Dir search.");
}

QString DirSearchPlugin::getPluginName()
{
    return tr("Dir Search");
}

void LingmoUISearch::DirSearchPlugin::KeywordSearch(QString keyword, DataQueue<ResultInfo> *searchResult)
{
    SearchManager::m_mutexDir.lock();
    ++SearchManager::uniqueSymbolDir;
    SearchManager::m_mutexDir.unlock();

    if(FileIndexerConfig::getInstance()->isFileIndexEnable()) {
        FileSearch *filesearch;
        filesearch = new FileSearch(searchResult, SearchManager::uniqueSymbolDir, keyword, DIR_SEARCH_VALUE, 1, 0, 5);
        m_pool.start(filesearch);
    } else {
        DirectSearch *directSearch;
        directSearch = new DirectSearch(keyword, searchResult, DIR_SEARCH_VALUE, SearchManager::uniqueSymbolDir);
        m_pool.start(directSearch);
    }
}

void DirSearchPlugin::stopSearch()
{
    SearchManager::m_mutexDir.lock();
    ++SearchManager::uniqueSymbolDir;
    SearchManager::m_mutexDir.unlock();
}

QList<SearchPluginIface::Actioninfo> DirSearchPlugin::getActioninfo(int type)
{
    Q_UNUSED(type)
    return m_actionInfo;
}

void DirSearchPlugin::openAction(int actionkey, QString key, int type)
{
    Q_UNUSED(type)
    //TODO add some return message here.
    switch (actionkey) {
        case 0:
            DataCollecter::collectLaunchEvent(QStringLiteral("dirSearch"), QStringLiteral("openFile"));
            FileUtils::openFile(key);
            break;
        case 1:
            DataCollecter::collectLaunchEvent(QStringLiteral("dirSearch"), QStringLiteral("openFileInDir"));
            FileUtils::openFile(key, true);
            break;
        case 2:
            DataCollecter::collectLaunchEvent(QStringLiteral("dirSearch"), QStringLiteral("copyPath"));
            FileUtils::copyPath(key);
        default:
            break;
    }
}

QWidget *DirSearchPlugin::detailPage(const ResultInfo &ri)
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
    m_pluginLabel->setText(tr("Directory"));

    m_pathLabel2->setText(m_pathLabel2->fontMetrics().elidedText(m_currentActionKey, Qt::ElideRight, m_pathLabel2->width()));
    m_pathLabel2->setToolTip(m_currentActionKey);
    QString modTimeText = ri.description.at(1).value;
    if (!DateFormat::getInstance().format().isEmpty()) {
        QDateTime modTime = QDateTime::fromString(modTimeText, DEFAULT_DATETIME_FORMAT);
        modTimeText = modTime.toString(DateFormat::getInstance().format());
    }
    m_timeLabel2->setText(modTimeText);
    return m_detailPage;
}

void DirSearchPlugin::initDetailPage()
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

    m_detailLyt->addSpacing(50);
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
        DataCollecter::collectLaunchEvent(QStringLiteral("dirSearch"), QStringLiteral("openFile"));
        FileUtils::openFile(m_currentActionKey);
    });
    connect(m_actionLabel2, &ActionLabel::actionTriggered, [ & ](){
        DataCollecter::collectLaunchEvent(QStringLiteral("dirSearch"), QStringLiteral("openFileInDir"));
        FileUtils::openFile(m_currentActionKey, true);
    });
    connect(m_actionLabel3, &ActionLabel::actionTriggered, [ & ](){
        DataCollecter::collectLaunchEvent(QStringLiteral("dirSearch"), QStringLiteral("copyPath"));
        FileUtils::copyPath(m_currentActionKey);
    });
}

//bool DirSearchPlugin::isPreviewEnable(QString key, int type)
//{
//    return false;
//}

//QWidget *DirSearchPlugin::previewPage(QString key, int type, QWidget *parent)
//{
//    return nullptr;
//}

FileContentSearchPlugin::FileContentSearchPlugin(QObject *parent) : QObject(parent)
{
    SearchPluginIface::Actioninfo open { 0, tr("Open")};
    SearchPluginIface::Actioninfo Openpath { 1, tr("Open path")};
    SearchPluginIface::Actioninfo CopyPath { 2, tr("Copy Path")};
    m_actionInfo << open << Openpath << CopyPath;
    m_pool.setMaxThreadCount(1);
    m_thumbnailPool.setMaxThreadCount(1);
    m_pool.setExpiryTimeout(1000);
    initDetailPage();
}

FileContentSearchPlugin::~FileContentSearchPlugin()
{
    m_pool.clear();
    m_thumbnailPool.clear();
    m_thumbnailPool.waitForDone();
    m_pool.waitForDone();
}

const QString FileContentSearchPlugin::name()
{
    return "File Content Search";
}

const QString FileContentSearchPlugin::description()
{
    return tr("File content search.");
}

QString FileContentSearchPlugin::getPluginName()
{
    return tr("File content search");
}

void LingmoUISearch::FileContentSearchPlugin::KeywordSearch(QString keyword, DataQueue<ResultInfo> *searchResult)
{
    SearchManager::m_mutexContent.lock();
    ++SearchManager::uniqueSymbolContent;
    SearchManager::m_mutexContent.unlock();

    m_keyWord = keyword;
    if(FileIndexerConfig::getInstance()->isContentIndexEnable()) {
        FileContentSearch *fileContentSearch;
        fileContentSearch = new FileContentSearch(searchResult, SearchManager::uniqueSymbolContent, keyword, FileIndexerConfig::getInstance()->isFuzzySearchEnable(), 0, 5);
        m_pool.start(fileContentSearch);
    }
}

void FileContentSearchPlugin::stopSearch()
{
    SearchManager::m_mutexContent.lock();
    ++SearchManager::uniqueSymbolContent;
    SearchManager::m_mutexContent.unlock();
    m_thumbnailPool.clear();
    m_pool.clear();
}

QList<SearchPluginIface::Actioninfo> FileContentSearchPlugin::getActioninfo(int type)
{
    Q_UNUSED(type)
    return m_actionInfo;
}

void FileContentSearchPlugin::openAction(int actionkey, QString key, int type)
{
    Q_UNUSED(type)
    //TODO add some return message here.
    switch (actionkey) {
    case 0:
        DataCollecter::collectLaunchEvent(QStringLiteral("fileContentSearch"), QStringLiteral("openFile"));
        FileUtils::openFile(key);
        break;
    case 1:
        DataCollecter::collectLaunchEvent(QStringLiteral("fileContentSearch"), QStringLiteral("openFileInDir"));
        FileUtils::openFile(key, true);
        break;
    case 2:
        DataCollecter::collectLaunchEvent(QStringLiteral("fileContentSearch"), QStringLiteral("copyPath"));
        FileUtils::copyPath(key);
    default:
        break;
    }
}

QWidget *FileContentSearchPlugin::detailPage(const ResultInfo &ri)
{
    //因为需要更新时间格式所以每次调用detailpage需要先设置时间文本 20230916@bjj
    QString modTimeText = ri.description.at(2).value;
    if (!DateFormat::getInstance().format().isEmpty()) {
        QDateTime modTime = QDateTime::fromString(modTimeText, DEFAULT_DATETIME_FORMAT);
        modTimeText = modTime.toString(DateFormat::getInstance().format());
    }
    m_timeLabel2->setText(modTimeText);

    if(ri.actionKey == m_currentActionKey) {
        return m_detailPage;
    }

    if (1 == ri.type) {
        auto creator = new ThumbnailCreator(ri.actionKey, QSize(OCR_ICONLABLE_WITH, OCR_ICONLABLE_HEIGHT), m_detailPage->window()->devicePixelRatio());
        connect(creator, &ThumbnailCreator::ready, this, [&, ri](const QString& uri, const QImage &image){
            if(uri != m_currentActionKey) {
                return;
            }
            QPixmap pixmap = QPixmap::fromImage(image);
            if(!pixmap.isNull()) {
                m_iconLabel->setPixmap(pixmap);
                m_detailLyt->setContentsMargins(8, (OCR_ICONLABLE_HEIGHT-pixmap.height())/2 + 8, 16, 0);
            } else {
                m_iconLabel->setPixmap(ri.icon.pixmap(120, 120));
            }
        }, Qt::QueuedConnection);
        m_thumbnailPool.start(creator);
        m_iconLabel->setPixmap(ri.icon.pixmap(120, 120));
        m_detailLyt->setContentsMargins(8, 50, 16, 0);
        m_pluginLabel->setText(tr("OCR"));
        m_snippetLabel->hide();
    } else {
        m_iconLabel->setPixmap(ri.icon.pixmap(120, 120));
        m_pluginLabel->setText(tr("File"));
        m_snippetLabel->setText(ri.description.at(0).value);
        m_snippetLabel->show();
        m_detailLyt->setContentsMargins(8, 50, 16, 0);
    }
    m_currentActionKey = ri.actionKey;

    QFontMetrics fontMetrics = m_nameLabel->fontMetrics();
    QString showname = fontMetrics.elidedText(ri.name, Qt::ElideRight, 215); //当字体长度超过215时显示为省略号
    m_nameLabel->setText(FileUtils::setAllTextBold(showname));
    if(QString::compare(showname, ri.name)) {
        m_nameLabel->setToolTip(ri.name);
    } else {
        m_nameLabel->setToolTip("");
    }

    m_pathLabel2->setText(m_pathLabel2->fontMetrics().elidedText(m_currentActionKey, Qt::ElideRight, m_pathLabel2->width()));
    m_pathLabel2->setToolTip(m_currentActionKey);
    return m_detailPage;
}

QString FileContentSearchPlugin::getHtmlText(const QString &text, const QString &keyword)
{
    QString htmlString;
    bool boldOpenned = false;
    for(int i = 0; i < text.length(); i++) {
        if((keyword.toUpper()).contains(QString(text.at(i)).toUpper())) {
            if(! boldOpenned) {
                boldOpenned = true;
                htmlString.append(QString("<b><font size=\"4\">"));
            }
            htmlString.append(FileUtils::escapeHtml(QString(text.at(i))));
        } else {
            if(boldOpenned) {
                boldOpenned = false;
                htmlString.append(QString("</font></b>"));
            }
            htmlString.append(FileUtils::escapeHtml(QString(text.at(i))));
        }
    }
    htmlString.replace("\n", "<br />");//替换换行符
    return "<pre>" + htmlString + "</pre>";
}

QString FileContentSearchPlugin::wrapData(QLabel *p_label, const QString &text)
{
    QString wrapText = text;

    QFontMetrics fontMetrics = p_label->fontMetrics();
    int textSize = fontMetrics.horizontalAdvance(wrapText);
    if(textSize > LABEL_MAX_WIDTH){
        int lastIndex = 0;
        int count = 0;

        for(int i = lastIndex; i < wrapText.length(); i++) {

            if(fontMetrics.horizontalAdvance(wrapText.mid(lastIndex, i - lastIndex)) == LABEL_MAX_WIDTH) {
                lastIndex = i;
                wrapText.insert(i, '\n');
                count++;
            } else if(fontMetrics.horizontalAdvance(wrapText.mid(lastIndex, i - lastIndex)) > LABEL_MAX_WIDTH) {
                lastIndex = i;
                wrapText.insert(i - 1, '\n');
                count++;
            } else {
                continue;
            }

            if(count == 2){
                break;
            }
        }
    }
    p_label->setText(wrapText);
    return wrapText;
}

void FileContentSearchPlugin::initDetailPage()
{
    m_detailPage = new QWidget();
    m_detailPage->setFixedWidth(360);
    m_detailPage->setAttribute(Qt::WA_TranslucentBackground);
    m_detailLyt = new QVBoxLayout(m_detailPage);
    m_detailLyt->setContentsMargins(8, 50, 16, 0);
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

    m_snippetLabel = new QLabel(m_detailPage);
//    m_snippetLabel->setWordWrap(true);
    m_snippetLabel->setContentsMargins(8, 0, 8, 0);

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

//    m_detailLyt->addSpacing(50);
    m_detailLyt->addWidget(m_iconLabel);
    m_detailLyt->addWidget(m_nameFrame);
    m_detailLyt->addWidget(m_line_1);
    m_detailLyt->addWidget(m_snippetLabel);
    m_detailLyt->addWidget(m_pathFrame);
    m_detailLyt->addWidget(m_timeFrame);
    m_detailLyt->addWidget(m_line_2);
    m_detailLyt->addWidget(m_actionFrame);
    m_detailPage->setLayout(m_detailLyt);
    m_detailLyt->addStretch();

    connect(m_actionLabel1, &ActionLabel::actionTriggered, [ & ](){
        DataCollecter::collectLaunchEvent(QStringLiteral("fileContentSearch"), QStringLiteral("openFile"));
        FileUtils::openFile(m_currentActionKey);
    });
    connect(m_actionLabel2, &ActionLabel::actionTriggered, [ & ](){
        DataCollecter::collectLaunchEvent(QStringLiteral("fileContentSearch"), QStringLiteral("openFileInDir"));
        FileUtils::openFile(m_currentActionKey, true);
    });
    connect(m_actionLabel3, &ActionLabel::actionTriggered, [ & ](){
        DataCollecter::collectLaunchEvent(QStringLiteral("fileContentSearch"), QStringLiteral("copyPath"));
        FileUtils::copyPath(m_currentActionKey);
    });
}

//bool FileContentSearchPlugin::isPreviewEnable(QString key, int type)
//{
//    return false;
//}

//QWidget *FileContentSearchPlugin::previewPage(QString key, int type, QWidget *parent)
//{
//    return nullptr;
//}

#include "file-search-plugin.moc"
