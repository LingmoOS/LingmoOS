/*
    SPDX-FileCopyrightText: 2009 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCOLORUTILSDEMO_H
#define KCOLORUTILSDEMO_H

#include "ui_kcolorutilsdemo.h"

class KColorUtilsDemo : public QWidget, Ui::form
{
    Q_OBJECT
public:
    KColorUtilsDemo(QWidget *parent = nullptr);
    ~KColorUtilsDemo() override
    {
    }

public Q_SLOTS:
    void inputChanged();
    void lumaChanged();
    void mixChanged();
    void shadeChanged();

    void inputSpinChanged();
    void inputSwatchChanged(const QColor &);

    void targetSpinChanged();
    void targetSwatchChanged(const QColor &);

protected:
    QImage _leOutImg, _mtMixOutImg, _mtTintOutImg;
    bool _noUpdate;
};

#endif
