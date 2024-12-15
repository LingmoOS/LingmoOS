// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UPDATECONTENT_H
#define UPDATECONTENT_H

#include <QWidget>
#include <QPushButton>

class UpdateContent : public QWidget
{
    Q_OBJECT
public:
    explicit UpdateContent(const std::pair<QString, QString> &version, QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QPushButton *m_enterBtn;
};

#endif // UPDATECONTENT_H
