// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SOUNDAPPLET_H
#define SOUNDAPPLET_H

#include "dockslider.h"
#include "jumpsettingbutton.h"
#include "slidercontainer.h"
#include "soundcardport.h"
#include "soundcontroller.h"
#include "pluginlistview.h"

#include <com_deepin_daemon_audio.h>
#include <com_deepin_daemon_audio_sink.h>

#include <DApplicationHelper>
#include <DIconButton>

#include <QLabel>
#include <QScrollArea>
#include <QSlider>
#include <QVBoxLayout>
#include <QSpacerItem>

DWIDGET_USE_NAMESPACE

using DBusAudio = com::deepin::daemon::Audio;
using DBusSink = com::deepin::daemon::audio::Sink;

class HorizontalSeparator;
class QGSettings;

namespace Dock {
class TipsWidget;
}

class SoundApplet : public QWidget {
    Q_OBJECT

public:
    explicit SoundApplet(QWidget* parent = 0);

    DockSlider* mainSlider();
    void setMinHeight(int minHeight);

private slots:
    void onDefaultSinkChanged();
    void onVolumeChanged(int volume);
    void volumeSliderValueChanged();
    void maxUiVolumeChanged();
    void updatePorts();
    void removePort(const QString& key);
    void addPort(const SoundCardPort* port);

signals:
    void requestHideApplet();

private:
    void initUi();
    void initConnections();
    void refreshIcon();
    void enableDevice(bool flag);
    void removeDisabledDevice(QString portId, unsigned int cardId);
    void updateVolumeSliderStatus(const QString& status);
    void resizeApplet();
    PluginItem* findItem(const QString &uniqueKey) const;
    void selectItem(PluginItem *targetItem);

protected:
    bool eventFilter(QObject *watcher, QEvent *event);

private:
    DockSlider* m_volumeSlider;
    QLabel* m_volumeLabel;
    QLabel* m_deviceLabel;
    QLabel* m_outputLabel;
    QWidget* m_deviceWidget;
    QVBoxLayout* m_centralLayout;
    SliderContainer* m_sliderContainer;
    DBusAudio* m_audioInter;
    DBusSink* m_defSinkInter;
    PluginListView* m_listView;
    JumpSettingButton* m_settingButton;
    QStandardItemModel* m_itemModel;
    const QGSettings* m_gsettings;
    int m_minHeight;
    QSpacerItem *m_spacerItem;
};

#endif // SOUNDAPPLET_H
