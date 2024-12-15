// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "service_manager_worker.h"
#include "ddlog.h"
#include "dbus/dbus_common.h"
#include "dbus/dbus_properties_interface.h"
#include "dbus/environment_file.h"
#include "dbus/systemd1_manager_interface.h"
#include "dbus/systemd1_service_interface.h"
#include "dbus/systemd1_unit_interface.h"
#include "dbus/unit_file_info.h"
#include "dbus/unit_info.h"
#include "service/system_service_entry.h"
#include "service_manager.h"

#include <QDebug>
#include <QtConcurrent>

using namespace DDLog;
ServiceManagerWorker::ServiceManagerWorker(QObject *parent)
    : QObject(parent)
{
}

void ServiceManagerWorker::startJob()
{
    QList<SystemServiceEntry> list;
    // id hash, only keys are used here
    QHash<QString, int> hash;
    ErrorContext ec;

    Systemd1ManagerInterface mgrIf(DBUS_SYSTEMD1_SERVICE,
                                   kSystemDObjectPath.path(),
                                   QDBusConnection::systemBus());

    auto unitFilesResult = mgrIf.ListUnitFiles();
    ec = unitFilesResult.first;
    if (ec) {
        qCDebug(app) << "ListUnitFiles failed:" << ec.getErrorName() << ec.getErrorMessage();
    }
    UnitFileInfoList unitFiles = unitFilesResult.second;

    auto unitsResult = mgrIf.ListUnits();
    ec = unitsResult.first;
    if (ec) {
        qCDebug(app) << "ListUnits failed:" << ec.getErrorName() << ec.getErrorMessage();
    }
    UnitInfoList units = unitsResult.second;
    for (auto u : units) {
        if (u.getName().endsWith(UnitTypeServiceSuffix)) {
            hash[u.getName()] = 0;
        }
    }

    // filter-out non service type units
    std::function<bool(const UnitInfo &)> fltUnits;
    fltUnits = [](const UnitInfo &unit) -> bool {
        return unit.getName().endsWith(UnitTypeServiceSuffix);
    };
    std::function<SystemServiceEntry(const UnitInfo &)> mapUnit = [&mgrIf](const UnitInfo &unit) {
        ErrorContext ec1 {};
        SystemServiceEntry entry {};
        // SName
        entry.setSName(unit.getName().left(unit.getName().lastIndexOf('.')));
        // LoadState
        entry.setLoadState(unit.getLoadState());
        // ActiveState
        entry.setActiveState(unit.getActiveState());
        // subState
        entry.setSubState(unit.getSubState());
        entry.setUnitObjectPath(unit.getUnitObjectPath());
        // Description
        entry.setDescription(unit.getDescription());

        Systemd1UnitInterface unitIf(DBUS_SYSTEMD1_SERVICE,
                                     unit.getUnitObjectPath(),
                                     QDBusConnection::systemBus());
        // Id
        auto id = unitIf.getId();
        ec1 = id.first;
        if (ec1) {
            qCDebug(app) << "call getId failed:" << ec1.getErrorName() << ec1.getErrorMessage();
        } else {
            entry.setId(id.second);
        }

        auto bStart = unitIf.canStart();
        ec1 = bStart.first;
        if (ec1) {
            qCDebug(app) << "call canStart failed:" << ec1.getErrorName() << ec1.getErrorMessage();
        } else {
            entry.setCanStart(bStart.second);
        }

        auto bStop = unitIf.canStop();
        ec1 = bStop.first;
        if (ec1) {
            qCDebug(app) << "call canStart failed:" << ec1.getErrorName() << ec1.getErrorMessage();
        } else {
            entry.setCanStop(bStop.second);
        }

        auto bReload = unitIf.canReload();
        ec1 = bReload.first;
        if (ec1) {
            qCDebug(app) << "call canStart failed:" << ec1.getErrorName() << ec1.getErrorMessage();
        } else {
            entry.setCanReload(bReload.second);
        }

        // mainPID
        Systemd1ServiceInterface svcIf(DBUS_SYSTEMD1_SERVICE,
                                       unit.getUnitObjectPath(),
                                       QDBusConnection::systemBus());
        auto pid = svcIf.getMainPID();
        ec1 = pid.first;
        if (ec1) {
            qCDebug(app) << "getMainPID failed" << ec1.getErrorName() << ec1.getErrorMessage();
        } else {
            entry.setMainPID(pid.second);
        }

        // unit state
        auto state = mgrIf.GetUnitFileState(unit.getName());
        ec1 = state.first;
        if (ec1) {
            qCDebug(app) << "getUnitFileState failed" << ec1.getErrorName() << ec1.getErrorMessage();
        } else {
            entry.setState(state.second);
        }

        // startupType
        entry.setStartupType(ServiceManager::getServiceStartupType(
                entry.getSName(),
                entry.getState()));

        return entry;
    };
    std::function<void(QList<SystemServiceEntry> & elist, const SystemServiceEntry &entry)> addServiceEntryFromUnitInfo;
    addServiceEntryFromUnitInfo = [](QList<SystemServiceEntry> &elist, const SystemServiceEntry &entry) {
        elist << entry;
    };
    auto fltufe = QtConcurrent::filter(units, fltUnits);
    fltufe.waitForFinished();
    auto ufe = QtConcurrent::mappedReduced<QList<SystemServiceEntry>>(units, mapUnit, addServiceEntryFromUnitInfo);

    std::function<void(QList<SystemServiceEntry> & elist, const SystemServiceEntry &entry)> addServiceEntryFromUnitFileInfo;
    addServiceEntryFromUnitFileInfo = [&hash](QList<SystemServiceEntry> &elist, const SystemServiceEntry &entry) {
        if (!hash.contains(entry.getId())) {
            elist << entry;
        }
    };
    // filter-out non service type unit-files
    std::function<bool(const UnitFileInfo &)> fltUnitFiles;
    fltUnitFiles = [&hash](const UnitFileInfo &unitFile) -> bool {
        auto id = unitFile.getName().mid(unitFile.getName().lastIndexOf('/') + 1);
        if (hash.contains(id)) {
            return false;
        } else {
            return unitFile.getName().endsWith(UnitTypeServiceSuffix);
        }
    };
    std::function<SystemServiceEntry(const UnitFileInfo &)> mapUnitFiles;
    mapUnitFiles = [](const UnitFileInfo &unf) {
        SystemServiceEntry entry {};
        ErrorContext ec1 {};

        auto id = unf.getName().mid(unf.getName().lastIndexOf('/') + 1);
        auto sname = id;
        sname.chop(strlen(UnitTypeServiceSuffix));

        entry.setSName(sname);
        entry.setState(unf.getStatus());
        entry.setStartupType(ServiceManager::getServiceStartupType(
                entry.getSName(),
                entry.getState()));
        if (sname.endsWith('@')) {
            // read description from unit file
            auto desc = readUnitDescriptionFromUnitFile(unf.getName());
            entry.setDescription(desc);
        } else {
            auto o = Systemd1UnitInterface::normalizeUnitPath(id);
            Systemd1UnitInterface unitIf(DBUS_SYSTEMD1_SERVICE,
                                         o.path(),
                                         QDBusConnection::systemBus());
            Systemd1ServiceInterface svcIf(DBUS_SYSTEMD1_SERVICE,
                                           o.path(),
                                           QDBusConnection::systemBus());

            // Id
            auto id1 = unitIf.getId();
            ec1 = id1.first;
            if (ec1) {
                qCDebug(app) << "call getId failed:" << ec1.getErrorName() << ec1.getErrorMessage();
            } else {
                entry.setId(id1.second);
            }

            auto desc = unitIf.getDescription();
            ec1 = desc.first;
            if (ec1) {
                qCDebug(app) << "getDescription failed:" << ec1.getErrorName() << ec1.getErrorMessage();
            } else {
                entry.setDescription(desc.second);
            }

            auto actState = unitIf.getActiveState();
            ec1 = actState.first;
            if (ec1) {
                qCDebug(app) << "getActiveState failed:" << ec1.getErrorName() << ec1.getErrorMessage();
            } else {
                entry.setActiveState(actState.second);
            }

            auto loadState = unitIf.getLoadState();
            ec1 = loadState.first;
            if (ec1) {
                qCDebug(app) << "getLoadState failed:" << ec1.getErrorName() << ec1.getErrorMessage();
            } else {
                entry.setLoadState(loadState.second);
            }

            auto subState = unitIf.getSubState();
            ec1 = subState.first;
            if (ec1) {
                qCDebug(app) << "getSubState failed:" << ec1.getErrorName() << ec1.getErrorMessage();
            } else {
                entry.setSubState(subState.second);
            }

            auto mainPID = svcIf.getMainPID();
            ec1 = mainPID.first;
            if (ec1) {
                qCDebug(app) << "getMainPID failed:" << ec1.getErrorName() << ec1.getErrorMessage();
            } else {
                entry.setMainPID(mainPID.second);
            }
        }

        return entry;
    };
    auto fltuffe = QtConcurrent::filter(unitFiles, fltUnitFiles);
    fltuffe.waitForFinished();
    auto uffe = QtConcurrent::mappedReduced<QList<SystemServiceEntry>>(unitFiles, mapUnitFiles, addServiceEntryFromUnitFileInfo);

    ufe.waitForFinished();
    uffe.waitForFinished();

    list << ufe.result();
    list << uffe.result();
    Q_EMIT resultReady(list);
}

QString ServiceManagerWorker::readUnitDescriptionFromUnitFile(const QString &path)
{
    FILE *fp { nullptr };
    const int BLEN { 2048 };
    QByteArray buf { BLEN, 0 };
    QByteArray desc { BLEN, 0 };
    bool b { false };

    fp = fopen(path.toLocal8Bit(), "r");
    QString descStr;
    if (fp) {
        while ((fgets(buf.data(), BLEN, fp))) {
            descStr = QString("Description=%1").arg(buf.data());
            QStringList descStrList = descStr.split(QRegExp("[\n]"), QString::SkipEmptyParts);
            if (descStrList.size() > 0) {
                descStr = descStrList.at(0);
                b = true;
                break;
            } else {
                b = false;
            }
        }
        fclose(fp);

        if (b) {
            return descStr;
        } else {
            return {};
        }
    }
    return {};
}
