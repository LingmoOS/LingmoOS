// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

extern "C" {
#include <ctype.h>
}

#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

#include "utils.h"

#include "global/builtinsearch.h"
#include "global/searchhelper.h"
#include "business/config/accessrecord/accessrecord.h"

#include "interfaces/daemongrandsearchinterface.h"

#include <DArrowRectangle>
#include <DDesktopEntry>
#include <DGuiApplicationHelper>
#include <DDesktopServices>

#include <QCollator>
#include <QFileInfo>
#include <QDesktopServices>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QDateTime>
#include <QIcon>
#include <QApplication>
#include <DUtil>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace GrandSearch;

#ifdef COMPILE_ON_V23
static const QString AppManagerService = "org.desktopspec.ApplicationManager1";
static const QString AppManagerPathPrefix = "/org/desktopspec/ApplicationManager1";
static const QString AppInterface = "org.desktopspec.ApplicationManager1.Application";

#else
static const QString SessionManagerService = "com.deepin.SessionManager";
static const QString StartManagerPath = "/com/deepin/StartManager";
static const QString StartManagerInterface = "com.deepin.StartManager";
#endif

static const int WeightDiffLimit = 21;

static const int CreateDateType = 1;
static const int ModifyDateType = 2;
static const int ReadDateType = 3;

static const double TimesWeight = 0.5;

class DCollator : public QCollator
{
public:
    DCollator() : QCollator()
    {
        setCaseSensitivity(Qt::CaseInsensitive);
    }
};

QMap<QString, QString> Utils::m_appIconNameMap;
QMimeDatabase Utils::m_mimeDb;

bool Utils::sort(MatchedItems &list, Qt::SortOrder order/* = Qt::AscendingOrder*/)
{
    QTime time;
    time.start();
    qStableSort(list.begin(), list.end(), [order](const MatchedItem &node1, const MatchedItem &node2) {
        return compareByString(node1.name, node2.name, order);
    });

    qDebug() << QString("sort matchItems done. cost [%1]ms").arg(time.elapsed());
    return true;
}

bool Utils::compareByString(QString str1, QString str2, Qt::SortOrder order)
{
    thread_local static DCollator sortCollator;

    // 先去掉前边重复的字符串
    int nMinLength = qMin(str1.size(), str2.size());
    int nMidIndex = -1;
    for (int i = 0; i < nMinLength; i++) {
        if (str1[i] != str2[i]) {
            nMidIndex = i;
            break;
        }
    }
    if (nMidIndex > 0) {
        str1=str1.mid(nMidIndex);
        str2=str2.mid(nMidIndex);
    }

    // 其他符号要排在最后
    if (startWithSymbol(str1)) {
        if (!startWithSymbol(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWithSymbol(str2)) {
        return order != Qt::DescendingOrder;
    }

    // 数字排在次次后
    if (startWidthNum(str1)) {
        if (!startWidthNum(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWidthNum(str2)) {
        return order != Qt::DescendingOrder;
    }

    // 其他语言排在次后
    if (startWithOtherLang(str1)) {
        if (!startWithOtherLang(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWithOtherLang(str2)) {
        return order != Qt::DescendingOrder;
    }

    // 英文排在第二
    if (startWithLatin(str1)) {
        if (!startWithLatin(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWithLatin(str2)) {
        return order != Qt::DescendingOrder;
    }

    // 中文排在最前
    if (startWithHanzi(str1)) {
        if (!startWithHanzi(str2)) {
            return order == Qt::DescendingOrder;
        }
    } else if (startWithHanzi(str2)) {
        return order != Qt::DescendingOrder;
    }

    return ((order == Qt::DescendingOrder) ^ (sortCollator.compare(str1, str2) < 0)) == 0x01;
}

bool Utils::startWithSymbol(const QString &text)
{
    if (text.isEmpty())
        return false;

    bool bRet = false;
    // 先匹配中文标点符号
    QRegExp regExp("^[\u3002\uff1b\uff0c\uff1a\u201c\u201d\uff08\uff09\u3001\uff1f\u300a\u300b].*$");
    bRet = regExp.exactMatch(text.at(0));

    // 若不是中文标点符号，再判断是否为其他符号
    if (!bRet)
        bRet = ispunct(text.at(0).toLatin1());

    return bRet;
}

bool Utils::startWithHanzi(const QString &text)
{
    if (text.isEmpty())
        return false;

    // 匹配中文
    QRegExp regExp("^[\u4e00-\u9fa5].*$");
    bool bRet = regExp.exactMatch(text.at(0));

    return bRet;
}

bool Utils::startWithLatin(const QString &text)
{
    if (text.isEmpty())
        return false;

    // 匹配英文
    QRegExp regExp("^[a-zA-Z].*$");
    bool bRet = regExp.exactMatch(text.at(0));

    return bRet;
}

bool Utils::startWithOtherLang(const QString &text)
{
    if (text.isEmpty())
        return false;

    QChar::Script textScript = text.at(0).script();

    bool bRet = textScript >= QChar::Script_Inherited
            && textScript <= QChar::Script_ZanabazarSquare
            && textScript != QChar::Script_Latin
            && textScript != QChar::Script_Han;

    //qDebug() <<QString("%1 is %2 otherLang").arg(text).arg(bRet ? "" : "not");
    return bRet;
}

bool Utils::startWidthNum(const QString &text)
{
    if (text.isEmpty())
        return false;

    // 匹配数字
    QRegExp regExp("^[0-9].*$");
    bool bRet = regExp.exactMatch(text.at(0));

    return bRet;
}

bool Utils::sortByWeight(MatchedItemMap &map, Qt::SortOrder order)
{
    QTime time;
    time.start();

    for (const QString &searchGroupName : map.keys()) {
        MatchedItems &list = map[searchGroupName];
        sortByWeight(list, order);
    }

    qDebug() << QString("sort matchItems by weight done.cost %1ms").arg(time.elapsed());
    return true;
}

/**
 * @brief Utils::compareByWeight 比较两个类目的权重
 * @param node1 类目1
 * @param node2 类目2
 * @param order 排序规则，升序/降序
 * @return 类目1>类目2返回true，否则返回false
 */
bool Utils::compareByWeight(const MatchedItem &node1, const MatchedItem &node2, Qt::SortOrder order)
{
    bool hasWeight1 = node1.extra.toHash().contains(GRANDSEARCH_PROPERTY_ITEM_WEIGHT);
    bool hasWeight2 = node2.extra.toHash().contains(GRANDSEARCH_PROPERTY_ITEM_WEIGHT);

    if (hasWeight1 && hasWeight2) {
        // 两项均有权重，则对比权重
        double weight1 = node1.extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 0).toDouble();
        double weight2 = node2.extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 0).toDouble();

        return order == Qt::DescendingOrder ? weight1 > weight2 : weight1 < weight2;
    } else if (hasWeight1) {

        return order == Qt::DescendingOrder;
    } else if (hasWeight2) {

        return order != Qt::DescendingOrder;
    } else {
        // 两项均无权重，采用名称排序
        return compareByString(node1.item, node2.item);
    }
}

/**
 * @brief Utils::updateItemsWeight 更新类目权重
 * @param map 匹配的全部返回结果
 * @param content 输入的搜索关键字
 */
void Utils::updateItemsWeight(MatchedItemMap &map, const QString &content)
{
    // 类目、后缀搜索时的关键字解析
    QStringList groupList, suffixList, keys;
    if (!searchHelper->parseKeyword(content, groupList, suffixList, keys))
        keys = QStringList() << content;

    for (const QString &searchGroupName : map.keys()) {
        MatchedItems &list = map[searchGroupName];
        for (MatchedItem &item : list) {
            if (!setWeightMethod(item))
                continue;

            // 计算权重
            double weight = 0;
            {
                const QVariantHash &extData = item.extra.toHash();
                const QString &method = extData.value(GRANDSEARCH_PROPERTY_WEIGHT_METHOD).toString();

                if (method == GRANDSEARCH_PROPERTY_WEIGHT_METHOD_LOCALFILE) {
                    weight = calcFileWeight(item.item, item.name, keys);
                } else if (method == GRANDSEARCH_PROPERTY_WEIGHT_METHOD_APP) {
                    weight = calcAppWeight(item, keys);
                } else if (method == GRANDSEARCH_PROPERTY_WEIGHT_METHOD_SETTING) {
                    weight = calcSettingWeight(item, keys);
                } else {
                    continue;
                }
            }

            QVariant &extra = item.extra;
            QVariantHash originalValue = extra.toHash();
            originalValue.insert(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, weight);
            extra = QVariant::fromValue(originalValue);
        }
    }
}

/**
 * @brief Utils::setWeightMethod 根据搜索项设置权重计算函数
 * @param item 搜索结果
 * @return 支持权重计算则返回true
 */
bool Utils::setWeightMethod(MatchedItem &item)
{
    QVariantHash ext = item.extra.toHash();

    // 已经设置权重数值
    if (ext.contains(GRANDSEARCH_PROPERTY_ITEM_WEIGHT))
        return false;

    // 已经设置了计算方法
    if (ext.contains(GRANDSEARCH_PROPERTY_WEIGHT_METHOD))
        return true;

    const QString &search = item.searcher;
    if (search == GRANDSEARCH_CLASS_FILE_DEEPIN ||
            search == GRANDSEARCH_CLASS_FILE_FSEARCH) {

        ext.insert(GRANDSEARCH_PROPERTY_WEIGHT_METHOD,
                   GRANDSEARCH_PROPERTY_WEIGHT_METHOD_LOCALFILE);
    } else if (search == GRANDSEARCH_CLASS_APP_DESKTOP) {
        ext.insert(GRANDSEARCH_PROPERTY_WEIGHT_METHOD,
                   GRANDSEARCH_PROPERTY_WEIGHT_METHOD_APP);
    } else if (search == GRANDSEARCH_CLASS_SETTING_CONTROLCENTER) {
        ext.insert(GRANDSEARCH_PROPERTY_WEIGHT_METHOD,
                   GRANDSEARCH_PROPERTY_WEIGHT_METHOD_SETTING);
    } else {
        // 不支持计算权重
        return false;
    }

    item.extra = QVariant::fromValue(ext);

    return true;
}

/**
 * @brief Utils::calcFileWeight 计算泛文件类目的权重
 * @param path 文件路径
 * @param name 文件名
 * @param keys 输入关键字列表
 * @return 文件权重
 */
double Utils::calcFileWeight(const QString &path, const QString &name, const QStringList &keys)
{
    double weight = 0;
    for (const QString &key : keys) {
        if (name.contains(key)) {
            weight += 43;
            break;
        }
    }

    QFileInfo fileInfo(path);

    const QDateTime &currentDateTime = QDateTime::currentDateTime();
    const QDateTime &createDateTime = fileInfo.created();
    const QDateTime &lastModifyTime = fileInfo.lastModified();
    const QDateTime &lastReadTime = fileInfo.lastRead();

    qint64 createDayDiff = calcDateDiff(createDateTime, currentDateTime);
    qint64 lastModifyDayDiff = calcDateDiff(lastModifyTime, currentDateTime);
    qint64 lastReadDayDiff = calcDateDiff(lastReadTime, currentDateTime);

    int createDayWeight = calcWeightByDateDiff(createDayDiff, CreateDateType);
    int lastModifyDayWeight = calcWeightByDateDiff(lastModifyDayDiff, ModifyDateType);
    int lastReadDayWeight = calcWeightByDateDiff(lastReadDayDiff, ReadDateType);

    weight += createDayWeight + lastModifyDayWeight + lastReadDayWeight;
    return weight;
}

/**
 * @brief Utils::calcDateDiff 计算绝对间隔天数
 * @param date1 时间1
 * @param date2 时间2
 * @return 绝对间隔天数
 */
qint64 Utils::calcDateDiff(const QDateTime &date1, const QDateTime &date2)
{
    static const qint64 day = 24 * 60 * 60;
    return date1.secsTo(date2) / day;
}

/**
 * @brief Utils::calcWeightByDateDiff 根据天数计算权重
 * @param diff 间隔天数
 * @param type 计算类型，创建/修改/访问
 * @return 天数的权重
 */
double Utils::calcWeightByDateDiff(const qint64 &diff, const int &type)
{
    switch (type) {
    case CreateDateType :
    case ModifyDateType : {
        switch (diff) {
        case 0:
            return 24;
        case 1:
        case 2:
            return 16;
        case 3:
        case 4:
        case 5:
        case 6:
            return 8;
        default:
            return 0;
        }
    }
        break;
    case ReadDateType : {
        switch (diff) {
        case 0:
            return 9;
        case 1:
        case 2:
            return 6;
        case 3:
        case 4:
        case 5:
        case 6:
            return 3;
        default:
            return 0;
        }
    }
        break;
    default:
        return 0;
    }
}

/**
 * @brief Utils::calcAppWeight 计算应用权重
 * @param item 类目
 * @param keys 关键字
 * @return 权重
 */
double Utils::calcAppWeight(const MatchedItem &item, const QStringList &keys)
{
    double weight = 0;
    for (const QString &key : keys) {
        if (item.name.contains(key)) {
            weight += 20;
            break;
        }
    }
    weight += Utils::calcRecordWeight(item);

    weight += 60;
    return weight;
}

double Utils::calcSettingWeight(const MatchedItem &item, const QStringList &keys)
{
    double weight = 0;
    for (const QString &key : keys) {
        if (item.name.contains(key)) {
            weight += 20;
            break;
        }
    }
    weight += Utils::calcRecordWeight(item);

    weight += 60;
    return weight;
}

/**
 * @brief Utils::packageBestMatch 从排序后的泛文件类目中提取权重最高项，打包为最佳匹配项
 * @param map 排序后的泛文件类目
 * @param maxQuantity 最佳匹配项的数目
 */
void Utils::packageBestMatch(MatchedItemMap &map, int maxQuantity)
{
    if (map.isEmpty() || maxQuantity <= 0)
        return;

    QTime time;
    time.start();

    static const QMap<QString, bool> supprotedSeracher = {
        {GRANDSEARCH_CLASS_FILE_DEEPIN, true},
        {GRANDSEARCH_CLASS_FILE_FSEARCH, true},
        {GRANDSEARCH_CLASS_APP_DESKTOP, true},
        {GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, true},
    };

    // 待文件搜索各组去重后删除该黑名单
    static const QMap<QString, bool> blackGroup = {
        {GRANDSEARCH_GROUP_FILE_VIDEO, false},
        {GRANDSEARCH_GROUP_FILE_AUDIO, false},
        {GRANDSEARCH_GROUP_FILE_PICTURE, false},
        {GRANDSEARCH_GROUP_FILE_DOCUMNET, false},
        {GRANDSEARCH_GROUP_FOLDER, false}
    };

    MatchedItems bestList;
    QList<QPair<MatchedItem, QString>> tempBestList;

    for (auto group : map.keys()) {
        // 不支持最佳匹配
        if (blackGroup.contains(group))
            continue;

        MatchedItems &list = map[group];

        for (auto item = list.begin(); item != list.end(); ++item) {
            // 支持纳入最佳匹配的搜索项
            if (!supprotedSeracher.contains(item->searcher))
                continue;

            if (!tempBestList.isEmpty()) {
                double weight1 = tempBestList.first().first.extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 0).toDouble();
                double weight2 = item->extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 0).toDouble();
                // 最佳匹配权重最高项高于组别第一项超过21
                if (weight1 - weight2 >= WeightDiffLimit) {
                    break;
                } else if (weight2 - weight1 > WeightDiffLimit) {
                    tempBestList.clear();
                    tempBestList.append(qMakePair(*item, group));
                } else {
                    // 组别与最佳匹配最高项相差小于21
                    auto it = tempBestList.begin();
                    for (; it != tempBestList.end(); ++it) {
                        // 寻找插入位置
                        double weightBest = it->first.extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 0).toDouble();
                        if (weight2 > weightBest) {
                            tempBestList.insert(it, qMakePair(*item, group));
                            break;
                        }
                    }

                    // 数量不够，且权重比已存在都小，直接放入尾部
                    if (it == tempBestList.end() && tempBestList.size() < maxQuantity)
                        tempBestList.append(qMakePair(*item, group));

                    // 超过最大显示数量，移除最后一个
                    if (tempBestList.size() > maxQuantity)
                        tempBestList.removeLast();

                    // 本组内不会有比last权重更大的值了，结束本组
                    if (tempBestList.size() == maxQuantity && tempBestList.last().second == group)
                        break;
                }
            } else {
                tempBestList.append(qMakePair(*item, group));
            }
        }
    }

    for (auto list : tempBestList) {
        // 在原分组中移除最佳匹配项
        map[list.second].removeOne(list.first);
        bestList.append(list.first);

        // 原分组为空，移除分组
        if (map[list.second].isEmpty()) {
            qDebug() << "group" << list.second << "is empty, remove it";
            map.remove(list.second);
        }
    }

    qDebug() << "find best match count:" << bestList.count() << QString(".cost [%1]ms").arg(time.elapsed());
    if (!bestList.isEmpty()) {
        map.insert(GRANDSEARCH_GROUP_BEST, bestList);
    }
}

double Utils::calcRecordWeight(const MatchedItem &item)
{
    auto recordHash = AccessRecord::instance()->getRecord();

    double weight = 0.0;
    if (recordHash.contains(item.searcher)) {
        auto itemHash = recordHash[item.searcher];
        if (itemHash.contains(item.item)) {
            weight += itemHash[item.item] * TimesWeight;
        }
    }

    return weight;
}

bool Utils::isResetSearcher(QString searcher)
{
    static const QStringList searchers = {
        GRANDSEARCH_CLASS_APP_DESKTOP,
        GRANDSEARCH_CLASS_SETTING_CONTROLCENTER
    };

    return searchers.contains(searcher);
}

QString Utils::appIconName(const MatchedItem &item)
{
    QString strAppIconName;
    if (item.item.isEmpty())
        return strAppIconName;

    // 搜索结果是否支持显示app,扩展插件不支持,如需特殊处理的放isShowAppIcon函数.
    if (!isShowAppIcon(item))
        return strAppIconName;

    // 搜索结果为应用，直接返回应用图标
    if (item.searcher == GRANDSEARCH_CLASS_APP_DESKTOP) {
        return item.icon;
    } else if (item.searcher == GRANDSEARCH_CLASS_WEB_STATICTEXT) {
        //默认浏览器图标
        auto defaultDesktopFile = defaultBrowser();
        if (!defaultDesktopFile.isEmpty()) {
            DDesktopEntry entry(defaultDesktopFile);
            strAppIconName = entry.stringValue("Icon");
        }
    } else if (item.searcher == GRANDSEARCH_CLASS_SETTING_CONTROLCENTER) {
        auto defaultDesktopFile = "/usr/share/applications/dde-control-center.desktop";
        if (QFile::exists(defaultDesktopFile)) {
            DDesktopEntry entry(defaultDesktopFile);
            strAppIconName = entry.stringValue("Icon");
        } else {
            qWarning() << defaultDesktopFile << "lost.";
        }

        if (strAppIconName.isEmpty())
            strAppIconName = "preferences-system";
    } else {
    // 搜索结果为文件，查询该文件对应默认打开应用图标的名称
        QString mimetype = item.type;
        if (mimetype.isEmpty())
            mimetype = getFileMimetype(item.item);

        QString defaultDesktopFile = getDefaultAppDesktopFileByMimeType(mimetype);
        if (m_appIconNameMap.find(defaultDesktopFile) == m_appIconNameMap.end()) {
            DDesktopEntry entry(defaultDesktopFile);
            strAppIconName = entry.stringValue("Icon");
            m_appIconNameMap[defaultDesktopFile] = strAppIconName;
        } else {
            strAppIconName = m_appIconNameMap[defaultDesktopFile];
        }
    }

    return strAppIconName;
}

bool Utils::isShowAppIcon(const MatchedItem &item)
{
    //特殊搜索项
    static QHash<QString, bool> extendSearcher = {
        {GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, true}
    };
    return isResultFromBuiltSearch(item) || extendSearcher.value(item.searcher, false);
}

QString Utils::getFileMimetype(const QString &filePath)
{
    QString mimetype = getFileMimetypeByQt(filePath);
    if (mimetype.isEmpty())
        mimetype = getFileMimetypeByGio(filePath);

    // 通过gio库和qt库都获取不到mimetype，则默认返回文本类型
    if (mimetype.isEmpty())
        mimetype = "text/plain";

    return mimetype;
}

QString Utils::getFileMimetypeByGio(const QString &path)
{
    GFile *file;
    GFileInfo *info;
    QString result;

    file = g_file_new_for_path(path.toUtf8());
    info = g_file_query_info(file, "standard::content-type", G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
    result = g_file_info_get_content_type(info);

    g_object_unref(file);

    return result;
}

QString Utils::getFileMimetypeByQt(const QString &path)
{
    QString mimetype;

    QFile fi(path);
    // 包含文件名后缀，通过后缀获取mimetype
    if (!QFileInfo(path).suffix().isEmpty()) {
        mimetype = m_mimeDb.mimeTypeForFile(path, QMimeDatabase::MatchExtension).name();
    } else if (fi.size() != 0 && fi.exists()) {
        // 文件名后缀为空，且内容不为空，根据文件内容获取mimetype
        mimetype = m_mimeDb.mimeTypeForFile(path, QMimeDatabase::MatchContent).name();
    } else {
        mimetype = "";
    }

    return mimetype;
}

QString Utils::getDefaultAppDesktopFileByMimeType(const QString &mimeType)
{
    GAppInfo* defaultApp = g_app_info_get_default_for_type(mimeType.toLocal8Bit().constData(), FALSE);
    if(!defaultApp)
        return "";

    const char* desktop_id = g_app_info_get_id(defaultApp);
    GDesktopAppInfo* desktopAppInfo = g_desktop_app_info_new(desktop_id);
    if(!desktopAppInfo) {
        g_object_unref(defaultApp);
        return "";
    }
    QString desktopFile = g_desktop_app_info_get_filename(desktopAppInfo);

    g_object_unref(defaultApp);
    g_object_unref(desktopAppInfo);

    return desktopFile;
}

bool Utils::isResultFromBuiltSearch(const MatchedItem &item)
{
    DEF_BUILTISEARCH_NAMES;
    if (predefBuiltinSearches.contains(item.searcher)) {
        return true;
    }

    return false;
}

bool Utils::openMatchedItem(const MatchedItem &item)
{
    if (Utils::isResetSearcher(item.searcher)) {
        QDateTime time = QDateTime::currentDateTime();
        qint64 timeT = time.toSecsSinceEpoch();
        AccessRecord::instance()->updateRecord(item, timeT);
    }
    bool bFromBuiltSearch = isResultFromBuiltSearch(item);
    if (!bFromBuiltSearch) {
        return openExtendSearchMatchedItem(item);
    }

    if (item.item.isEmpty())
        return false;

    bool result = false;
    if (item.searcher == GRANDSEARCH_CLASS_APP_DESKTOP) {   // 启动应用
        result = Utils::launchApp(item.item);
    } else if (item.searcher == GRANDSEARCH_CLASS_WEB_STATICTEXT) { // 跳转浏览器
        result = Utils::openWithBrowser(item.item);
    } else {    // 打开文件
        result = openFile(item);
    }

    return result;
}

bool Utils::openMatchedItemWithCtrl(const MatchedItem &item)
{
    if (GRANDSEARCH_CLASS_FILE_DEEPIN == item.searcher || GRANDSEARCH_CLASS_FILE_FSEARCH == item.searcher) {
        QFileInfo fileInfo(item.item);
        if (!fileInfo.isDir())
            return openInFileManager(item);
    }

    return openMatchedItem(item);
}

bool Utils::openInFileManager(const MatchedItem &item)
{
    return DDesktopServices::showFileItem(item.item);
}

bool Utils::openExtendSearchMatchedItem(const MatchedItem &item)
{
    // 搜索结果来自扩展插件，使用Dbus通知主控调用扩展插件打开接口打开搜索结果
    DaemonGrandSearchInterface daemonDbus;
    daemonDbus.OpenWithPlugin(item.searcher, item.item);

    return true;
}

bool Utils::openFile(const MatchedItem &item)
{
    QString filePath = item.item;
    if (filePath.isEmpty())
        return false;

    bool result = false;

    // 获取mimetype
    QString mimetype = item.type;
    if (mimetype.isEmpty())
        mimetype = getFileMimetype(item.item);

    // fix bug98384 社区版在打开特殊字符名称的文件夹时，丢失路径导致打开主目录
    QString fileUrlString = QUrl::fromLocalFile(filePath).toString();

    // 获取对应默认打开应用
    QString defaultDesktopFile = getDefaultAppDesktopFileByMimeType(mimetype);
    if (defaultDesktopFile.isEmpty()) {
        result = QProcess::startDetached(QString("dde-file-manager"), {QString("-o"), fileUrlString});
        qDebug() << "open file dialog" << result;
    } else {
        QStringList filePaths(fileUrlString);
        result = launchApp(defaultDesktopFile, filePaths);
    }

    return result;
}

bool Utils::launchApp(const QString& desktopFile, const QStringList &filePaths)
{
    bool ok = launchAppByDBus(desktopFile, filePaths);
    if (!ok) {
        ok = launchAppByGio(desktopFile, filePaths);
    }

    return ok;
}

bool Utils::launchAppByDBus(const QString &desktopFile, const QStringList &filePaths)
{
#ifdef COMPILE_ON_V23
    const auto &file = QFileInfo{desktopFile};
    constexpr auto kDesktopSuffix { u8"desktop" };

    if (file.suffix() != kDesktopSuffix) {
        qDebug() << "invalid desktop file:" << desktopFile << file;
        return false;
    }

    const auto &DBusAppId = DUtil::escapeToObjectPath(file.completeBaseName());
    const auto &currentAppPath = QString { AppManagerPathPrefix } + "/" + DBusAppId;
    qDebug() << "app object path:" << currentAppPath;
    QDBusInterface appManager(AppManagerService,
                              currentAppPath,
                              AppInterface,
                              QDBusConnection::sessionBus());

    auto reply = appManager.callWithArgumentList(QDBus::Block, QStringLiteral("Launch"), { QVariant::fromValue(QString {}), QVariant::fromValue(filePaths), QVariant::fromValue(QVariantMap {}) });

    return reply.type() == QDBusMessage::ReplyMessage;

#else
    QDBusInterface interface(SessionManagerService,
                             StartManagerPath,
                             StartManagerInterface,
                             QDBusConnection::sessionBus(),
                             qApp);

    QList<QVariant> args;
    args << desktopFile
         << QDateTime::currentDateTime().toTime_t()
         << filePaths;

    QDBusPendingCall call = interface.asyncCallWithArgumentList("LaunchApp", args);

    QDBusReply<void> reply = call.reply();
    if (!reply.isValid()) {
            qCritical() << "Launch app by DBus failed:" << reply.error();
            return false;
    }
#endif

    return true;
}

bool Utils::launchAppByGio(const QString &desktopFile, const QStringList &filePaths)
{
    // 使用gio接口启动应用
    std::string stdDesktopFilePath = desktopFile.toStdString();
    const char *cDesktopPath = stdDesktopFilePath.data();

    GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(cDesktopPath);
    if (!appInfo) {
        //qDebug() << "Failed to open desktop file with gio: g_desktop_app_info_new_from_filename returns NULL. Check PATH maybe?";
        return false;
    }

    GList *g_files = nullptr;
    foreach (const QString &filePath, filePaths) {
        std::string stdFilePath = filePath.toStdString();
        const char *cFilePath = stdFilePath.data();
        GFile *f = g_file_new_for_uri(cFilePath);
        g_files = g_list_append(g_files, f);
    }

    GError *gError = nullptr;
    gboolean ok = g_app_info_launch(reinterpret_cast<GAppInfo *>(appInfo), g_files, nullptr, &gError);

    if (gError) {
        qWarning() << "Error when trying to open desktop file with gio:" << gError->message;
        g_error_free(gError);
    }

    if (!ok) {
        qWarning() << "Failed to open desktop file with gio: g_app_info_launch returns false";
    }
    g_object_unref(appInfo);
    g_list_free(g_files);

    for (auto filePath : filePaths) {
        if (!filePath.isEmpty())
            QProcess::startDetached("gio", QStringList() << "open" << filePath);
    }

    return ok;
}

bool Utils::openWithBrowser(const QString &words)
{
    if (words.isEmpty())
        return false;

    // 获取默认浏览器
    QString defaultDesktopFile = defaultBrowser();
    if (defaultDesktopFile.isEmpty()) {
        qWarning() << "no browser to open url.";
        return false;
    }
    //默认浏览器
    qDebug() << "open with" << defaultDesktopFile;
    return launchApp(defaultDesktopFile, {words});
}

QIcon Utils::defaultIcon(const MatchedItem &item)
{
    if (item.searcher == GRANDSEARCH_CLASS_APP_DESKTOP)
        return QIcon::fromTheme("application-x-desktop");
    else if (item.searcher == GRANDSEARCH_CLASS_FILE_DEEPIN || item.searcher == GRANDSEARCH_CLASS_FILE_FSEARCH) {
        return QIcon::fromTheme(m_mimeDb.mimeTypeForFile(item.item).genericIconName());
    } else if (item.searcher == GRANDSEARCH_CLASS_WEB_STATICTEXT) {
        // 使用默认浏览器的图标
        QString iconName = appIconName(item);
        if (QFile::exists(iconName))
            return QIcon(iconName);
        else
            return QIcon::fromTheme(iconName, QIcon::fromTheme("application-x-desktop"));
    }

    return QIcon();
}

QString Utils::defaultBrowser()
{
    static QString mimetype = "x-scheme-handler/http";
    QString defaultDesktopFile = getDefaultAppDesktopFileByMimeType(mimetype);
    return defaultDesktopFile;
}

bool Utils::isLevelItem(const MatchedItem &item, int &level)
{
    if (QVariant::Hash == item.extra.type() && item.extra.toHash().keys().contains(GRANDSEARCH_PROPERTY_ITEM_LEVEL)) {
        bool ok = false;
        level = item.extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_LEVEL).toInt(&ok);
        return ok;
    }
    return false;
}

bool Utils::isLevelGroup(const QString &searchGroupName)
{
    static const QStringList containLevelGroup{
        GRANDSEARCH_GROUP_FILE_VIDEO
      , GRANDSEARCH_GROUP_FILE_AUDIO
      , GRANDSEARCH_GROUP_FILE_PICTURE
      , GRANDSEARCH_GROUP_FILE_DOCUMNET
      , GRANDSEARCH_GROUP_FILE
    };

    return containLevelGroup.contains(searchGroupName);
}

bool Utils::canPreview(const QString &searchGroupName)
{
    static const QStringList containPreviewGroup{
        GRANDSEARCH_GROUP_FOLDER
      , GRANDSEARCH_GROUP_FILE
      , GRANDSEARCH_GROUP_FILE_VIDEO
      , GRANDSEARCH_GROUP_FILE_AUDIO
      , GRANDSEARCH_GROUP_FILE_PICTURE
      , GRANDSEARCH_GROUP_FILE_DOCUMNET
      , GRANDSEARCH_GROUP_FILE_INFERENCE
    };

    return containPreviewGroup.contains(searchGroupName);
}

QString Utils::iconThemeSuffix()
{
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        return QString("-light");
    return QString("-dark");
}

bool Utils::isWayland()
{
    //! 该函数只能在QApplication之后调用才能返回有效的值，在此之前platformName会返回空值
    Q_ASSERT(qApp);

    static bool wayland = QApplication::platformName() == "wayland";

    return wayland;
}
