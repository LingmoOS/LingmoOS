// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file
 * 此文件为影院播放截图相关。
 */
#ifndef _DMR_BURST_SCREENSHOTS_DIALOG_H
#define _DMR_BURST_SCREENSHOTS_DIALOG_H

#include <QtWidgets>
#include <DDialog>
#include <DTitlebar>

DWIDGET_USE_NAMESPACE

namespace dmr {
class PlayItemInfo;

/**
 * @brief The ThumbnailFrame class
 * 单个截图图像窗口
 */
class ThumbnailFrame: public QLabel
{
    Q_OBJECT
public:
    /**
     * @brief ThumbnailFrame 构造函数
     * @param parent 父窗口
     */
    explicit ThumbnailFrame(QWidget *parent) : QLabel(parent)
    {
        //参考设计图
        setFixedSize(178, 100);
        QGraphicsDropShadowEffect *pGraphicsShadow = new QGraphicsDropShadowEffect(this);
        pGraphicsShadow->setColor(QColor(0, 0, 0, 255 * 2 / 10));
        //参考设计图
        pGraphicsShadow->setOffset(0, 2);
        //参考设计图
        pGraphicsShadow->setBlurRadius(4);
        setGraphicsEffect(pGraphicsShadow);
    }
};

/**
 * @brief The BurstScreenshotsDialog class
 * 截图窗口
 */
class BurstScreenshotsDialog: public DAbstractDialog
{
    Q_OBJECT
public:
    /**
     * @brief BurstScreenshotsDialog 构造函数
     * @param strPlayItemInfo 播放项信息
     */
    explicit BurstScreenshotsDialog(const PlayItemInfo &strPlayItemInfo);
    /**
     * @brief updateWithFrames 更新截图图像
     * @param frames 截图图像
     */
    void updateWithFrames(const QList<QPair<QImage, qint64>> &frames);
    /**
     * @brief savedPosterPath 保存截图路径
     * @return 返回设置的截图保存路径
     */
    QString savedPosterPath();

public slots:
    /**
     * @brief exec 返回执行函数的标识符
     * @return 执行函数的标识符
     */
    int exec() override;
    /**
     * @brief savePoster 保存截图
     */
    void savePoster();

private:
    QGridLayout             *m_pGrid;       ///截图窗口布局
    QPushButton             *m_pSaveBtn;    ///截图保存按键
    QString                  m_sPosterPath; ///截图保存路径
    DTitlebar               *m_pTitlebar;   ///截图窗口标题栏
};
}

#endif /* ifndef _DMR_BURST_SCREENSHOTS_DIALOG_H */
