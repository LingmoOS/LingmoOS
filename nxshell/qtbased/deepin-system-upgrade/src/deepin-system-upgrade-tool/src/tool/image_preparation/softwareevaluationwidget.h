// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DWidget>
#include <DLabel>
#include <DFontSizeManager>

#include <QWidget>
#include <QVBoxLayout>

#include <vector>

#include "../../widgets/simplelabel.h"
#include "../../widgets/softwaretablewidget.h"

struct AppInfo {
    QString name;
    QString pkgName;
    QString iconName;
    bool noDisplay;
};

DWIDGET_USE_NAMESPACE

class SoftwareEvaluationWidget : public DWidget
{
    Q_OBJECT
public:
    SoftwareEvaluationWidget(QWidget *parent = nullptr);
    void fillTable();
    void clearAppInfos();
    QStringList getMigratelist();

private Q_SLOTS:
    void updateAppInfos(const QString &pkg, int status);
    void onItemCheckboxClicked(int state, const QString &infoName);

private:
    void initUI();
    void initConnections();

    QMap<QString, QStringList> *m_pkgDesktopMap;
    SimpleLabel                *m_powerTipLabel;
    DLabel                  *dataAboutTipsLabel;
    DLabel                  *appAboutTipsLabel;
    QVBoxLayout                *m_mainLayout;
    QWidget                    *m_mainWidget;
    DLabel                     *m_softwareChangeTitleLabel;
    SoftwareTableWidget        *m_softwareChangeTableWidget;
    std::vector<AppInfo>       m_compatAppInfos;
    std::vector<AppInfo>       m_incompatAppInfos;
    QStringList                m_migrateLists;
};
