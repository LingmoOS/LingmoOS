// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPENWITHDIALOG_H
#define OPENWITHDIALOG_H

#include "durl.h"
#include "mimesappsmanager.h"
#include "singleton.h"

#include <DDialog>
#include <dflowlayout.h>
#include <DIconButton>
#include <QCommandLinkButton>
#include <DLabel>
#include <DTitlebar>
#include <DHorizontalLine>

DWIDGET_USE_NAMESPACE

// 选择默认应用程序界面选项
class OpenWithDialogListItem : public QWidget
{
    Q_OBJECT

public:
    explicit OpenWithDialogListItem(const QIcon &icon, const QString &text, QWidget *parent = nullptr);
    ~OpenWithDialogListItem() override;

    void setChecked(bool checked);
    QString text() const;

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *e) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

private:
    QIcon m_icon;
    DIconButton *m_pCheckBtn;
    DLabel *m_pIconLbl;
    DLabel *m_pTextLbl;
};

class OpenWithDialogListSparerItem : public QWidget
{
public:
    explicit OpenWithDialogListSparerItem(const QString &title, QWidget *parent = nullptr);
    ~OpenWithDialogListSparerItem() override;

private:
    DHorizontalLine *m_separator;
    QLabel *m_title;
};

// 打开文件
class OpenWithDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit OpenWithDialog(const QString &strFileName, QWidget *parent = nullptr);
    ~OpenWithDialog() override;

    enum ShowType {
        SelectType,
        OpenType,
    };

    /**
     * @brief getOpenStyle   根据文件名获取打开方式
     * @param strFileName      文件名
     * @return
     */
    static QList<DesktopFile> getOpenStyle(const QString &strFileName);

    /**
     * @brief openWithProgram    用默认程序打开文件
     * @param strFileName               文件名
     * @param strExec               应用程序
     */
    static void openWithProgram(const QString &strFileName, const QString &strExec = "");

    /**
     * @brief showOpenWithDialog    显示选择应用程序对话框
     * @param eType         显示类型
     * @return      选择的应用程序
     */
    QString showOpenWithDialog(ShowType eType);

    /**
     * @brief getProgramPathByExec  根据传入的应用程序名称获取应用程序路径
     * @param strExec
     * @return
     */
    static QString getProgramPathByExec(const QString &strExec);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:

    /**
     * @brief init  初始化
     */
    void init();

    /**
     * @brief initConnections   信号槽连接
     */
    void initConnections();

    /**
     * @brief initData  初始化数据
     */
    void initData();

    /**
     * @brief checkItem     选择项
     * @param item
     */
    void checkItem(OpenWithDialogListItem *item);

    /**
     * @brief createItem    创建item
     * @param icon
     * @param name
     * @param filePath
     * @return
     */
    OpenWithDialogListItem *createItem(const QIcon &icon, const QString &name, const QString &filePath, const QString &strExec);


private slots:
    /**
     * @brief slotUseOtherApplication   使用其它应用程序
     */
    void slotUseOtherApplication();

    /**
     * @brief slotOpenFileByApp 点击打开
     */
    void slotOpenFileByApp();

private:
    DTitlebar *m_titlebar;

    QScrollArea *m_pScrollArea;
    DFlowLayout *m_pRecommandLayout;
    DFlowLayout *m_pOtherLayout;

    QCommandLinkButton *m_pOpenFileChooseButton;
    QCheckBox *m_pSetToDefaultCheckBox;
    QPushButton *m_pCancelButton;
    QPushButton *m_pChooseButton;
    QString m_strFileName;
    QMimeType m_mimeType;

    OpenWithDialogListItem *m_pCheckedItem = nullptr;

    ShowType m_showType;
    QString m_strAppExecName;       // 应用程序名

    bool m_bOk = false; // 是否确定
};

#endif // OPENWITHDIALOG_H
