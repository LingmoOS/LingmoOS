// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWMOREBUTTON_H
#define VIEWMOREBUTTON_H

#include <QToolButton>

namespace GrandSearch {

class ViewMoreButton : public QToolButton
{
    Q_OBJECT
public:
    explicit ViewMoreButton(const QString &text, QWidget *parent = nullptr);
    void setSelected(bool selected);
    bool isSelected();

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    bool m_bIsSelected = false;
};

}

#endif // VIEWMOREBUTTON_H
