// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DLabel>

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <DPushButton>
#include <QStackedWidget>
#include <QSpacerItem>

#include "../../widgets/dropframe.h"
#include "../../widgets/baselabel.h"
#include "../../widgets/iconlabel.h"

DWIDGET_USE_NAMESPACE

class RetrieveImageMethodWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RetrieveImageMethodWidget(QWidget *parent);
    void initUI();
    void initConnections();
    bool isLocalFileSelected() {
        return m_localRadioButton->isChecked();
    }
    QString getFileUrl() {
        return m_fileUrl;
    }

Q_SIGNALS:
    void FileAdded();
    void FileClear();
    void ToggleLocalButton(bool checked);

private:
    QString         m_fileUrl;
    QVBoxLayout     *m_verticalLayout = nullptr;
    QRadioButton    *m_networkRadioButton = nullptr;
    QRadioButton    *m_localRadioButton = nullptr;
    DPushButton     *m_networkPushButton = nullptr;
    DPushButton     *m_localPushButton = nullptr;
    QHBoxLayout     *m_buttonsHBoxLayout = nullptr;
    DLabel          *m_label = nullptr;
    IconLabel       *m_dropAreaIconLabel = nullptr;
    DropFrame       *m_dropAreaFrame = nullptr;
    BaseLabel       *m_fileTextLabel = nullptr;
    QLabel          *m_uploadTextLink = nullptr;
    DLabel          *m_filenameLabel = nullptr;
    QSpacerItem     *m_fileLabelSpacerItem = nullptr;
    DPushButton     *m_removeFileButton = nullptr;
    QString         m_darkBtnStylesheet;
    QString         m_lightBtnStylesheet;

    QString loadStyleSheet(const QString &styleFile);

    void updateTipsColor();

private Q_SLOTS:
    void onFileSelected(const QString &filename);
    void onFileCleared();
    void setDefaultDropFrameStyle();
};
