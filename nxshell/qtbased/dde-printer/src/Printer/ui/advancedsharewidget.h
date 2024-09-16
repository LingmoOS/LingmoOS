// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ADVANCEDSHAREWIDGET_H
#define ADVANCEDSHAREWIDGET_H

#include <QWidget>
#include <QLabel>

#include <DMainWindow>
#include <DListView>
#include <QCloseEvent>
DWIDGET_USE_NAMESPACE

typedef struct tagAdvancedPackageInfo {
    QString strExec;
    QString strIcon;
    QString strName;
    QString strNameCn;
} PACKAGEINFO_S;

class AdvanceShareWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdvanceShareWidget(QWidget *parent = nullptr);
    void click();
    void setLabelText(const QString &text);

signals:
    void clicked();

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

public slots:
    void updateIcon();

private:
    QLabel *m_textLabel;
    QLabel *m_enterIcon;
    bool m_hover = false;
};

class ManagementToolWidget : public DMainWindow
{
    Q_OBJECT
public:
    explicit ManagementToolWidget(QWidget *parent = nullptr);
    virtual ~ManagementToolWidget() override;
    int exec();

private:
    void initUI();
    void initConnections();
    void getPackageInfo(const QString &packageName);

signals:
    void finished();

protected slots:
    void listClickedSlot(const QModelIndex &index);
    void closeEvent(QCloseEvent *event) override;

private:
    DListView *m_pDeviceList;
    QList<PACKAGEINFO_S> m_packageInfo;
};

#endif
