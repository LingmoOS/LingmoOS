#include "OobeWindow.h"

#include <QApplication>
#include <QBoxLayout>
#include <QFile>
#include <QGraphicsBlurEffect>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QPropertyAnimation>
#include <QScreen>
#include <QStyle>
#include <QTimer>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

static const double CARD_RATIO = 0.60;
static const int PAGE_PADDING = 40;
static const char* OOBE_FLAG = "/etc/.firstboot-complete";

OobeWindow::OobeWindow(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("oobeWindow");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    QSize screenSize = qApp->primaryScreen()->geometry().size();
    setGeometry(0, 0, screenSize.width(), screenSize.height());

    setupBackground();

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addStretch();
    {
        QHBoxLayout *hrow = new QHBoxLayout;
        hrow->addStretch();
        hrow->addWidget(createCard());
        hrow->addStretch();
        root->addLayout(hrow);
    }
    root->addStretch();

    QMetaObject::invokeMethod(this, "showFullScreen", Qt::QueuedConnection);

    // Entrance fade-in
    auto *entranceEffect = new QGraphicsOpacityEffect(m_cardWidget);
    m_cardWidget->setGraphicsEffect(entranceEffect);
    entranceEffect->setOpacity(0.0);
    auto *entranceAnim = new QPropertyAnimation(entranceEffect, "opacity", this);
    entranceAnim->setDuration(400);
    entranceAnim->setStartValue(0.0);
    entranceAnim->setEndValue(1.0);
    entranceAnim->setEasingCurve(QEasingCurve::OutCubic);
    entranceAnim->start(QAbstractAnimation::DeleteWhenStopped);
    connect(entranceAnim, &QPropertyAnimation::finished, this, [this]() {
        m_cardWidget->setGraphicsEffect(nullptr);
    });
}

void OobeWindow::setupBackground()
{
    QSize screenSize = size();

    QLabel *overlay = new QLabel(this);
    overlay->setGeometry(rect());
    overlay->setStyleSheet("background: rgba(0, 0, 0, 0.3);");

    QLabel *bg = new QLabel(this);
    bg->setGeometry(rect());
    bg->setObjectName("oobeBg");

    QStringList candidates = {
        "/usr/share/backgrounds/lingmo/default.png",
        "/usr/share/backgrounds/default.png",
        "/usr/share/wallpapers/Deepin/contents/images/default.jpg"
    };

    QPixmap bgPix;
    for (const auto &path : candidates) {
        if (bgPix.load(path)) break;
    }

    if (!bgPix.isNull()) {
        bgPix = bgPix.scaled(screenSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        int x = (bgPix.width() - screenSize.width()) / 2;
        int y = (bgPix.height() - screenSize.height()) / 2;
        bgPix = bgPix.copy(qMax(0, x), qMax(0, y), screenSize.width(), screenSize.height());
        bg->setPixmap(bgPix);

        auto *blur = new QGraphicsBlurEffect(bg);
        blur->setBlurRadius(40.0);
        blur->setBlurHints(QGraphicsBlurEffect::QualityHint);
        bg->setGraphicsEffect(blur);
    } else {
        QPixmap gradient(screenSize);
        QPainter p(&gradient);
        QLinearGradient g(0, 0, 0, screenSize.height());
        g.setColorAt(0.0, QColor("#0f0c29"));
        g.setColorAt(0.5, QColor("#302b63"));
        g.setColorAt(1.0, QColor("#24243e"));
        p.fillRect(gradient.rect(), g);
        p.end();
        bg->setPixmap(gradient);
    }

    bg->lower();
    overlay->lower();
    bg->show();
    overlay->show();
}

QWidget* OobeWindow::createCard()
{
    QSize screenSize = qApp->primaryScreen()->geometry().size();
    int cardW = qMin(static_cast<int>(screenSize.width() * CARD_RATIO), screenSize.width() - 100);

    m_cardWidget = new QWidget(this);
    m_cardWidget->setObjectName("oobeCard");
    m_cardWidget->setFixedWidth(cardW);

    QVBoxLayout *cardLayout = new QVBoxLayout(m_cardWidget);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(0);

    // --- Header ---
    QWidget *header = new QWidget;
    header->setObjectName("cardHeader");
    QHBoxLayout *hl = new QHBoxLayout(header);
    hl->setContentsMargins(24, 18, 24, 10);

    QLabel *logo = new QLabel;
    QPixmap lpix(":/logo.png");
    if (!lpix.isNull())
        logo->setPixmap(lpix.scaled(28, 28, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    hl->addWidget(logo);

    QLabel *title = new QLabel(tr("Lingmo OS"));
    title->setObjectName("cardTitleLabel");
    QFont tf = title->font();
    tf.setPointSize(tf.pointSize() + 2);
    tf.setBold(true);
    title->setFont(tf);
    hl->addWidget(title);
    hl->addStretch();
    cardLayout->addWidget(header);

    // --- Stacked pages ---
    m_stack = new QStackedWidget;
    m_stack->setObjectName("oobeStack");
    m_stack->addWidget(createWelcomePage());
    m_stack->addWidget(createUserPage());
    m_stack->addWidget(createHostnamePage());
    m_stack->addWidget(createConfirmPage());
    m_stack->addWidget(createInstallPage());
    m_stack->addWidget(createFinishedPage());
    cardLayout->addWidget(m_stack, 1);

    // --- Navigation ---
    QWidget *nav = new QWidget;
    nav->setObjectName("cardNav");
    QHBoxLayout *nl = new QHBoxLayout(nav);
    nl->setContentsMargins(24, 12, 24, 20);

    m_backBtn = new QPushButton(tr("Back"));
    m_backBtn->setObjectName("navBackBtn");
    m_backBtn->setFixedSize(100, 36);
    connect(m_backBtn, &QPushButton::clicked, this, &OobeWindow::backPage);

    m_nextBtn = new QPushButton(tr("Next"));
    m_nextBtn->setObjectName("navNextBtn");
    m_nextBtn->setFixedSize(120, 36);
    connect(m_nextBtn, &QPushButton::clicked, this, &OobeWindow::nextPage);

    nl->addWidget(m_backBtn);
    nl->addStretch();
    nl->addWidget(m_nextBtn);
    cardLayout->addWidget(nav);

    updateNavButtons();

    m_cardWidget->setStyleSheet(
        "#oobeCard {"
        "  background: rgba(255, 255, 255, 0.93);"
        "  border-radius: 14px;"
        "}"
        "#cardHeader {"
        "  background: transparent;"
        "  border-bottom: 1px solid rgba(0, 0, 0, 0.08);"
        "}"
        "#cardTitleLabel {"
        "  color: #1a1a1a;"
        "  background: transparent;"
        "  margin-left: 8px;"
        "}"
        "#oobeStack {"
        "  background: transparent;"
        "}"
        "#cardNav {"
        "  background: transparent;"
        "  border-top: 1px solid rgba(0, 0, 0, 0.06);"
        "}"
        "#navBackBtn {"
        "  background: rgba(0, 0, 0, 0.06);"
        "  border: none;"
        "  border-radius: 6px;"
        "  color: #333;"
        "  font-size: 13px;"
        "}"
        "#navBackBtn:hover { background: rgba(0, 0, 0, 0.1); }"
        "#navNextBtn {"
        "  background: #0058a9;"
        "  border: none;"
        "  border-radius: 6px;"
        "  color: white;"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "}"
        "#navNextBtn:hover { background: #0072d2; }"
        "#navNextBtn:disabled { background: #aaa; }"
    );

    return m_cardWidget;
}

QWidget* OobeWindow::createWelcomePage()
{
    QWidget *page = new QWidget;
    page->setObjectName("pageWelcome");

    QHBoxLayout *hl = new QHBoxLayout(page);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(0);

    // Left panel: logo / illustration
    QWidget *leftPanel = new QWidget;
    leftPanel->setObjectName("welcomeLeft");
    leftPanel->setFixedWidth(480);

    QVBoxLayout *ll = new QVBoxLayout(leftPanel);
    ll->setAlignment(Qt::AlignCenter);

    QLabel *icon = new QLabel(leftPanel);
    QPixmap ipix(":/logo.png");
    if (!ipix.isNull())
        icon->setPixmap(ipix.scaled(160, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    icon->setAlignment(Qt::AlignCenter);
    ll->addWidget(icon);

    QLabel *osName = new QLabel("Lingmo OS", leftPanel);
    osName->setAlignment(Qt::AlignCenter);
    QFont nf = osName->font();
    nf.setPointSize(nf.pointSize() + 4);
    nf.setBold(true);
    osName->setFont(nf);
    osName->setStyleSheet("color: white; background: transparent;");
    ll->addWidget(osName);

    QLabel *versionLabel = new QLabel("5.0", leftPanel);
    versionLabel->setAlignment(Qt::AlignCenter);
    QFont vf = versionLabel->font();
    vf.setPointSize(vf.pointSize());
    versionLabel->setFont(vf);
    versionLabel->setStyleSheet("color: rgba(255,255,255,0.7); background: transparent;");
    ll->addWidget(versionLabel);

    // Right panel: welcome text
    QWidget *rightPanel = new QWidget;
    rightPanel->setObjectName("welcomeRight");
    rightPanel->setStyleSheet("#welcomeRight { background: transparent; }");
    QVBoxLayout *rl = new QVBoxLayout(rightPanel);
    rl->setContentsMargins(50, 40, 50, 40);
    rl->setSpacing(16);

    rl->addStretch();

    QLabel *welcome = new QLabel(tr("Welcome to Lingmo OS"));
    welcome->setObjectName("welcomeTitle");
    QFont wf = welcome->font();
    wf.setPointSize(wf.pointSize() + 10);
    wf.setBold(true);
    welcome->setFont(wf);
    welcome->setStyleSheet("color: #1a1a1a; background: transparent;");
    rl->addWidget(welcome);

    QLabel *desc = new QLabel(tr("Let's set up your system.\nClick \"Next\" to continue."));
    desc->setObjectName("welcomeDesc");
    QFont df = desc->font();
    df.setPointSize(df.pointSize() + 2);
    desc->setFont(df);
    desc->setStyleSheet("color: #666; background: transparent;");
    desc->setWordWrap(true);
    rl->addWidget(desc);

    rl->addStretch();

    hl->addWidget(leftPanel);
    hl->addWidget(rightPanel, 1);

    return page;
}

QWidget* OobeWindow::createUserPage()
{
    QWidget *page = new QWidget;
    page->setObjectName("pageUser");
    QVBoxLayout *l = new QVBoxLayout(page);
    l->setContentsMargins(PAGE_PADDING, 20, PAGE_PADDING, 10);
    l->setSpacing(16);

    QLabel *heading = new QLabel(tr("Create User"));
    QFont hf = heading->font();
    hf.setPointSize(hf.pointSize() + 4);
    hf.setBold(true);
    heading->setFont(hf);
    l->addWidget(heading);

    QLabel *sub = new QLabel(tr("Create a user account to log into the system."));
    l->addWidget(sub);
    l->addSpacing(8);

    auto makeField = [](const QString &label, QLineEdit *&edit, bool pwd = false) {
        QWidget *w = new QWidget;
        QVBoxLayout *vl = new QVBoxLayout(w);
        vl->setContentsMargins(0, 0, 0, 0);
        vl->setSpacing(4);
        QLabel *lb = new QLabel(label);
        lb->setStyleSheet("font-size: 12px; color: #555;");
        vl->addWidget(lb);
        edit = new QLineEdit;
        edit->setFixedHeight(36);
        edit->setStyleSheet(
            "QLineEdit { border: 1px solid #ccc; border-radius: 6px; padding: 4px 10px; font-size: 14px; }"
            "QLineEdit:focus { border-color: #0058a9; }");
        if (pwd) edit->setEchoMode(QLineEdit::Password);
        vl->addWidget(edit);
        return w;
    };

    l->addWidget(makeField(tr("Username"), m_usernameEdit));
    l->addWidget(makeField(tr("Password"), m_passwordEdit, true));
    l->addWidget(makeField(tr("Confirm Password"), m_confirmPwdEdit, true));

    l->addStretch();
    return page;
}

QWidget* OobeWindow::createHostnamePage()
{
    QWidget *page = new QWidget;
    page->setObjectName("pageHostname");
    QVBoxLayout *l = new QVBoxLayout(page);
    l->setContentsMargins(PAGE_PADDING, 20, PAGE_PADDING, 10);
    l->setSpacing(16);

    QLabel *heading = new QLabel(tr("Computer Name"));
    QFont hf = heading->font();
    hf.setPointSize(hf.pointSize() + 4);
    hf.setBold(true);
    heading->setFont(hf);
    l->addWidget(heading);

    QLabel *sub = new QLabel(tr("Give this computer a name."));
    l->addWidget(sub);
    l->addSpacing(8);

    m_hostnameEdit = new QLineEdit(tr("lingmo-pc"));
    m_hostnameEdit->setFixedHeight(36);
    m_hostnameEdit->setStyleSheet(
        "QLineEdit { border: 1px solid #ccc; border-radius: 6px; padding: 4px 10px; font-size: 14px; }"
        "QLineEdit:focus { border-color: #0058a9; }");
    l->addWidget(m_hostnameEdit);

    l->addStretch();
    return page;
}

QWidget* OobeWindow::createConfirmPage()
{
    QWidget *page = new QWidget;
    page->setObjectName("pageConfirm");
    QVBoxLayout *l = new QVBoxLayout(page);
    l->setContentsMargins(PAGE_PADDING, 20, PAGE_PADDING, 10);
    l->setSpacing(16);

    QLabel *heading = new QLabel(tr("Confirm Settings"));
    QFont hf = heading->font();
    hf.setPointSize(hf.pointSize() + 4);
    hf.setBold(true);
    heading->setFont(hf);
    l->addWidget(heading);

    QLabel *info = new QLabel;
    info->setObjectName("confirmInfo");
    info->setStyleSheet("color: #333; font-size: 14px; line-height: 1.8; background: #f5f5f5; border-radius: 8px; padding: 20px;");
    info->setWordWrap(true);
    l->addWidget(info);

    l->addStretch();

    connect(m_stack, &QStackedWidget::currentChanged, this, [this, info](int idx) {
        if (idx == 3) {
            info->setText(
                tr("Username: %1\nComputer Name: %2")
                    .arg(m_username.isEmpty() ? tr("(not set)") : m_username)
                    .arg(m_hostname.isEmpty() ? tr("(not set)") : m_hostname));
        }
    });

    return page;
}

QWidget* OobeWindow::createInstallPage()
{
    QWidget *page = new QWidget;
    page->setObjectName("pageInstall");
    QVBoxLayout *l = new QVBoxLayout(page);
    l->setContentsMargins(PAGE_PADDING, 30, PAGE_PADDING, 30);
    l->setSpacing(24);

    QLabel *icon = new QLabel;
    QPixmap ipix(":/logo.png");
    if (!ipix.isNull())
        icon->setPixmap(ipix.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    icon->setAlignment(Qt::AlignCenter);
    l->addWidget(icon);

    QLabel *heading = new QLabel(tr("Applying Settings"));
    QFont hf = heading->font();
    hf.setPointSize(hf.pointSize() + 4);
    hf.setBold(true);
    heading->setFont(hf);
    heading->setAlignment(Qt::AlignCenter);
    l->addWidget(heading);

    m_progressLabel = new QLabel(tr("Please wait..."));
    m_progressLabel->setAlignment(Qt::AlignCenter);
    m_progressLabel->setStyleSheet("color: #666; font-size: 13px;");
    l->addWidget(m_progressLabel);

    // Simple progress bar widget
    m_progressBar = new QWidget;
    m_progressBar->setFixedHeight(4);
    m_progressBar->setStyleSheet("background: #e0e0e0; border-radius: 2px;");
    QHBoxLayout *pbl = new QHBoxLayout(m_progressBar);
    pbl->setContentsMargins(0, 0, 0, 0);

    QWidget *fill = new QWidget;
    fill->setObjectName("progressFill");
    fill->setFixedHeight(4);
    fill->setStyleSheet("background: #0058a9; border-radius: 2px;");
    fill->setFixedWidth(0);
    pbl->addWidget(fill);

    l->addWidget(m_progressBar);

    l->addStretch();

    connect(m_stack, &QStackedWidget::currentChanged, this, [this, fill](int idx) {
        if (idx == 4) {
            m_backBtn->setEnabled(false);
            m_nextBtn->setEnabled(false);

            auto *timer = new QTimer(this);
            timer->setInterval(80);
            int *w = new int(0);
            connect(timer, &QTimer::timeout, this, [this, timer, fill, w]() {
                *w += 3;
                if (*w > m_progressBar->width())
                    *w = m_progressBar->width();
                fill->setFixedWidth(*w);
                if (*w < m_progressBar->width() * 0.15)
                    m_progressLabel->setText(tr("Creating user..."));
                else if (*w < m_progressBar->width() * 0.40)
                    m_progressLabel->setText(tr("Setting password..."));
                else if (*w < m_progressBar->width() * 0.65)
                    m_progressLabel->setText(tr("Setting hostname..."));
                else if (*w < m_progressBar->width() * 0.85)
                    m_progressLabel->setText(tr("Configuring auto login..."));
                else if (*w < m_progressBar->width() - 5)
                    m_progressLabel->setText(tr("Finishing..."));
                else {
                    timer->stop();
                    applySettings();
                }
            });
            timer->start();
        }
    });

    return page;
}

QWidget* OobeWindow::createFinishedPage()
{
    QWidget *page = new QWidget;
    page->setObjectName("pageFinished");
    QVBoxLayout *l = new QVBoxLayout(page);
    l->setContentsMargins(PAGE_PADDING, 30, PAGE_PADDING, 10);
    l->setSpacing(20);

    QLabel *icon = new QLabel;
    QPixmap ipix(":/logo.png");
    if (!ipix.isNull())
        icon->setPixmap(ipix.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    icon->setAlignment(Qt::AlignCenter);
    l->addWidget(icon);

    QLabel *heading = new QLabel(tr("Setup Complete"));
    QFont hf = heading->font();
    hf.setPointSize(hf.pointSize() + 6);
    hf.setBold(true);
    heading->setFont(hf);
    heading->setAlignment(Qt::AlignCenter);
    l->addWidget(heading);

    QLabel *desc = new QLabel(
        tr("Welcome to Lingmo OS!\n\nYour system is ready.\nAfter reboot, log in with \"%1\".").arg(m_username));
    desc->setObjectName("finishDesc");
    desc->setAlignment(Qt::AlignCenter);
    QFont df = desc->font();
    df.setPointSize(df.pointSize() + 1);
    desc->setFont(df);
    desc->setWordWrap(true);
    l->addWidget(desc);

    l->addStretch();

    QPushButton *rebootBtn = new QPushButton(tr("Reboot Now"));
    rebootBtn->setObjectName("rebootBtn");
    rebootBtn->setFixedWidth(260);
    rebootBtn->setCursor(Qt::PointingHandCursor);
    l->addWidget(rebootBtn, 0, Qt::AlignCenter);

    connect(rebootBtn, &QPushButton::clicked, this, [this, rebootBtn]() {
        rebootBtn->setEnabled(false);
        rebootBtn->setText(tr("Rebooting..."));
        QCoreApplication::processEvents();
#ifdef Q_OS_UNIX
        if (getuid() != 0)
            QProcess::startDetached("pkexec", { "systemctl", "reboot" });
        else
            QProcess::startDetached("systemctl", { "reboot" });
#endif
    });

    page->setStyleSheet(
        "#pageFinished { background: transparent; }"
        "#finishDesc { color: #444; }"
        "#rebootBtn {"
        "  background: #0058a9;"
        "  border: none;"
        "  border-radius: 8px;"
        "  color: white;"
        "  font-size: 15px;"
        "  font-weight: bold;"
        "  padding: 12px 0;"
        "}"
        "#rebootBtn:hover { background: #0072d2; }"
        "#rebootBtn:disabled { background: #aaa; }"
    );

    return page;
}

void OobeWindow::updateNavButtons()
{
    int idx = m_stack->currentIndex();
    int last = m_stack->count() - 1;

    m_backBtn->setVisible(idx > 0 && idx < last - 1);

    if (idx == 0) {
        m_nextBtn->setText(tr("Next"));
    } else if (idx < last - 1) {
        m_nextBtn->setText(tr("Next"));
    } else if (idx == last - 1) {
        m_nextBtn->setText(tr("Finish"));
    } else {
        m_nextBtn->setVisible(false);
        m_backBtn->setVisible(false);
    }
}

void OobeWindow::nextPage()
{
    int idx = m_stack->currentIndex();

    if (idx == 1) {
        m_username = m_usernameEdit->text().trimmed();
        m_password = m_passwordEdit->text();
        QString confirm = m_confirmPwdEdit->text();

        if (m_username.isEmpty() || m_password.isEmpty()) {
            return;
        }
        if (m_password != confirm) {
            return;
        }
    }

    if (idx == 2) {
        m_hostname = m_hostnameEdit->text().trimmed();
        if (m_hostname.isEmpty()) m_hostname = "lingmo-pc";
    }

    if (idx == m_stack->count() - 1) {
        QProcess::startDetached("systemctl", QStringList() << "reboot");
        qApp->quit();
        return;
    }

    fadeToPage(idx + 1);
}

void OobeWindow::backPage()
{
    int idx = m_stack->currentIndex();
    if (idx > 0) {
        fadeToPage(idx - 1);
    }
}

void OobeWindow::fadeToPage(int index)
{
    if (index == m_stack->currentIndex())
        return;

    auto *effect = new QGraphicsOpacityEffect(m_stack);
    m_stack->setGraphicsEffect(effect);
    effect->setOpacity(1.0);

    auto *fadeOut = new QPropertyAnimation(effect, "opacity", this);
    fadeOut->setDuration(150);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    connect(fadeOut, &QPropertyAnimation::finished, this, [this, index, effect]() {
        m_stack->setCurrentIndex(index);
        effect->setOpacity(0.0);

        auto *fadeIn = new QPropertyAnimation(effect, "opacity", this);
        fadeIn->setDuration(150);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);

        connect(fadeIn, &QPropertyAnimation::finished, this, [this]() {
            m_stack->setGraphicsEffect(nullptr);
            updateNavButtons();
        });

        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}

void OobeWindow::applySettings()
{
    m_progressLabel->setText(tr("Applying settings..."));
    QCoreApplication::processEvents();

    QStringList cmds;
    cmds << QString("useradd -m -G sudo,audio,video,plugdev,netdev,bluetooth -s /bin/bash %1").arg(m_username);
    cmds << QString("echo '%1:%2' | chpasswd").arg(m_username, m_password);
    cmds << QString("hostnamectl set-hostname %1").arg(m_hostname);

    // SDDM autologin – replace first-boot config and set real DE session
    cmds << QString("mkdir -p /etc/sddm.conf.d");
    cmds << QString("rm -f /etc/sddm.conf.d/lingmo-firstboot.conf");
    cmds << QString("echo '[Autologin]' > /etc/sddm.conf.d/lingmo-autologin.conf");
    cmds << QString("echo 'User=%1' >> /etc/sddm.conf.d/lingmo-autologin.conf").arg(m_username);
    cmds << QString("echo 'Session=lingmo.desktop' >> /etc/sddm.conf.d/lingmo-autologin.conf");

    // Remove OOBE session files so SDDM won't try to use them on next login
    // cmds << "rm -f /usr/share/xsessions/lingmo-oobe.desktop /usr/share/wayland-sessions/lingmo-oobe.desktop /etc/xdg/autostart/oobe.desktop /lib/systemd/system/firstboot.service";

    // Remove default live user
    cmds << "userdel -r liveuser 2>/dev/null; true";

    // Mark OOBE complete
    cmds << QString("touch %1").arg(OOBE_FLAG);

    // Self-remove OOBE package to reduce footprint
    cmds << "apt-get remove -y lingmo-oobe 2>/dev/null || dpkg --purge lingmo-oobe 2>/dev/null || true";

    QString executor;
#ifdef Q_OS_UNIX
    if (getuid() != 0) {
        executor = "pkexec";
    }
#else
    executor = "pkexec";
#endif

    bool ok = true;
    for (const auto &cmd : cmds) {
        QString fullCmd;
        if (!executor.isEmpty())
            fullCmd = QString("%1 bash -c '%2'").arg(executor, cmd);
        else
            fullCmd = cmd;

        int ret = system(fullCmd.toUtf8().constData());
        if (ret != 0) {
            ok = false;
            break;
        }
        QCoreApplication::processEvents();
    }

    m_progressLabel->setText(ok ? tr("Setup complete") : tr("Setup failed"));

    QTimer::singleShot(500, this, [this]() {
        m_stack->setCurrentIndex(m_stack->count() - 1);
        m_nextBtn->setText(tr("Reboot"));
        m_nextBtn->setEnabled(true);
        updateNavButtons();
    });
}
