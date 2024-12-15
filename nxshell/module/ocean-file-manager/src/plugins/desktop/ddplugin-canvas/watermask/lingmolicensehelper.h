// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINGMOLICENSEHELPER_H
#define LINGMOLICENSEHELPER_H

#include "ddplugin_canvas_global.h"

#include <QObject>
#include <QFuture>
#include <QTimer>

#include <mutex>

class ComLingmoLicenseInterface;

namespace ddplugin_canvas {

class LingmoLicenseHelper : public QObject
{
    Q_OBJECT
public:
    enum LicenseState {
        Unauthorized = 0,
        Authorized,
        AuthorizedLapse,
        TrialAuthorized,
        TrialExpired
    };
    enum LicenseProperty{
        Noproperty = 0,
        Secretssecurity,
        Government,
        Enterprise,
        Office,
        BusinessSystem,
        Equipment
    };

    static LingmoLicenseHelper *instance();
    void init();
    void delayGetState();
signals:
    void postLicenseState(int state, int prop);

protected:
    explicit LingmoLicenseHelper(QObject *parent = nullptr);
    ~LingmoLicenseHelper();

private slots:
    void requestLicenseState();
    void initFinshed(void *interface);

private:
    static void createInterface();
    static void getLicenseState(LingmoLicenseHelper *);
    LicenseProperty getServiceProperty();
    LicenseProperty getAuthorizationProperty();
private:
    std::once_flag initFlag;
    QFuture<void> work;
    QTimer reqTimer;
    ComLingmoLicenseInterface *licenseInterface = nullptr;
};

}
#endif // LINGMOLICENSEHELPER_H
