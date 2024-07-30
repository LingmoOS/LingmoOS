/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPAGEWIDGETTEST_H
#define KPAGEWIDGETTEST_H

#include <QWidget>

#include "kpagewidget.h"

class KPageWidgetTest : public QWidget
{
    Q_OBJECT

public:
    KPageWidgetTest(QWidget *parent = nullptr);
    ~KPageWidgetTest() override;

private Q_SLOTS:
    void setAutoFace();
    void setPlainFace();
    void setListFace();
    void setTreeFace();
    void setTabbedFace();
    void setFlatListFace();

    void addPage();
    void addSubPage();
    void insertPage();
    void deletePage();

    void currentPageChanged(KPageWidgetItem *, KPageWidgetItem *);
    void pageToggled(KPageWidgetItem *, bool);

private:
    KPageWidget *mWidget;
};

#endif
