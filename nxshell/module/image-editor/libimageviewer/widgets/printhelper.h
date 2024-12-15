// SPDX-FileCopyrightText: 2020 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PRINTHELPER_H
#define PRINTHELPER_H

#include <QObject>

#include <dprintpreviewwidget.h>
#include <dprintpreviewdialog.h>
DWIDGET_USE_NAMESPACE

//重构printhelper，因为dtk更新
//绘制图片处理类
class RequestedSlot : public QObject
{
    Q_OBJECT

public:
    explicit RequestedSlot(QObject *parent = nullptr);
    ~RequestedSlot();

    // 清理打印状态，当前是否触发打印
    void clearPrintState();
    inline bool isPrinted() { return m_printed; }

    void setPaths(const QStringList &paths);
    void appendImage(const QImage &img);

private:
    Q_SLOT void paintRequestSync(DPrinter *_printer);

private:
    QStringList m_paths;
    QList<QImage> m_imgs;
    bool m_printed = false;
};

class PrintHelper : public QObject
{
    Q_OBJECT

public:
    static PrintHelper *getIntance();
    ~PrintHelper();

    void showPrintDialog(const QStringList &paths, QWidget *parent = nullptr);

    RequestedSlot *m_re = nullptr;

private:
    explicit PrintHelper(QObject *parent = nullptr);
    static PrintHelper *m_Printer;
};

#endif  // PRINTHELPER_H
