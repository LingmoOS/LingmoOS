// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "user_widget.h"
#include "useravatar.h"

#include <DFontSizeManager>
#include <DIcon>

const int BlurRadius = 15;
const int BlurTransparency = 70;
const int UserNameHeight = 42;

UserWidget::UserWidget(QWidget *parent)
    : QWidget(parent)
    , m_isSelected(false)
    , m_uid(UINT_MAX)
    , m_mainLayout(new QVBoxLayout(this))
    , m_blurEffectWidget(new DBlurEffectWidget(this))
    , m_avatar(new UserAvatar(this))
    , m_loginState(new DLabel(this))
    , m_nameLabel(new DLabel(this))
    , m_nameWidget(new QWidget(this))
{
    setObjectName(QStringLiteral("UserWidget"));
    setAccessibleName(QStringLiteral("UserWidget"));

    setGeometry(0, 0, 280, 176);
    setFixedSize(UserFrameWidth, UserFrameHeight);

    setFocusPolicy(Qt::NoFocus);
}

void UserWidget::initUI()
{
    /* 头像 */
    m_avatar->setFocusPolicy(Qt::NoFocus);
    m_avatar->setIcon(m_user->avatar());
    m_avatar->setAvatarSize(UserAvatar::AvatarSmallSize);

    /* 用户名 */
    m_nameWidget->setAccessibleName(QStringLiteral("NameWidget"));
    QHBoxLayout *nameLayout = new QHBoxLayout(m_nameWidget);
    nameLayout->setContentsMargins(0, 0, 0, 0);
    nameLayout->setSpacing(5);

    QPixmap pixmap = DIcon::loadNxPixmap(":/misc/images/select.svg");
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    m_loginState->setAccessibleName("LoginState");
    m_loginState->setPixmap(pixmap);
    m_loginState->setVisible(m_user->isLogin());
    nameLayout->addWidget(m_loginState, 0, Qt::AlignVCenter | Qt::AlignRight);

    m_nameLabel->setAccessibleName("NameLabel");
    m_nameLabel->setTextFormat(Qt::TextFormat::PlainText);
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_nameLabel->setFixedHeight(UserNameHeight);
    m_nameLabel->setText(m_user->displayName());
    DFontSizeManager::instance()->bind(m_nameLabel, DFontSizeManager::T2);
    QPalette palette = m_nameLabel->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_nameLabel->setPalette(palette);
    nameLayout->addWidget(m_nameLabel, 1, Qt::AlignVCenter | Qt::AlignLeft);

    /* 模糊背景 */
    m_blurEffectWidget->setMaskColor(DBlurEffectWidget::LightColor);
    m_blurEffectWidget->setMaskAlpha(BlurTransparency);
    m_blurEffectWidget->setBlurRectXRadius(BlurRadius);
    m_blurEffectWidget->setBlurRectYRadius(BlurRadius);

    m_mainLayout->addWidget(m_avatar);
    m_mainLayout->addWidget(m_nameWidget, 0, Qt::AlignHCenter);
    m_mainLayout->addSpacing(20);
}

void UserWidget::initConnections()
{
    connect(m_user.get(), &User::avatarChanged, this, &UserWidget::setAvatar);
    connect(m_user.get(), &User::displayNameChanged, this, &UserWidget::updateUserNameLabel);
    connect(m_user.get(), &User::loginStateChanged, this, &UserWidget::setLoginState);
    connect(qGuiApp, &QGuiApplication::fontChanged, this, &UserWidget::updateUserNameLabel);
    connect(m_avatar, &UserAvatar::clicked, this, &UserWidget::clicked);
}

/**
 * @brief 设置用户信息
 * @param user
 */
void UserWidget::setUser(std::shared_ptr<User> user)
{
    m_user = user;

    initUI();
    initConnections();

    setUid(user->uid());
    updateUserNameLabel();
}

/**
 * @brief 设置用户选中标识
 * @param isSelected
 */
void UserWidget::setSelected(bool isSelected)
{
    m_isSelected = isSelected;
    update();
}

/**
 * @brief 设置用户选中标识
 * @param isSelected
 */
void UserWidget::setFastSelected(bool isSelected)
{
    m_isSelected = isSelected;
    repaint();
}

/**
 * @brief 给当前 Widget 设置一个 id，用于排序
 * @param uid
 */
void UserWidget::setUid(const uint uid)
{
    m_uid = uid;
}

/**
 * @brief 设置用户头像
 * @param avatar
 */
void UserWidget::setAvatar(const QString &avatar)
{
    m_avatar->setIcon(avatar);
}

/**
 * @brief 设置用户名字体
 * @param font
 */
void UserWidget::updateUserNameLabel()
{
    const QString &name = m_user->displayName();
    int nameWidth = m_nameLabel->fontMetrics().boundingRect(name).width();
    int labelMaxWidth = width() - 25 * 2;

    if (nameWidth > labelMaxWidth) {
        QString str = m_nameLabel->fontMetrics().elidedText(name, Qt::ElideRight, labelMaxWidth);
        m_nameLabel->setText(str);
    } else {
        m_nameLabel->setText(name);
    }
}

/**
 * @brief 设置登录状态
 * @param isLogin
 */
void UserWidget::setLoginState(const bool isLogin)
{
    m_loginState->setVisible(isLogin);
}

/**
 * @brief 更新模糊背景参数
 */
void UserWidget::updateBlurEffectGeometry()
{
    QRect rect = layout()->geometry();
    rect.setTop(m_avatar->geometry().top() + m_avatar->height() / 2);
    rect.setBottom(m_nameWidget->geometry().bottom() + 10);
    m_blurEffectWidget->setGeometry(rect);
}

void UserWidget::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
    QWidget::mousePressEvent(event);
}

void UserWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // 不再向上传递mouseReleaseEvent信号，避免影响用户列表模式处理点击空白处的逻辑
    Q_UNUSED(event)
}

void UserWidget::paintEvent(QPaintEvent *event)
{
    if (m_isSelected) {
        QPainter painter(this);
        painter.setPen(QColor(255, 255, 255, 76));
        painter.setBrush(QColor(255, 255, 255, 76));
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.drawRoundedRect(QRect(width() / 2 - 46, rect().bottom() - 4, 92, 4), 2, 2);
    }
    QWidget::paintEvent(event);
}

void UserWidget::resizeEvent(QResizeEvent *event)
{
    updateBlurEffectGeometry();
    QWidget::resizeEvent(event);
}
