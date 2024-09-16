// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RENAMEPRINTERWINDOW_H
#define RENAMEPRINTERWINDOW_H
#include <dtkwidget_global.h>
#include <DMainWindow>
#include <DPushButton>
DWIDGET_USE_NAMESPACE
DWIDGET_BEGIN_NAMESPACE
class DMainWindow;
class DLineEdit;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QPushButton;
QT_END_NAMESPACE

class RenamePrinterWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit RenamePrinterWindow(QWidget *parent = nullptr);
    virtual ~RenamePrinterWindow() override;

    int result();
    QString getName();
    void setName(const QString &name);
    QString getLocation();
    void setLocation(const QString &location);
    QString getDescription();
    void setDescription(const QString &description);

private:
    void initUI();
    void initConnections();

private slots:
    void btnClickedSlot();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    DLineEdit *m_pNameEdit;
    DLineEdit *m_pLocationEdit;
    DLineEdit *m_pDescriptionEdit;

    DPushButton *m_pCancelBtn;
    DPushButton *m_pOKBtn;

private:
    int m_ret;
signals:
    void accepted(int ret);
};

#endif // RENAMEPRINTERWINDOW_H
