// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SFAWIDGET_H
#define SFAWIDGET_H

#include "auth_widget.h"
#include "userinfo.h"

#include <DArrowRectangle>
#include <DBlurEffectWidget>
#include <DButtonBox>
#include <DClipEffectWidget>
#include <DFloatingButton>
#include <DLabel>

#include <QWidget>

class AuthModule;
class KbLayoutWidget;
class KeyboardMonitor;

class DLineEditEx;
class QVBoxLayout;
class UserAvatar;
class QSpacerItem;

class SFAWidget : public AuthWidget
{
    Q_OBJECT

public:
    explicit SFAWidget(QWidget *parent = nullptr);

    void setModel(const SessionBaseModel *model) override;
    void setAuthType(const int type) override;
    void setAuthState(const int type, const int state, const QString &message) override;
    int getTopSpacing() const override;

public slots:
    void onRetryButtonVisibleChanged(bool visible);

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    void initUI();
    void initConnections();

    void initSingleAuth();
    void initPasswdAuth();
    void initFingerprintAuth();
    void initUKeyAuth();
    void initFaceAuth();
    void initIrisAuth();
    void initCustomAuth();

    void checkAuthResult(const int type, const int state) override;

    void syncAuthType(const QVariant &value);
    void replaceWidget(AuthModule *authModule);
    void setBioAuthStateVisible(AuthModule *authModule, bool visible);
    void updateSpaceItem();
    void updateFocus();

private:
    QVBoxLayout *m_mainLayout;

    DButtonBox *m_chooseAuthButtonBox; // 认证选择按钮
    DLabel *m_biometricAuthState;      // 生物认证状态

    QMap<int, DButtonBoxButton *> m_authButtons;
    DFloatingButton *m_retryButton;
    QSpacerItem *m_bioAuthStatePlaceHolder;
    QSpacerItem *m_bioBottomSpacingHolder;
    QSpacerItem *m_authTypeBottomSpacingHolder;
};

#endif // SFAWIDGET_H
