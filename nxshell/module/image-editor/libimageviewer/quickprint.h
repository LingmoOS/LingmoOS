// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QUICKPRINT_H
#define QUICKPRINT_H

#include "image-viewer_global.h"
#include <QObject>

class QuickPrintPrivate;
class IMAGEVIEWERSHARED_EXPORT QuickPrint : public QObject
{
    Q_OBJECT
public:
    explicit QuickPrint(QObject *parent = nullptr);
    ~QuickPrint() override;

    // 加载图片并弹出打印窗口，使用 EventLoop 等待
    bool showPrintDialog(const QStringList &fileList, QWidget *parentWidget = nullptr);
    // 加载图片并弹出打印窗口，异步加载，处理后弹出打印预览窗口，通过 printFinsihed() 可以判断打印是否结束
    bool showPrintDialogAsync(const QStringList &fileList, QWidget *parentWidget = nullptr);
    // 当前是否仍在加载状态
    bool isLoading();
    // 中断加载，不会关闭已弹出的打印预览框
    void cancel();

    // 打印结束信号
    Q_SIGNAL void printFinished(int ret);

private:
    QScopedPointer<QuickPrintPrivate> dd_ptr;
    Q_DECLARE_PRIVATE_D(dd_ptr, QuickPrint)
    Q_DISABLE_COPY(QuickPrint)
};

// C-Style API
extern "C" {
bool quickPrintDialog(const QStringList &fileList, QWidget *parentWidget);
};

#endif  // QUICKPRINT_H
