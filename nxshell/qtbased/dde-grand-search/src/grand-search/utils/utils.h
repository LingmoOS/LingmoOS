// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SORTMATCHITEM_H
#define SORTMATCHITEM_H

#include "global/matcheditem.h"
#include "global/builtinsearch.h"

#include <QStringList>

namespace GrandSearch {

class Utils
{
public:
    // 排序算法 用于搜索结果排序 规则为：中文 > 英文 > 其他语言 > 标点符号
    static bool sort(MatchedItems &list, Qt::SortOrder order = Qt::AscendingOrder);

    static bool compareByString(QString str1, QString str2, Qt::SortOrder order = Qt::AscendingOrder);
    static bool startWithSymbol(const QString &text);
    static bool startWithHanzi(const QString &text);
    static bool startWithLatin(const QString &text);
    static bool startWithOtherLang(const QString &text);
    static bool startWidthNum(const QString &text);

    // 通过权重进行排序
    static bool sortByWeight(MatchedItemMap &map, Qt::SortOrder order = Qt::DescendingOrder);
    static inline bool sortByWeight(MatchedItems &list, Qt::SortOrder order = Qt::DescendingOrder) {
        std::stable_sort(list.begin(), list.end(), [order](const MatchedItem &node1, const MatchedItem &node2){
            return compareByWeight(node1, node2, order);
        });

        return true;
    }

    static bool compareByWeight(const MatchedItem &node1, const MatchedItem &node2, Qt::SortOrder order = Qt::DescendingOrder);

    // 更新结果项的权重
    static void updateItemsWeight(MatchedItemMap &map, const QString &content);

    // 设置权重函数
    static bool setWeightMethod(MatchedItem &item);

    // 计算文件类的权重
    static double calcFileWeight(const QString &path, const QString &name, const QStringList &keys);
    static qint64 calcDateDiff(const QDateTime &date1, const QDateTime &date2);
    static double calcWeightByDateDiff(const qint64 &diff, const int &type);

    // 计算应用和设置的权重
    static double calcAppWeight(const MatchedItem &item, const QStringList &keys);
    static double calcSettingWeight(const MatchedItem &item, const QStringList &keys);

    // 组装最佳匹配类目
    static void packageBestMatch(MatchedItemMap &map, int maxQuantity);

    // 计算点选事件的权重
    static double calcRecordWeight(const MatchedItem &item);

    // 判断类目所属搜索项是否支持重排
    static bool isResetSearcher(QString searcher);

    // 获取搜索结果默认打开应用图标
    static QString appIconName(const MatchedItem &item);
    static bool isShowAppIcon(const MatchedItem &item);

    // 根据文件路径获取mimetype
    static QString getFileMimetype(const QString &filePath);
    // 通过Gio接口获取mimeType
    static QString getFileMimetypeByGio(const QString &path);
    // 通过Qt接口获取mimeType
    static QString getFileMimetypeByQt(const QString &path);
    // 根据mimeType获取默认打开应用路径
    static QString getDefaultAppDesktopFileByMimeType(const QString &mimeType);
    // 搜索结果是否来自内置搜索项
    static bool isResultFromBuiltSearch(const MatchedItem &item);
    // 打开搜索结果项，如扩展搜索项/应用/文件/浏览器等
    static bool openMatchedItem(const MatchedItem &item);
    // 按住Ctrl键时打开搜索项
    static bool openMatchedItemWithCtrl(const MatchedItem &item);
    // 在文件管理器中显示
    static bool openInFileManager(const MatchedItem &item);
    // 打开扩展搜索项
    static bool openExtendSearchMatchedItem(const MatchedItem &item);
    // 打开文件
    static bool openFile(const MatchedItem &item);
    // 启动应用，若filePaths不为空，则用该应用打开传入的文件, 否则，仅启动应用
    static bool launchApp(const QString& desktopFile, const QStringList &filePaths = {});
    // 使用Dbus启动应用
    static bool launchAppByDBus(const QString &desktopFile, const QStringList &filePaths = {});
    // 使用gio启动应用
    static bool launchAppByGio(const QString &desktopFile, const QStringList &filePaths = {});
    // 跳转到浏览器
    static bool openWithBrowser(const QString &words);
    // 列表左边图标
    static QIcon defaultIcon(const MatchedItem &item);

    // 默认浏览器的desktop文件
    static QString defaultBrowser();
    /*!
     * \brief isLevelItem 判断item是否属于分层项，并输出具体的层级
     * \param item 待判断的搜索结果项
     * \param level 输出参数，item所属的层级
     * \return 项item是否属于分层项
     */
    static bool isLevelItem(const MatchedItem &item, int &level);

    // 返回该组是否包分层显示项
    static bool isLevelGroup(const QString &searchGroupName);

    // 返回该组是否支持预览
    static bool canPreview(const QString &searchGroupName);

    // 根据当前主题返回合适的图标后缀（-light or -dark）
    static QString iconThemeSuffix();

    static bool isWayland();

private:
    static QMap<QString, QString> m_appIconNameMap;// 存放应用desktop文件对应的图标名称，用于搜索框应用图标刷新
    static QMimeDatabase m_mimeDb;
};

}

#endif // SORTMATCHITEM_H
