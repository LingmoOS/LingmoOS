// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PIXLABEL_H
#define PIXLABEL_H

#include <QLabel>

class PixLabel : public QLabel
{
    Q_OBJECT

public:
    explicit PixLabel(QWidget *parent = nullptr);
    void setContent(const QPixmap pix = QPixmap(), const QString str = QString());

protected:
    virtual void paintEvent(QPaintEvent *) override;

private:
    QPixmap m_pixmap;
    QString m_text;
};

#endif // PIXLABEL_H
