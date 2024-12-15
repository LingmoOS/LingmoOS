// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DFloatingMessage>

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QStackedLayout>

#include "../../widgets/basecontainerwidget.h"
#include "checkimagewidget.h"
#include "checkimageresultwidget.h"
#include "checksizethread.h"
#include "retrieveimagemethodwidget.h"
#include "imagedownloadwidget.h"
#include "../../widgets/dropframe.h"
#include "../../widgets/baselabel.h"
#include "../../widgets/iconlabel.h"
#include "../../widgets/errormessagewidget.h"
#include "softwarecheckprogresswidget.h"
#include "softwareevaluationwidget.h"
#include "softwareevaluationofflinewidget.h"


DWIDGET_USE_NAMESPACE

class ImagePreparationWidget : public BaseContainerWidget
{
    Q_OBJECT
public:
    explicit ImagePreparationWidget(QWidget *parent);
    void initUI();
    void initConnections();

Q_SIGNALS:
    void StartUpgrade(const QString isoPath);

public Q_SLOTS:
    void checkNetwork();

private Q_SLOTS:
    void showNoNetworkMessage();

private:
    bool                            m_networkConnected;
    DFloatingMessage                *m_noNetworkMessage;
    QStackedLayout                  *m_stackedLayout;
    RetrieveImageMethodWidget       *m_imageMethodWidget;
    CheckImageWidget                *m_checkImageWidget;
    CheckImageResultWidget          *m_checkImageResultWidget;
    CheckSizeThread                 *m_checkNetworkThread;
    ImageDownloadWidget             *m_imageDownloadWidget;
    ErrorMessageWidget              *m_errorMessageWidget;
    SoftwareCheckProgressWidget     *m_softwareCheckProgressWidget;
    SoftwareEvaluationWidget        *m_softwareTableWidget;
    SoftwareEvaluationOfflineWidget *m_softwareOfflineResultWidget;

    void setCancelButtonStyle();
    void showAllButtons();
    void jumpImageMethodWidget();
    void showPowerWarning();
};
