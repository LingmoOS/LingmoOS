// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "proxy_package_list_model.h"
#include "deblistmodel.h"
#include "uab/uab_package_list_model.h"
#include "utils/utils.h"

ProxyPackageListModel::ProxyPackageListModel(QObject *parent)
    : AbstractPackageListModel{parent}
{
    m_supportPackageType = Pkg::Proxy;
}

QVariant ProxyPackageListModel::data(const QModelIndex &index, int role) const
{
    if (m_packageModels.empty()) {
        return {};
    }

    auto modelWithIndex = findFromProxyIndex(index.row());
    ModelPtr model = modelWithIndex.first;
    if (!model) {
        return {};
    }

    return model->data(model->index(modelWithIndex.second), role);
}

bool ProxyPackageListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (m_packageModels.empty()) {
        return false;
    }

    auto modelWithIndex = findFromProxyIndex(index.row());
    ModelPtr model = modelWithIndex.first;
    if (!model) {
        return false;
    }

    return model->setData(model->index(modelWithIndex.second), value, role);
}

int ProxyPackageListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    int count = 0;
    for (const ModelInfo &info : m_packageModels) {
        count += info.model->rowCount();
    }

    return count;
}

void ProxyPackageListModel::slotAppendPackage(const QStringList &packageList)
{
    QMap<Pkg::PackageType, QStringList> filterList;
    for (const QString &packagePath : packageList) {
        const Pkg::PackageType type = Utils::detectPackage(packagePath);
        filterList[type].append(packagePath);
    };

    for (auto itr = filterList.begin(); itr != filterList.end(); ++itr) {
        switch (itr.key()) {
            case Pkg::Uab:
                Q_FALLTHROUGH();
            case Pkg::Deb: {
                ModelPtr model = modelFromType(itr.key());
                if (model) {
                    model->slotAppendPackage(itr.value());
                }
            } break;
            default:
                return;
        }
    }
}

void ProxyPackageListModel::removePackage(int index)
{
    auto modelWithIndex = findFromProxyIndex(index);
    if (!modelWithIndex.first) {
        return;
    }

    modelWithIndex.first->removePackage(modelWithIndex.second);
}

QString ProxyPackageListModel::checkPackageValid(const QString &packagePath)
{
    Pkg::PackageType type = Utils::detectPackage(packagePath);
    ModelPtr model = modelFromType(type);

    // TODO: static package check or remove it
    if (!model) {
        return "";
    }

    return model->checkPackageValid(packagePath);
}

Pkg::PackageInstallStatus ProxyPackageListModel::checkInstallStatus(const QString &packagePath)
{
    ModelPtr model = addModelFromFile(packagePath);
    if (model) {
        return model->checkInstallStatus(packagePath);
    }

    return Pkg::NotInstalled;
}

Pkg::DependsStatus ProxyPackageListModel::checkDependsStatus(const QString &packagePath)
{
    ModelPtr model = addModelFromFile(packagePath);
    if (model) {
        return model->checkDependsStatus(packagePath);
    }

    return Pkg::DependsOk;
}

QStringList ProxyPackageListModel::getPackageInfo(const QString &packagePath)
{
    ModelPtr model = addModelFromFile(packagePath);
    if (model) {
        return model->getPackageInfo(packagePath);
    }

    return {};
}

QString ProxyPackageListModel::lastProcessError()
{
    QString errorMessage;
    (void)std::find_if(m_packageModels.rbegin(), m_packageModels.rend(), [&errorMessage](const ModelInfo &info) {
        errorMessage = info.model->lastProcessError();
        return !errorMessage.isEmpty();
    });

    return errorMessage;
}

bool ProxyPackageListModel::containsSignatureFailed() const
{
    auto findItr = std::find_if(m_packageModels.begin(), m_packageModels.end(), [](const ModelInfo &info) {
        return info.model->containsSignatureFailed();
    });
    return findItr != m_packageModels.end();
}

bool ProxyPackageListModel::slotInstallPackages()
{
    if (!isWorkerPrepare()) {
        return false;
    }

    if (m_packageModels.isEmpty() || m_packageModels.last().rightCount <= 0) {
        return false;
    }

    setWorkerStatus(WorkerProcessing);
    m_procModelIndex = -1;
    return nextModelInstall();
}

bool ProxyPackageListModel::slotUninstallPackage(int index)
{
    if (!isWorkerPrepare()) {
        return false;
    }

    auto modelWithIndex = findFromProxyIndex(index);
    if (!modelWithIndex.first) {
        return false;
    }

    setWorkerStatus(WorkerUnInstall);
    return modelWithIndex.first->slotUninstallPackage(modelWithIndex.second);
}

void ProxyPackageListModel::reset()
{
    for (ModelInfo &info : m_packageModels) {
        info.model->reset();
        info.count = 0;
        info.rightCount = 0;
    }
    setWorkerStatus(WorkerPrepare);
}

void ProxyPackageListModel::resetInstallStatus()
{
    for (const ModelInfo &info : m_packageModels) {
        info.model->resetInstallStatus();
    }
    setWorkerStatus(WorkerPrepare);
}

bool ProxyPackageListModel::nextModelInstall()
{
    ++m_procModelIndex;
    if (m_procModelIndex < 0) {
        setWorkerStatus(WorkerFinished);
        return false;
    }

    // check if all package finished
    if (m_procModelIndex >= m_packageModels.count()) {
        setWorkerStatus(WorkerFinished);
        Q_EMIT signalWholeProgressChanged(100);
        return true;
    }

    const ModelInfo &info = m_packageModels.at(m_procModelIndex);
    if (info.count > 0) {
        return info.model->slotInstallPackages();
    } else {
        return nextModelInstall();
    }
}

/**
   @return Pointer to an Abstract Package ListModel of a specific type,
        or null if cannot create model.
 */
ProxyPackageListModel::ModelPtr ProxyPackageListModel::modelFromType(Pkg::PackageType type)
{
    for (const ModelInfo &info : m_packageModels) {
        if (type == info.model->supportPackage()) {
            return info.model;
        }
    }

    return addModel(type);
}

ProxyPackageListModel::ModelPtr ProxyPackageListModel::addModelFromFile(const QString &packagePath)
{
    Pkg::PackageType type = Utils::detectPackage(packagePath);
    return modelFromType(type);
}

ProxyPackageListModel::ModelPtr ProxyPackageListModel::addModel(Pkg::PackageType type)
{
    ModelPtr newModel{nullptr};
    switch (type) {
        case Pkg::Uab:
            newModel = new Uab::UabPackageListModel(this);
            break;
        case Pkg::Deb:
            newModel = new DebListModel(this);
            break;
        default:
            break;
    }

    if (newModel) {
        connectModel(newModel);

        ModelInfo newInfo;
        newInfo.model = newModel;
        m_packageModels.append(newInfo);

        // sort, ensure deb package model first.
        std::sort(m_packageModels.begin(), m_packageModels.end(), [](const ModelInfo &info1, const ModelInfo &info2) {
            return info1.model->supportPackage() < info2.model->supportPackage();
        });

        // refresh all count
        int rightCount = 0;
        for (ModelInfo &info : m_packageModels) {
            info.count = info.model->rowCount();
            info.rightCount = rightCount + info.count;
            rightCount = info.rightCount;
        }
    }

    return newModel;
}

void ProxyPackageListModel::connectModel(ModelPtr model)
{
    if (!model) {
        return;
    }

    // pass-through
    QObject::connect(model, &AbstractPackageListModel::signalAppendStart, this, &ProxyPackageListModel::signalAppendStart);
    QObject::connect(model, &AbstractPackageListModel::signalAppendFinished, this, &ProxyPackageListModel::signalAppendFinished);
    QObject::connect(
        model, &AbstractPackageListModel::signalAppendFailMessage, this, &ProxyPackageListModel::signalAppendFailMessage);
    QObject::connect(
        model, &AbstractPackageListModel::signalAppendOutputInfo, this, &ProxyPackageListModel::signalAppendOutputInfo);
    QObject::connect(
        model, &AbstractPackageListModel::signalPackageCannotFind, this, &ProxyPackageListModel::signalPackageCannotFind);

    QObject::connect(model, &AbstractPackageListModel::signalLockForAuth, this, &ProxyPackageListModel::signalLockForAuth);
    QObject::connect(model, &AbstractPackageListModel::signalAuthCancel, this, [this]() {
        // if auth canceled, reset to prepare state.
        setWorkerStatus(WorkerPrepare);
        Q_EMIT signalAuthCancel();
    });
    QObject::connect(
        model, &AbstractPackageListModel::signalEnableReCancelBtn, this, &ProxyPackageListModel::signalEnableReCancelBtn);
    QObject::connect(model, &AbstractPackageListModel::signalDependResult, this, &ProxyPackageListModel::signalDependResult);
    QObject::connect(
        model, &AbstractPackageListModel::signalEnableCloseButton, this, &ProxyPackageListModel::signalEnableCloseButton);

    // pass-through, only single page need current package progress
    QObject::connect(model,
                     &AbstractPackageListModel::signalCurrentPacakgeProgressChanged,
                     this,
                     &ProxyPackageListModel::signalCurrentPacakgeProgressChanged);

    // signals forwarded through proxy
    QObject::connect(
        model, &AbstractPackageListModel::signalPackageCountChanged, this, &ProxyPackageListModel::onSourcePacakgeCountChanged);
    QObject::connect(
        model, &AbstractPackageListModel::signalWholeProgressChanged, this, &ProxyPackageListModel::onSourceWholeProgressChanged);
    QObject::connect(model,
                     &AbstractPackageListModel::signalCurrentProcessPackageIndex,
                     this,
                     &ProxyPackageListModel::onSourceCurrentProcessPackageIndex);
    // signalWorkerStart() does not need to be forwarded from the model being proxied, managed by proxy package list model.
    QObject::connect(model, &AbstractPackageListModel::signalWorkerFinished, this, &ProxyPackageListModel::onSoureWorkerFinished);

    // qt interface
    QObject::connect(model, &QAbstractListModel::dataChanged, this, &ProxyPackageListModel::onSourceDataChanged);
}

QPair<ProxyPackageListModel::ModelPtr, int> ProxyPackageListModel::findFromProxyIndex(int proxyIndex) const
{
    if (m_packageModels.isEmpty()) {
        return qMakePair(nullptr, -1);
    }

    if (proxyIndex < 0 || proxyIndex >= m_packageModels.last().rightCount) {
        return qMakePair(nullptr, -1);
    }

    auto findItr =
        std::lower_bound(m_packageModels.begin(), m_packageModels.end(), proxyIndex, [](const ModelInfo &info, int proxyIndex) {
            return info.rightCount <= proxyIndex;
        });

    if (findItr != m_packageModels.end()) {
        const int sourceIndex = proxyIndex - ((*findItr).rightCount - (*findItr).count);
        return qMakePair((*findItr).model, sourceIndex);
    }

    return qMakePair(nullptr, -1);
}

int ProxyPackageListModel::proxyIndexFromModel(ModelPtr findModel, int index)
{
    if (m_packageModels.isEmpty()) {
        return -1;
    }

    if (index < 0 || index >= m_packageModels.last().rightCount) {
        return -1;
    }

    for (const ModelInfo &info : m_packageModels) {
        if (findModel == info.model) {
            if (info.count > index) {
                return info.rightCount - info.count + index;
            }

            break;
        }
    }

    return -1;
}

void ProxyPackageListModel::onSourcePacakgeCountChanged(int count)
{
    const auto sendModel = qobject_cast<ModelPtr>(sender());
    if (sendModel && !m_packageModels.isEmpty()) {
        int newCount = 0;
        for (ModelInfo &info : m_packageModels) {
            if (sendModel == info.model) {
                info.count = count;
                // not break, update remaining model count.
            }

            newCount += info.count;
            info.rightCount = newCount;
        }

        Q_EMIT signalPackageCountChanged(m_packageModels.last().rightCount);

    } else {
        qWarning() << qPrintable("Inavlid model signal sender");
    }
}

void ProxyPackageListModel::onSourceWholeProgressChanged(int progress)
{
    const auto sendModel = qobject_cast<ModelPtr>(sender());
    if (sendModel && !m_packageModels.empty()) {
        if (m_procModelIndex < 0 || m_procModelIndex >= m_packageModels.count()) {
            return;
        }

        const ModelInfo &info = m_packageModels.at(m_procModelIndex);
        if (info.model != sendModel) {
            return;
        }

        const int allCount = m_packageModels.last().rightCount;
        const int wholeProgress = static_cast<int>(progress * (info.count * 1.0f / allCount));
        Q_EMIT signalWholeProgressChanged(wholeProgress);

    } else {
        qWarning() << qPrintable("Inavlid model signal sender");
    }
}

void ProxyPackageListModel::onSourceCurrentProcessPackageIndex(int index)
{
    const auto sendModel = qobject_cast<ModelPtr>(sender());
    if (sendModel && !m_packageModels.empty()) {
        if (m_procModelIndex < 0 || m_procModelIndex >= m_packageModels.count()) {
            return;
        }

        const ModelInfo &info = m_packageModels.at(m_procModelIndex);
        if (info.model != sendModel) {
            return;
        }

        const int proxyIndex = proxyIndexFromModel(sendModel, index);
        Q_EMIT signalCurrentProcessPackageIndex(proxyIndex);

    } else {
        qWarning() << qPrintable("Inavlid model signal sender");
    }
}

void ProxyPackageListModel::onSoureWorkerFinished()
{
    if (!m_packageModels.empty()) {
        switch (m_workerStatus) {
            case AbstractPackageListModel::WorkerProcessing:
                nextModelInstall();
                break;
            case AbstractPackageListModel::WorkerUnInstall:
                setWorkerStatus(WorkerFinished);
                break;
            default:
                break;
        }

    } else {
        qWarning() << qPrintable("Inavlid model signal sender");
    }
}

void ProxyPackageListModel::onSourceDataChanged(const QModelIndex &topLeft,
                                                const QModelIndex &bottomRight,
                                                const QVector<int> &roles)
{
    for (const ModelInfo &info : m_packageModels) {
        if (topLeft.model() == info.model) {
            if (topLeft.row() > info.count || bottomRight.row() > info.count) {
                return;
            }

            const int leftCount = info.rightCount - info.count;
            const int proxyTopIndex = leftCount + topLeft.row();
            const int proxyBottomIndex = leftCount + bottomRight.row();

            // check index valid interal
            const QModelIndex proxyTopLeft = this->index(proxyTopIndex);
            const QModelIndex proxyBottomRight = this->index(proxyBottomIndex);

            if (proxyTopLeft.isValid() && proxyBottomRight.isValid()) {
                Q_EMIT dataChanged(proxyTopLeft, proxyBottomRight, roles);
            }

            return;
        }
    }
}
