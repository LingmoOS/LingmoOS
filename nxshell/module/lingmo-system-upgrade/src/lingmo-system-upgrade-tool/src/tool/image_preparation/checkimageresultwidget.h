// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QWidget>
#include <QLabel>
#include <QIcon>
#include <QString>
#include <QVBoxLayout>

#include "../../widgets/iconlabel.h"
#include "../../widgets/simplelabel.h"
#include "../../widgets/backgroundframe.h"

class CheckImageResultWidget : public QWidget
{
    Q_OBJECT
public:
    CheckImageResultWidget(QWidget *parent = nullptr);

    bool getResult() const {
        return m_result;
    }
    const QString getResultIsoPath() const {
        return m_resultIsoPath;
    }

Q_SIGNALS:
    void SetResult(bool passed);
    void ReadImageInfo(QString filepath);

private Q_SLOTS:
    void onSetResult(bool passed);

private:
    bool            m_result = false;
    QString         m_resultIsoPath;
    QVBoxLayout     *m_mainLayout;
    QHBoxLayout     *m_frameLayout;
    BackgroundFrame *m_backgroundFrame;
    IconLabel       *m_iconLabel;
    SimpleLabel     *m_resultLabel;
    SimpleLabel     *m_imageSizeTitleLabel;
    SimpleLabel     *m_imageSizeLabel;
    SimpleLabel     *m_imageVersionTitleLabel;
    SimpleLabel     *m_imageVersionLabel;
    SimpleLabel     *m_appTipLabel;

    void initUI();
    void initConnections();
};
