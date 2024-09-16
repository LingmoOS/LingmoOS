// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QUICKPRINT_P_H
#define QUICKPRINT_P_H

#include "quickprint.h"
#include "printimageloader.h"

#include <DSpinner>
#include <DDialog>
#include <DPrintPreviewDialog>

#include <QBasicTimer>

DWIDGET_USE_NAMESPACE

class QuickPrintPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QuickPrint)
    Q_DISABLE_COPY(QuickPrintPrivate)

public:
    explicit QuickPrintPrivate(QuickPrint *q);

    // 界面交互
    void startLoadSpinnerTimer();
    void startSpinner();
    void stopSpinner();
    void showWarningNotify(const QString &errorString);
    int showPrintDialog(QWidget *parentWidget = nullptr);

    // 数据加载设置
    bool setPrintImage(const QStringList &fileList);
    bool checkNeedAsyncLoadData(const QStringList &fileList);

    // 后端数据加载完成
    Q_SLOT void printLoadFinished(bool error, const QString &errorString);

    // 等待处理完成，对话框处理完成后调用
    Q_SIGNAL void notifyLoadFinished(int ret, bool error);
    bool waitLoadFinished();

    // 图片打印绘制
    Q_SLOT void asyncPrint(DPrinter *printer, const QVector<int> &pageRange);

protected:
    void timerEvent(QTimerEvent *e) override;

public:
    QuickPrint *q_ptr;
    QWidget *parentWidget = nullptr;  // 打印预览父窗口

    QBasicTimer procSpinnerTimer;      // 处理时间超过500ms，显示加载动画
    QScopedPointer<DSpinner> spinner;  // 加载动画

    PrintDataList loadDataList;                    // 加载的图片信息
    QScopedPointer<PrintImageLoader> imageLoader;  // 图像数据加载器
};

#endif  // QUICKPRINT_P_H
