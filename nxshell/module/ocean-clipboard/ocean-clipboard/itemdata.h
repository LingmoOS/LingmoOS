// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ITEMDATA_H
#define ITEMDATA_H

#include <QDateTime>
#include <QIcon>
#include <QMimeData>
#include <QObject>
#include <QPixmap>
#include <QUrl>
#include <QDBusArgument>
#include <QTextLayout>
#include "constants.h"
#include "dbus/iteminfo.h"

/*!
 * ~chinese \class ItemData
 * ~chinese \brief 存放每个剪切块中的数据
 */
class ItemData : public QObject
{
    Q_OBJECT
public:
    explicit ItemData(const QByteArray &buf);

    /*!
     * \~chinese \brief 提供剪切块属性的接口
     */
    QString title();                            // 类型名称
    QString subTitle();                         // 字符数，像素信息，文件名称（多个文件显示XXX等X个文件）
    const QList<QUrl> &urls();                  // 文件链接
    const QDateTime &time();                    // 复制时间
    const QString &text();                      // 内容预览
    QStringList get_text() const {
        return m_text_list;
    };
    QSize sizeHint(int height);

    int itemHeight(int fontHeight);
    inline bool dataEnabled() { return m_enable; }
    void setDataEnabled(bool enable) { m_enable = enable; }

    void setPixmap(const QPixmap &pixmap);
    QPixmap pixmap();                           // 缩略图
    const DataType &type() {return m_type;}
    const QVariant &imageData();
    const QMap<QString, QByteArray> &formatMap();
    void saveFileIcons(const QList<QPixmap> &list);
    const QList<QPixmap> &FileIcons();          //IconDataList没有数据时再使用FileIcons
    const QList<FileIconData> &IconDataList();  //优先使用IconDataList
    const QSize &pixSize() const;               //返回m_variantImage中pixmap原始size

    void remove();
    void popTop();

Q_SIGNALS:
    /*!
     * \~chinese \name destroy
     * \~chinese \brief 点击剪切块上的关闭按钮时会调用ItemData::remove函数,产生该信号后关联的槽函数
     * \~chinese ClipboardModel::destroy会被执行删除数据
     * \~chinese \param data 需要删除的数据
     */
    void destroy(ItemData *data);
    /*!
     * \~chinese \name reborn
     * \~chinese \brief 双击剪切块时会发出该信号,会执行ItemData::popTop函数,发出ItemData::reborn
     * \~chinese 的信号,产生该信号后会将当前数据项置顶
     * \~chinese \param data 需要置顶的数据
     */
    void reborn(ItemData *data);

private:
    QMap<QString, QByteArray> m_formatMap;
    DataType m_type = Unknown;
    QList<QUrl> m_urls;
    QVariant m_variantImage;
    QSize m_pixSize;
    QString m_text;
    QStringList m_text_list;
    bool m_enable;
    QDateTime m_createTime;
    QList<FileIconData> m_iconDataList;
    QPixmap m_thumnail;
    QList<QPixmap> m_fileIcons;
};
#endif // ITEMDATA_H
