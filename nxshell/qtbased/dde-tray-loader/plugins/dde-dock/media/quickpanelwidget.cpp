// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "quickpanelwidget.h"
#include "commoniconbutton.h"
#include "mediacontroller.h"
#include "mediamodel.h"

#include <QHBoxLayout>

#include <DStyleOption>
#include <DHiDPIHelper>

QuickPanelWidget::QuickPanelWidget(MediaController *controller, QWidget* parent)
    : QWidget(parent)
    , m_controller(controller)
    , m_pixmap(new DLabel(this))
    , m_titleLab(new DLabel(this))
    , m_artistLab(new DLabel(this))
    , m_playButton(new CommonIconButton(this))
    , m_nextButton(new CommonIconButton(this))
{
    init();
}

void QuickPanelWidget::init()
{
    m_pixmap->setFixedWidth(32);
    m_pixmap->setPixmap(MediaModel::ref().mediaInfo().pixmap);
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(14, 0, 18, 0);
    mainLayout->addWidget(m_pixmap);
    mainLayout->addSpacing(10);

    QWidget *textWidget = new QWidget;
    m_titleLab->setElideMode(Qt::ElideRight);
    m_titleLab->setAlignment(Qt::AlignLeft);
    DFontSizeManager::instance()->bind(m_titleLab, DFontSizeManager::T9);
    m_titleLab->setForegroundRole(QPalette::BrightText);
    QVBoxLayout *vLayout = new QVBoxLayout(textWidget);

    m_artistLab->setElideMode(Qt::ElideRight);
    m_artistLab->setAlignment(Qt::AlignLeft);
    DFontSizeManager::instance()->bind(m_artistLab, DFontSizeManager::T10);

    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    vLayout->addStretch();
    vLayout->addWidget(m_titleLab, 0, Qt::AlignVCenter | Qt::AlignLeft);
    vLayout->addSpacing(2);
    vLayout->addWidget(m_artistLab, 0, Qt::AlignVCenter | Qt::AlignLeft);
    vLayout->addStretch();

    mainLayout->addWidget(textWidget);

    QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    mainLayout->addItem(spacer);

    QWidget *buttonWidget = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout(buttonWidget);

    m_playButton->setClickable(true);
    m_playButton->setIcon(QIcon::fromTheme(MediaModel::ref().playState() ? "play-pause" : "play-start"), Qt::black, Qt::white);
    m_nextButton->setClickable(true);
    m_nextButton->setIcon(QIcon::fromTheme("play-next"), Qt::black, Qt::white);

    hLayout->setSpacing(0);
    hLayout->setMargin(0);
    hLayout->addWidget(m_playButton);
    hLayout->addSpacing(26);
    hLayout->addWidget(m_nextButton);

    mainLayout->addWidget(buttonWidget);

    connect(&MediaModel::ref(), &MediaModel::mediaInfoChanged, this, &QuickPanelWidget::updateUI);

    connect(m_nextButton, &CommonIconButton::clicked, m_controller, &MediaController::next);

    connect(&MediaModel::ref(), &MediaModel::playStateChanged, this, [this] (bool state) {
        m_playButton->setIcon(QIcon::fromTheme(state ? "play-pause" : "play-start"), Qt::black, Qt::white);
    });
    connect(m_playButton, &CommonIconButton::clicked, this, [this] {
        MediaModel::ref().playState() ? m_controller->pause() : m_controller->play();
    });
    connect(this, &QuickPanelWidget::clicked, this, [this] {
        if (!m_controller->mediaPath().isEmpty()) {
            m_controller->raise();
            Q_EMIT requestHideApplet();
        }
    });
    updateUI();
}

void QuickPanelWidget::updateUI()
{
    MediaModel::MediaInfo info = MediaModel::ref().mediaInfo();
    const bool pixmapExist = !info.pixmap.isNull();
    if (!pixmapExist) {
        info.pixmap = DHiDPIHelper::loadNxPixmap(":/deepin-music.svg").scaled(QSize(32 * qApp->devicePixelRatio(), 32 * qApp->devicePixelRatio()), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    const bool textInfoExist = !info.artist.isEmpty() || !info.title.isEmpty();
    if (!textInfoExist) {
        info.title = tr("Music");
    } else {
        if (info.title.isEmpty()) {
            info.title = tr("Unknown");
        }
        if (info.artist.isEmpty()) {
            info.artist = tr("Unknown");
        }
    }

    m_artistLab->setText(info.artist);
    m_pixmap->setPixmap(info.pixmap);
    m_titleLab->setText(info.title);
    const bool enable = pixmapExist || textInfoExist;
    m_artistLab->setVisible(enable);
}

void QuickPanelWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (underMouse()) {
        Q_EMIT clicked();
    }
    return QWidget::mouseReleaseEvent(event);
}

QuickPanelWidget::~QuickPanelWidget()
{
}
