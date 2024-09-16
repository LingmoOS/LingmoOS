// Copyright (C) 2016 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "downloadurl.h"
#include "avatarwidget.h"
#include "logininfo.h"
#include "trans_string.h"
#include "utils.h"

#include <DDBusSender>
#include <DFloatingButton>
#include <DFontSizeManager>
#include <DLabel>
#include <DLineEdit>
#include <DToolButton>
#include <DApplicationHelper>
#include <DDesktopServices>
#include <DListView>

#include <QEvent>
#include <QTimer>
#include <QBoxLayout>
#include <QObject>
#include <QWidget>
#include <QDir>
#include <QLabel>
#include <QDebug>
#include <QPushButton>
#include <QDesktopServices>
#include <QDBusPendingReply>
#include <QMargins>
#include <QToolTip>
#include <QStandardPaths>


DWIDGET_USE_NAMESPACE
Q_DECLARE_METATYPE(QMargins)

const static QString NicknameInvalid = QStringLiteral("7515");
const static QString NicknameNULL = QStringLiteral("7501");
const static QString NicknameOnce = QStringLiteral("7518");
const static QString OtherInvalid = QStringLiteral("7500");



LoginInfoPage::LoginInfoPage(QWidget *parent)
    : QWidget (parent)
    , m_mainLayout(new QVBoxLayout)
    , m_model(nullptr)
    , m_downloader(nullptr)
    , m_avatarPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                   + QDir::separator() + "sync")
    , m_avatar(new AvatarWidget(this))
    , m_fristLogin(true)
    , m_username(new DLabel(this))
    , m_editNameBtn(new DToolButton(this))
    , m_inputLineEdit(new DLineEdit(this))
    , m_region(new DLabel(this))
    , m_listView(new DListView)
    , m_listModel(new QStandardItemModel(this))
    , m_logoutBtn(new QPushButton (this))
    , m_editInfoBtn(new QPushButton(this))
{
    m_listIcon = {
        {0, "dcc_utcloud"},
        {1, "dcc_account"},
        {2, "dcc_password"},
    };
    initUI();
    initConnection();
    initGroupInfo();
}

LoginInfoPage::~LoginInfoPage()
{
    if (m_downloader != nullptr)
        m_downloader->deleteLater();
}

void LoginInfoPage::setModel(SyncModel *model)
{
    m_model = model;
    m_userFullName = m_model->userinfo()["Nickname"].toString();
    connect(m_model, &SyncModel::userInfoChanged, this, &LoginInfoPage::onUserInfoChanged);
    connect(m_model, &SyncModel::resetUserNameError, this, &LoginInfoPage::onResetError, Qt::QueuedConnection);

    onUserInfoChanged(m_model->userinfo());
}

void LoginInfoPage::onLogin()
{
    QModelIndex currIndex = m_listView->currentIndex();
    if(!currIndex.isValid()) {
        m_listView->setCurrentIndex(m_listModel->index(0, 0));
    }
}

void LoginInfoPage::initUI()
{
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);

    m_avatar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_avatar->setFixedSize(QSize(100, 100));
    //m_username->setFixedWidth(60);
    //m_username->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_username->setAlignment(Qt::AlignLeft);
    m_username->setElideMode(Qt::ElideRight);

    m_region->setAlignment(Qt::AlignCenter);

    m_editNameBtn->setIcon(QIcon::fromTheme("dcc_edit"));
    m_editNameBtn->setIconSize(QSize(12, 12));
    // 关闭在控制中心编辑账户昵称的功能
    m_editNameBtn->setVisible(false);

    m_inputLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_inputLineEdit->setVisible(false);
    m_inputLineEdit->lineEdit()->setFrame(false);
    m_inputLineEdit->lineEdit()->installEventFilter(this);

    DFontSizeManager::instance()->bind(m_username, DFontSizeManager::T6);
    DFontSizeManager::instance()->bind(m_inputLineEdit, DFontSizeManager::T5);
    DFontSizeManager::instance()->bind(m_region, DFontSizeManager::T8);

    // 头像名称布局
    QVBoxLayout *avatarLayout = new QVBoxLayout;
    avatarLayout->setContentsMargins(10, 0, 10, 0);
    avatarLayout->setSpacing(10);
    avatarLayout->addWidget(m_avatar, 0, Qt::AlignHCenter);
    avatarLayout->addSpacing(20);

    // 名称编辑区域设置固定
    QWidget *inputLine = new QWidget;
    inputLine->setFixedHeight(50);
    //inputLine->setStyleSheet("background-color:rgb(0, 255, 0)");
    QHBoxLayout *inputLineLayout = new QHBoxLayout;
    inputLineLayout->setSpacing(1);
    inputLineLayout->addWidget(m_username, 0, Qt::AlignLeft | Qt::AlignVCenter);
    inputLineLayout->addWidget(m_editNameBtn, 0, Qt::AlignVCenter);

    inputLineLayout->addWidget(m_inputLineEdit, 0, Qt::AlignLeft);
    m_inputLineEdit->setVisible(false);
    inputLineLayout->addStretch();

    inputLine->setLayout(inputLineLayout);
    avatarLayout->addWidget(m_region, 0, Qt::AlignHCenter);
    avatarLayout->addSpacing(-18);
    avatarLayout->addWidget(inputLine, 0, Qt::AlignHCenter);

    // 用户信息
    QHBoxLayout *listLayout = new QHBoxLayout;
    listLayout->setContentsMargins(10, 0, 10, 0);
    m_mainLayout->setSpacing(0);
    m_listView->setBackgroundType(DStyledItemDelegate::BackgroundType::RoundedBackground);
    //m_listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setSelectionMode(QListView::SelectionMode::SingleSelection);
    m_listView->setEditTriggers(DListView::NoEditTriggers);
    m_listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setFrameShape(DListView::NoFrame);
    m_listView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    m_listView->setViewportMargins(0, 0, 0, 0);
    m_listView->setItemSpacing(0);
    m_listView->setIconSize(QSize(32, 32));
    m_listView->setModel(m_listModel);
    listLayout->addWidget(m_listView);//Qt::AlignHCenter

    // 底部按钮布局
    m_editInfoBtn->setToolTip(tr("Edit account"));
    m_editInfoBtn->setIcon(QIcon::fromTheme("dcc_sync_Setting"));
    m_editInfoBtn->setIconSize(QSize(16, 16));
    m_editInfoBtn->setFixedSize(36,36);
    m_logoutBtn->setToolTip(tr("Sign out"));
    m_logoutBtn->setIcon(QIcon::fromTheme("dcc_sync_out"));
    m_logoutBtn->setIconSize(QSize(16, 16));
    m_logoutBtn->setFixedSize(36,36);
    QHBoxLayout *buttLayout = new QHBoxLayout;
    buttLayout->setAlignment(Qt::AlignHCenter);
    buttLayout->setContentsMargins(10, 10, 10, 10);
    buttLayout->addWidget(m_editInfoBtn, 0, Qt::AlignHCenter);
    buttLayout->addSpacing(10);
    buttLayout->addWidget(m_logoutBtn, 0, Qt::AlignHCenter);
    m_mainLayout->addSpacing(60);
    m_mainLayout->addLayout(avatarLayout);
    m_mainLayout->addSpacing(50);
    m_mainLayout->addLayout(listLayout);
    m_mainLayout->addStretch();
    m_mainLayout->addLayout(buttLayout);
    setLayout(m_mainLayout);
}

void LoginInfoPage::initConnection()
{
    connect(m_inputLineEdit, &DLineEdit::textChanged, this, [ = ](const QString &userFullName) {
        m_inputLineEdit->setAlert(false);
        if (userFullName.isEmpty() || userFullName.size() > 32) {
            m_inputLineEdit->setAlert(true);
            m_inputLineEdit->showAlertMessage(tr("The nickname must be 1~32 characters long"), m_inputLineEdit, 2000);
        }
    });

    connect(m_inputLineEdit, &DLineEdit::textEdited, this, [ = ](const QString &userFullName) {
        QString fullName = userFullName;
        fullName.remove(":");
        if (fullName != userFullName) {
            m_inputLineEdit->setText(fullName);
        }
        if (fullName.size() > 32) {
            m_inputLineEdit->setAlert(true);
            m_inputLineEdit->showAlertMessage(tr("The full name is too long"), this);
            DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_Error);
        }
    });

    connect(m_inputLineEdit, &DLineEdit::returnPressed, this, [ = ] {
        QString userFullName = m_inputLineEdit->lineEdit()->text();
        onEditingFinished(userFullName);
    });

    //点击用户全名编辑按钮
    connect(m_editNameBtn, &DIconButton::clicked, this, [ = ]() {
        m_username->setVisible(false);
        m_editNameBtn->setVisible(false);
        m_inputLineEdit->setVisible(true);
        m_inputLineEdit->setAlert(false);
        m_inputLineEdit->setText(m_userFullName);
        m_inputLineEdit->hideAlertMessage();
        m_inputLineEdit->lineEdit()->setFocus();
    });

    connect(m_logoutBtn, &DFloatingButton::clicked, this, [this] {
        m_avatar->clearAvatar();
        Q_EMIT requestLogoutUser();
    });
    connect(m_editInfoBtn, &DFloatingButton::clicked, this, &LoginInfoPage::openWeb);

    connect(m_listView, QOverload<const QModelIndex&>::of(&DListView::currentChanged), [this](const QModelIndex &preindex) {
        QModelIndex index = this->m_listView->currentIndex();
        QString iconName = m_listIcon[index.row()];
        QString preIcon = m_listIcon[preindex.row()];

        QString selectIcon = iconName + "_select";
        this->m_listModel->itemFromIndex(index)->setIcon(QIcon::fromTheme(selectIcon));
        if(preindex.isValid()) {
            this->m_listModel->itemFromIndex(preindex)->setIcon(QIcon::fromTheme(preIcon));
        }

        Q_EMIT this->selectIndex(m_listView->currentIndex().row());
    });
}

void LoginInfoPage::onUserInfoChanged(const QVariantMap &infos)
{
    m_userFullName = m_model->userinfo()["Nickname"].toString();
    const bool isLogind = !infos["Username"].toString().isEmpty();
    const QString region = infos["Region"].toString();
    QString profile_image = infos.value("ProfileImage").toString();

    if (!isLogind) {
        m_fristLogin = true;
        return;
    }

    if (m_fristLogin && isLogind) {
        QTimer::singleShot(5000, this, [this]() {
            Q_EMIT requestPullMessage();
        });

        m_fristLogin = false;
    }
    onAvatarChanged(profile_image);
    m_username->setText(handleNameTooLong(m_model->userDisplayName()).toHtmlEscaped());

    QString phone = infos["Phone"].toString();
    QString mail = infos["Email"].toString();
    QString wechat = infos["WechatNickname"].toString();
    QString regionDate = (region == "CN") ? tr("Mainland China") : tr("Other regions");
    m_region->setText(regionDate);

    qDebug() << "tonken Info : " << phone << mail << wechat;
    QList<QPair<QString, QString>> infoTs {
        {"dcc_sync_phone", phone},
        {"dcc_sync_mail", mail},
        {"dcc_sync_wechat", wechat},
        {"dcc_sync_place", regionDate},
    };
    onUserInfoListChanged(infoTs);

    QModelIndex currIndex = m_listView->currentIndex();
    if(!currIndex.isValid()) {
        qDebug() << "uset info is invalid, set to index 0";
        m_listView->setCurrentIndex(m_listModel->index(0, 0));
    }
}

bool LoginInfoPage::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_inputLineEdit->lineEdit() && event->type() == QEvent::MouseButtonPress) {
        m_inputLineEdit->hideAlertMessage();
        m_inputLineEdit->lineEdit()->setFocus();
    }
    return false;
}

// 处理编辑输入完成后的逻辑
void LoginInfoPage::onEditingFinished(const QString &userFullName)
{
    QString fullName = userFullName.trimmed();
    m_inputLineEdit->lineEdit()->clearFocus();
    m_inputLineEdit->setVisible(false);
    m_username->setVisible(true);
    m_editNameBtn->setVisible(true);
    if (fullName.isEmpty() || fullName.size() > 32) {
        m_inputLineEdit->setAlert(false);
        m_inputLineEdit->hideAlertMessage();
        return;
    }

    m_username->setText(handleNameTooLong(fullName).toHtmlEscaped());
    Q_EMIT requestSetFullname(fullName);
}

void LoginInfoPage::onAvatarChanged(const QString &avaPath)
{
    QString profile_image = avaPath;
    QString avatarPath = m_avatarPath + QDir::separator() + m_model->userDisplayName();
    QDir dir;
    dir.mkpath(avatarPath);
    qDebug() << " ProfileImage = " << profile_image << ", avatarPath " << avatarPath;
    if (profile_image.isEmpty())
        return;

    QString localAvatar = avatarPath + profile_image.right(profile_image.size() - profile_image.lastIndexOf("/"));
    QString localDefault = avatarPath + "default.svg";
    if(QFile::exists(localAvatar)) {
        setAvatarPath(localAvatar);
    } else if (QFile::exists(localDefault)) {
        setAvatarPath(localDefault);
    }

    if (m_downloader == nullptr)
        m_downloader = new DownloadUrl;

    connect(m_downloader, &DownloadUrl::fileDownloaded, this, &LoginInfoPage::setAvatarPath, Qt::UniqueConnection);
    m_downloader->downloadFileFromURL(profile_image, avatarPath);
}

void LoginInfoPage::onUserInfoListChanged(const QList<QPair<QString, QString>> &moduleTs)
{
//    m_listModel->clear();
//    QString theme = (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) ? "_dark" : QString();
//    for (auto it = moduleTs.cbegin(); it != moduleTs.cend(); ++it) {
//        QString itemIcon = it->first;
//        QString itemText = it->second;
//        DStandardItem *item = new DStandardItem;

//        item->setText(itemText);
//        item->setData(itemIcon);
//        item->setFontSize(DFontSizeManager::T9);
//        item->setTextColorRole(DPalette::TextTips);
//        item->setIcon(QIcon::fromTheme(item->data().toString()+theme));

//        if (!itemText.isEmpty())
//            m_listModel->appendRow(item);
//    }
//    m_listView->setModel(m_listModel);
}

void LoginInfoPage::setAvatarPath(const QString &avatarPath)
{
    qDebug() << "downloaded filename = " << avatarPath;
    m_avatar->setAvatarPath(avatarPath);
}

void LoginInfoPage::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    m_username->setText(handleNameTooLong(m_userFullName).toHtmlEscaped());
    qInfo() << "login page width:" << width();
    qInfo() << "login page view width:" << m_listView->width();
}

void LoginInfoPage::initGroupInfo()
{
    m_listModel->clear();
    QMargins itemMargin(10, 8, 0, 8);
    DStandardItem *itemCloud = new DStandardItem();
    itemCloud->setBackgroundRole(DPalette::Base);
    itemCloud->setText(TransString::getTransString(STRING_CLOUDITEM));
    itemCloud->setIcon(QIcon::fromTheme(m_listIcon[0]).pixmap(QSize(32, 32)));
    itemCloud->setData(QVariant::fromValue(itemMargin), Dtk::MarginsRole);
    itemCloud->setSizeHint(QSize(178, 48));
    itemCloud->setToolTip(TransString::getTransString(STRING_CLOUDITEM));
    m_listModel->appendRow(itemCloud);

    DStandardItem *itemDevice = new DStandardItem();
    itemDevice->setBackgroundRole(DPalette::Base);
    itemDevice->setText(TransString::getTransString(STRING_DEVITEM));
    itemDevice->setIcon(QIcon::fromTheme(m_listIcon[1]).pixmap(QSize(32, 32)));
    itemDevice->setSizeHint(QSize(178, 48));
    itemDevice->setData(QVariant::fromValue(itemMargin), Dtk::MarginsRole);
    itemDevice->setToolTip(TransString::getTransString(STRING_DEVITEM));
    m_listModel->appendRow(itemDevice);

    DStandardItem *itemSecurity = new DStandardItem();
    itemSecurity->setBackgroundRole(DPalette::Base);
    itemSecurity->setText(TransString::getTransString(STRING_ACCOUNTINFO));
    itemSecurity->setIcon(QIcon::fromTheme(m_listIcon[2]).pixmap(QSize(32, 32)));
    itemSecurity->setSizeHint(QSize(178, 48));
    itemSecurity->setData(QVariant::fromValue(itemMargin), Dtk::MarginsRole);
    itemSecurity->setToolTip(TransString::getTransString(STRING_ACCOUNTINFO));
    m_listModel->appendRow(itemSecurity);
}

QString LoginInfoPage::handleNameTooLong(const QString &fullName)
{
    QFontMetrics metrics(fullName);
    QString name = metrics.elidedText(fullName, Qt::ElideRight, this->width() * 2/3);
    return name;
}

void LoginInfoPage::onResetError(const QString &error)
{
    qDebug() << "ResetUsername error: " << error;
    if (error.contains(NicknameInvalid)) {
        utils::sendSysNotify(TransString::getTransString(STRING_NICKNAMEINVALID));
        m_username->setText(handleNameTooLong(m_userFullName).toHtmlEscaped());
    } else if (error.contains(NicknameNULL)) {
        utils::sendSysNotify(TransString::getTransString(STRING_NICKNAMENULL));
        m_username->setText(handleNameTooLong(m_userFullName).toHtmlEscaped());
    } else if (error.contains(NicknameOnce)) {
        qDebug() << "show reset once limit";
        utils::sendSysNotify(TransString::getTransString(STRING_NICKNAMEONCE));
        m_username->setText(handleNameTooLong(m_userFullName).toHtmlEscaped());
    }
}

QString LoginInfoPage::loadCodeURL()
{
    auto func_getToken = [this] {
        QDBusPendingReply<QString> retToken = DDBusSender()
                .service("com.deepin.sync.Daemon")
                .interface("com.deepin.utcloud.Daemon")
                .path("/com/deepin/utcloud/Daemon")
                .method("UnionLoginToken")
                .call();
        retToken.waitForFinished();
        QString token = retToken.value();
        if (token.isEmpty())
            qDebug() << retToken.error().message();
        return token;
    };

    QString oauthURI = "https://login.deepin.org";

    if (!qEnvironmentVariableIsEmpty("DEEPINID_OAUTH_URI")) {
        oauthURI = qgetenv("DEEPINID_OAUTH_URI");
    }

    QString url = oauthURI += QString("/oauth2/authorize/registerlogin?autoLoginKey=%1").arg(func_getToken());
    return url;
}

void LoginInfoPage::openWeb()
{
    qDebug() << "open web";
    QString url = loadCodeURL();
    QUrl::toPercentEncoding(url);
    QDesktopServices::openUrl(QUrl(url));
}
