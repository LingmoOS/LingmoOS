// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTENTWIDGET_H
#define CONTENTWIDGET_H

#include <DImageButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

DWIDGET_USE_NAMESPACE

class QScrollArea;
class QPropertyAnimation;
class QPushButton;

namespace dcc_fcitx_configtool {

namespace widgets {
class BackButton;
}
} // namespace dcc_fcitx_configtool

namespace dcc_fcitx_configtool {
namespace widgets {
class FcitxContentWidget : public QWidget {
    Q_OBJECT

public:
    explicit FcitxContentWidget(QWidget *parent = 0);
    ~FcitxContentWidget();

    QWidget *content() const { return m_content; }
    QWidget *setContent(QWidget *const w);
    void scrollTo(int dy);

protected:
    void resizeEvent(QResizeEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

protected:
    QScrollArea *m_contentArea{nullptr};
    QWidget *m_content{nullptr};
};

} // namespace widgets
} // namespace dcc_fcitx_configtool

#endif // CONTENTWIDGET_H
