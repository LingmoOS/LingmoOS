// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAJTOOLBAR_H
#define MAJTOOLBAR_H

#include <DLabel>
#include <DPushButton>
#include <QHBoxLayout>
#include <DStackedWidget>
#include <DWidget>

DWIDGET_USE_NAMESPACE

class MajToolBar : public DLabel
{
    Q_OBJECT
public:
    explicit MajToolBar(DWidget *parent = 0);
    ~MajToolBar();

signals:
    void buttonChecked(bool checked, QString type);
    void mainColorChanged(QColor currentColor);
    void saveImage();
    void showSaveTooltip(QString tooltips);
    void hideSaveTooltip();
    void shapePressed(QString shape);
    void specificedSavePath();
    void saveSpecificedPath();
    void closed();

public slots:
    void initWidgets();
    //bool isButtonChecked();

private:
    QHBoxLayout *m_baseLayout;

    bool m_isChecked;
    bool m_listBtnChecked;
    QString m_currentShape;
};

#endif // MAJTOOLBAR_H
