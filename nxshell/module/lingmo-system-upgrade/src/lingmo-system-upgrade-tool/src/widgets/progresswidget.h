// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DProgressBar>

#include <QString>
#include <QSpacerItem>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>

#include "../widgets/simplelabel.h"
#include "../widgets/iconlabel.h"

QString secondsToReadableTime(time_t sec);


class ProgressWidget: public QWidget
{
    Q_OBJECT
public:
    explicit ProgressWidget(QWidget *parent = nullptr);

protected:
    // -1 is the uninitialized status. Due to resume downloading, the initial progress value have to be retrieved everytime a download is started.
    qint64      m_initProgress = -1;
    qint64      m_lastProgress = 0;
    time_t      m_startTime = 0;

    DProgressBar    *m_progressbar;
    QVBoxLayout     *m_mainLayout;
    QHBoxLayout     *m_progressbarLayout;
    SimpleLabel     *m_iconLabel;
    SimpleLabel     *m_titleLabel;
    SimpleLabel     *m_tipLabel;
    SimpleLabel     *m_progressLabel;
    SimpleLabel     *m_etaLabel;
    QSpacerItem     *m_iconSpacerItem;
    QSpacerItem     *m_tipSpacerItem;

    void initUI();
    void initConnections();

protected Q_SLOTS:
    // The default ETA time updating SLOT. It is disconnected to the updateProgress signal by default, so connect them manually if needed.
    void updateDefaultEtaText(qint64 p);

    // The slot for starting progress. It will initialize start time and initial progress value in order to calculate ETA time.
    void onStart();

Q_SIGNALS:
    void updateProgress(qint64 progress);

private Q_SLOTS:
    void onUpdateProgress(qint64 progress);
};
