// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <dabstractdialog.h>
#include <DIconButton>

DWIDGET_USE_NAMESPACE

class Content;
class QLabel;
class MainWindow : public DAbstractDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setTitle(const QString& title);
    void setEnTitle(const QString& title);
    void setEnableAllowCheckBox(const bool enable);
    void setAllowCheckBoxText(const QString& text);
    void setEnAllowCheckBoxText(const QString& text);
    void setCnSource(const QString& source);
    void setEnSource(const QString& source);
    void updateLocaleSource();
    void setHideBottom(const QString& param);

private:
    QLabel* m_title;
    Content* m_content;

    QString m_cnTitle;
    QString m_enTitle;

    DIconButton *btnclose;
    const int windowFixedWidth = 520;
};

#endif // MAINWINDOW_H
