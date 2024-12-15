// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTHFACE_H
#define AUTHFACE_H

#include "auth_module.h"

#define Face_Auth QStringLiteral(":/misc/images/auth/face.svg")

class AuthFace : public AuthModule
{
    Q_OBJECT
public:
    explicit AuthFace(QWidget *parent = nullptr);

    void reset();

public slots:
    void setAuthState(const int state, const QString &result) override;
    void setAnimationState(const bool start) override;
    void setLimitsInfo(const LimitsInfo &info) override;
    void setAuthFactorType(AuthFactorType authFactorType) override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initUI();
    void initConnections();
    void updateUnlockPrompt() override;
    void doAnimation() override;

signals:
    void retryButtonVisibleChanged(bool visible);

private:
    int m_aniIndex;
    DLabel *m_textLabel;
};

#endif // AUTHFACE_H
