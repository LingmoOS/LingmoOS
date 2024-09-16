// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ITEMBASEWIDGET_H
#define ITEMBASEWIDGET_H
#include <DWidget>
#include <DLabel>

#include <QDateTime>
#include <QPointer>

#include "itemdata.h"
#include "iconbutton.h"

DWIDGET_USE_NAMESPACE

class QTimer;
class QVBoxLayout;
class PixmapLabel;
/*!
 * \~chinese \class ItemWidget
 * \~chinese \brief 剪切块窗口类,继承于DWidget
 */
class ItemWidget : public DWidget
{
    Q_OBJECT
    Q_PROPERTY(double opacity READ getOpacity WRITE setOpacity)
public:
    ItemWidget(QPointer<ItemData> data, QWidget *parent = nullptr);

    /*!
     * \~chinese \brief 设置剪切块属性的接口
     */
    const QString& text();
    void setTextShown(const QString &length);

    void setThumnail(const QPixmap &pixmap/*未经处理的原图*/);
    void setThumnail(const FileIconData &data);

    void setFileIcon(const QPixmap &pixmap/*未经处理的原图*/);                     //单个文件
    void setFileIcon(const FileIconData &data);

    void setFileIcons(const QList<QPixmap> &list);                              //多个文件

    void setClipType(const QString &text);                                      //剪贴类型
    void setCreateTime(const QDateTime &time);

    /*!
     * \~chinese \brief 获取与设置剪切块样式的接口
     */
    void setAlpha(int alpha);

    int hoverAlpha() const;
    void setHoverAlpha(int alpha);

    int unHoverAlpha() const;
    void setUnHoverAlpha(int alpha);

    int radius() {return m_radius;}
    void setRadius(int radius);

    void setOpacity(double opacity);

    const QPointer<ItemData> itemData();

    static QList<QRectF> getCornerGeometryList(const QRectF &baseRect, const QSizeF &cornerSize);
    static QPixmap getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio, QIcon::Mode mode, QIcon::State state);
    static QPixmap GetFileIcon(QString path);
    static QPixmap GetFileIcon(const FileIconData &data);

Q_SIGNALS:
    void close();
    /*!
     * \~chinese \name hoverStateChanged
     * \~chinese \brief 当鼠标或焦点移入移出该控件时发出该信号
     */
    void hoverStateChanged(bool);
    /*!
     * \~chinese \name closeHasFocus
     * \~chinese \brief 通知别人，关闭按钮的‘焦点’状态改变了
     */
    void closeHasFocus(bool has);

public Q_SLOTS:
    /*!
     * \~chinese \name onHoverStateChanged
     * \~chinese \brief 当鼠标或焦点悬停状态改变时执行该槽函数
     */
    void onHoverStateChanged(bool hover);

private Q_SLOTS:
    /*!
     * \~chinese \name onRefreshTime
     * \~chinese \brief 刷新复制时间的定时器达到设定的超时时间后就会发出timeout信号，timeout信号关联到了这个槽函数，这里就会执行
     */
    void onRefreshTime();
    void onClose();

private:
    /*!
     * \~chinese \name initUI
     * \~chinese \brief 初始化剪切块界面布局
     */
    void initUI();
    /*!
     * \~chinese \name initData
     * \~chinese \brief 初始化剪切块窗口中的数据
     * \~chinese \param 当前剪切块的数据
     */
    void initData(QPointer<ItemData> data);
    /*!
     * \~chinese \name initConnect
     * \~chinese \brief 初始化信号的连接
     */
    void initConnect();

    /*!
     * \~chinese \name CreateTimeString
     * \~chinese \brief 创建复制时间的字符串
     * \~chinese \param 剪切块创建的时间
     * \~chinese \return 返回创建时间的字符串
     */
    QString CreateTimeString(const QDateTime &time);

    double getOpacity() const { return 0.0; }

    /*!
     * \~chinese \name onSelect
     * \~chinese \brief 当内容被选中时执行的函数
     */
    void onSelect();


private:
    QPointer<ItemData> m_data;

    // title
    DLabel *m_nameLabel = nullptr;
    DLabel *m_timeLabel = nullptr;
    IconButton *m_closeButton = nullptr;

    // content
    PixmapLabel *m_contentLabel = nullptr;
    DLabel *m_statusLabel = nullptr;

    QTimer *m_refreshTimer = nullptr;

    //--- data
    QPixmap m_pixmap;       //显示的缩略图原图
    QDateTime m_createTime;

    //--- set style
    int m_radius = 0;
    int m_hoverAlpha = 0;
    int m_unHoverAlpha = 0;
    bool m_havor = false;
    bool m_closeFocus = false;  //关闭按钮是否置于选中状态
    bool m_destroy = false;

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void focusInEvent(QFocusEvent *event) override;
    virtual void focusOutEvent(QFocusEvent *event) override;
    virtual bool eventFilter(QObject *watcher, QEvent *event) override;
};
#endif // ITEMBASEWIDGET_H
