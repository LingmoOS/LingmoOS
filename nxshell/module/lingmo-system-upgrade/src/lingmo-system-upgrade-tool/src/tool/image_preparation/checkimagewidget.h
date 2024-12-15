// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QVBoxLayout>
#include <QWidget>

#include "../../widgets/spinnerwidget.h"

class CheckImageWidget: public QWidget
{
    Q_OBJECT
public:
    CheckImageWidget(QWidget *parent = nullptr);

Q_SIGNALS:
    void Cancel();
    // Validate the imported system image
    void CheckImported(const QString path);
    // 检查下载好的镜像文件。如不通过就删掉以便重新下载。
    void CheckDownloaded();
    void CheckDone(bool state);

private Q_SLOTS:
    void onCanceled();
    void onCheckImported(const QString path);
    void onCheckDownloaded();

private:
    bool            m_canceled;
    QString         m_isoPath;
    QVBoxLayout     *m_mainLayout;
    SpinnerWidget   *m_spinnerWidget;

    void initUI();
    void initConnections();
};
