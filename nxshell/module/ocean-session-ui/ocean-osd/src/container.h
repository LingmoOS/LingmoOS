// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTAINER_H
#define CONTAINER_H

#include <DBlurEffectWidget>
#include <DWindowManagerHelper>
#include <QScopedPointer>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

namespace Dtk
{
namespace Widget
{
class DPlatformWindowHandle;
}
}

class QHBoxLayout;
/*!
 * \~chinese \class Container
 * \~chinese \brief 用来显示osd listview的窗口
 */
class Container : public DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit Container(QWidget *parent = nullptr);
    ~Container() override;

    void setContent(QWidget *content);
    void moveToCenter();

    void updateWindowRadius(int radius = -1);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void onDelayQuit();

private:
    QHBoxLayout *m_layout;
    QTimer *m_quitTimer;
    QScopedPointer<Dtk::Widget::DPlatformWindowHandle> m_handle;
};

#endif // CONTAINER_H
