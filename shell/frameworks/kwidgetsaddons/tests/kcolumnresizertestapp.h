/*
    This file is part of the KDE frameworks
    SPDX-FileCopyrightText: 2011-2014 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#ifndef KCOLUMNRESIZERTESTAPP_H
#define KCOLUMNRESIZERTESTAPP_H

#include <QWidget>

class Ui_ColumnResizerTestWidget;

class Window : public QWidget
{
    Q_OBJECT
public:
    Window();
    ~Window() override;

public Q_SLOTS:
    void updateAdjustableLabel();

private:
    Ui_ColumnResizerTestWidget *m_ui;
};

#endif /* KCOLUMNRESIZERTESTAPP_H */
