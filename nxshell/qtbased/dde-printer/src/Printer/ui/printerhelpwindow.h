// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PRINTERHELPWINDOW_H
#define PRINTERHELPWINDOW_H

#include <QLabel>
#include <DMainWindow>
#include <DFrame>
#include <DLabel>

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QWidget;
class QPushButton;
QT_END_NAMESPACE

class PrinterHelpWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit PrinterHelpWindow(QWidget *parent = nullptr);
    virtual ~PrinterHelpWindow() override;

private:
    void initUi();
    void initConnections();

private slots:

signals:

private:
    QPushButton *m_copyButton;
    QLabel *m_copyLabel;
};

class PrinterManufacturerWidget : public DMainWindow
{
    Q_OBJECT
public:
    explicit PrinterManufacturerWidget(DMainWindow *parent = nullptr);
    virtual ~PrinterManufacturerWidget() override;

private:
    void initUi();
    void initConnections();
};

class CustomLabel : public QLabel
{
    Q_OBJECT

public:
    CustomLabel(QWidget *parent = nullptr) : QLabel(parent)
    {
       initUi();
       initConnection();
    }

    void initUi();
    void initConnection();

signals:
    void signalExpand();

protected slots:
    void slotOpenFaqDoc();
    void slotOpenHelpInfo();
    void updateIcon();

private:
    void initSubUi();

protected:
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QMenu *m_main;
    QLabel *m_textLabel;
    QLabel *m_imageLabel;
    QPushButton *m_popupButton1;
    QPushButton *m_popupButton2;
    PrinterHelpWindow *m_pHelpWindow = nullptr;

    QPushButton *m_popupButton;
    PrinterManufacturerWidget *m_manufacturerWidget = nullptr;
};

class PrinterManufacturerItem : public DFrame
{
    Q_OBJECT
public:
    PrinterManufacturerItem(QWidget *parent = nullptr);
    ~PrinterManufacturerItem() override;

    void setMfgText(const QString &text);
    void setLinkText(const QString &text);
    void setItemIcon(const QString &iconPath);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

Q_SIGNALS:
    void linkClicked();
    void clicked();

private:
    DLabel *m_manufacturerIcon;
    DLabel *m_mfgLabel;
    DLabel *m_linkLabel;

    QString m_tipStr;
};

#endif // PRINTERHELPWINDOW_H
