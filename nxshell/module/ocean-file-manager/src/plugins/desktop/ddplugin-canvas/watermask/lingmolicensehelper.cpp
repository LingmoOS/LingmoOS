// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmolicensehelper.h"

#include "licenceInterface.h"

#include <QApplication>
#include <QtConcurrent>

using namespace ddplugin_canvas;

class LingmoLicenseHelperGlobal : public LingmoLicenseHelper {};
Q_GLOBAL_STATIC(LingmoLicenseHelperGlobal, lingmoLicenseHelperGlobal)

LingmoLicenseHelper *LingmoLicenseHelper::instance()
{
    return lingmoLicenseHelperGlobal;
}

LingmoLicenseHelper::LingmoLicenseHelper(QObject *parent) : QObject(parent)
{
    Q_ASSERT(qApp->thread() == thread());
    reqTimer.setInterval(500);
    reqTimer.setSingleShot(true);

    connect(&reqTimer, &QTimer::timeout, this, &LingmoLicenseHelper::requestLicenseState);
}

LingmoLicenseHelper::~LingmoLicenseHelper()
{
    work.waitForFinished();
    delete licenseInterface;
    licenseInterface = nullptr;
}

void LingmoLicenseHelper::init()
{
    std::call_once(initFlag, [this](){
        work = QtConcurrent::run(&LingmoLicenseHelper::createInterface);
    });
}

void LingmoLicenseHelper::delayGetState()
{
    reqTimer.start();
}

void LingmoLicenseHelper::requestLicenseState()
{
    if (work.isRunning() || !licenseInterface) {
        fmWarning() << "requetLicenseState: interface is invalid.";
        return;
    }

    work = QtConcurrent::run(&LingmoLicenseHelper::getLicenseState, this);
}

void LingmoLicenseHelper::initFinshed(void *interface)
{
    Q_ASSERT(interface);
    Q_ASSERT(!licenseInterface);
    licenseInterface = static_cast<ComLingmoLicenseInterface *>(interface);
    connect(licenseInterface, &ComLingmoLicenseInterface::LicenseStateChange,
                     this, &LingmoLicenseHelper::requestLicenseState);
    work.waitForFinished();
    fmInfo() << "interface inited";

    requestLicenseState();
}

void LingmoLicenseHelper::createInterface()
{
    fmInfo() << "create ComLingmoLicenseInterface...";
    auto licenseInterface = new ComLingmoLicenseInterface(
            "com.lingmo.license",
            "/com/lingmo/license/Info",
            QDBusConnection::systemBus());

    licenseInterface->moveToThread(qApp->thread());
    fmInfo() << "create /com/lingmo/license/Info ...";

    QMetaObject::invokeMethod(LingmoLicenseHelper::instance(), "initFinshed", Q_ARG(void *, licenseInterface));
}

void LingmoLicenseHelper::getLicenseState(LingmoLicenseHelper *self)
{
    Q_ASSERT(self);
    Q_ASSERT(self->licenseInterface);
    fmInfo() << "get active state from com.lingmo.license.Info";
    int state = self->licenseInterface->authorizationState();
    LicenseProperty prop = self->getServiceProperty();

    if (prop == LicenseProperty::Noproperty) {
        fmInfo() << "no service property obtained,try to get AuthorizetionProperty";
        prop = self->getAuthorizationProperty();
    }

    fmInfo() << "Get AuthorizationState" << state << prop;
    emit self->postLicenseState(state, prop);
}

LingmoLicenseHelper::LicenseProperty LingmoLicenseHelper::getServiceProperty()
{
    // 不直接使用serviceProperty接口，需要通过QVariant是否有效判断接口是否存在
    LicenseProperty prop = LicenseProperty::Noproperty;
    QVariant servProp = licenseInterface->property("ServiceProperty");
    if (!servProp.isValid()) {
        fmInfo() << "no such property: ServiceProperty in license";
    } else {
        bool ok = false;
        prop = servProp.toInt(&ok) ? LicenseProperty::Secretssecurity : LicenseProperty::Noproperty;
        if (!ok) {
            fmWarning() << "invalid value of serviceProperty" << servProp;
            prop = LicenseProperty::Noproperty;
        }
    }
    return prop;
}

LingmoLicenseHelper::LicenseProperty LingmoLicenseHelper::getAuthorizationProperty()
{
    // 不直接使用AuthorizationProperty接口，需要通过QVariant是否有效判断接口是否存在
    LicenseProperty prop = LicenseProperty::Noproperty;
    QVariant authprop = licenseInterface->property("AuthorizationProperty");
    if (!authprop.isValid()) {
        fmInfo() << "no such property: AuthorizationProperty in license.";
    } else {
        bool ok = false;
        int value = authprop.toInt(&ok);
        static QMap<int, LicenseProperty> licenseProperty = {
            {1, LicenseProperty::Government},
            {2, LicenseProperty::Enterprise},
            {5, LicenseProperty::Office},
            {6, LicenseProperty::BusinessSystem},
            {7, LicenseProperty::Equipment},
        };

        prop = licenseProperty.value(value, LicenseProperty::Noproperty);
        if (!ok) {
            fmWarning() << "invalid value of AuthorizationProperty" << authprop;
            prop = LicenseProperty::Noproperty;
        }
    }
    return prop;
}
