// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GENERALPREVIEWPLUGIN_P_H
#define GENERALPREVIEWPLUGIN_P_H

#include "generalpreviewplugin.h"
#include "utils/filestatisticsthread.h"

#include <DHorizontalLine>
#include <DWidget>

#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QToolButton>

DWIDGET_BEGIN_NAMESPACE
class DHorizontalLine;
DWIDGET_END_NAMESPACE

namespace GrandSearch {

class NameLabel: public QLabel
{
    Q_OBJECT
public:
    explicit NameLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class SizeLabel: public QLabel
{
    Q_OBJECT
public:
    explicit SizeLabel(const QString &text = "", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};


class GeneralPreviewPluginPrivate
{
public:
    explicit GeneralPreviewPluginPrivate(GeneralPreviewPlugin *parent = nullptr);
    ~GeneralPreviewPluginPrivate();    

    GeneralPreviewPlugin *q_p = nullptr;

    MatchedItem m_item;
    DetailInfoList m_detailInfos;

    QVBoxLayout *m_vMainLayout = nullptr;
    QPointer<QWidget> m_contentWidget = nullptr;

    // 图标和名称
    QLabel *m_iconLabel = nullptr;
    NameLabel *m_nameLabel = nullptr;

    // 大小
    SizeLabel *m_sizeLabel = nullptr;
    FileStatisticsThread *m_sizeWorker = nullptr;
};

}

#endif // UNKNOWNPREVIEWPLUGIN_P_H
