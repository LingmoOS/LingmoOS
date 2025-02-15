/*
    SPDX-FileCopyrightText: 2009 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KIMAGEFRAME_H
#define KIMAGEFRAME_H

#include <QFrame>

class KImageFrame : public QFrame
{
    Q_OBJECT
public:
    KImageFrame(QWidget *parent = nullptr);
    ~KImageFrame() override
    {
    }

public Q_SLOTS:
    void setImage(const QImage &);

protected Q_SLOTS:
    void paintEvent(QPaintEvent *) override;

protected:
    QImage _img;
    int _w, _h;
};

#endif
