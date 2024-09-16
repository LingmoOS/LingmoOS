// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFQUICKINSTALL_H
#define DFQUICKINSTALL_H

#include <DApplication>
#include <DComboBox>
#include <DLabel>
#include <DMainWindow>
#include <DPushButton>

DWIDGET_USE_NAMESPACE

class DFontInfoManager;
class FontManagerCore;
struct DFontInfo;
class DFontPreviewer;
class DFMDBManager;

class DFQuickInstallWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit DFQuickInstallWindow(const QStringList &files = QStringList(), QWidget *parent = nullptr);

    ~DFQuickInstallWindow() override;

    static constexpr int DEFAULT_WINDOW_W = 480;
    static constexpr int DEFAULT_WINDOW_H = 380;

protected:
    void initUI();
    void initConnections();
    void InitPreviewFont(DFontInfo &fontInfo);
    void installFont(const QStringList &files);

    void resizeEvent(QResizeEvent *event) override;
signals:
    void fileSelected(const QStringList &fileList);
    void quickInstall();
    void requestShowMainWindow(const QStringList &fileList);

public slots:
    void onFileSelected(const QStringList &fileList);
    void onInstallBtnClicked();
    void onFontInstallFinished();

private:
    DLabel *m_titleLabel {nullptr};
    QWidget *m_mainFrame {nullptr};

    DComboBox *m_fontType {nullptr};
    DFontPreviewer *m_fontPreviewTxt {nullptr};

    DLabel *m_stateLabel {nullptr};
    DPushButton *m_actionBtn {nullptr};

    DFontInfoManager *m_fontInfoManager;
    FontManagerCore *m_fontManager;
    DFMDBManager *m_dbManager;


    QStringList m_installFiles;

    DPalette m_oldPaStateLbl;
};

#endif  // DFQUICKINSTALL_H
