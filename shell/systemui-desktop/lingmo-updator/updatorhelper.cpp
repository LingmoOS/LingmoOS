/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     Lingmo OS Team <team@lingmo.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "updatorhelper.h"
#include "upgradeablemodel.h"
#include <QSettings>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QString>
#include <QDebug>
#include <QThread>
#include <QDebug>
#include <QLocale>

const static QString s_dbusName = "com.lingmo.Session";
const static QString s_pathName = "/Session";
const static QString s_interfaceName = "com.lingmo.Session";

UpdatorHelper::UpdatorHelper(QObject *parent)
    : QObject(parent)
    , m_checkProgress(0)
    , m_backend(new QApt::Backend(this))
    , m_trans(nullptr)
    , m_updateText()
{
    m_backend->init();

    QSettings settings("/etc/os-release",QSettings::IniFormat);
    m_currentVersion = settings.value("PRETTY_NAME").toString();

    QTimer::singleShot(100, this, &UpdatorHelper::checkUpdates);

    connect(m_trans, &QApt::Transaction::downloadSpeedChanged, this, &UpdatorHelper::updateDownloadSpeed);
    connect(m_trans, &QApt::Transaction::progressChanged, this, &UpdatorHelper::updateTotalProgress);
    connect(m_trans, &QApt::Transaction::downloadETAChanged, this, &UpdatorHelper::updateRemainingTime);

}

UpdatorHelper::~UpdatorHelper()
{
}

QString formatLocale(const QString &localeName) {
    // 分割语言代码，获取语言和区域部分
    QStringList parts = localeName.split('_');
    if (parts.size() >= 2) {
        return parts[0] + "-" + parts[1].toLower();
    } else {
        // 如果没有区域部分，只返回语言部分
        return parts[0];
    }
}

void UpdatorHelper::checkUpdates()
{
    if (m_trans)
        return;

    m_checkProgress = 0;
    m_trans = m_backend->updateCache();
    m_trans->setLocale(QLatin1String(setlocale(LC_MESSAGES, 0)));

    QLocale currentLocale = QLocale::system();
    QString currentLanguageName = formatLocale(currentLocale.name());
    qDebug() << "CurrentLanguage:" << currentLanguageName;

    fetchURLContent("https://packages.lingmo.org/lingmo/release/notes/" + currentLanguageName + "/index.html", this, &UpdatorHelper::updatelogs);

    connect(m_trans, &QApt::Transaction::progressChanged, this, [=] (int progress) {
        m_checkProgress = progress <= 100 ? progress : 100;
        emit checkProgressChanged();
    });

    connect(m_trans, &QApt::Transaction::statusChanged, this, [=] (QApt::TransactionStatus status) {
        switch (status) {
        case QApt::RunningStatus: {
            break;
        }
        case QApt::FinishedStatus: {
            m_backend->reloadCache();

            bool success = m_trans->error() == QApt::Success;
            QString errorDetails = m_trans->errorDetails();

            m_trans->cancel();
            m_trans->deleteLater();
            m_trans = nullptr;

            if (success) {
                // Add packages.
                bool lingmo_found = false;
                for (QApt::Package *package : m_backend->upgradeablePackages()) {
                    if (!package)
                        continue;

                    // 检查是否是lingmo包
                    if (QString::compare(package->name(), "system-core", Qt::CaseInsensitive) == 0) {
                        lingmo_found = true;
                        // 添加lingmo包
                        UpgradeableModel::self()->addPackage(package->name(),
                                                             package->availableVersion(),
                                                             package->installedSize());
                        break; // 找到后停止搜索/
                    }
            }

                if (lingmo_found) {
                    emit checkUpdateFinished(); // 发出更新完成信号
                } else {
                    // emit checkError("lingmo package not found"); // 发出未找到包的错误信号
                    emit checkUpdateFinished();
                }
            } else {
                emit checkError(errorDetails);
            }

            break;
        }
        default:
            break;
        }
    });

    m_trans->run();
}

void UpdatorHelper::fetchURLContent(const QString &url, QObject *parent, void (UpdatorHelper::*updateLogsMethod)(const QString&)) {
    QNetworkAccessManager *manager = new QNetworkAccessManager(parent);
    QNetworkRequest request(url);
    connect(manager, &QNetworkAccessManager::finished, parent, [=](QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QString content = QString::fromUtf8(reply->readAll());
            // Call the updateLogs method with the fetched content
            (this->*updateLogsMethod)(content);
        }
        reply->deleteLater();
        manager->deleteLater();
    });
    manager->get(request);
}

// Method to be called with fetched content
void UpdatorHelper::updatelogs(const QString &content) {
    // Process and integrate the fetched content into the update logs
    // For example, you could add it to a log file or display it in the UI
    // qDebug() << "Update logs fetched:" << content;
    // emit updateLogsFetched(content);
    m_updateText = content;
    emit updateTextChanged(content);
}

void UpdatorHelper::updateDownloadSpeed(quint64 speed)
{
    // 更新下载速度
    qDebug() << "Download speed:" << speed << "bytes/sec";
    // 发出信号，如果需要在其他地方显示下载速度
    emit downloadSpeedChanged(speed);
}

void UpdatorHelper::updateTotalProgress(double progress)
{
    // 更新总进度，确保进度在0到1之间
    double progressValue = qBound(0.0, progress / 100.0, 1.0);
    qDebug() << "Total progress:" << progressValue * 100 << "%";
    emit totalProgressChanged(progressValue);
}

void UpdatorHelper::updateRemainingTime(quint64 ETA)
{
    // 更新剩余时间
    qDebug() << "Remaining time:" << ETA << "seconds";
    // 发出信号，如果需要在其他地方显示剩余时间
    emit remainingTimeChanged(ETA);
}

/**
 * @brief Starts the upgrade process.
 *
 * This function starts the upgrade process by creating a transaction object and running it.
 * The status of the transaction is monitored using the statusChanged() and statusDetailsChanged() signals.
 * If the upgrade is successful, the system is rebooted using the reboot() function.
 *
 * @return True if the upgrade was successful, false otherwise.
 */
void UpdatorHelper::upgrade()
{
    if (m_trans) {
        return;
    }

    m_statusDetails.clear();
    m_trans = m_backend->upgradeSystem(QApt::FullUpgrade);
    m_trans->setLocale(QLatin1String(setlocale(LC_MESSAGES, 0)));

    connect(m_trans, &QApt::Transaction::statusDetailsChanged, this, [=] (const QString &statusDetails) {
        m_statusDetails.append(statusDetails + "\n");
        emit statusDetailsChanged();
    });

    connect(m_trans, &QApt::Transaction::statusChanged, this, [=] (QApt::TransactionStatus status) {
        switch (status) {
        case QApt::RunningStatus: {
            emit startingUpdate();
            break;
        }
        case QApt::FinishedStatus: {
            bool success = m_trans->error() == QApt::Success;

            m_trans->cancel();
            m_trans->deleteLater();
            m_trans = nullptr;

            if (success) {
                emit updateFinished();
            } else {
                emit updateError();
            }

            break;
        }
        default:
            break;
        }
    });

    m_trans->run();
}

void UpdatorHelper::reboot()
{
    QDBusInterface iface(s_dbusName, s_pathName, s_interfaceName, QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.call("reboot");
    }
}

QString UpdatorHelper::version()
{
    return m_currentVersion;
}

QString UpdatorHelper::statusDetails()
{
    return m_statusDetails;
}

int UpdatorHelper::checkProgress()
{
    return m_checkProgress;
}

QString UpdatorHelper::updateInfo()
{
    return m_updateText;
}
