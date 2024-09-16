// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sfa_widget.h"

#include "auth_custom.h"
#include "auth_face.h"
#include "auth_fingerprint.h"
#include "auth_iris.h"
#include "auth_password.h"
#include "auth_single.h"
#include "auth_ukey.h"
#include "dlineeditex.h"
#include "framedatabind.h"
#include "keyboardmonitor.h"
#include "modules_loader.h"
#include "sessionbasemodel.h"
#include "useravatar.h"

#include <DFontSizeManager>
#include <DHiDPIHelper>

#include <QSpacerItem>

using namespace dss::module;

const QSize AuthButtonSize(60, 36);
const QSize AuthButtonIconSize(24, 24);

SFAWidget::SFAWidget(QWidget *parent)
    : AuthWidget(parent)
    , m_mainLayout(new QVBoxLayout(this))
    , m_chooseAuthButtonBox(nullptr)
    , m_biometricAuthState(nullptr)
    , m_retryButton(new DFloatingButton(this))
    , m_bioAuthStatePlaceHolder(new QSpacerItem(0, BIO_AUTH_STATE_PLACE_HOLDER_HEIGHT))
    , m_bioBottomSpacingHolder(new QSpacerItem(0, BIO_AUTH_STATE_BOTTOM_SPACING))
    , m_authTypeBottomSpacingHolder(new QSpacerItem(0, CHOOSE_AUTH_TYPE_BUTTON_BOTTOM_SPACING))
{
    setObjectName(QStringLiteral("SFAWidget"));
    setAccessibleName(QStringLiteral("SFAWidget"));

    setGeometry(0, 0, 280, 176);
    setMinimumSize(280, 176);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void SFAWidget::initUI()
{
    AuthWidget::initUI();
    /* 用户名输入框 */
    std::function<void(QVariant)> accountChanged = std::bind(&SFAWidget::syncAccount, this, std::placeholders::_1);
    m_registerFunctions["SFAAccount"] = m_frameDataBind->registerFunction("SFAAccount", accountChanged);
    m_frameDataBind->refreshData("SFAAccount");
    /* 认证选择 */
    m_chooseAuthButtonBox = new DButtonBox(this);
    m_chooseAuthButtonBox->setOrientation(Qt::Horizontal);
    m_chooseAuthButtonBox->setFocusPolicy(Qt::NoFocus);
    m_chooseAuthButtonBox->setContentsMargins(0, 0, 0, 0);
    m_chooseAuthButtonBox->setMaximumHeight(36);

    /* 生物认证状态 */
    m_biometricAuthState = new DLabel(this);
    m_biometricAuthState->hide();

    /* 重试按钮 */
    m_retryButton->setIcon(QIcon(":/img/bottom_actions/reboot.svg"));
    m_retryButton->hide();

    m_mainLayout->setContentsMargins(10, 0, 10, 0);
    m_mainLayout->setSpacing(10);
    m_mainLayout->addWidget(m_biometricAuthState, 0, Qt::AlignCenter);
    m_mainLayout->addItem(m_bioAuthStatePlaceHolder);
    m_mainLayout->addItem(m_bioBottomSpacingHolder);
    m_mainLayout->addWidget(m_chooseAuthButtonBox, 0, Qt::AlignCenter);
    m_mainLayout->addItem(m_authTypeBottomSpacingHolder);

    m_mainLayout->addWidget(m_userAvatar);
    m_mainLayout->addWidget(m_nameLabel, 0, Qt::AlignVCenter);
    m_mainLayout->addWidget(m_accountEdit, 0, Qt::AlignVCenter);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(m_expiredStateLabel);
    m_mainLayout->addItem(m_expiredSpacerItem);
    m_mainLayout->addWidget(m_lockButton, 0, Qt::AlignCenter);
    m_mainLayout->addWidget(m_retryButton, 0, Qt::AlignCenter);
}

void SFAWidget::initConnections()
{
    AuthWidget::initConnections();
    connect(m_model, &SessionBaseModel::authTypeChanged, this, &SFAWidget::setAuthType);
    connect(m_model, &SessionBaseModel::authStateChanged, this, &SFAWidget::setAuthState);
    connect(m_accountEdit, &DLineEditEx::textChanged, this, [this](const QString &value) {
        m_frameDataBind->updateValue(QStringLiteral("SFAAccount"), value);
        m_lockButton->setEnabled(!value.isEmpty());
    });
}

void SFAWidget::setModel(const SessionBaseModel *model)
{
    AuthWidget::setModel(model);

    initUI();
    initConnections();

    setAuthType(model->getAuthProperty().AuthType);
    setUser(model->currentUser());
}

/**
 * @brief 设置认证类型
 * @param type  认证类型
 */
void SFAWidget::setAuthType(const int type)
{
    qDebug() << "SFAWidget::setAuthType:" << type;
    if (type & AT_Password) {
        initPasswdAuth();
    } else if (m_passwordAuth) {
        delete m_passwordAuth;
        m_passwordAuth = nullptr;
        m_authButtons.value(AT_Password)->deleteLater();
        m_authButtons.remove(AT_Password);
        m_frameDataBind->clearValue("SFPasswordAuthState");
        m_frameDataBind->clearValue("SFPasswordAuthMsg");
    }
    if (type & AT_Face) {
        initFaceAuth();
    } else if (m_faceAuth) {
        delete m_faceAuth;
        m_faceAuth = nullptr;
        m_authButtons.value(AT_Face)->deleteLater();
        m_authButtons.remove(AT_Face);
        m_frameDataBind->clearValue("SFFaceAuthState");
        m_frameDataBind->clearValue("SFFaceAuthMsg");
    }
    if (type & AT_Iris) {
        initIrisAuth();
    } else if (m_irisAuth) {
        delete m_irisAuth;
        m_irisAuth = nullptr;
        m_authButtons.value(AT_Iris)->deleteLater();
        m_authButtons.remove(AT_Iris);
        m_frameDataBind->clearValue("SFIrisAuthState");
        m_frameDataBind->clearValue("SFIrisAuthMsg");
    }
    if (type & AT_Fingerprint) {
        initFingerprintAuth();
    } else if (m_fingerprintAuth) {
        delete m_fingerprintAuth;
        m_fingerprintAuth = nullptr;
        m_authButtons.value(AT_Fingerprint)->deleteLater();
        m_authButtons.remove(AT_Fingerprint);
        m_frameDataBind->clearValue("SFFingerprintAuthState");
        m_frameDataBind->clearValue("SFFingerprintAuthMsg");
    }
    if (type & AT_Ukey) {
        initUKeyAuth();
    } else if (m_ukeyAuth) {
        delete m_ukeyAuth;
        m_ukeyAuth = nullptr;
        m_authButtons.value(AT_Ukey)->deleteLater();
        m_authButtons.remove(AT_Ukey);
        m_frameDataBind->clearValue("SFUKeyAuthState");
        m_frameDataBind->clearValue("SFUKeyAuthMsg");
    }
    if (type & AT_PAM) {
        initSingleAuth();
    } else if (m_singleAuth) {
        delete m_singleAuth;
        m_singleAuth = nullptr;
        m_authButtons.value(AT_PAM)->deleteLater();
        m_authButtons.remove(AT_PAM);
        m_frameDataBind->clearValue("SFSingleAuthState");
        m_frameDataBind->clearValue("SFSingleAuthMsg");
    }
    if (ModulesLoader::instance().findModulesByType(BaseModuleInterface::LoginType).size() > 0) {
        initCustomAuth();
    } else if (m_customAuth) {
        delete m_customAuth;
        m_customAuth = nullptr;
        m_authButtons.value(AT_Custom)->deleteLater();
        m_authButtons.remove(AT_Custom);
        m_frameDataBind->clearValue("SFCustomAuthStatus");
        m_frameDataBind->clearValue("SFCustomAuthMsg");
    }

    const int count = m_authButtons.values().size();
    if (count > 0) {
        m_chooseAuthButtonBox->setButtonList(m_authButtons.values(), true);
        QMap<int, DButtonBoxButton *>::const_iterator iter = m_authButtons.constBegin();
        while (iter != m_authButtons.constEnd()) {
            m_chooseAuthButtonBox->setId(iter.value(), iter.key());
            ++iter;
        }
        if (count > 1) {
            m_chooseAuthButtonBox->show();
            if (type & m_user->lastAuthType()) {
                if (m_chooseAuthButtonBox->checkedId() == m_user->lastAuthType()) {
                    emit m_chooseAuthButtonBox->button(m_user->lastAuthType())->toggled(true);
                } else {
                    m_chooseAuthButtonBox->button(m_user->lastAuthType())->setChecked(true);
                }
            } else {
                if (m_chooseAuthButtonBox->checkedId() == m_authButtons.firstKey()) {
                    emit m_chooseAuthButtonBox->button(m_authButtons.firstKey())->toggled(true);
                } else {
                    m_chooseAuthButtonBox->button(m_authButtons.firstKey())->setChecked(true);
                }
            }
        } else {
            if (m_chooseAuthButtonBox->checkedId() == m_authButtons.firstKey()) {
                emit m_chooseAuthButtonBox->button(m_authButtons.firstKey())->toggled(true);
            } else {
                m_chooseAuthButtonBox->button(m_authButtons.firstKey())->setChecked(true);
            }
            m_chooseAuthButtonBox->hide();
        }
        std::function<void(QVariant)> authTypeChanged = std::bind(&SFAWidget::syncAuthType, this, std::placeholders::_1);
        registerSyncFunctions("SFAType", authTypeChanged);
    } else {
        m_chooseAuthButtonBox->hide();
    }

    if (m_model->currentUser()->isNoPasswordLogin()) {
        m_lockButton->setEnabled(true);
        setFocusProxy(m_lockButton);
        setFocus();
    }

    updateSpaceItem();
}

/**
 * @brief 设置认证状态
 * @param type      认证类型
 * @param state    认证状态
 * @param message   认证消息
 */
void SFAWidget::setAuthState(const int type, const int state, const QString &message)
{
    qDebug() << "SFAWidget::setAuthState:" << type << state << message;
    switch (type) {
    case AT_Password:
        if (m_passwordAuth) {
            m_passwordAuth->setAuthState(state, message);
            m_frameDataBind->updateValue("SFPasswordAuthState", state);
            m_frameDataBind->updateValue("SFPasswordAuthMsg", message);
        }
        break;
    case AT_Fingerprint:
        if (m_fingerprintAuth) {
            m_fingerprintAuth->setAuthState(state, message);
            m_frameDataBind->updateValue("SFFingerprintAuthState", state);
            m_frameDataBind->updateValue("SFFingerprintAuthMsg", message);
        }
        break;
    case AT_Face:
        if (m_faceAuth) {
            m_faceAuth->setAuthState(state, message);
            m_frameDataBind->updateValue("SFFaceAuthState", state);
            m_frameDataBind->updateValue("SFFaceAuthMsg", message);
        }
        break;
    case AT_Ukey:
        if (m_ukeyAuth) {
            m_ukeyAuth->setAuthState(state, message);
            m_frameDataBind->updateValue("SFUKeyAuthState", state);
            m_frameDataBind->updateValue("SFUKeyAuthMsg", message);
        }
        break;
    case AT_Iris:
        if (m_irisAuth) {
            m_irisAuth->setAuthState(state, message);
            m_frameDataBind->updateValue("SFIrisAuthState", state);
            m_frameDataBind->updateValue("SFIrisAuthMsg", message);
        }
        break;
    case AT_PAM:
        if (m_singleAuth) {
            m_singleAuth->setAuthState(state, message);
            m_frameDataBind->updateValue("SFSingleAuthState", state);
            m_frameDataBind->updateValue("SFSingleAuthMsg", message);
        }
        break;
    case AT_Custom:
        if (m_customAuth) {
            m_customAuth->setAuthState(state, message);
        }
        break;
    case AT_All:
        checkAuthResult(AT_All, state);
        break;
    default:
        break;
    }
}

/**
 * @brief 初始化单因认证
 * 用于兼容开源 PAM
 */
void SFAWidget::initSingleAuth()
{
    if (m_singleAuth) {
        m_singleAuth->reset();
        return;
    }
    m_singleAuth = new AuthSingle(this);
    m_singleAuth->setCurrentUid(m_model->currentUser()->uid());
    replaceWidget(m_singleAuth);
    m_frameDataBind->updateValue("SFAType", AT_PAM);

    connect(m_singleAuth, &AuthSingle::activeAuth, this, [ this ] {
        emit requestStartAuthentication(m_model->currentUser()->name(), AT_PAM);
    });
    connect(m_singleAuth, &AuthSingle::authFinished, this, [this](const int authState) {
        if (authState == AS_Success) {
            m_user->setLastAuthType(AT_PAM);
            m_lockButton->setEnabled(true);
            emit authFinished();
        }
    });
    connect(m_singleAuth, &AuthSingle::requestAuthenticate, this, [ this ] {
        if (m_singleAuth->lineEditText().isEmpty()) {
            return;
        }
        emit sendTokenToAuth(m_model->currentUser()->name(), AT_PAM, m_singleAuth->lineEditText());
    });
    connect(m_capslockMonitor, &KeyboardMonitor::capslockStatusChanged, m_singleAuth, &AuthSingle::setCapsLockVisible);
    connect(m_lockButton, &QPushButton::clicked, m_singleAuth, &AuthSingle::requestAuthenticate);
    /* 输入框数据同步（可能是密码或PIN） */
    std::function<void(QVariant)> tokenChanged = std::bind(&SFAWidget::syncSingle, this, std::placeholders::_1);
    registerSyncFunctions("SFSingleAuth", tokenChanged);
    connect(m_singleAuth, &AuthSingle::lineEditTextChanged, this, [ this ] (const QString &value) {
        m_frameDataBind->updateValue("SFSingleAuth", value);
        m_lockButton->setEnabled(!value.isEmpty());
    });

    /* 重置密码可见性数据同步 */
    std::function<void(QVariant)> resetPasswordVisibleChanged = std::bind(&SFAWidget::syncSingleResetPasswordVisibleChanged, this, std::placeholders::_1);
    registerSyncFunctions("SingleResetPasswordVisible", resetPasswordVisibleChanged);
    connect(m_singleAuth, &AuthSingle::resetPasswordMessageVisibleChanged, this, [ this ] (const bool value) {
        m_frameDataBind->updateValue("SingleResetPasswordVisible", value);
    });

    m_singleAuth->setKeyboardButtonVisible(m_keyboardList.size() > 1 ? true : false);
    m_singleAuth->setKeyboardButtonInfo(m_keyboardType);
    m_singleAuth->setCapsLockVisible(m_capslockMonitor->isCapslockOn());
    m_singleAuth->setPasswordHint(m_model->currentUser()->passwordHint());
    // m_singleAuth->setAuthState(m_frameDataBind->getValue("SFSingleAuthState").toInt(),
    //                             m_frameDataBind->getValue("SFSingleAuthMsg").toString());

    /* 认证选择按钮 */
    DButtonBoxButton *btn = new DButtonBoxButton(QIcon(Password_Auth), QString(), this);
    btn->setIconSize(AuthButtonIconSize);
    btn->setFixedSize(AuthButtonSize);
    btn->setFocusPolicy(Qt::NoFocus);
    m_authButtons.insert(AT_PAM, btn);
    connect(btn, &DButtonBoxButton::toggled, this, [this](const bool checked) {
        if (checked) {
            replaceWidget(m_singleAuth);
            m_frameDataBind->updateValue("SFAType", AT_PAM);
            m_biometricAuthState->hide();
            setBioAuthStateVisible(nullptr, false);
            emit requestStartAuthentication(m_user->name(), AT_PAM);
        } else {
            m_singleAuth->hide();
            m_lockButton->setEnabled(false);
            emit requestEndAuthentication(m_user->name(), AT_PAM);
        }
    });
}

/**
 * @brief 初始化密码认证
 */
void SFAWidget::initPasswdAuth()
{
    if (m_passwordAuth) {
        m_passwordAuth->reset();
        return;
    }
    m_passwordAuth = new AuthPassword(this);
    m_passwordAuth->setCurrentUid(m_model->currentUser()->uid());
    m_passwordAuth->hide();
    m_passwordAuth->setShowAuthState(false);

    connect(m_passwordAuth, &AuthPassword::activeAuth, this, [this] {
        emit requestStartAuthentication(m_user->name(), AT_Password);
    });
    connect(m_passwordAuth, &AuthPassword::authFinished, this, [this](const int authState) {
        checkAuthResult(AT_Password, authState);
    });
    connect(m_passwordAuth, &AuthPassword::requestAuthenticate, this, [this] {
        const QString &text = m_passwordAuth->lineEditText();
        if (text.isEmpty()) {
            return;
        }
        m_passwordAuth->setAuthStateStyle(LOGIN_SPINNER);
        m_passwordAuth->setAnimationState(true);
        m_passwordAuth->setLineEditEnabled(false);
        m_lockButton->setEnabled(false);
        emit sendTokenToAuth(m_user->name(), AT_Password, text);
    });
    connect(m_lockButton, &QPushButton::clicked, m_passwordAuth, &AuthPassword::requestAuthenticate);
    connect(m_capslockMonitor, &KeyboardMonitor::capslockStatusChanged, m_passwordAuth, &AuthPassword::setCapsLockVisible);
    /* 输入框数据同步 */
    std::function<void(QVariant)> passwordChanged = std::bind(&SFAWidget::syncPassword, this, std::placeholders::_1);
    registerSyncFunctions("SFPasswordAuth", passwordChanged);
    connect(m_passwordAuth, &AuthPassword::lineEditTextChanged, this, [this](const QString &value) {
        m_frameDataBind->updateValue("SFPasswordAuth", value);
        m_lockButton->setEnabled(!value.isEmpty());
    });
    /* 重置密码可见性数据同步 */
    std::function<void(QVariant)> resetPasswordVisibleChanged = std::bind(&SFAWidget::syncPasswordResetPasswordVisibleChanged, this, std::placeholders::_1);
    registerSyncFunctions("ResetPasswordVisible", resetPasswordVisibleChanged);
    connect(m_passwordAuth, &AuthPassword::resetPasswordMessageVisibleChanged, this, [ = ](const bool value) {
        m_frameDataBind->updateValue("ResetPasswordVisible", value);
    });

    m_passwordAuth->setCapsLockVisible(m_capslockMonitor->isCapslockOn());
    m_passwordAuth->setPasswordHint(m_user->passwordHint());
    // m_passwordAuth->setAuthState(m_frameDataBind->getValue("SFPasswordAuthState").toInt(),
    //                               m_frameDataBind->getValue("SFPasswordAuthMsg").toString());

    /* 认证选择按钮 */
    DButtonBoxButton *btn = new DButtonBoxButton(QIcon(Password_Auth), QString(), this);
    btn->setIconSize(AuthButtonIconSize);
    btn->setFixedSize(AuthButtonSize);
    btn->setFocusPolicy(Qt::NoFocus);
    m_authButtons.insert(AT_Password, btn);
    connect(btn, &DButtonBoxButton::toggled, this, [this](const bool checked) {
        if (checked) {
            replaceWidget(m_passwordAuth);
            m_frameDataBind->updateValue("SFAType", AT_Password);
            m_biometricAuthState->hide();
            setBioAuthStateVisible(nullptr, false);
            emit requestStartAuthentication(m_user->name(), AT_Password);
            m_passwordAuth->updateResetPasswordUI();
        } else {
            m_passwordAuth->hide();
            m_lockButton->setEnabled(false);
            emit requestEndAuthentication(m_user->name(), AT_Password);
            m_passwordAuth->closeResetPasswordMessage();
        }
    });
}

/**
 * @brief 初始化指纹认证
 */
void SFAWidget::initFingerprintAuth()
{
    if (m_fingerprintAuth) {
        m_fingerprintAuth->reset();
        return;
    }
    m_fingerprintAuth = new AuthFingerprint(this);
    m_fingerprintAuth->hide();
    m_fingerprintAuth->setAuthFactorType(DDESESSIONCC::SingleAuthFactor);
    m_fingerprintAuth->setAuthStateLabel(m_biometricAuthState);

    connect(m_fingerprintAuth, &AuthFingerprint::activeAuth, this, [this] {
        emit requestStartAuthentication(m_model->currentUser()->name(), AT_Fingerprint);
    });
    connect(m_fingerprintAuth, &AuthFingerprint::authFinished, this, [this](const int authState) {
        checkAuthResult(AT_Fingerprint, authState);
    });

    // m_fingerprintAuth->setAuthState(m_frameDataBind->getValue("SFPasswordAuthState").toInt(),
    //                                  m_frameDataBind->getValue("SFPasswordAuthMsg").toString());

    /* 认证选择按钮 */
    DButtonBoxButton *btn = new DButtonBoxButton(QIcon(Fingerprint_Auth), QString(), this);
    btn->setIconSize(AuthButtonIconSize);
    btn->setFixedSize(AuthButtonSize);
    btn->setFocusPolicy(Qt::NoFocus);
    m_authButtons.insert(AT_Fingerprint, btn);
    connect(btn, &DButtonBoxButton::toggled, this, [this](const bool checked) {
        if (checked) {
            replaceWidget(m_fingerprintAuth);
            setBioAuthStateVisible(m_fingerprintAuth, true);
            m_frameDataBind->updateValue("SFAType", AT_Fingerprint);
            emit requestStartAuthentication(m_user->name(), AT_Fingerprint);
        } else {
            m_fingerprintAuth->hide();
            m_lockButton->setEnabled(false);
            emit requestEndAuthentication(m_user->name(), AT_Fingerprint);
        }
    });
}

/**
 * @brief 初始化 UKey 认证
 */
void SFAWidget::initUKeyAuth()
{
    if (m_ukeyAuth) {
        m_ukeyAuth->reset();
        return;
    }
    m_ukeyAuth = new AuthUKey(this);
    m_ukeyAuth->hide();

    connect(m_ukeyAuth, &AuthUKey::activeAuth, this, [this] {
        emit requestStartAuthentication(m_model->currentUser()->name(), AT_Ukey);
    });
    connect(m_ukeyAuth, &AuthUKey::authFinished, this, [this](const int authState) {
        checkAuthResult(AT_Ukey, authState);
    });
    connect(m_ukeyAuth, &AuthUKey::requestAuthenticate, this, [ = ] {
        const QString &text = m_ukeyAuth->lineEditText();
        if (text.isEmpty()) {
            return;
        }
        m_ukeyAuth->setAuthStateStyle(LOGIN_SPINNER);
        m_ukeyAuth->setAnimationState(true);
        m_ukeyAuth->setLineEditEnabled(false);
        m_lockButton->setEnabled(false);
        emit sendTokenToAuth(m_model->currentUser()->name(), AT_Ukey, text);
    });
    connect(m_lockButton, &QPushButton::clicked, m_ukeyAuth, &AuthUKey::requestAuthenticate);
    connect(m_capslockMonitor, &KeyboardMonitor::capslockStatusChanged, m_ukeyAuth, &AuthUKey::setCapsLockVisible);
    /* 输入框数据同步 */
    std::function<void(QVariant)> PINChanged = std::bind(&SFAWidget::syncUKey, this, std::placeholders::_1);
    registerSyncFunctions("SFUKeyAuth", PINChanged);
    connect(m_ukeyAuth, &AuthUKey::lineEditTextChanged, this, [this](const QString &value) {
        m_frameDataBind->updateValue("SFUKeyAuth", value);
        if (m_model->getAuthProperty().PINLen > 0 && value.size() >= m_model->getAuthProperty().PINLen) {
            emit m_ukeyAuth->requestAuthenticate();
        }
    });

    m_ukeyAuth->setCapsLockVisible(m_capslockMonitor->isCapslockOn());

    /* 认证选择按钮 */
    DButtonBoxButton *btn = new DButtonBoxButton(QIcon(UKey_Auth), QString(), this);
    btn->setIconSize(AuthButtonIconSize);
    btn->setFixedSize(AuthButtonSize);
    btn->setFocusPolicy(Qt::NoFocus);
    m_authButtons.insert(AT_Ukey, btn);
    connect(btn, &DButtonBoxButton::toggled, this, [this](const bool checked) {
        if (checked) {
            replaceWidget(m_ukeyAuth);
            m_frameDataBind->updateValue("SFAType", AT_Ukey);
            m_biometricAuthState->hide();
            setBioAuthStateVisible(nullptr, false);
            emit requestStartAuthentication(m_user->name(), AT_Ukey);
        } else {
            m_ukeyAuth->hide();
            m_lockButton->setEnabled(false);
            emit requestEndAuthentication(m_user->name(), AT_Ukey);
        }
    });
}

/**
 * @brief 初始化人脸认证
 */
void SFAWidget::initFaceAuth()
{
    if (m_faceAuth) {
        m_chooseAuthButtonBox->setEnabled(true);
        m_faceAuth->reset();
        return;
    }
    m_faceAuth = new AuthFace(this);
    m_faceAuth->setAuthStateLabel(m_biometricAuthState);
    m_faceAuth->setAuthFactorType(DDESESSIONCC::SingleAuthFactor);
    m_faceAuth->hide();

    connect(m_faceAuth, &AuthFace::retryButtonVisibleChanged, this, &SFAWidget::onRetryButtonVisibleChanged);
    connect(m_retryButton, &DFloatingButton::clicked, this, [this] {
        onRetryButtonVisibleChanged(false);
        emit requestStartAuthentication(m_model->currentUser()->name(), AT_Face);
    });
    connect(m_faceAuth, &AuthFace::activeAuth, this, [this] {
        emit requestStartAuthentication(m_model->currentUser()->name(), AT_Face);
    });
    connect(m_faceAuth, &AuthFace::authFinished, this, [this](const int authState) {
        checkAuthResult(AT_Face, authState);
    });
    connect(m_lockButton, &QPushButton::clicked, this, [this] {
        if (m_faceAuth == nullptr) return;
        if (m_faceAuth->authState() == AS_Success) {
            m_faceAuth->setAuthState(AS_Ended, "Ended");
            emit authFinished();
        }
    });

    // m_faceAuth->setAuthState(m_frameDataBind->getValue("SFFaceAuthState").toInt(),
    //                           m_frameDataBind->getValue("SFFaceAuthMsg").toString());

    /* 认证选择按钮 */
    DButtonBoxButton *btn = new DButtonBoxButton(QIcon(Face_Auth), QString(), this);
    btn->setIconSize(AuthButtonIconSize);
    btn->setFixedSize(AuthButtonSize);
    btn->setFocusPolicy(Qt::NoFocus);
    m_authButtons.insert(AT_Face, btn);
    connect(btn, &DButtonBoxButton::toggled, this, [this](const bool checked) {
        if (checked) {
            replaceWidget(m_faceAuth);
            setBioAuthStateVisible(m_faceAuth, true);
            m_frameDataBind->updateValue("SFAType", AT_Face);
            if (m_faceAuth->authState() != AS_Success) {
                emit requestStartAuthentication(m_user->name(), AT_Face);
            } else {
                m_lockButton->setEnabled(true);
            }
        } else {
            m_faceAuth->hide();
            if (m_faceAuth->authState() != AS_Success) {
                m_lockButton->setEnabled(false);
                emit requestEndAuthentication(m_user->name(), AT_Face);
            }
        }
    });
}

/**
 * @brief 初始化虹膜认证
 */
void SFAWidget::initIrisAuth()
{
    if (m_irisAuth) {
        m_irisAuth->reset();
        return;
    }
    m_irisAuth = new AuthIris(this);
    m_irisAuth->hide();
    m_irisAuth->setAuthStateLabel(m_biometricAuthState);
    m_irisAuth->setAuthFactorType(DDESESSIONCC::SingleAuthFactor);

    connect(m_irisAuth, &AuthIris::retryButtonVisibleChanged, this, &SFAWidget::onRetryButtonVisibleChanged);
    connect(m_retryButton, &DFloatingButton::clicked, this, [ this ] {
        onRetryButtonVisibleChanged(false);
        emit requestStartAuthentication(m_model->currentUser()->name(), AT_Iris);
    });
    connect(m_irisAuth, &AuthIris::activeAuth, this, [ this ] {
        emit requestStartAuthentication(m_model->currentUser()->name(), AT_Iris);
    });
    connect(m_irisAuth, &AuthIris::authFinished, this, [this](const int authState) {
        checkAuthResult(AT_Iris, authState);
    });
    connect(m_lockButton, &QPushButton::clicked, this, [this] {
        if (m_irisAuth == nullptr) return;
        if (m_irisAuth->authState() == AS_Success) {
            m_irisAuth->setAuthState(AS_Ended, "Ended");
            emit authFinished();
        }
    });

    /* 认证选择按钮 */
    DButtonBoxButton *btn = new DButtonBoxButton(QIcon(Iris_Auth), QString(), this);
    btn->setIconSize(AuthButtonIconSize);
    btn->setFixedSize(AuthButtonSize);
    btn->setFocusPolicy(Qt::NoFocus);
    m_authButtons.insert(AT_Iris, btn);
    connect(btn, &DButtonBoxButton::toggled, this, [this](const bool checked) {
        if (checked) {
            replaceWidget(m_irisAuth);
            setBioAuthStateVisible(m_irisAuth, true);
            m_frameDataBind->updateValue("SFAType", AT_Iris);
            if (m_irisAuth->authState() != AS_Success) {
                emit requestStartAuthentication(m_user->name(), AT_Iris);
            } else {
                m_lockButton->setEnabled(true);
            }
        } else {
            m_irisAuth->hide();
            if (m_irisAuth->authState() != AS_Success) {
                m_lockButton->setEnabled(false);
                emit requestEndAuthentication(m_user->name(), AT_Iris);
            }
        }
    });
}

/**
 * @brief 初始化自定义认证
 */
void SFAWidget::initCustomAuth()
{
    if (m_customAuth) {
        return;
    }

    m_customAuth = new AuthCustom(this);

    LoginModuleInterface *module = dynamic_cast<LoginModuleInterface *>(ModulesLoader::instance().findModulesByType(BaseModuleInterface::ModuleType::LoginType).values().first());
    module->init();
    m_customAuth->setModule(module);
    m_customAuth->hide();

    connect(m_customAuth, &AuthCustom::requestAuthenticate, this, [this] {
        m_lockButton->setEnabled(false);
        emit sendTokenToAuth(m_user->name(), AT_Custom, "");
    });
    connect(m_customAuth, &AuthCustom::authFinished, this, [ this ] (const int authStatus) {
        if (authStatus == AS_Success) {
            m_lockButton->setEnabled(true);
        }
    });

    /* 认证选择按钮 */
    DButtonBoxButton *btn = new DButtonBoxButton(DStyle::SP_SelectElement, QString(), this);
    const QIcon icon(QString::fromStdString(module->icon()));
    if (!icon.isNull()) {
        btn->setIcon(icon);
    }
    btn->setIconSize(AuthButtonIconSize);
    btn->setFixedSize(AuthButtonSize);
    btn->setFocusPolicy(Qt::NoFocus);
    m_authButtons.insert(AT_Custom, btn);
    connect(btn, &DButtonBoxButton::toggled, this, [this](const bool checked) {
        if (checked) {
            replaceWidget(m_customAuth);
            m_frameDataBind->updateValue("SFAType", AT_Custom);
            m_biometricAuthState->hide();
            setBioAuthStateVisible(nullptr, false);
            emit requestStartAuthentication(m_user->name(), AT_Custom);
            m_nameLabel->hide();
        } else {
            m_customAuth->hide();
            m_nameLabel->show();
            m_lockButton->setEnabled(false);
            emit requestEndAuthentication(m_user->name(), AT_Custom);
        }
    });
}

/**
 * @brief SFAWidget::checkAuthResult
 *
 * @param type  认证类型
 * @param state 认证状态
 */
void SFAWidget::checkAuthResult(const int type, const int state)
{
    // 等所有类型验证通过的时候在发送验证完成信息，否则DA的验证结果可能还没有刷新，导致lightdm调用pam验证失败
    // 人脸和虹膜是手动点击解锁按钮后发送，无需处理
    if (type == AT_All && state == AS_Success) {
        if ((m_passwordAuth && AS_Success == m_passwordAuth->authState())
            || (m_ukeyAuth && AS_Success == m_ukeyAuth->authState())
            || (m_fingerprintAuth && AS_Success == m_fingerprintAuth->authState())
            || (m_customAuth && m_customAuth->authState() == AS_Success)) {
            if (m_faceAuth) m_faceAuth->setAuthState(AS_Ended, "Ended");
            if (m_irisAuth) m_irisAuth->setAuthState(AS_Ended, "Ended");
            emit authFinished();
        }
    } else if (type != AT_All && state == AS_Success) {
        m_user->setLastAuthType(type);
        m_lockButton->setEnabled(true);
        m_lockButton->setFocus();

        if (type == AT_Face) {
            // 禁止切换其他认证方式
            m_chooseAuthButtonBox->setEnabled(false);
        }
    } else if (type == AT_Face && state == AS_Failure) {
        // 获取焦点后，可响应键盘enter/return事件，重新人脸认证
        m_retryButton->setFocus();
    }
}

/**
 * @brief 多屏同步认证类型
 * @param value
 */
void SFAWidget::syncAuthType(const QVariant &value)
{
    QAbstractButton *btn = m_chooseAuthButtonBox->button(value.toInt());
    if (btn)
        btn->setChecked(true);
}

void SFAWidget::replaceWidget(AuthModule *authModule)
{
    m_mainLayout->insertWidget(layout()->indexOf(m_userAvatar) + 3, authModule);
    authModule->show();
    setFocus();
    if (authModule->isLocked())
        authModule->clearFocus();
    else
        authModule->setFocus();
    m_lockButton->setEnabled(false);
    onRetryButtonVisibleChanged(false);
}

void SFAWidget::onRetryButtonVisibleChanged(bool visible)
{
    if (!visible && m_retryButton->hasFocus()) {
        // 人脸认证失败后，retryButton获取焦点，enter事件后隐藏btn造成焦点跳转到其他btn上
        m_retryButton->clearFocus();
    }

    m_retryButton->setVisible(visible);
    m_lockButton->setVisible(!visible);
}

void SFAWidget::setBioAuthStateVisible(AuthModule *authModule, bool visible)
{
    bool hasBioAuth = (m_faceAuth || m_fingerprintAuth || m_irisAuth);
    m_bioAuthStatePlaceHolder->changeSize(0, (visible || !hasBioAuth) ? 0 : BIO_AUTH_STATE_PLACE_HOLDER_HEIGHT);
    if (authModule)
        authModule->setAuthStatueVisible(visible);
}

/**
 * @brief 计算需要增加的顶部间隔，以实现用户头像到屏幕顶端的距离为屏幕高度的35%
 */
int SFAWidget::getTopSpacing() const
{
    int calcTopHeight = static_cast<int>(topLevelWidget()->geometry().height() * AUTH_WIDGET_TOP_SPACING_PERCENT);

    // 在低分辨率（高度<916）的时候，如果用户头像到屏幕顶端的距离为整个高度的35%，那么验证窗口整体是偏下的。
    // 计算居中时的顶部高度，用来保证验证窗口最起码是居中（在分辨率非常低的时候也无法保证，根据测试768是没问题的）。
    int centerTop = static_cast<int>((topLevelWidget()->geometry().height() - MIN_AUTH_WIDGET_HEIGHT) / 2);
    const int topHeight = qMin(calcTopHeight, centerTop);

    // 需要额外增加的顶部间隔高度 = 屏幕高度*0.35 - 时间控件高度 - 布局间隔 - 生物认证按钮底部间隔
    // - 生物认证切换按钮底部间隔 - 生物认证图标高度(如果有生物认证因子) - 切换验证类型按钮高度（如果认证因子数量大于1)
    int deltaY = topHeight - calcCurrentHeight(LOCK_CONTENT_CENTER_LAYOUT_MARGIN)
            - calcCurrentHeight(LOCK_CONTENT_TOPBOTTOM_WIDGET_HEIGHT)
            - m_bioBottomSpacingHolder->sizeHint().height()
            - m_authTypeBottomSpacingHolder->sizeHint().height()
            - ((m_faceAuth || m_fingerprintAuth || m_irisAuth) ? BIO_AUTH_STATE_PLACE_HOLDER_HEIGHT : 0)
            - (m_authButtons.size() > 1 ? CHOOSE_AUTH_TYPE_BUTTON_BOTTOM_SPACING : 0);

    return qMax(15, deltaY);
}

void SFAWidget::resizeEvent(QResizeEvent *event)
{
    AuthWidget::resizeEvent(event);

    updateSpaceItem();
}

/**
 * @brief 根据是否有多个因子，因子中是否含有生物认证来动态调整spaceItem的高度
 */
void SFAWidget::updateSpaceItem()
{
    m_authTypeBottomSpacingHolder->changeSize(0, m_authButtons.size() > 1 ? calcCurrentHeight(CHOOSE_AUTH_TYPE_BUTTON_BOTTOM_SPACING) : 0);

    if (m_faceAuth || m_fingerprintAuth || m_irisAuth) {
        m_bioBottomSpacingHolder->changeSize(0, calcCurrentHeight(CHOOSE_AUTH_TYPE_BUTTON_BOTTOM_SPACING));
        m_bioAuthStatePlaceHolder->changeSize(0, m_bioAuthStatePlaceHolder->sizeHint().height() == 0 ? 0 : BIO_AUTH_STATE_PLACE_HOLDER_HEIGHT);
    } else {
        m_bioBottomSpacingHolder->changeSize(0, 0);
        m_bioAuthStatePlaceHolder->changeSize(0, 0);
    }

    emit updateParentLayout();
}

void SFAWidget::updateFocus()
{
    for (int i = 0; i < m_mainLayout->layout()->count(); ++i) {
        AuthModule *authModule = qobject_cast<AuthModule *>(m_mainLayout->layout()->itemAt(i)->widget());
        if (authModule && authModule->authType() == m_chooseAuthButtonBox->checkedId()) {
            setFocus();
            if (authModule->isLocked())
                authModule->clearFocus();
            else
                authModule->setFocus();

            break;
        }
    }
}

void SFAWidget::showEvent(QShowEvent *event)
{
    AuthWidget::showEvent(event);

    QTimer::singleShot(0, this, &SFAWidget::updateFocus);
}
