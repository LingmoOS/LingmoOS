// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "soundview.h"
#include "constants.h"
#include "tipswidget.h"
#include "imageutil.h"
#include "utils.h"
#include "soundmodel.h"

#include <DApplication>
#include <DDBusSender>
#include <DGuiApplicationHelper>

#include <QPainter>
#include <QIcon>
#include <QMouseEvent>
#include <QApplication>
#include <QDBusInterface>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

// menu actions
#define MUTE     "mute"
#define SETTINGS "settings"

using namespace Dock;

SoundView::SoundView(QWidget *parent)
    : QWidget(parent)
    , m_tipsLabel(new TipsWidget(this))
    , m_applet(new SoundApplet)
    , m_iconWidget(new CommonIconButton(this))
{
    m_tipsLabel->setAccessibleName("soundtips");
    m_tipsLabel->setVisible(false);
    m_applet->setVisible(false);
    m_iconWidget->setFixedSize(Dock::DOCK_PLUGIN_ITEM_FIXED_SIZE);
    m_iconWidget->installEventFilter(this);

    connect(&SoundModel::ref(), &SoundModel::volumeChanged, this, &SoundView::refresh, Qt::QueuedConnection);
    connect(&SoundModel::ref(), &SoundModel::muteStateChanged, this, &SoundView::refresh, Qt::QueuedConnection);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ] {
        refreshIcon();
    });
    connect(m_applet.data(), &SoundApplet::requestHideApplet, this, &SoundView::requestHideApplet);

    refresh(SoundModel::ref().volume());
}

QWidget *SoundView::tipsWidget()
{
    refreshTips(std::min(150, SoundModel::ref().volume()), true);
    m_tipsLabel->resize(m_tipsLabel->sizeHint().width() + 10,
                        m_tipsLabel->sizeHint().height());

    return m_tipsLabel;
}

QWidget *SoundView::popupApplet()
{
    return m_applet.get();
}

const QString SoundView::contextMenu()
{
    QList<QVariant> items;
    items.reserve(2);

    QMap<QString, QVariant> open;
    open["itemId"] = MUTE;
    if (!SoundController::ref().existActiveOutputDevice()) {
        open["itemText"] = tr("Unmute");
        open["isActive"] = false;
    } else {
        if (SoundModel::ref().isMute()) {
            open["itemText"] = tr("Unmute");
        } else {
            open["itemText"] = tr("Mute");
        }
        open["isActive"] = true;
    }
    items.push_back(open);

    if (!QFile::exists(ICBC_CONF_FILE)) {
        QMap<QString, QVariant> settings;
        settings["itemId"] = SETTINGS;
        settings["itemText"] = tr("Sound settings");
        settings["isActive"] = true;
        items.push_back(settings);
    }

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

void SoundView::invokeMenuItem(const QString menuId, const bool checked)
{
    Q_UNUSED(checked);

    if (menuId == MUTE) {
        SoundController::ref().SetMuteQueued(!SoundModel::ref().isMute());
    } else if (menuId == SETTINGS) {
        DDBusSender()
        .service("com.deepin.dde.ControlCenter")
        .interface("com.deepin.dde.ControlCenter")
        .path("/com/deepin/dde/ControlCenter")
        .method(QString("ShowModule"))
        .arg(QString("sound"))
        .call();
        Q_EMIT requestHideApplet();
    }
}

void SoundView::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    const Dock::Position position = qApp->property(PROP_POSITION).value<Dock::Position>();
    // 保持横纵比
    if (position == Dock::Bottom || position == Dock::Top) {
        setMaximumWidth(height());
        setMaximumHeight(QWIDGETSIZE_MAX);
    } else {
        setMaximumHeight(width());
        setMaximumWidth(QWIDGETSIZE_MAX);
    }

    refreshIcon();
}

bool SoundView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_iconWidget && event->type() == QEvent::Wheel) {
        auto e = dynamic_cast<QWheelEvent*>(event);
        if (e) {
            qApp->postEvent(m_applet->mainSlider(), new QWheelEvent(e->pos(), e->delta(), e->buttons(), e->modifiers()));
            e->accept();
        }
    }
    return QWidget::eventFilter(watched, event);
}

void SoundView::refresh(const int volume)
{
    refreshIcon();
    refreshTips(volume, false);
}

void SoundView::refreshIcon()
{
    const double volume = SoundModel::ref().volume();
    const double maxVolume = SoundModel::ref().maxVolumeUI();
    const bool mute = !SoundController::ref().existActiveOutputDevice() ? true : SoundModel::ref().isMute();

    QString iconString;
    QString volumeString;
    if (mute)
        volumeString = "muted";
    else if (int(volume) == 0)
        volumeString = "off";
    else if (volume / maxVolume > 0.6)
        volumeString = "high";
    else if (volume / maxVolume > 0.3)
        volumeString = "medium";
    else
        volumeString = "low";

    iconString = QString("audio-volume-%1-symbolic").arg(volumeString);
    m_iconWidget->setIcon(QIcon::fromTheme(iconString));
}

void SoundView::refreshTips(const int volume, const bool force)
{
    if (!force && !m_tipsLabel->isVisible())
        return;

    if (!SoundController::ref().existActiveOutputDevice()) {
        m_tipsLabel->setText(QString(tr("No output devices")));
    } else {
        if (SoundModel::ref().isMute()) {
            m_tipsLabel->setText(QString(tr("Mute")));
        } else {
            m_tipsLabel->setText(QString(tr("Volume %1").arg(QString::number(volume) + '%')));
        }
    }
}

void SoundView::setAppletMinHeight(int minHeight)
{
    if (m_applet) {
        m_applet->setMinHeight(minHeight);
    }
}
