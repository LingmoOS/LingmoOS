// SPDX-FileCopyrightText: 2023-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FIRSTTIPWIDGETH
#define FIRSTTIPWIDGETH

#include "global_defines.h"
#include <QFrame>
#include <QToolButton>
class QLabel;
class QGraphicsDropShadowEffect;

namespace cooperation_core {
class LineWidget;
class ElidedLabel;
class FirstTipWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FirstTipWidget(QWidget *parent = nullptr);

    void themeTypeChanged();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void initUI();
    void initbackgroundFrame();
    void inittipBtn();
    void drawLine();

    QFrame *backgroundFrame { nullptr };
    QLabel *bannerLabel { nullptr };
    QLabel *action { nullptr };
    QGraphicsDropShadowEffect *shadowEffect { nullptr };
    QList<QLabel *> lineBalls { nullptr };
    LineWidget *line { nullptr };
    QToolButton *tipBtn { nullptr };
};

class LineWidget : public QWidget
{
public:
    LineWidget(QWidget *parent = nullptr)
        : QWidget(parent) {}

protected:
    void paintEvent(QPaintEvent *event) override;
};

class ElidedLabel : public QLabel
{
public:
    ElidedLabel(const QString &text, int maxwidth, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int maxWidth;
};

}   // namespace cooperation_core
#endif   // FIRSTTIPWIDGETH
