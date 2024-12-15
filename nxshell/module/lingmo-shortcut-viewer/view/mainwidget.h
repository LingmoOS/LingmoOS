// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <DBlurEffectWidget>
#include <QVBoxLayout>

#define CONTENT_MARGINS 30

DWIDGET_USE_NAMESPACE

class ShortcutView;
class MainWidget : public DBlurEffectWidget
{
    Q_OBJECT
public:
    MainWidget(QWidget *parent = nullptr);

    void setJsonData(const QString &data);

protected:
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void focusInEvent(QFocusEvent *e) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

protected:
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *e) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *e) override;

private:
    void initUI();
    void initMargins();

    ShortcutView *m_mainView = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;
};

#endif   // MAINWIDGET_H
