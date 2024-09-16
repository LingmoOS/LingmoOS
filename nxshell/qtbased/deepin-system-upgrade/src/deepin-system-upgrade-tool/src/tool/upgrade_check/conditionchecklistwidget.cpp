// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DDialog>
#include <DSwitchButton>

#include <QtMath>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QStringList>
#include <QVersionNumber>

#include "conditionchecklistwidget.h"
#include "../../core/constants.h"
#include "../../core/dbusworker.h"
#include "../../widgets/backgroundframe.h"

#define RESULT_ICON_W 18
#define ICONLABEL_RIGHT_SPACE 28

ConditionChecklistWidget::ConditionChecklistWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(new QVBoxLayout(this))
    , m_warningTextLabel(new DLabel(this))
    , m_versionResultWidget(new CheckResultWidget(this))
#ifdef SHOW_ACTIVATION
    , m_activationResultWidget(new CheckResultWidget(this))
#endif
    , m_cpuResultWidget(new CheckResultWidget(this))
    , m_storageResultWidget(new StorageResultWidget(this))
{
    initUI();
    initConnections();
}

void ConditionChecklistWidget::initUI()
{
    m_versionResultWidget->setIcon(":/icons/system_version.svg", RESULT_ICON_W, RESULT_ICON_W);
    m_versionResultWidget->setTitle(tr("System version"));
    m_versionResultWidget->setToolTip(
        QString("<p style = \"font-family:SourceHanSansSC;font-size:12px;font-weight:normal;\">") +
        tr("The system version is too old, and the upgrade may fail. To upgrade successfully, please update your system to the latest version in the Control Center first (you can ignore it and force an upgrade as well).") +
        QString("</p>")
    );

    if (DBusWorker::getInstance()->GetDistroID() == QString("Deepin"))
    {
        m_versionResultWidget->setRequirement(tr("Requirement: deepin (%1)").arg(kDeepinMinimalVersion.toString()));
    }
    else
    {
        m_versionResultWidget->setRequirement(tr("Requirement: Professional (%1)").arg(kUosMinimalVersion.toString()));
    }

    m_versionResultWidget->addSpacerItem(new QSpacerItem(ICONLABEL_RIGHT_SPACE, 0));

    m_mainLayout->addWidget(m_versionResultWidget);
    m_mainLayout->addSpacerItem(new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::Minimum));

    m_mainLayout->setContentsMargins(145, 0, 112, 0);
#ifdef SHOW_ACTIVATION
    m_activationResultWidget->setIcon(":/icons/activation_status.svg", RESULT_ICON_W, RESULT_ICON_W);
    m_activationResultWidget->setTitle(tr("Authorization status"));
    m_activationResultWidget->setRequirement(tr("Requirement: Activated"));
    m_activationResultWidget->addSpacerItem(new QSpacerItem(ICONLABEL_RIGHT_SPACE, 0));

    m_mainLayout->addWidget(m_activationResultWidget);
    m_mainLayout->addSpacerItem(new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::Minimum));
#endif
    m_cpuResultWidget->setIcon(":/icons/cpu.svg", RESULT_ICON_W, RESULT_ICON_W);
    m_cpuResultWidget->setTitle(tr("CPU architecture"));
    m_cpuResultWidget->setRequirement(tr("Requirement: X86"));
    m_cpuResultWidget->addSpacerItem(new QSpacerItem(ICONLABEL_RIGHT_SPACE, 0));

    m_mainLayout->addWidget(m_cpuResultWidget);
    m_mainLayout->addSpacerItem(new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::Minimum));

    m_storageResultWidget->setIcon(":/icons/storage_space.svg", RESULT_ICON_W, RESULT_ICON_W);
    m_storageResultWidget->setTitle(tr("Storage space"));

    m_mainLayout->addWidget(m_storageResultWidget);

    m_warningTextLabel->setForegroundRole(DPalette::TextWarning);
    m_warningTextLabel->setFont(QFont("SourceHanSansSC", 9, QFont::Normal));
    m_warningTextLabel->setText(tr("There are risks, but you can force an upgrade."));
    m_mainLayout->addWidget(m_warningTextLabel, 0, Qt::AlignHCenter);
    m_mainLayout->addSpacing(6);

    setLayout(m_mainLayout);
}

void ConditionChecklistWidget::initConnections()
{}

/**
 * @brief Update UI，will be triggered by UpgradeCheckResultWidget::updateUI() when upgrade check is done.
 *
 */
void ConditionChecklistWidget::updateUI()
{
    DBusWorker *dbusWorker = DBusWorker::getInstance(this);
    const QMap<QString, QString> resultMap = dbusWorker->getCheckResultMap();

    m_isPassed = true;

    QString editionName = resultMap["editionName"];
    if (editionName == "Community")
    {
        editionName = tr("Community");
    }
    else
    {
        editionName = tr("Professional");
    }
    QVersionNumber minorVersionNumber = QVersionNumber::fromString(resultMap["minorVersion"]);
    QVersionNumber requiredVersionNumber;
    if (DBusWorker::getInstance()->GetDistroID() == QString("Deepin"))
    {
        requiredVersionNumber = kDeepinMinimalVersion;
    }
    else
    {
        requiredVersionNumber = kUosMinimalVersion;
    }
    qDebug() << "check result -> minorVersion:" << minorVersionNumber;
    CheckResultType versionResult = minorVersionNumber >= requiredVersionNumber ? CheckResultType::PASSED : CheckResultType::WARNING;
    QString versionResultText = versionResult == CheckResultType::PASSED ? QString("%1 (%2)").arg(editionName).arg(minorVersionNumber.toString()) : tr("Too old");
    m_versionResultWidget->setResult(versionResultText);
    m_versionResultWidget->setStatus(versionResult);
#ifdef SHOW_ACTIVATION
    CheckResultType activationResult = resultMap["active"] == "1" || resultMap["editionName"] == "Community" ? CheckResultType::PASSED : CheckResultType::FAILED;
    m_activationResultWidget->setResult(activationResult == CheckResultType::PASSED ? tr("Activated") : tr("Unactivated"));
    m_activationResultWidget->setStatus(activationResult);
    if (activationResult == CheckResultType::FAILED)
    {
        m_isPassed = false;
    }
#endif
    QStringList validArch = {
        "x86_64",
    };
    QString arch = resultMap["arch"];
    CheckResultType archResult = validArch.contains(arch) ? CheckResultType::PASSED : CheckResultType::FAILED;
    m_cpuResultWidget->setResult(arch);
    m_cpuResultWidget->setStatus(archResult);
    if (archResult == CheckResultType::FAILED)
    {
        m_isPassed = false;
    }

    double dataBase = resultMap["database"].toDouble();
    double dataFree = resultMap["datafree"].toDouble();
    double systemBase = resultMap["systembase"].toDouble();
    double systemFree = resultMap["systemfree"].toDouble();
    CheckResultType dataResultStatus = dataFree >= dataBase ? CheckResultType::PASSED : CheckResultType::FAILED;
    CheckResultType systemResultStatus = systemFree >= systemBase ? CheckResultType::PASSED : CheckResultType::FAILED;

    m_storageResultWidget->setRequirement(tr("Requirement: %1 GB").arg(qCeil(dataBase + systemBase)));
    m_storageResultWidget->setDataResult(tr("Data %1 GB (Available)/ %2 GB").arg(dataFree).arg(dataBase));
    m_storageResultWidget->setDataStatus(dataResultStatus);
    m_storageResultWidget->setSystemResult(tr("System %1 GB (Available)/ %2 GB").arg(systemFree).arg(systemBase));
    m_storageResultWidget->setSystemStatus(systemResultStatus);
    if (dataResultStatus == CheckResultType::FAILED || systemResultStatus == CheckResultType::FAILED)
    {
        m_isPassed = false;
    }

    if (versionResult == CheckResultType::WARNING && m_isPassed)
    {
        m_warningTextLabel->setVisible(true);
    }
    else
    {
        m_warningTextLabel->setVisible(false);
    }
    Q_EMIT updateCompleted();
}
