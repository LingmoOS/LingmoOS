/*

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef KGPGMD5WIDGET_H
#define KGPGMD5WIDGET_H

#include <QDialog>
#include <QUrl>

class QLabel;

class KLed;

class Md5Widget : public QDialog
{
    Q_OBJECT

public:
    explicit Md5Widget(QWidget *parent = nullptr, const QUrl &url = QUrl());

public Q_SLOTS:
    void slotApply();

private:
    QString m_md5sum;
    QLabel *m_label;
    KLed *m_led;
};

#endif // KGPGMD5WIDGET_H
