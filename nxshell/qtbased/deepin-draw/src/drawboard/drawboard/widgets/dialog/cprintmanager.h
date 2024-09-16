// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPRINTMANAGER_H
#define CPRINTMANAGER_H
#include <globaldefine.h>

#include <QWidget>
#include <QPixmap>


#ifdef USE_DTK
#include <dprintpreviewdialog.h>
DWIDGET_USE_NAMESPACE
#endif

class QPrinter;

class DRAWLIB_EXPORT CPrintManager : public QObject
{
    Q_OBJECT

public:
    explicit CPrintManager(QObject *parent = nullptr);
    ~CPrintManager();

    void showPrintDialog(const QImage &image, QWidget *widget, const QString &title, QWidget *parent = nullptr);

private slots:
#ifdef USE_DTK
    void slotPaintRequest(DPrinter *_printer);
#else
    void slotPaintRequest(QPrinter *_printer);
#endif
private:
    QImage m_image;
};

#endif // CPRINTMANAGER_H
