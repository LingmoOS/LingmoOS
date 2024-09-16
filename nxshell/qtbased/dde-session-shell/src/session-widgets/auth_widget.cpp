// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "auth_widget.h"

#include "auth_face.h"
#include "auth_fingerprint.h"
#include "auth_iris.h"
#include "auth_password.h"
#include "auth_single.h"
#include "auth_ukey.h"
#include "dlineeditex.h"
#include "framedatabind.h"
#include "keyboardmonitor.h"
#include "sessionbasemodel.h"
#include "useravatar.h"

#include <DFontSizeManager>

const int BlurRadius = 15;
const int BlurTransparency = 70;
const int EXPIRED_SPACER_ITEM_HEIGHT = 10;

const QColor ShutdownColor(QColor(247, 68, 68));

AuthWidget::AuthWidget(QWidget *parent)
    : QWidget(parent)
    , m_model(nullptr)
    , m_frameDataBind(FrameDataBind::Instance())
    , m_blurEffectWidget(new DBlurEffectWidget(this))
    , m_lockButton(nullptr)
    , m_userAvatar(nullptr)
    , m_expiredStateLabel(new DLabel(this))
    , m_expiredSpacerItem(new QSpacerItem(0, 0))
    , m_nameLabel(nullptr)
    , m_accountEdit(nullptr)
    , m_capslockMonitor(nullptr)
    , m_keyboardTypeClip(nullptr)
    , m_singleAuth(nullptr)
    , m_passwordAuth(nullptr)
    , m_fingerprintAuth(nullptr)
    , m_ukeyAuth(nullptr)
    , m_faceAuth(nullptr)
    , m_irisAuth(nullptr)
    , m_customAuth(nullptr)
{
    setObjectName(QStringLiteral("AuthWidget"));
    setAccessibleName(QStringLiteral("AuthWidget"));

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFocusPolicy(Qt::NoFocus);

    m_capslockMonitor = KeyboardMonitor::instance();
    m_capslockMonitor->start(QThread::LowestPriority);
    m_frameDataBind = FrameDataBind::Instance();
}

AuthWidget::~AuthWidget()
{
    for (auto it = m_registerFunctions.constBegin(); it != m_registerFunctions.constEnd(); ++it) {
        m_frameDataBind->unRegisterFunction(it.key(), it.value());
    }
}

void AuthWidget::initUI()
{
    /* 头像 */
    m_userAvatar = new UserAvatar(this);
    m_userAvatar->setFocusPolicy(Qt::NoFocus);
    m_userAvatar->setIcon(m_user->avatar());
    m_userAvatar->setAvatarSize(UserAvatar::AvatarLargeSize);
    /* 用户名 */
    m_nameLabel = new DLabel(this);
    m_nameLabel->setAccessibleName(QStringLiteral("NameLabel"));
    m_nameLabel->setTextFormat(Qt::TextFormat::PlainText);
    m_nameLabel->setText(m_model->currentUser()->displayName());
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setTextFormat(Qt::TextFormat::PlainText);
    DFontSizeManager::instance()->bind(m_nameLabel, DFontSizeManager::T2);
    QPalette palette = m_nameLabel->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_nameLabel->setPalette(palette);

    /* 用户名输入框 */
    m_accountEdit = new DLineEditEx(this);
    m_accountEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_accountEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_accountEdit->setClearButtonEnabled(false);
    m_accountEdit->setPlaceholderText(tr("Account"));

    /* 密码过期提示 */
    m_expiredStateLabel->setAccessibleName("ExpiredStateLabel");
    m_expiredStateLabel->setWordWrap(true);
    m_expiredStateLabel->setAlignment(Qt::AlignHCenter);
    m_expiredStateLabel->setVisible(false);

    /* 解锁按钮 */
    m_lockButton = new DFloatingButton(this);
    m_lockButton->setFocusPolicy(Qt::StrongFocus);

    if (m_user->isLogin()) {
        m_lockButton->setIcon(DStyle::SP_LockElement);
    } else {
        m_lockButton->setIcon(DStyle::SP_ArrowNext);
    }

    /* 模糊背景 */
    m_blurEffectWidget->setMaskColor(DBlurEffectWidget::LightColor);
    m_blurEffectWidget->setMaskAlpha(BlurTransparency);
    m_blurEffectWidget->setBlurRectXRadius(BlurRadius);
    m_blurEffectWidget->setBlurRectYRadius(BlurRadius);
}

void AuthWidget::initConnections()
{
    /* model */
    connect(m_model, &SessionBaseModel::authFailedTipsMessage, this, &AuthWidget::setAccountErrorMsg);
    connect(m_model, &SessionBaseModel::currentUserChanged, this, &AuthWidget::setUser);
    connect(m_model, &SessionBaseModel::onPowerActionChanged, this, [this](SessionBaseModel::PowerAction action) {
        setLockButtonType(action);
    });
    /* 用户名 */
    connect(qGuiApp, &QGuiApplication::fontChanged, this, &AuthWidget::updateUserNameLabel);
    /* 用户名输入框 */
    connect(m_accountEdit, &DLineEditEx::returnPressed, this, [this] {
        if (m_accountEdit->isVisible() && !m_accountEdit->text().isEmpty()) {
            emit requestCheckAccount(m_accountEdit->text());
        }
    });
    connect(m_accountEdit, &DLineEditEx::editingFinished, this, [this] {
        emit m_accountEdit->returnPressed();
    });
    /* 解锁按钮 */
    connect(m_lockButton, &DFloatingButton::clicked, this, [this] {
        if (m_user->isNoPasswordLogin()) {
            emit requestCheckAccount(m_user->name());
        } else if (!m_passwordAuth && !m_ukeyAuth && !m_singleAuth) {
            emit m_accountEdit->returnPressed();
        }
    });
}

void AuthWidget::setModel(const SessionBaseModel *model)
{
    m_model = model;
    m_user = model->currentUser();
}

/**
 * @brief 设置当前用户
 * @param user
 */
void AuthWidget::setUser(std::shared_ptr<User> user)
{
    for (const QMetaObject::Connection &connection : qAsConst(m_connectionList)) {
        m_user->disconnect(connection);
    }
    m_connectionList.clear();
    m_user = user;
    m_connectionList << connect(user.get(), &User::avatarChanged, this, &AuthWidget::setAvatar)
                     << connect(user.get(), &User::displayNameChanged, this, &AuthWidget::updateUserNameLabel)
                     << connect(user.get(), &User::passwordHintChanged, this, &AuthWidget::setPasswordHint)
                     << connect(user.get(), &User::limitsInfoChanged, this, &AuthWidget::setLimitsInfo)
                     << connect(user.get(), &User::passwordExpiredInfoChanged, this, &AuthWidget::updatePasswordExpiredState)
                     << connect(user.get(), &User::limitsInfoChanged, this, &AuthWidget::setLimitsInfo);

    setAvatar(user->avatar());
    setPasswordHint(user->passwordHint());
    setLimitsInfo(user->limitsInfo());
    updatePasswordExpiredState();
    updateUserNameLabel();

    /* 根据用户类型设置用户名和用户名输入框的可见性 */
    if (user->type() == User::Default) {
        m_nameLabel->hide();
        m_accountEdit->show();
        m_accountEdit->setFocus();
    } else {
        m_nameLabel->show();
        m_accountEdit->hide();
        m_accountEdit->clear();
    }

    if (m_singleAuth) {
        m_singleAuth->setCurrentUid(user->uid());
    }
    if (m_passwordAuth) {
        m_passwordAuth->setCurrentUid(user->uid());
    }

    if (user->isLogin()) {
        m_lockButton->setIcon(DStyle::SP_LockElement);
    } else {
        m_lockButton->setIcon(DStyle::SP_ArrowNext);
    }
}

/**
 * @brief 设置认证类型
 * @param type
 */
void AuthWidget::setAuthType(const int type)
{
    Q_UNUSED(type)
}

/**
 * @brief 设置认证状态
 * @param type
 * @param state
 * @param message
 */
void AuthWidget::setAuthState(const int type, const int state, const QString &message)
{
    Q_UNUSED(type)
    Q_UNUSED(state)
    Q_UNUSED(message)
}

/**
 * @brief AuthWidget::checkAuthResult
 * @param type
 * @param state
 */
void AuthWidget::checkAuthResult(const int type, const int state)
{
    Q_UNUSED(type)
    Q_UNUSED(state)
}

/**
 * @brief 设置认证限制信息
 * @param limitsInfo
 */
void AuthWidget::setLimitsInfo(const QMap<int, User::LimitsInfo> *limitsInfo)
{
    User::LimitsInfo limitsInfoTmpU;
    LimitsInfo limitsInfoTmp;

    QMap<int, User::LimitsInfo>::const_iterator i = limitsInfo->constBegin();
    while (i != limitsInfo->end()) {
        limitsInfoTmpU = i.value();
        limitsInfoTmp.locked = limitsInfoTmpU.locked;
        limitsInfoTmp.maxTries = limitsInfoTmpU.maxTries;
        limitsInfoTmp.numFailures = limitsInfoTmpU.numFailures;
        limitsInfoTmp.unlockSecs = limitsInfoTmpU.unlockSecs;
        limitsInfoTmp.unlockTime = limitsInfoTmpU.unlockTime;
        switch (i.key()) {
        case AT_PAM:
            if (m_singleAuth) {
                m_singleAuth->setLimitsInfo(limitsInfoTmp);
            }
            break;
        case AT_Password:
            if (m_passwordAuth) {
                m_passwordAuth->setLimitsInfo(limitsInfoTmp);
            }
            break;
        case AT_Fingerprint:
            if (m_fingerprintAuth) {
                m_fingerprintAuth->setLimitsInfo(limitsInfoTmp);
            }
            break;
        case AT_Ukey:
            if (m_ukeyAuth) {
                m_ukeyAuth->setLimitsInfo(limitsInfoTmp);
            }
            break;
        case AT_Face:
            if (m_faceAuth) {
                m_faceAuth->setLimitsInfo(limitsInfoTmp);
            }
            break;
        case AT_Iris:
            if (m_irisAuth) {
                m_irisAuth->setLimitsInfo(limitsInfoTmp);
            }
            break;
        case AT_ActiveDirectory:
        case AT_Custom:
            break;
        default:
            qWarning() << "Error! Authentication type is wrong." << i.key();
            break;
        }
        ++i;
    }
}

/**
 * @brief 设置用户头像
 * @param avatar
 */
void AuthWidget::setAvatar(const QString &avatar)
{
    m_userAvatar->setIcon(avatar);
}

/**
 * @brief 设置用户名字体
 * @param font
 */
void AuthWidget::updateUserNameLabel()
{
    const QString &name = m_user->displayName();
    int nameWidth = m_nameLabel->fontMetrics().boundingRect(name).width();
    int labelMaxWidth = width() - 10 * 2;

    if (nameWidth > labelMaxWidth) {
        QString str = m_nameLabel->fontMetrics().elidedText(name, Qt::ElideRight, labelMaxWidth);
        m_nameLabel->setText(str);
    } else {
        m_nameLabel->setText(name);
    }
}

/**
 * @brief 设置密码提示
 * @param hint
 */
void AuthWidget::setPasswordHint(const QString &hint)
{
    if (m_singleAuth) {
        m_singleAuth->setPasswordHint(hint);
    }
    if (m_passwordAuth) {
        m_passwordAuth->setPasswordHint(hint);
    }
}

/**
 * @brief 设置解锁按钮的样式
 * @param type
 */
void AuthWidget::setLockButtonType(const int type)
{
    QPalette lockPalette;
    switch (type) {
    case SessionBaseModel::RequireRestart:
        m_lockButton->setIcon(QIcon(":/img/bottom_actions/reboot.svg"));
        lockPalette.setColor(QPalette::Highlight, ShutdownColor);
        break;
    case SessionBaseModel::RequireShutdown:
        m_lockButton->setIcon(QIcon(":/img/bottom_actions/shutdown.svg"));
        lockPalette.setColor(QPalette::Highlight, ShutdownColor);
        break;
    default:
        if (m_user->isLogin()) {
            m_lockButton->setIcon(DStyle::SP_LockElement);
        } else {
            m_lockButton->setIcon(DStyle::SP_ArrowNext);
        }
        break;
    }
    m_lockButton->setPalette(lockPalette);
}

/**
 * @brief 设置账户错误信息
 * @param message
 */
void AuthWidget::setAccountErrorMsg(const QString &message)
{
    m_accountEdit->showAlertMessage(message);
}

/**
 * @brief 更新模糊背景参数
 */
void AuthWidget::updateBlurEffectGeometry()
{
    QRect rect = this->rect();
    rect.setTop(m_userAvatar->geometry().top() + m_userAvatar->height() / 2);
    if (m_user->expiredState() == User::ExpiredNormal) {
        rect.setBottom(m_lockButton->geometry().top() - 10);
    } else {
        rect.setBottom(m_expiredStateLabel->geometry().top() - 10);
    }
    m_blurEffectWidget->setGeometry(rect);
}

/**
 * @brief 密码过期提示
 */
void AuthWidget::updatePasswordExpiredState()
{
    switch (m_user->expiredState()) {
    case User::ExpiredNormal:
        this->setDisabled(false);
        m_expiredSpacerItem->changeSize(0, 0);
        m_expiredStateLabel->clear();
        m_expiredStateLabel->setVisible(false);
        break;
    case User::ExpiredSoon:
        this->setDisabled(false);
        m_expiredStateLabel->setText(tr("Your password will expire in %n days, please change it timely", "", m_user->expiredDayLeft()));
        m_expiredStateLabel->setVisible(true);
        m_expiredSpacerItem->changeSize(0, EXPIRED_SPACER_ITEM_HEIGHT);
        break;
    case User::ExpiredAlready:
        if (m_model && m_model->appType() == AuthCommon::AppType::Login) {
            // 当前是登录界面的标准用户且密码过期时，禁止登录、禁止跳转到“修改密码流程”、置灰密码输入框
            if (m_user && m_user->accountType() == User::AccountType::Standard) {
                // 直接限制整个界面，除非下次打开此应用或此类被初始化
                this->setDisabled(true);
                m_expiredStateLabel->setText(tr("Your password has expired. Please contact the administrator to change it."));
                m_expiredStateLabel->setVisible(true);
                m_expiredSpacerItem->changeSize(0, EXPIRED_SPACER_ITEM_HEIGHT);
            } else if ( m_user && m_user->accountType() == User::AccountType::Admin) {
                // 已经在LoginContent处理了
            }
        } else if (m_model && m_model->appType() == AuthCommon::AppType::Lock) {
            if (m_user && m_user->accountType() == User::AccountType::Standard) {
                m_expiredStateLabel->setText(tr("Your password has expired. Please contact the administrator to change it."));
            } else if ( m_user && m_user->accountType() == User::AccountType::Admin) {
                m_expiredStateLabel->setText(tr("Your password has expired. Please change it."));
            }

            m_expiredStateLabel->setVisible(true);
            m_expiredSpacerItem->changeSize(0, EXPIRED_SPACER_ITEM_HEIGHT);
        }
        break;
    default:
        break;
    }
}

/**
 * @brief 将同步数据的方法注册到单例类
 * @param flag
 * @param function
 */
void AuthWidget::registerSyncFunctions(const QString &flag, std::function<void(QVariant)> function)
{
    if (!m_registerFunctions.contains(flag))
        m_registerFunctions[flag] = m_frameDataBind->registerFunction(flag, function);
    m_frameDataBind->refreshData(flag);
}

/**
 * @brief 单因数据同步
 * @param value
 */
void AuthWidget::syncSingle(const QVariant &value)
{
    if (m_singleAuth)
        m_singleAuth->setLineEditInfo(value.toString(), AuthPassword::InputText);
}

/**
 * @brief 单因重置密码可见性数据同步
 *
 * @param value
 */
void AuthWidget::syncSingleResetPasswordVisibleChanged(const QVariant &value)
{
    if (m_singleAuth) {
        m_singleAuth->setResetPasswordMessageVisible(value.toBool());
        m_singleAuth->updateResetPasswordUI();
    }
}

/**
 * @brief 用户名编辑框数据同步
 * @param value
 */
void AuthWidget::syncAccount(const QVariant &value)
{
    int cursorIndex = m_accountEdit->lineEdit()->cursorPosition();
    m_accountEdit->setText(value.toString());
    m_accountEdit->lineEdit()->setCursorPosition(cursorIndex);
}

/**
 * @brief 密码编辑框数据同步
 * @param value
 */
void AuthWidget::syncPassword(const QVariant &value)
{
    if (m_passwordAuth)
        m_passwordAuth->setLineEditInfo(value.toString(), AuthPassword::InputText);
}

/**
 * @brief 密码重置密码可见性数据同步
 *
 * @param value
 */
void AuthWidget::syncPasswordResetPasswordVisibleChanged(const QVariant &value)
{
    if (m_passwordAuth) {
        m_passwordAuth->setResetPasswordMessageVisible(value.toBool());
        if (m_passwordAuth->isVisible())
            m_passwordAuth->updateResetPasswordUI();
    }
}

/**
 * @brief 密码重置密码可见性UI同步
 */
void AuthWidget::syncResetPasswordUI()
{
    if (m_singleAuth) {
        m_singleAuth->updateResetPasswordUI();
    }
    if (m_passwordAuth && m_passwordAuth->isVisible()) {
        m_passwordAuth->updateResetPasswordUI();
    }
}

/**
 * @brief UKey 编辑框数据同步
 * @param value
 */
void AuthWidget::syncUKey(const QVariant &value)
{
    if (m_passwordAuth)
        m_ukeyAuth->setLineEditInfo(value.toString(), AuthUKey::InputText);
}

void AuthWidget::resizeEvent(QResizeEvent *event)
{
    updateBlurEffectGeometry();
    QWidget::resizeEvent(event);
}

void AuthWidget::showEvent(QShowEvent *event)
{
    activateWindow();
    QWidget::showEvent(event);
}

void AuthWidget::paintEvent(QPaintEvent *event)
{
#ifdef CONTRAST_BACKGROUND
    Q_UNUSED(event);

    QPainter painter(this);
    painter.fillRect(rect(), Qt::lightGray);
#else
    QWidget::paintEvent(event);
#endif
}

int AuthWidget::getTopSpacing() const
{
    const int topHeight = static_cast<int>(topLevelWidget()->geometry().height() * AUTH_WIDGET_TOP_SPACING_PERCENT);
    const int deltaY = topHeight - calcCurrentHeight(LOCK_CONTENT_TOPBOTTOM_WIDGET_HEIGHT)
            - calcCurrentHeight(LOCK_CONTENT_CENTER_LAYOUT_MARGIN);

    return qMax(15, deltaY);
}

int AuthWidget::calcCurrentHeight(const int height) const
{
    const int h = static_cast<int>(((double) height / (double) BASE_SCREEN_HEIGHT) * topLevelWidget()->geometry().height());
    return qMin(h, height);
}
