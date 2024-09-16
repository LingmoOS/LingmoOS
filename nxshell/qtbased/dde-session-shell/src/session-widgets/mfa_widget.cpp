// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mfa_widget.h"

#include "auth_face.h"
#include "auth_fingerprint.h"
#include "auth_iris.h"
#include "auth_password.h"
#include "auth_ukey.h"
#include "dlineeditex.h"
#include "framedatabind.h"
#include "keyboardmonitor.h"
#include "sessionbasemodel.h"
#include "useravatar.h"

MFAWidget::MFAWidget(QWidget *parent)
    : AuthWidget(parent)
    , m_index(0)
    , m_mainLayout(new QVBoxLayout(this))
{
    setObjectName(QStringLiteral("MFAWidget"));
    setAccessibleName(QStringLiteral("MFAWidget"));

    setGeometry(0, 0, 280, 176);
    setMinimumSize(280, 176);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void MFAWidget::initUI()
{
    AuthWidget::initUI();
    /* 用户名输入框 */
    std::function<void(QVariant)> accountChanged = std::bind(&MFAWidget::syncAccount, this, std::placeholders::_1);
    m_registerFunctions["MFAAccount"] = FrameDataBind::Instance()->registerFunction("MFAAccount", accountChanged);
    FrameDataBind::Instance()->refreshData("MFAAccount");

    m_mainLayout->setContentsMargins(10, 0, 10, 0);
    m_mainLayout->setSpacing(10);
    m_mainLayout->addWidget(m_userAvatar);
    m_mainLayout->addWidget(m_nameLabel, 0, Qt::AlignVCenter);
    m_mainLayout->addWidget(m_accountEdit, 0, Qt::AlignVCenter);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(m_expiredStateLabel);
    m_mainLayout->addItem(m_expiredSpacerItem);
    m_mainLayout->addWidget(m_lockButton, 0, Qt::AlignCenter);
}

void MFAWidget::initConnections()
{
    AuthWidget::initConnections();
    connect(m_model, &SessionBaseModel::authTypeChanged, this, &MFAWidget::setAuthType);
    connect(m_model, &SessionBaseModel::authStateChanged, this, &MFAWidget::setAuthState);
    connect(m_accountEdit, &DLineEditEx::textChanged, this, [](const QString &value) {
        FrameDataBind::Instance()->updateValue("MFAAccount", value);
    });
}

void MFAWidget::setModel(const SessionBaseModel *model)
{
    AuthWidget::setModel(model);

    initUI();
    initConnections();

    setAuthType(model->getAuthProperty().AuthType);
    setUser(model->currentUser());
}

void MFAWidget::setAuthType(const int type)
{
    qDebug() << "MFAWidget::setAuthType:" << type;
    m_index = 2;
    /* 面容 */
    if (type & AT_Face) {
        initFaceAuth();
        m_index++;
    } else if (m_faceAuth) {
        m_faceAuth->deleteLater();
        m_faceAuth = nullptr;
    }
    /* 虹膜 */
    if (type & AT_Iris) {
        initIrisAuth();
        m_index++;
    } else if (m_irisAuth) {
        m_irisAuth->deleteLater();
        m_irisAuth = nullptr;
    }
    /* 指纹 */
    if (type & AT_Fingerprint) {
        initFingerprintAuth();
        m_index++;
    } else if (m_fingerprintAuth) {
        m_fingerprintAuth->deleteLater();
        m_fingerprintAuth = nullptr;
    }
    /* Ukey */
    if (type & AT_Ukey) {
        initUKeyAuth();
        m_index++;
    } else if (m_ukeyAuth) {
        m_ukeyAuth->deleteLater();
        m_ukeyAuth = nullptr;
    }
    /* 密码 */
    if (type & AT_Password) {
        initPasswdAuth();
        m_index++;
    } else if (m_passwordAuth) {
        m_passwordAuth->deleteLater();
        m_passwordAuth = nullptr;
    }
    /* 账户 */
    if (type == AT_None) {
        if (m_model->currentUser()->isNoPasswordLogin()) {
            m_lockButton->setEnabled(true);
            m_accountEdit->hide();
            m_nameLabel->show();
        } else {
            m_accountEdit->clear();
            m_accountEdit->show();
            m_nameLabel->hide();
        }
    } else {
        const bool visible = m_model->isServerModel() && m_model->appType() == Login;
        m_accountEdit->setVisible(visible);
        m_nameLabel->setVisible(!visible);
    }

    updatePasswordExpiredState();
    updateGeometry();

    if (m_lockButton->isVisible() && m_lockButton->isEnabled()) {
        setFocusProxy(m_lockButton);
    }
    if (m_passwordAuth) {
        setFocusProxy(m_passwordAuth);
    }
    if (m_ukeyAuth) {
        setFocusProxy(m_ukeyAuth);
    }
    if (m_accountEdit->isVisible()) {
        setFocusProxy(m_accountEdit);
    }

    m_widgetList.clear();
    m_widgetList << m_accountEdit << m_ukeyAuth << m_passwordAuth << m_lockButton;

    for (int i = 0; i < m_widgetList.size(); i++) {
        if (m_widgetList[i]) {
            for (int j = i + 1; j < m_widgetList.size(); j++) {
                if (m_widgetList[j]) {
                    setTabOrder(m_widgetList[i]->focusProxy(), m_widgetList[j]->focusProxy());
                    i = j - 1;
                    break;
                }
            }
        }
    }
    setFocus();
}

/**
 * @brief 认证状态
 * @param type    认证类型
 * @param state  认证结果
 * @param message 消息
 */
void MFAWidget::setAuthState(const int type, const int state, const QString &message)
{
    qDebug() << "MFAWidget::setAuthState:" << type << state << message;
    switch (type) {
    case AT_Password:
        if (m_passwordAuth) {
            m_passwordAuth->setAuthState(state, message);
            FrameDataBind::Instance()->updateValue("MFPasswordAuthState", state);
            FrameDataBind::Instance()->updateValue("MFPasswordAuthMsg", message);
        }
        break;
    case AT_Fingerprint:
        if (m_fingerprintAuth) {
            m_fingerprintAuth->setAuthState(state, message);
            FrameDataBind::Instance()->updateValue("MFFingerprintAuthState", state);
            FrameDataBind::Instance()->updateValue("MFFingerprintAuthMsg", message);
        }
        break;
    case AT_Ukey:
        if (m_ukeyAuth) {
            m_ukeyAuth->setAuthState(state, message);
            FrameDataBind::Instance()->updateValue("MFUKeyAuthState", state);
            FrameDataBind::Instance()->updateValue("MFUKeyAuthMsg", message);
        }
        break;
    case AT_Face:
        if (m_faceAuth) {
            m_faceAuth->setAuthState(state, message);
            FrameDataBind::Instance()->updateValue("MFFaceAuthState", state);
            FrameDataBind::Instance()->updateValue("MFFaceAuthMsg", message);
        }
        break;
    case AT_Iris:
        if (m_irisAuth) {
            m_irisAuth->setAuthState(state, message);
            FrameDataBind::Instance()->updateValue("MFIrisAuthState", state);
            FrameDataBind::Instance()->updateValue("MFIrisAuthMsg", message);
        }
        break;
    case AT_All:
        checkAuthResult(type, state);
        break;
    default:
        break;
    }
}

void MFAWidget::autoUnlock()
{
    if (m_model->currentUser()->isNoPasswordLogin()) {
        m_lockButton->click();
    }
}

/**
 * @brief 密码认证
 */
void MFAWidget::initPasswdAuth()
{
    if (m_passwordAuth) {
        m_passwordAuth->reset();
        m_mainLayout->insertWidget(m_index, m_passwordAuth);
        return;
    }
    m_passwordAuth = new AuthPassword(this);
    m_passwordAuth->setCurrentUid(m_model->currentUser()->uid());
    m_passwordAuth->setCapsLockVisible(m_capslockMonitor->isCapslockOn());
    m_passwordAuth->setPasswordHint(m_user->passwordHint());
    m_mainLayout->insertWidget(m_index, m_passwordAuth);

    connect(m_passwordAuth, &AuthPassword::activeAuth, this, [this] {
        emit requestStartAuthentication(m_model->currentUser()->name(), AT_Password);
    });
    connect(m_passwordAuth, &AuthPassword::requestAuthenticate, this, [this] {
        const QString &text = m_passwordAuth->lineEditText();
        if (text.isEmpty()) {
            return;
        }
        m_passwordAuth->setAuthStateStyle(LOGIN_SPINNER);
        m_passwordAuth->setAnimationState(true);
        m_passwordAuth->setLineEditEnabled(false);
        emit sendTokenToAuth(m_model->currentUser()->name(), AT_Password, text);
    });
    connect(m_passwordAuth, &AuthPassword::authFinished, this, [this](const int value) {
        checkAuthResult(AT_Password, value);
    });
    connect(m_lockButton, &QPushButton::clicked, m_passwordAuth, &AuthPassword::requestAuthenticate);
    connect(m_capslockMonitor, &KeyboardMonitor::capslockStatusChanged, m_passwordAuth, &AuthPassword::setCapsLockVisible);

    /* 输入框数据同步 */
    std::function<void(QVariant)> passwordChanged = std::bind(&MFAWidget::syncPassword, this, std::placeholders::_1);
    m_registerFunctions["MFPasswordAuth"] = FrameDataBind::Instance()->registerFunction("MFPasswordAuth", passwordChanged);
    connect(m_passwordAuth, &AuthPassword::lineEditTextChanged, this, [](const QString &value) {
        FrameDataBind::Instance()->updateValue("MFPasswordAuth", value);
    });
    FrameDataBind::Instance()->refreshData("MFPasswordAuth");
    /* 重置密码可见性数据同步 */
    std::function<void(QVariant)> resetPasswordVisibleChanged = std::bind(&AuthWidget::syncPasswordResetPasswordVisibleChanged, this, std::placeholders::_1);
    m_registerFunctions["MFResetPasswordVisible"] = FrameDataBind::Instance()->registerFunction("MFResetPasswordVisible", resetPasswordVisibleChanged);
    connect(m_passwordAuth, &AuthPassword::resetPasswordMessageVisibleChanged, this, [ = ](const bool value) {
        FrameDataBind::Instance()->updateValue("MFResetPasswordVisible", value);
    });
    FrameDataBind::Instance()->refreshData("MFResetPasswordVisible");

    connect(m_passwordAuth, &AuthPassword::requestChangeFocus, this, &MFAWidget::updateFocusPosition);
}

/**
 * @brief 指纹认证
 */
void MFAWidget::initFingerprintAuth()
{
    if (m_fingerprintAuth) {
        m_fingerprintAuth->reset();
        m_mainLayout->insertWidget(m_index, m_fingerprintAuth);
        return;
    }
    m_fingerprintAuth = new AuthFingerprint(this);
    m_fingerprintAuth->setAuthFactorType(DDESESSIONCC::MultiAuthFactor);
    m_mainLayout->insertWidget(m_index, m_fingerprintAuth);

    connect(m_fingerprintAuth, &AuthFingerprint::activeAuth, this, [this] {
        emit requestStartAuthentication(m_model->currentUser()->name(), AT_Fingerprint);
    });
    connect(m_fingerprintAuth, &AuthFingerprint::authFinished, this, [this](const int value) {
        checkAuthResult(AT_Fingerprint, value);
    });
}

/**
 * @brief Ukey认证
 */
void MFAWidget::initUKeyAuth()
{
    if (m_ukeyAuth) {
        m_ukeyAuth->reset();
        m_mainLayout->insertWidget(m_index, m_ukeyAuth);
        return;
    }
    m_ukeyAuth = new AuthUKey(this);
    m_ukeyAuth->setCapsLockVisible(m_capslockMonitor->isCapslockOn());
    m_mainLayout->insertWidget(m_index, m_ukeyAuth);

    connect(m_ukeyAuth, &AuthUKey::activeAuth, this, [this] {
        emit requestStartAuthentication(m_model->currentUser()->name(), AT_Ukey);
    });
    connect(m_ukeyAuth, &AuthUKey::requestAuthenticate, this, [this] {
        const QString &text = m_ukeyAuth->lineEditText();
        if (text.isEmpty()) {
            return;
        }
        m_ukeyAuth->setAuthStateStyle(LOGIN_SPINNER);
        m_ukeyAuth->setAnimationState(true);
        m_ukeyAuth->setLineEditEnabled(false);
        emit sendTokenToAuth(m_model->currentUser()->name(), AT_Ukey, text);
    });
    connect(m_lockButton, &QPushButton::clicked, m_ukeyAuth, &AuthUKey::requestAuthenticate);
    connect(m_ukeyAuth, &AuthUKey::authFinished, this, [this](const bool state) {
        checkAuthResult(AT_Ukey, state);
    });
    connect(m_capslockMonitor, &KeyboardMonitor::capslockStatusChanged, m_ukeyAuth, &AuthUKey::setCapsLockVisible);

    /* 输入框数据同步 */
    std::function<void(QVariant)> PINChanged = std::bind(&MFAWidget::syncUKey, this, std::placeholders::_1);
    m_registerFunctions["MFUKeyAuth"] = FrameDataBind::Instance()->registerFunction("MFUKeyAuth", PINChanged);
    connect(m_ukeyAuth, &AuthUKey::lineEditTextChanged, this, [this](const QString &value) {
        FrameDataBind::Instance()->updateValue("MFUKeyAuth", value);
        if (m_model->getAuthProperty().PINLen > 0 && value.size() >= m_model->getAuthProperty().PINLen) {
            emit m_ukeyAuth->requestAuthenticate();
        }
    });
    FrameDataBind::Instance()->refreshData("MFUKeyAuth");

    connect(m_ukeyAuth, &AuthUKey::requestChangeFocus, this, &MFAWidget::updateFocusPosition);
}

/**
 * @brief 人脸认证
 */
void MFAWidget::initFaceAuth()
{
    if (m_faceAuth) {
        m_faceAuth->reset();
        m_mainLayout->insertWidget(m_index, m_faceAuth);
        return;
    }
    m_faceAuth = new AuthFace(this);
    m_faceAuth->setAuthFactorType(DDESESSIONCC::MultiAuthFactor);
    m_mainLayout->insertWidget(m_index, m_faceAuth);

    connect(m_faceAuth, &AuthFace::activeAuth, this, [this] {
        emit requestStartAuthentication(m_model->currentUser()->name(), AT_Face);
    });
    connect(m_faceAuth, &AuthFace::authFinished, this, [this](const bool state) {
        checkAuthResult(AT_Face, state);
    });
}

/**
 * @brief 虹膜认证
 */
void MFAWidget::initIrisAuth()
{
    if (m_irisAuth) {
        m_irisAuth->reset();
        m_mainLayout->insertWidget(m_index, m_irisAuth);
        return;
    }
    m_irisAuth = new AuthIris(this);
    m_irisAuth->setAuthFactorType(DDESESSIONCC::MultiAuthFactor);
    m_mainLayout->insertWidget(m_index, m_irisAuth);

    connect(m_irisAuth, &AuthIris::activeAuth, this, [this] {
        emit requestStartAuthentication(m_model->currentUser()->name(), AT_Iris);
    });
    connect(m_irisAuth, &AuthIris::authFinished, this, [this](const bool state) {
        checkAuthResult(AT_Iris, state);
    });
}

/**
 * @brief 检查多因子认证结果
 *
 * @param type
 * @param succeed
 */
void MFAWidget::checkAuthResult(const int type, const int state)
{
    if (type == AT_Password && state == AS_Success) {
        if (m_fingerprintAuth && m_fingerprintAuth->authState() == AS_Locked) {
            m_fingerprintAuth->reset();
            emit m_fingerprintAuth->activeAuth(AT_Fingerprint);
        }
        if (m_faceAuth && m_faceAuth->authState() == AS_Locked) {
            m_faceAuth->reset();
            emit m_faceAuth->activeAuth(AT_Face);
        }
        if (m_irisAuth && m_irisAuth->authState() == AS_Locked) {
            m_irisAuth->reset();
            emit m_irisAuth->activeAuth(AT_Iris);
        }
    }

    if (state == AS_Cancel) {
        if (m_ukeyAuth) {
            m_ukeyAuth->setAuthState(AS_Cancel, "Cancel");
        }
        if (m_passwordAuth) {
            m_passwordAuth->setAuthState(AS_Cancel, "Cancel");
        }
        if (m_fingerprintAuth) {
            m_fingerprintAuth->setAuthState(AS_Cancel, "Cancel");
        }
    }
}

void MFAWidget::updateFocusPosition()
{
    AuthModule *module = static_cast<AuthModule *>(sender());
    if (module == m_passwordAuth) {
        if (m_ukeyAuth && m_ukeyAuth->authState() == AS_Prompt) {
            setFocusProxy(m_ukeyAuth);
        } else {
            setFocusProxy(m_lockButton);
        }
    } else {
        if (m_passwordAuth && m_passwordAuth->isEnabled()) {
            setFocusProxy(m_passwordAuth);
        } else {
            setFocusProxy(m_lockButton);
        }
    }
    setFocus();
}

/**
 * @brief 计算需要增加的顶部间隔(加在用户头像的上方)，以实现用户头像到屏幕顶端的距离为屏幕高度的35%
 */
int MFAWidget::getTopSpacing() const
{
    const int calcTopHeight = static_cast<int>(topLevelWidget()->geometry().height() * AUTH_WIDGET_TOP_SPACING_PERCENT);
    // 验证类型数量*高度
    const int authWidgetHeight = (m_index - 2) * 47 + MIN_AUTH_WIDGET_HEIGHT;

    // 当分辨率过低时，如果仍然保持用户头像到屏幕顶端的距离为屏幕高度的35%，那么验证窗口整体时偏下的。
    // 计算当验证窗口居中时距离屏幕顶端的高度，与屏幕高度*0.35对比取较小值。
    const int centerTop = static_cast<int>((topLevelWidget()->geometry().height() - authWidgetHeight) / 2);
    const int topHeight = qMin(calcTopHeight, centerTop);

    // 需要额外增加的顶部间隔高度 = 屏幕高度*0.35 - 验证窗口高度 - 时间控件高度 - 布局间隔
    const int deltaY = topHeight - calcCurrentHeight(LOCK_CONTENT_TOPBOTTOM_WIDGET_HEIGHT)
            - calcCurrentHeight(LOCK_CONTENT_CENTER_LAYOUT_MARGIN);

    return qMax(15, deltaY);
}
