// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H

#include <QEvent>

//#include "controller/viewerthememanager.h"
#include <dimagebutton.h>
#include <DLabel>

using namespace Dtk::Widget;
typedef DLabel QLbtoDLabel;

class ImageButton : public DImageButton
{
    Q_OBJECT
public:
    explicit ImageButton(QWidget *parent = nullptr);
    explicit ImageButton(const QString &normalPic, const QString &hoverPic,
                         const QString &pressPic, const QString &disablePic,
                         QWidget *parent = nullptr);

    void setDisabled(bool d);

    void setTooltipVisible(bool visible);

    inline const QString getDisablePic() const { return m_disablePic_; }
signals:
    void mouseLeave();

protected:
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    bool event(QEvent *e) Q_DECL_OVERRIDE;

private:
    void showTooltip(const QPoint &gPos);
//    void onThemeChanged(ViewerThemeManager::AppTheme theme);
private:
    bool m_tooltipVisiable;
    QString m_disablePic_;
};

#endif // IMAGEBUTTON_H
