// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resolutionwidget.h"
#include "utils.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QTimer>
#include <DApplicationHelper>


ResolutionWidget::ResolutionWidget(ComDeepinDaemonGraphicsDriverInterface *graphicsDriver, const Resolution &resolution,  QWidget *parent)
    : QFrame(parent),
      m_graphicsDriver(graphicsDriver),
      m_resolution(resolution)
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ResolutionWidget::onThemeChanged);
}

ResolutionWidget::~ResolutionWidget()
= default;

void ResolutionWidget::initUI()
{
    m_checkedBtn = new QLabel;
//    m_checkedBtn->setPixmap(Utils::hidpiPixmap(":/resources/icons/select.svg", QSize(16, 16)));
//    m_checkedBtn->setFixedSize(16, 16);

    m_title = new QLabel;
    m_title->setText(m_resolution.title());

    m_title->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    m_title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    DFontSizeManager::instance()->bind(m_title, DFontSizeManager::T6);

    QString version = m_resolution.currVersion();
    if ( version.isEmpty() )
    {
        version = m_resolution.repVersion();
    }

    m_version = new QLabel;
    m_version->setText(tr("Driver/Version: %1/%2").arg(m_resolution.driver()).arg(version));
    m_version->setWordWrap(true);
    m_version->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_version->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    DFontSizeManager::instance()->bind(m_version, DFontSizeManager::T8);


    m_description = new QLabel;
    m_description->setText(m_resolution.description());
    m_description->setWordWrap(true);
    m_description->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_description->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    DFontSizeManager::instance()->bind(m_description, DFontSizeManager::T8);

    if (m_resolution.name().contains("glamor") || m_resolution.name().contains("uxa") || m_resolution.name().contains("sna")) {
        m_description->setVisible(false);
    }


    auto *infoLayout = new QVBoxLayout;
    infoLayout->addWidget(m_title);
    infoLayout->addWidget(m_version);
    infoLayout->addWidget(m_description);
    infoLayout->setSpacing(3);
    infoLayout->setContentsMargins(0, 0, 0, 0);

    auto *centralLayout = new QHBoxLayout;
    centralLayout->addLayout(infoLayout);
    centralLayout->addWidget(m_checkedBtn);
    centralLayout->setAlignment(m_checkedBtn, Qt::AlignTop | Qt::AlignRight);
    centralLayout->setSpacing(3);
    centralLayout->setContentsMargins(10, 10, 10, 10);

    setLayout(centralLayout);
    onThemeChanged(DGuiApplicationHelper::instance()->themeType());
    setChecked(m_resolution.enable());
    setObjectName("ResolutionWidget");


}

void ResolutionWidget::setChecked(const bool checked)
{
    m_checked = checked;
    m_checkedBtn->setVisible(checked);
}

void  ResolutionWidget::prepareInstall()
{
#ifdef TEST_UI
    if (m_timer == nullptr) {
        m_timer = new QTimer();
    }

    static int progress = 0;
    m_timer->setInterval(500);
    m_timer->start();
    connect(m_timer, &QTimer::timeout, [&] {
        if (progress <= 100) {
            Q_EMIT preInstallProgress(progress);
            progress += 10;
        } else {
            m_timer->stop();
        }
    });

#else
    QDBusPendingReply<void> preInstallReply = m_graphicsDriver->PrepareInstall(m_resolution.name(), QLocale::system().name());
    preInstallReply.waitForFinished();
    qInfo() << "m_resolution.name = " << m_resolution.name();
    if (!preInstallReply.isValid()) {
        qCritical() << "prepareInstall error:" << preInstallReply.error();
    } else {
        connect(m_graphicsDriver, &ComDeepinDaemonGraphicsDriverInterface::ReportProgress, [=](QString ratio){
            int process = ratio.toInt();
            Q_EMIT preInstallProgress(process);
        });
    }
#endif
}

bool ResolutionWidget::canUpdate()
{
    return m_resolution.currVersion().compare(m_resolution.repVersion()) < 0 && m_resolution.enable();
}

void ResolutionWidget::onThemeChanged(DGuiApplicationHelper::ColorType type)
{
    if (type == DGuiApplicationHelper::ColorType::LightType) {
        DGuiApplicationHelper::instance()->setThemeType(type);
        m_version->setStyleSheet("QLabel {"
                                 "color: #001A2E;"
                                 "}");

        m_title->setStyleSheet("QLabel {"
                               "font-weight: 500;"
                               "color: #414d68;"
                               "}");

        m_description->setStyleSheet("QLabel {"
                                     "color: #526a7f;"
                                     "}");

        setStyleSheet("QFrame#ResolutionWidget {"
                      "background-color: rgba(0, 0, 0, 0.03);"
                      "border-radius: 8px"
                      "}");

        m_checkedBtn->setPixmap(Utils::hidpiPixmap(":/resources/icons/select.svg", QSize(16, 16)));
        m_checkedBtn->setFixedSize(16, 16);

    } else if (type == DGuiApplicationHelper::ColorType::DarkType) {
        DGuiApplicationHelper::instance()->setThemeType(type);
        m_version->setStyleSheet("QLabel {"
                                 "color: #c0c6d4;"
                                 "}");

        m_title->setStyleSheet("QLabel {"
                               "font-weight: 500;"
                               "color: #C0C6D4;"
                               "}");

        m_description->setStyleSheet("QLabel {"
                                     "color: #6d7c88;"
                                     "}");

        setStyleSheet("QFrame#ResolutionWidget {"
                      "background-color: rgba(255, 255, 255, 0.05);"
                      "border-radius: 8px"
                      "}");

        m_checkedBtn->setPixmap(Utils::hidpiPixmap(":/resources/icons/select_dark.svg", QSize(16, 16)));
        m_checkedBtn->setFixedSize(16, 16);
    }
}

void ResolutionWidget::mouseReleaseEvent(QMouseEvent *e)
{
    e->accept();
    Q_EMIT clicked();
}



