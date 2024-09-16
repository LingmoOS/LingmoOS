// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PERMISSIONSWINDOW_H
#define PERMISSIONSWINDOW_H
#include <DDialog>

DWIDGET_USE_NAMESPACE
DWIDGET_BEGIN_NAMESPACE
class DLineEdit;
class DPasswordEdit;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
QT_END_NAMESPACE
class PermissionsWindow : public DDialog
{
    Q_OBJECT
public:
    explicit PermissionsWindow(QWidget *parent = nullptr);
    virtual ~PermissionsWindow() override;
    void setHost(const QString &host);
    QString getUser();
    QString getGroup();
    QString getPassword();

private:
    void initUI();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    QLabel *m_pTipLabel;
    DLineEdit *m_pUserEdit;
    DLineEdit *m_pGroupEdit;
    DPasswordEdit *m_pPasswordEdit;

private:
    QString m_host;
    QString m_user;
    QString m_group;
    QString m_password;
    int m_ret;
signals:
    void finished();
};

#endif // PERMISSIONSWINDOW_H
