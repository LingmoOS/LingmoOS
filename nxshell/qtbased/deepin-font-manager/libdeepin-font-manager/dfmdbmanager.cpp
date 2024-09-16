// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmdbmanager.h"
#include "dfontinfomanager.h"

#include <QDir>
#include <QFontDatabase>

static DFMDBManager *INSTANCE = nullptr;
QList<QMap<QString, QString>> DFMDBManager::recordList = QList<QMap<QString, QString>>();

DFMDBManager::DFMDBManager(QObject *parent)
    : QObject(parent)
    , m_sqlUtil(new DSqliteUtil(QDir::homePath() + "/.local/share/deepin/deepin-font-manager/.font_manager.db"))
{
}

DFMDBManager::~DFMDBManager()
{
    delete m_sqlUtil;
}

DFMDBManager *DFMDBManager::instance()
{
    if (!INSTANCE) {
        INSTANCE = new DFMDBManager;
    }

    return INSTANCE;
}

/*************************************************************************
 <Function>      parseRecordToItemData
 <Description>   将查询记录转化为itemdata
 <Author>        null
 <Input>
    <param1>     record                        Description:查询记录
 <Return>        DFontPreviewItemData          Description:转化后的数据
 <Note>          null
*************************************************************************/
DFontPreviewItemData DFMDBManager::parseRecordToItemData(const QMap<QString, QString> &record)
{
    DFontPreviewItemData itemData(record.value("filePath"), record.value("familyName"), record.value("styleName"),
                                  record.value("type"), record.value("version"), record.value("copyright"),
                                  record.value("description"), record.value("sysVersion"), record.value("fullname"),
                                  record.value("psname"), record.value("trademark"), record.value("isInstalled").toInt(),
                                  record.value("isError").toInt(), isSystemFont(record.value("filePath")),
                                  record.value("isEnabled").toInt(), record.value("isCollected").toInt(),
                                  record.value("isChineseFont").toInt(), record.value("isMonoSpace").toInt(),
                                  record.value("fontName"),
                                  record.value("fontPreview"), record.value("fontId"));

    DFontInfoManager::instance()->getDefaultPreview(itemData.fontInfo);

    if (!itemData.fontData.strFontName.endsWith(itemData.fontInfo.styleName) && !itemData.fontInfo.styleName.isEmpty())
        itemData.fontData.strFontName += QString("-%1").arg(itemData.fontInfo.styleName);

    return itemData;
}

///*************************************************************************
// <Function>      getDFontInfo
// <Description>   通过查询记录获取字体信息
// <Author>        null
// <Input>
//    <param1>     record               Description:查询记录
// <Return>        DFontInfo            Description:字体信息
// <Note>          null
//*************************************************************************/
//DFontInfo DFMDBManager::getDFontInfo(const QMap<QString, QString> &record)
//{
//    DFontInfo fontInfo;

//    fontInfo.filePath = record.value("filePath");
//    fontInfo.familyName = record.value("familyName");
//    fontInfo.styleName = record.value("styleName");
//    fontInfo.type = record.value("type");
//    fontInfo.version = record.value("version");
//    fontInfo.copyright = record.value("copyright");
//    fontInfo.description = record.value("description");
//    fontInfo.sysVersion = record.value("sysVersion");
//    fontInfo.isInstalled = record.value("isInstalled").toInt();
//    fontInfo.isError = record.value("isError").toInt();
//    fontInfo.isSystemFont = isSystemFont(fontInfo.filePath);
//    //add
//    fontInfo.fullname = record.value("fullname");
//    fontInfo.psname = record.value("psname");
//    fontInfo.trademark = record.value("trademark");
//    fontInfo.sp3FamilyName = record.value("fontPreview");
//    DFontInfoManager::instance()->getDefaultPreview(fontInfo);

//    return fontInfo;
//}


/*************************************************************************
 <Function>      appendAllKeys
 <Description>   添加关键字
 <Author>        null
 <Input>
    <param1>     keyList            Description:传入参数,待添加的关键字链表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFMDBManager::appendAllKeys(QList<QString> &keyList)
{
    keyList.append("fontId");
    keyList.append("fontName");
    keyList.append("isEnabled");
    keyList.append("isCollected");
    keyList.append("filePath");
    keyList.append("familyName");
    keyList.append("styleName");
    keyList.append("type");
    keyList.append("version");
    keyList.append("copyright");
    keyList.append("description");
    keyList.append("sysVersion");
    keyList.append("isInstalled");
    keyList.append("isError");
    keyList.append("isChineseFont");
    keyList.append("isMonoSpace");
    //add
    keyList.append("fullname");
    keyList.append("psname");
    keyList.append("trademark");
    //add for SP3 familyName
    keyList.append("fontPreview");
}

/*************************************************************************
 <Function>      getAllFontInfo
 <Description>   获取所有的字体的字体信息
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        QList<DFontPreviewItemData>  Description:所有字体的字体信息
 <Note>          null
*************************************************************************/
QList<DFontPreviewItemData> DFMDBManager::getAllFontInfo(QList<DFontPreviewItemData> *deletedFontInfo)
{
    QList<DFontPreviewItemData> fontItemDataList;

    QList<QMap<QString, QString>> recordList;

    QList<QString> keyList;
    appendAllKeys(keyList);

    m_sqlUtil->findAllRecords(keyList, recordList);
    for (QMap<QString, QString> &record : recordList) {
        if (record.size() > 0) {
            DFontPreviewItemData itemData = parseRecordToItemData(record);
            if (QFileInfo(itemData.fontInfo.filePath).exists()) {
                fontItemDataList.push_back(itemData);
            } else {
                if (deletedFontInfo != nullptr)
                    deletedFontInfo->push_back(itemData);
            }
        }
    }

    return fontItemDataList;
}

QList<DFontPreviewItemData> DFMDBManager::getFontInfo(const int count, QList<DFontPreviewItemData> *deletedFontInfo)
{
    QMutex m_mutex;
    QMutexLocker locker(&m_mutex);
    QList<DFontPreviewItemData> fontItemDataList;
    QList<QMap<QString, QString>> list ;

    if (count > recordList.count()) {
        list = recordList;
    } else {
        list = recordList.mid(0, count);
    }

    for (QMap<QString, QString> &record : list) {
        if (record.size() > 0) {
            DFontPreviewItemData itemData = parseRecordToItemData(record);
            if (QFileInfo(itemData.fontInfo.filePath).exists()) {
//                DFontPreviewListDataThread::instance()->addPathWatcher(itemData.fontInfo.filePath);
                fontItemDataList.push_back(itemData);
            } else {
                if (deletedFontInfo != nullptr)
                    deletedFontInfo->push_back(itemData);
            }
        }
    }

    recordList = recordList.mid(count, recordList.count());

    return fontItemDataList;

}

QList<DFontPreviewItemData> DFMDBManager::getFontInfo(QList<QMap<QString, QString> > list, QList<DFontPreviewItemData> *deletedFontInfo)
{
    QList<DFontPreviewItemData> fontItemDataList;

    for (QMap<QString, QString> &record : list) {
        if (record.size() > 0) {
            DFontPreviewItemData itemData = parseRecordToItemData(record);
            if (QFileInfo(itemData.fontInfo.filePath).exists()) {
//                DFontPreviewListDataThread::instance()->addPathWatcher(itemData.fontInfo.filePath);
                fontItemDataList.push_back(itemData);
            } else {
                if (deletedFontInfo != nullptr)
                    deletedFontInfo->push_back(itemData);
            }
        }
    }

    return fontItemDataList;
}

/*************************************************************************
 <Function>      getRecordCount
 <Description>   获取查询记录的数目
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        int             Description:查询记录的数目
 <Note>          null
*************************************************************************/
int DFMDBManager::getRecordCount()
{
    return m_sqlUtil->getRecordCount();
}

/*************************************************************************
 <Function>      getCurrMaxFontId
 <Description>   获取现在数据库中最大的fontid
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        int             Description:数据库中最大的fontid
 <Note>          null
*************************************************************************/
int DFMDBManager::getCurrMaxFontId()
{
    return m_sqlUtil->getMaxFontId();
}

/*************************************************************************
 <Function>      getInstalledFontsPath
 <Description>   获取已安装字体路径
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        QStringList     Description:获取到的已安装字体路径
 <Note>          null
*************************************************************************/
QStringList DFMDBManager::getInstalledFontsPath()
{
    return m_sqlUtil->getInstalledFontsPath();
}


/*************************************************************************
 <Function>      isFontInfoExist
 <Description>   得到字体的路径
 <Author>        null
 <Input>
    <param1>     newFileFontInfo               Description:需要判断的字体
 <Return>        QString                       Description:字体的路径
 <Note>          null
*************************************************************************/
QString DFMDBManager::isFontInfoExist(const DFontInfo &newFileFontInfo)
{
    QList<QMap<QString, QString>> recordList;

    QList<QString> keyList;
    keyList.append("filePath");

    QMap<QString, QString> whereMap;
    whereMap.insert("familyName", newFileFontInfo.familyName);
    whereMap.insert("styleName", newFileFontInfo.styleName);

    if (newFileFontInfo.filePath.endsWith(".ttc", Qt::CaseInsensitive)) {
        QStringList fontFamilyList = QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(newFileFontInfo.filePath));
        if (fontFamilyList.size() > 1) {
            for (QString &fontFamily : fontFamilyList) {
                whereMap.insert("familyName", fontFamily);
                m_sqlUtil->findRecords(keyList, whereMap, &recordList);
                if (recordList.size() > 0) {
                    QString result = recordList.first().value("filePath");
                    return result;
                }
            }
        }
    }

    m_sqlUtil->findRecords(keyList, whereMap, &recordList);

    if (recordList.size() > 0) {
        QString result = recordList.first().value("filePath");
        return result;
    }

    return QString();
}

/*************************************************************************
 <Function>      getSpecifiedFontName
 <Description>   获取指定字体文件的fontname
 <Author>        null
 <Input>
    <param1>     filePath               字体文件全路径名
 <Return>        QStringList
 <Note>          null
*************************************************************************/
QStringList DFMDBManager::getSpecifiedFontName(const QString &filePath)
{
    QList<QString> keyList;
    keyList.append("fontName");

    QMap<QString, QString> whereMap;
    whereMap.insert("filePath", filePath);

    QStringList result;
    QList<QMap<QString, QString>> recordList;

    m_sqlUtil->findRecords(keyList, whereMap, &recordList);
    if (!recordList.empty()) {
        QListIterator<QMap<QString, QString>> iter(recordList);
        while (iter.hasNext()) {
            result.append(iter.next().value("fontName"));
        }
    }
    return result;
}

///*************************************************************************
// <Function>      mapItemData
// <Description>   通过itemdata来构建字体信息的map
// <Author>        null
// <Input>
//    <param1>     itemData                          Description:字体的itemdata
// <Return>        QMap<QString, QString>            Description:字体信息的map
// <Note>          null
//*************************************************************************/
//QMap<QString, QString> DFMDBManager::mapItemData(DFontPreviewItemData itemData)
//{
//    QMap<QString, QString> mapData;
//    //mapData.insert("fontId", itemData.strFontId);   //auto increament ,Don't need supply
//    mapData.insert("fontName", itemData.fontData.strFontName);
//    mapData.insert("isEnabled", QString::number(itemData.fontData.isEnabled()));
//    mapData.insert("isCollected", QString::number(itemData.fontData.isCollected()));
//    mapData.insert("isChineseFont", QString::number(itemData.fontData.isChinese()));
//    mapData.insert("isMonoSpace", QString::number(itemData.fontData.isMonoSpace()));
//    mapData.insert("filePath", itemData.fontInfo.filePath);
//    mapData.insert("familyName", itemData.fontInfo.familyName);
//    mapData.insert("styleName", itemData.fontInfo.styleName);
//    mapData.insert("type", itemData.fontInfo.type);
//    mapData.insert("version", itemData.fontInfo.version);
//    mapData.insert("copyright", itemData.fontInfo.copyright);
//    mapData.insert("description", itemData.fontInfo.description);
//    mapData.insert("sysVersion", itemData.fontInfo.sysVersion);
//    mapData.insert("isInstalled", QString::number(itemData.fontInfo.isInstalled));
//    mapData.insert("isError", QString::number(itemData.fontInfo.isError));
//    //add
//    mapData.insert("fullname", itemData.fontInfo.fullname);
//    mapData.insert("psname", itemData.fontInfo.psname);
//    mapData.insert("trademark", itemData.fontInfo.trademark);
//    mapData.insert("fontPreview", itemData.fontInfo.sp3FamilyName);

//    return mapData;
//}

/*************************************************************************
 <Function>      addFontInfo
 <Description>   记录需要添加的字体数据,用于之后批量添加
 <Author>        null
 <Input>
    <param1>     itemData            Description:需要添加的字体数据
 <Return>        bool                Description:是否成功
 <Note>          null
*************************************************************************/
bool DFMDBManager::addFontInfo(const DFontPreviewItemData &itemData)
{
//    qDebug() << __FUNCTION__ << itemData.fontInfo.toString();
    if (!m_addFontList.contains(itemData) || itemData.fontInfo.isSystemFont) {
        m_addFontList.append(itemData);
    }
    return true;
//    return m_sqlUtil->addRecord(mapItemData(itemData));
}

/*************************************************************************
 <Function>      updateFontInfo
 <Description>   更新数据库中的数据
 <Author>        null
 <Input>
    <param1>     null            Description:null
    <param2>     null            Description:null
    <param3>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
bool DFMDBManager::updateFontInfo(const QMap<QString, QString> &whereMap, const QMap<QString, QString> &dataMap)
{
    return m_sqlUtil->updateRecord(whereMap, dataMap);
}

/**
* @brief DFMDBManager::updateSP3FamilyName sp3的familyName兼容sp2 update1及之前的版本
* @param fontList 字体列表
* @param inFontList 是否使用字体列表，true：使用 false：不使用
* @return void
*/
void DFMDBManager::updateSP3FamilyName(const QList<DFontInfo> &fontList, bool inFontList)
{
    m_sqlUtil->updateSP3FamilyName(fontList, inFontList);
}

/*************************************************************************
 <Function>      commitAddFontInfo
 <Description>   开启事物,批量增加数据
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFMDBManager::commitAddFontInfo()
{
    if (m_addFontList.isEmpty())
        return;

//    QMutexLocker locker(&m_mutex);
    beginTransaction();
    addFontInfo(m_addFontList);
    endTransaction();
    m_addFontList.clear();
}

/*************************************************************************
 <Function>      addFontInfo
 <Description>   向数据库中添加数据
 <Author>        null
 <Input>
    <param1>     fontList            Description:需要添加的数据链表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFMDBManager::addFontInfo(const QList<DFontPreviewItemData> &fontList)
{
    return m_sqlUtil->addFontInfo(fontList);
}

/*************************************************************************
 <Function>      deleteFontInfo
 <Description>   收集需要删除的字体数据,等待批量删除
 <Author>        null
 <Input>
    <param1>     itemData            Description:需要删除的字体数据
 <Return>        null                Description:null
 <Note>          null
*************************************************************************/
void DFMDBManager::deleteFontInfo(const DFontPreviewItemData &itemData)
{
    if (!m_delFontList.contains(itemData))
        m_delFontList << itemData;
}

/*************************************************************************
 <Function>      deleteFontInfo
 <Description>   批量从数据库中删除数据
 <Author>        null
 <Input>
    <param1>     fontList            Description:需要删除的字体信息链表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFMDBManager::deleteFontInfo(const QList<DFontPreviewItemData> &fontList)
{
    m_sqlUtil->deleteFontInfo(fontList);
}

/*************************************************************************
 <Function>      commitDeleteFontInfo
 <Description>   开启事物,批量进行删除
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFMDBManager::commitDeleteFontInfo()
{
    if (m_delFontList.isEmpty())
        return;

    beginTransaction();
    m_sqlUtil->deleteFontInfo(m_delFontList);
    endTransaction();
    m_delFontList.clear();
}

/*************************************************************************
 <Function>      updateFontInfo
 <Description>   准备批量更新的字体数据
 <Author>        null
 <Input>
    <param1>     itemData            Description:需要更新的字体数据
    <param2>     strKey              Description:更新的字节
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFMDBManager::updateFontInfo(const DFontPreviewItemData &itemData, const QString &strKey)
{
    if (!m_updateFontList.contains(itemData) || itemData.fontInfo.isSystemFont) {
        m_updateFontList << itemData;
        if (m_strKey != strKey)
            m_strKey = strKey;
    }
}

/*************************************************************************
 <Function>      updateFontInfo
 <Description>   批量更新数据
 <Author>        null
 <Input>
    <param1>     fontList            Description:需要更新的字体数据
    <param2>     strKey              Description:关键字
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFMDBManager::updateFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &strKey)
{
    return m_sqlUtil->updateFontInfo(fontList, strKey);
}

/*************************************************************************
 <Function>      commitUpdateFontInfo
 <Description>   开启事务,批量更新数据
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFMDBManager::commitUpdateFontInfo()
{
    if (m_updateFontList.isEmpty())
        return;

    beginTransaction();
    m_sqlUtil->updateFontInfo(m_updateFontList, m_strKey);
    //m_sqlUtil->updateOld2Record();//数据库表被重建时，先saveRecord，再updateOld2Record。
    endTransaction();
    m_updateFontList.clear();
}
/*************************************************************************
 <Function>      syncOldRecords
 <Description>   开启事务,批量更新数据，将更新语言时被删除的数据同步到新数据库中
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFMDBManager::syncOldRecords()
{
    beginTransaction();
    m_sqlUtil->updateOld2Record();//数据库表被重建时，先saveRecord，再updateOld2Record。
    endTransaction();
}

void DFMDBManager::getAllRecords()
{
    QList<QString> keyList;
    appendAllKeys(keyList);

    m_sqlUtil->findAllRecords(keyList, recordList);
}

void DFMDBManager::checkIfEmpty()
{
    m_sqlUtil->checkIfEmpty();
}
