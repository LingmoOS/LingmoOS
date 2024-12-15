// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <DLabel>

#include <QMenu>
#include <QAction>
#include <QFileInfo>
#include <QDateTime>
#include <QPropertyAnimation>
#include <QShortcut>

#include <dprintpreviewwidget.h>
#include <dprintpreviewdialog.h>
DWIDGET_USE_NAMESPACE

#define THUMBNAIL_PIXMAP_SIZE 50//缩略图图像缩放前尺寸,考虑了内存占用和清晰度两方面情况的取值
#define SELECTED_WIDTH 58//选中的缩略图图元宽高

#define ANIMATION_DURATION 160 //动画时长

//默认触发一次，避免长按一直触发快捷键槽函数，引起的程序异常
class QShortcutEx: public QShortcut
{
    Q_OBJECT
public:
    QShortcutEx(const QKeySequence& key, QWidget *parent,
                const char *member = nullptr, const char *ambiguousMember = nullptr,
                Qt::ShortcutContext context = Qt::WindowShortcut);
};

/**
 * @brief AnimationWidget 文件更新动画的上层控件
 * 用于实现图像预览区域文件更新的动画效果
 */
class AnimationWidget : public DLabel
{
    Q_OBJECT
public:
    AnimationWidget(QPixmap pixmap, QWidget *parent = nullptr);
    ~AnimationWidget();
    /**
     * @brief setPixmap 更新缩略图
     * @param pixmap
     */
    void setPixmap(QPixmap pixmap)
    {
        m_animatePix = pixmap;
    }
    /**
     * @brief getPixmap 获取缩略图
     * @return 缩略图
     */
    QPixmap getPixmap()
    {
        return m_animatePix;
    }

protected:
    /**
     * @brief paintEvent 控件绘制事件
     */
    void paintEvent(QPaintEvent *e) override;

private:
    QPixmap m_animatePix;      //缩略图
};

/**
 * @brief ImageItem 处理图片视频相关的内容
 * 图片路径获取与设置，视频时长，图片单选多选，打印，删除，鼠标事件
 * 若为图片有打印功能，视频没有
 */
class ImageItem : public DLabel
{
    Q_OBJECT
public:
    ImageItem(QWidget *parent = nullptr);

    ~ImageItem();

    /**
     * @brief updatePicPath 更新文件路径
     * @param filePath 文件路径
     */
    void updatePicPath(const QString &filePath);

    /**
     * @brief updatePic 更新图片
     * @param pixmap
     */
    void updatePic(QPixmap pixmap);

    /**
     * @brief openFile 打开文件
     */
    void openFile();

signals:
    /**
     * @brief trashFile 删除文件信号
     * @param filePath  删除文件路径
     */
    void trashFile(const QString &filePath);

public slots:
    /**
     * @brief onPrint 打印信号
     */
    void onPrint();

    /**
     * @brief showMenu 显示菜单
     */
    void onShowMenu();

    /**
     * @brief onOpenFolder 打开文件夹
     */
    void onOpenFolder();

    /**
    * @brief onCopy　右键菜单复制
    */
    void onCopy();

    /**
    * @brief onShortcutDel　delete按下
    */
    void onShortcutDel();

protected:
    /**
     * @brief paintEvent 控件绘制事件
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief mouseReleaseEvent 鼠标按下事件
     * @param event
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief mouseMoveEvent 鼠标移动事件
     * @param event
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief mouseReleaseEvent 鼠标释放事件
     * @param event
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * @brief focusInEvent 焦点事件
     * @param event
     */
    void focusInEvent(QFocusEvent *event) override;

    /**
     * @brief focusOutEvent 失去焦点事件
     * @param event
     */
    void focusOutEvent(QFocusEvent *event) override;

private slots:
#ifndef UNITTEST
#if (DTK_VERSION_MAJOR > 5 \
    || (DTK_VERSION_MAJOR >=5 && DTK_VERSION_MINOR > 4) \
    || (DTK_VERSION_MAJOR >= 5 && DTK_VERSION_MINOR >= 4 && DTK_VERSION_PATCH >= 10))//5.4.7暂时没有合入
    /**
     * @brief paintRequestedAsyn 同步打印
     * @param _printer 打印机
     * @param pageRange 页数
     */
    void paintRequestedAsyn(DPrinter *_printer, const QVector<int> &pageRange);
#endif

    /**
     * @brief paintRequestSync 异步打印
     * @param _printer 打印机
     */
    void paintRequestSync(DPrinter *_printer);
#endif
private:
    /**
     * @brief initShortcut 初始化快捷键
     */
    void initShortcut();

    /**
     * @brief showPrintDialog 显示打印对话框
     */
    void showPrintDialog(const QStringList &paths, QWidget *parent);

private:
    bool                m_bVideo;//是否视频
    QList<QImage>       m_imgs;//需要打印的图片
    QString             m_path;//文件路径
    QPixmap             m_pixmap;//缩略图
    QString             m_pixmapstring;//默认缩略图路径
    QMenu               *m_menu;//右键菜单
    QAction             *m_actCopy;//复制
    QAction             *m_actDel;//删除
    QAction             *m_actOpenFolder;//打开文件夹
    QAction             *m_actPrint;//打印
    QDateTime           m_lastDelTime;//最后一次删除文件时间，避免过快删除导致显示空白
    AnimationWidget     *m_pAniWidget;//动画控件
    QPropertyAnimation  *m_pAnimation;

    bool                m_bFocus;
};

#endif // IMAGEITEM_H
