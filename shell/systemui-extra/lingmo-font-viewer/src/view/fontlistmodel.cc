#include <QFileInfo>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QDebug>
#include <QProcess>
#include <QStandardPaths>

#include "fontlistmodel.h"
#include "include/core.h"
#include "globalsizedata.h"
#include "getfontthread.h"

FontListModel::FontListModel() :
    QStandardItemModel(NULL)
{
    QHash<int, QByteArray> roleNames;
    roleNames[GlobalSizeData::FontPath] = "fontPath";
    roleNames[GlobalSizeData::FontName] = "fontName";
    roleNames[GlobalSizeData::FontStyle] = "fontStyle";
    roleNames[GlobalSizeData::FontFamily] = "fontFamily";
    roleNames[GlobalSizeData::CollectState] = "collectState";
    roleNames[GlobalSizeData::FontHeight] = "fontHeight";
    setItemRoleNames(roleNames);

    // m_timer = new QTimer(this);

    getFontModel();

}

FontListModel::~FontListModel()
{
    if (m_timer != nullptr) {
        m_timer->deleteLater();
    }
}

FontListModel *FontListModel::getInstance()
{
    static FontListModel instance;

    return &instance;
}

void FontListModel::getFontModel()
{
    m_newFont.clear();

    m_allFontData.clear();
    m_allFontData = Core::getInstance()->getViewData();   /* 获取全部字体信息 */

    m_collectFontData.clear();
    m_collectFontData = Core::getInstance()->getCollectFont();   /* 获取收藏字体信息 */

    GetFontThread *thread = new GetFontThread(m_allFontData, m_collectFontData, m_fontSize, m_fontItemsHash);
    connect(thread, &GetFontThread::sigCreatItem, this, &FontListModel::slotAddItem);
    if (m_isAddFont) {
        connect(thread, &GetFontThread::finished, this, &FontListModel::slotAddNewItem);
    } else {
        connect(thread, &GetFontThread::finished, this, &FontListModel::slotAGetAllFontOver);
    }
    
    thread->start();

    return;
}

QStringList FontListModel::addFont(QStringList fontList)
{
    m_isAddFont = true;
    QString installFontPath = QStandardPaths::writableLocation(QStandardPaths::FontsLocation) + FONT_PATH;
    m_installSuccess.clear();

    QStringList uninstallFont;
    uninstallFont.clear();
    int num = 0;

    for (int i = 0; i < fontList.size(); i++) {
        QString path = fontList[i];

        QFileInfo fileinfo;
        fileinfo = QFileInfo(path);
        QString fileName = fileinfo.fileName();   /* 文件名 */

        if (Core::getInstance()->judgeBadFontFile(path)) {
            uninstallFont.insert(num, fileName);
            num++;
            continue;
        }

        if (!Core::getInstance()->installFont(path)) {
            uninstallFont.insert(num, fileName);
            num++;
        } else {
            QString filePath = installFontPath + fileName;
            QFontDatabase::addApplicationFont(filePath);
            m_installSuccess.append(filePath);
        }
    }
    Core::getInstance()->updateFontList();

    this->getFontModel();

    m_isAddFont = false;

    return uninstallFont;
}

QStandardItem *FontListModel::getItemByPath(QString path)
{
    QStandardItem *resItem = nullptr;

    if (m_fontItemsHash.contains(path)) {
        resItem = m_fontItemsHash.value(path);
    }

    return resItem;
}

void FontListModel::removeFont(QString path, int row)
{
    QStandardItem *item = this->getItemByPath(path);
    if (item != nullptr) {
        QString fontName = item->data(GlobalSizeData::FontName).toString();
        if (Core::getInstance()->uninstallFont(path, fontName)) { 
            this->removeRow(row);
             m_fontItemsHash.remove(path);
        }
    }
        
    return;
}

void FontListModel::collectFont(QString path)
{
    QStandardItem *item = this->getItemByPath(path);

    if (item != nullptr) {
        int fontCollect = item->data(GlobalSizeData::CollectState).toInt();

        if (fontCollect == GlobalSizeData::CollectType::NotCollected) {
            /* 添加 收藏字体 SDK的打点 */
            GlobalSizeData::getInstance()->collectionFontSDKPoint();

            if (Core::getInstance()->collectFont(path)) {
                item->setData(GlobalSizeData::CollectType::IsCollected, GlobalSizeData::CollectState);
            }
        } else {
            /* 添加 取消收藏字体 SDK的打点 */
            GlobalSizeData::getInstance()->cancelCollectionFontSDKPoint();
            
            if (Core::getInstance()->unCollectFont(path)){
                item->setData(GlobalSizeData::CollectType::NotCollected, GlobalSizeData::CollectState);
            }
        }
    }
    return;
}

void FontListModel::previewChangeSize(int size)
{
    m_fontSize = size;
    if ( m_timer == nullptr) {
        m_timer = new QTimer();
    }

    /* 判断当前字号是否存在 */
    if (fontSizeExist()) {
        changeFontSize();
    } else {
        m_timer->stop();
        connect(m_timer, &QTimer::timeout, this, &FontListModel::getFontSize);
        connect(m_timer, &QTimer::timeout, m_timer, &QTimer::stop);
        m_timer->start(200);
    }
    return;
}

void FontListModel::changeFontSize()
{
    QHash<QString, QStandardItem*>::iterator fontIt = m_fontItemsHash.begin();

    while (fontIt != m_fontItemsHash.end()) {
        QStandardItem *fontItem = fontIt.value();
        QHash<int, int> fontSizeHieght;   /* 字体不同字号的高度 <字号，字体高度> */

        fontSizeHieght = m_fontSizeHash.value(fontIt.key());
        fontItem->setData(fontSizeHieght.value(m_fontSize), GlobalSizeData::FontHeight);
        fontIt++;
    }
    return;
}

bool FontListModel::fontSizeExist()
{
    QHash<QString, QStandardItem*>::iterator fontIt = m_fontItemsHash.begin();

    while (fontIt != m_fontItemsHash.end()) {
        QHash<int, int> fontSizeHieght;   /* <字号，字体高度> */
        fontSizeHieght = m_fontSizeHash.value(fontIt.key());
        if (!fontSizeHieght.contains(m_fontSize)) {
            return false;
        }
        fontIt++;
    }
    return true;
}

void FontListModel::getFontSize()
{
    // m_timer->stop();

    QHash<QString, QStandardItem*>::iterator fontIt = m_fontItemsHash.begin();

    while (fontIt != m_fontItemsHash.end()) {

        QStandardItem *fontItem = fontIt.value();
        QHash<int, int> fontSizeHieght;   /* 字体不同字号的高度 <字号，字体高度> */

        /* 判断字体hash表中是否有字体 */
        if (m_fontSizeHash.contains(fontIt.key())) {

            fontSizeHieght = m_fontSizeHash.value(fontIt.key());

            /* 在字号高度hash表中查找当前号的高度 */
            if (fontSizeHieght.contains(m_fontSize)) {
                fontItem->setData(fontSizeHieght.value(m_fontSize), GlobalSizeData::FontHeight);
            } else {
                QFont textFont = QFont(fontItem->data(GlobalSizeData::FontName).toString(), m_fontSize);
                QFontMetrics fontmet = QFontMetrics(textFont);
                int fontHeight = fontmet.height();

                fontSizeHieght.insert(m_fontSize, fontHeight);

                m_fontSizeHash[fontIt.key()] = fontSizeHieght;

                fontItem->setData(fontHeight, GlobalSizeData::FontHeight);
            }
        } else {
            QFont textFont = QFont(fontItem->data(GlobalSizeData::FontName).toString(), m_fontSize);
            QFontMetrics fontmet = QFontMetrics(textFont);
            int fontHeight = fontmet.height();

            fontSizeHieght.insert(m_fontSize, fontHeight);

            m_fontSizeHash.insert(fontIt.key(), fontSizeHieght);

            fontItem->setData(fontHeight, GlobalSizeData::FontHeight);
        }
        fontIt++;
    }
    return;

}

void FontListModel::newFontGetSize(QString fontPath)
{
    QStandardItem *fontItem = this->getItemByPath(fontPath);
    QHash<int, int> fontSizeHieght;

    /* 获取新字体从14-60号字体的高度 */
    for (int size = 14; size <= 60; size++) {
        QFont textFont = QFont(fontItem->data(GlobalSizeData::FontName).toString(), size);
        QFontMetrics fontmet = QFontMetrics(textFont);
        int fontHeight = fontmet.lineSpacing();   /* 展示字体的高度 */

        fontSizeHieght.insert(size, fontHeight);
    }
    m_fontSizeHash.insert(fontPath, fontSizeHieght);

    fontItem->setData(fontSizeHieght.value(m_fontSize), GlobalSizeData::FontHeight);
    return;
}

void FontListModel::slotAddItem(QString path, QStandardItem *item)
{
    if (m_isFirstAddItem == false) {
        this->appendRow(item);
    }
    m_fontItemsHash.insert(path, item);
    m_newFont.push_back(path);
    return;
}

void FontListModel::slotAddNewItem()
{
    for (int i = 0; i < m_newFont.size(); ++i) {
        this->newFontGetSize(m_newFont[i]);
    }
    emit sigInstallFontOver();
    return;
}

bool FontListModel::isFontFile(QString path)
{
    QString command = QString("file -i ") + '"' + path + '"';

    QProcess process;
    process.start(command);

    if (!process.waitForFinished()) {
        return false;
    }

    QString ret = process.readAll();
    
    if (ret.contains("font/sfnt") || ret.contains("application/vnd.ms-opentype")) {
        return true;
    }

    return false;
}

void FontListModel::slotAGetAllFontOver()
{
    if (m_isFirstAddItem) {
        updateListViewItem();
        m_isFirstAddItem = false;  
    }
    emit sigGetAllFontOver();
    return;
}

void FontListModel::updateListViewItem()
{
    QHash<QString, QStandardItem*>::iterator fontIt = m_fontItemsHash.begin();

    while (fontIt != m_fontItemsHash.end()) {
        QStandardItem *fontItem = fontIt.value();
        this->appendRow(fontItem);
        fontIt++;
    }

    return;
}

QStringList FontListModel::getInstallSuccessList()
{
    return m_installSuccess;
}