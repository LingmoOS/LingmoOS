// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPDRAWERWIDGET_H
#define APPDRAWERWIDGET_H

#include "appitemdelegate.h"
#include "multipagesview.h"

#include <QWidget>
#include <QSharedPointer>

#include <DBlurEffectWidget>

DWIDGET_USE_NAMESPACE

class AppDrawerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AppDrawerWidget(QWidget *parent = Q_NULLPTR);
    virtual ~AppDrawerWidget();

    void initUi();
    void initAccessible();
    void initConnection();
    void updateBackgroundImage(const QPixmap & img);
    void refreshDrawerTitle(const QString &title = QString());
    void setCurrentIndex(const QModelIndex &index);

signals:
    void drawerClicked(AppGridView *view);

private slots:
    void onTitleChanged();

protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;

private:
    QWidget *m_maskWidget;
    AppItemDelegate *m_appDelegate;
    MultiPagesView *m_multipageView;

    DBlurEffectWidget *m_blurBackground;
    QPixmap m_pix;
    QModelIndex m_clickIndex;
};

#endif
