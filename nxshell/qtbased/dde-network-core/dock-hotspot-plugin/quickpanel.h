// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QUICKPANEL_H
#define QUICKPANEL_H

#include "hotspotplugin_utils.h"
#include <DIconButton>
#include <QVariant>
#include <DGuiApplicationHelper>
#include <DLabel>
#include <QWidget>

HOTSPOTPLUGIN_BEGIN_NAMESPACE

DWIDGET_USE_NAMESPACE

class QuickPanel : public QWidget
{
    Q_OBJECT

public:
    explicit QuickPanel(QWidget *parent = nullptr);
    const QString text() const;

public Q_SLOTS:
    void setIcon(const QIcon &icon);
    void setText(const QString &text);

    void updateState(DGuiApplicationHelper::ColorType type,bool enabled);

Q_SIGNALS:
    void iconClicked();

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void initUi();

private:
    DIconButton *m_icon{nullptr};
    DLabel *m_text{nullptr};
};

HOTSPOTPLUGIN_END_NAMESPACE

#endif // QUICKPANEL_H
