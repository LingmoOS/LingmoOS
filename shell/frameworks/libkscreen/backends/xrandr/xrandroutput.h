/*
 *  SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#include "xrandrmode.h"

#include "../xcbwrapper.h"

#include "output.h"

#include <QMap>
#include <QObject>
#include <QVariant>
#include <cstdint>
#include <limits>

class XRandRConfig;
class XRandRCrtc;
namespace KScreen
{
class Config;
class Output;
}

class XRandROutput : public QObject
{
    Q_OBJECT

public:
    typedef QMap<xcb_randr_output_t, XRandROutput *> Map;

    using Priority = uint32_t;
    static constexpr size_t PRIORITY_FORMAT = std::numeric_limits<Priority>::digits;

    explicit XRandROutput(xcb_randr_output_t id, XRandRConfig *config);
    ~XRandROutput() override;

    void disabled();
    void disconnected();

    void update(xcb_randr_crtc_t crtc, xcb_randr_mode_t mode, xcb_randr_connection_t conn);

    xcb_randr_output_t id() const;

    bool isEnabled() const;
    bool isConnected() const;
    bool isPrimary() const;

    Priority priority() const;
    void setPriority(Priority priority);

    QPoint position() const;
    QSize size() const;
    QSizeF logicalSize() const;

    QString currentModeId() const;
    XRandRMode::Map modes() const;
    XRandRMode *currentMode() const;

    KScreen::Output::Rotation rotation() const;
    bool isHorizontal() const;

    QByteArray edid() const;
    XRandRCrtc *crtc() const;

    KScreen::OutputPtr toKScreenOutput() const;

    void updateLogicalSize(const KScreen::OutputPtr &output, XRandRCrtc *crtc = nullptr);

private:
    void init();
    void updateModes(const XCB::OutputInfo &outputInfo);
    Priority outputPriorityFromProperty() const;
    void setOutputPriorityToProperty(Priority priority);
    void setAsPrimary();

    static KScreen::Output::Type fetchOutputType(xcb_randr_output_t outputId, const QString &name);
    static QByteArray typeFromProperty(xcb_randr_output_t outputId);

    xcb_render_transform_t currentTransform() const;

    XRandRConfig *m_config;
    xcb_randr_output_t m_id;
    QString m_name;
    QString m_icon;
    mutable QByteArray m_edid;

    xcb_randr_connection_t m_connected;
    KScreen::Output::Type m_type;

    XRandRMode::Map m_modes;
    QStringList m_preferredModes;

    QList<xcb_randr_output_t> m_clones;

    unsigned int m_widthMm;
    unsigned int m_heightMm;

    bool m_hotplugModeUpdate = false;
    XRandRCrtc *m_crtc;
};
