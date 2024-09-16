// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTHIRIS_H
#define AUTHIRIS_H

#include "auth_module.h"

#define Iris_Auth QStringLiteral(":/misc/images/auth/iris.svg")

class AuthIris : public AuthModule
{
    Q_OBJECT
public:
    explicit AuthIris(QWidget *parent = nullptr);

    void reset();

    void setAuthFactorType(AuthFactorType authFactorType) override;

public slots:
    void setAuthState(const int state, const QString &result) override;
    void setAnimationState(const bool start) override;
    void setLimitsInfo(const LimitsInfo &info) override;

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

#endif // AUTHIRIS_H
