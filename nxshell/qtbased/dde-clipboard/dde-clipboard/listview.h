// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LISTVIEW_H
#define LISTVIEW_H
#include <QListView>
#include <QPointer>

/*!
 * \~chinese \class ListView
 * \~chinese \brief 继承于QListView,将剪切块以列表的形式展示出来
 */
class ListView : public QListView
{
    Q_OBJECT
public:
    explicit ListView(QWidget *parent = nullptr);

    /*!
     * \~chinese \name keyPressEvent
     * \~chinese \brief 重写父类的keyPressEvent方法实现上下方向键切换剪贴块。
     */
    virtual void keyPressEvent(QKeyEvent *event) override;
    /*!
     * \~chinese \name mouseMoveEvent
     * \~chinese \brief 重写父类的mouseMoveEvent方法,鼠标移动到剪贴块上方时设置剪切块的索引
     */
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    /*!
     * \~chinese \name showEvent
     * \~chinese \brief 重写父类的showEvent方法使ListView在显示的时候便获得焦点，从而能够
     * \~chinese 让应用显示时可以直接使用上下方向键切换剪贴块，而不是通过鼠标点击获得焦点后才
     * \~chinese 可以使用方向键。
     */
    virtual void showEvent(QShowEvent *event) override;
    virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;

    void startAni(int index);
    bool CreateAnimation(int idx);

Q_SIGNALS:
    void extract(const QModelIndex &index);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;

private:
    bool m_mousePressed;
    QPointer<QMimeData> m_mimeData;
};

#endif // LISTVIEW_H
