// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLIPBOARDMODEL_H
#define CLIPBOARDMODEL_H

#include <QAbstractListModel>

#include "listview.h"
#include "itemdata.h"
#include "dbus/clipboardloaderinterface.h"

using ClipboardLoader = com::deepin::dde::ClipboardLoader;
/*!
 * \~chinese \class ClipboardModel
 * \~chinese \brief 继承于QAbstractListModel,这个类定义了用于views和delegates访问数据的接口。
 * \~chinese 剪贴板因为使用了QListView作为基类显示剪贴块，这里选择QAbstractListModel。ClipboardModel
 * \~chinese 类对系统剪贴板进行监听，若系统剪贴板发生变化，则通知ListView进行数据的更新。
 */
class ClipboardModel : public QAbstractListModel
{
    friend class TstListView_uiTest_Test;
    Q_OBJECT
public:
    explicit ClipboardModel(ListView *list, QObject *parent = nullptr);

    /*!
     * \~chinese \name data
     * \~chinese \brief 返回从系统剪切板获取到的数据
     * \~chinese \return 返回存放数据的容器
     */
    const QList<ItemData *> data();

public Q_SLOTS:
    /*!
     * \~chinese \name clear
     * \~chinese \brief 点击全部清除按钮时清除剪切板中的全部数据全部数据,当鼠标点击全部清除按钮时IconButton::clicked
     * \~chinese 信号发出,关联的槽函数ClipboardModel::clear被执行
     */
    void clear();
    /*!
     * \~chinese \name destroy
     * \~chinese \brief 清除当前剪切块的数据,当点击剪切块上的关闭按钮时发出IconButton::clicked信号,ItemData::remove()
     * \~chinese 函数被执行,发送ItemDatadestroy信号,关联的槽函数ClipboardModel::destroy被执行
     * \~chinese \param data需要清理的数据项
     */
    void destroy(ItemData *data);
    /*!
     * \~chinese \name reborn
     * \~chinese \brief 将当前剪切块的数据删除后,重新插入到第一个.当ItemWidget::mouseDoubleClickEvent事
     * \~chinese 件产生时,ItemData::popTop()函数执行,发送出ItemData::reborn信号,ItemData::reborn信号
     * \~chinese 关联到的槽函数ClipboardModel::reborn被执行
     * \~chinese \param 当前剪切块的数据
     */
    void reborn(ItemData *data);

Q_SIGNALS:
    /*!
     * \~chinese \name dataChanged
     * \~chinese \brief 当系统剪切板中的数据改变槽函数clipDataChanged被执行,clipDataChanged执行后发送出该信号
     */
    void dataChanged();
    /*!
     * \~chinese \name dataChanged
     * \~chinese \brief 当鼠标双击事件mouseDoubleClickEvent发生时,调用ItemData::popTop函数,在popTop函数中发出
     * \~chinese ItemData::Reborn信号,ClipboardModel::reborn函数执行,ClipboardModel::reborn函数中发送
     * \~chinese ClipboardModel::rdataReborn信号
     */
    void dataReborn();

private:
    void checkDbusConnect();

protected:
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

protected slots:
    /*!
     * \~chinese \name dataComing
     * \~chinese \brief 当系统剪切块中的数据发生改变时,该槽函数被执行
     */
    void dataComing(const QByteArray &buf);

private:
    QList<ItemData *> m_data;
    ListView *m_list;
    ClipboardLoader *m_loaderInter;
};

#endif // CLIPBOARDMODEL_H
