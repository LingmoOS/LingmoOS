// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTHFINGERPRINT_H
#define AUTHFINGERPRINT_H

#include "auth_module.h"

#define Fingerprint_Auth QStringLiteral(":/misc/images/auth/fingerprint.svg")

class AuthFingerprint : public AuthModule
{
    Q_OBJECT
public:
    explicit AuthFingerprint(QWidget *parent = nullptr);

    void reset();

    void setAnimationState(const bool start) override;
    void setAuthState(const int state, const QString &result) override;
    void setLimitsInfo(const LimitsInfo &info) override;
    void setAuthFactorType(AuthFactorType authFactorType) override;
    void doAnimation() override;

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void initUI();
    void initConnections();
    void updateUnlockPrompt() override;

private:
    int m_aniIndex;
    DLabel *m_textLabel;
};

#endif // AUTHFINGERPRINT_H
