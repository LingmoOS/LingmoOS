// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIPSLABEL_H
#define TIPSLABEL_H

#include <QLabel>

namespace GrandSearch {

class TipsLabel : public QLabel
{
    Q_OBJECT
public:
    explicit TipsLabel(const QString &text, QWidget *parent);
protected slots:
    void onThemeChanged();
};

}
#endif // TIPSLABEL_H
