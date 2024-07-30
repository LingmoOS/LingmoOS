/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_PPP_SETTING_H
#define NETWORKMANAGERQT_PPP_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class PppSettingPrivate;

/**
 * Represents ppp setting
 */
class NETWORKMANAGERQT_EXPORT PppSetting : public Setting
{
public:
    typedef QSharedPointer<PppSetting> Ptr;
    typedef QList<Ptr> List;
    PppSetting();
    explicit PppSetting(const Ptr &other);
    ~PppSetting() override;

    QString name() const override;

    void setNoAuth(bool require);
    bool noAuth() const;

    void setRefuseEap(bool refuse);
    bool refuseEap() const;

    void setRefusePap(bool refuse);
    bool refusePap() const;

    void setRefuseChap(bool refuse);
    bool refuseChap() const;

    void setRefuseMschap(bool refuse);
    bool refuseMschap() const;

    void setRefuseMschapv2(bool refuse);
    bool refuseMschapv2() const;

    void setNoBsdComp(bool require);
    bool noBsdComp() const;

    void setNoDeflate(bool require);
    bool noDeflate() const;

    void setNoVjComp(bool require);
    bool noVjComp() const;

    void setRequireMppe(bool require);
    bool requireMppe() const;

    void setRequireMppe128(bool require);
    bool requireMppe128() const;

    void setMppeStateful(bool used);
    bool mppeStateful() const;

    void setCRtsCts(bool control);
    bool cRtsCts() const;

    void setBaud(quint32 baud);
    quint32 baud() const;

    void setMru(quint32 mru);
    quint32 mru() const;

    void setMtu(quint32 mtu);
    quint32 mtu() const;

    void setLcpEchoFailure(quint32 number);
    quint32 lcpEchoFailure() const;

    void setLcpEchoInterval(quint32 interval);
    quint32 lcpEchoInterval() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    PppSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(PppSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const PppSetting &setting);

}

#endif // NETWORKMANAGERQT_PPP_SETTING_H
