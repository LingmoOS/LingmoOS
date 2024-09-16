// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTENTTITLE_H
#define CONTENTTITLE_H

#include <QWidget>
#include <QLabel>

class ContentTitle : public QWidget
{
    Q_OBJECT
public:
    explicit ContentTitle(QWidget *parent = nullptr);
    QLabel *label() const;

public :
    void setTitle(const QString &title);

protected:
    QLabel *titleLabel = nullptr;
};

#endif // CONTENTTITLE_H
