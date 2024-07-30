/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "xrandroutput.h"

#include "xrandr.h"
#include "xrandrconfig.h"

#include "../utils.h"

#include "mode.h"

#include <array>
#include <cstring>
#include <qglobal.h>
#include <utility>
#include <xcb/randr.h>
#include <xcb/render.h>

#define DOUBLE_TO_FIXED(d) ((xcb_render_fixed_t)((d)*65536))
#define FIXED_TO_DOUBLE(f) ((double)((f) / 65536.0))

xcb_render_fixed_t fOne = DOUBLE_TO_FIXED(1);
xcb_render_fixed_t fZero = DOUBLE_TO_FIXED(0);

XRandROutput::XRandROutput(xcb_randr_output_t id, XRandRConfig *config)
    : QObject(config)
    , m_config(config)
    , m_id(id)
    , m_type(KScreen::Output::Unknown)
    , m_crtc(nullptr)
{
    init();
}

XRandROutput::~XRandROutput()
{
}

xcb_randr_output_t XRandROutput::id() const
{
    return m_id;
}

bool XRandROutput::isConnected() const
{
    return m_connected == XCB_RANDR_CONNECTION_CONNECTED;
}

bool XRandROutput::isEnabled() const
{
    return m_crtc != nullptr && m_crtc->mode() != XCB_NONE;
}

bool XRandROutput::isPrimary() const
{
    return priority() == 1;
}

uint32_t XRandROutput::priority() const
{
    if (isConnected() && isEnabled()) {
        return outputPriorityFromProperty();
    } else {
        return 0;
    }
}

void XRandROutput::setPriority(XRandROutput::Priority newPriority)
{
    if (priority() != newPriority) {
        setOutputPriorityToProperty(newPriority);
    }

    // Always update the primary regardless of it having changed. If a primary gets unplugged and plugged back in we'd
    // otherwise end up with priorities not being in sync with xrandr primary because the effective priorities in the
    // atoms haven't changed; also see priority().
    if (newPriority == 1) {
        setAsPrimary();
    }
}

QPoint XRandROutput::position() const
{
    return m_crtc ? m_crtc->geometry().topLeft() : QPoint();
}

QSize XRandROutput::size() const
{
    return m_crtc ? m_crtc->geometry().size() : QSize();
}

XRandRMode::Map XRandROutput::modes() const
{
    return m_modes;
}

QString XRandROutput::currentModeId() const
{
    return m_crtc ? QString::number(m_crtc->mode()) : QString();
}

XRandRMode *XRandROutput::currentMode() const
{
    if (!m_crtc) {
        return nullptr;
    }

    unsigned int modeId = m_crtc->mode();
    if (!m_modes.contains(modeId)) {
        return nullptr;
    }

    return m_modes[modeId];
}

KScreen::Output::Rotation XRandROutput::rotation() const
{
    return static_cast<KScreen::Output::Rotation>(m_crtc ? m_crtc->rotation() : XCB_RANDR_ROTATION_ROTATE_0);
}

bool XRandROutput::isHorizontal() const
{
    const auto rot = rotation();
    return rot == KScreen::Output::Rotation::None || rot == KScreen::Output::Rotation::Inverted || rot == KScreen::Output::Rotation::Flipped
        || rot == KScreen::Output::Rotation::Flipped180;
}

QByteArray XRandROutput::edid() const
{
    if (m_edid.isNull()) {
        m_edid = XRandR::outputEdid(m_id);
    }
    return m_edid;
}

XRandRCrtc *XRandROutput::crtc() const
{
    return m_crtc;
}

void XRandROutput::update(xcb_randr_crtc_t crtc, xcb_randr_mode_t mode, xcb_randr_connection_t conn)
{
    qCDebug(KSCREEN_XRANDR) << "XRandROutput" << m_id << "update"
                            << "\n"
                            << "\tm_connected:" << m_connected << "\n"
                            << "\tm_crtc" << m_crtc << "\n"
                            << "\tCRTC:" << crtc << "\n"
                            << "\tMODE:" << mode << "\n"
                            << "\tConnection:" << conn;

    // Connected or disconnected
    if (isConnected() != (conn == XCB_RANDR_CONNECTION_CONNECTED)) {
        if (conn == XCB_RANDR_CONNECTION_CONNECTED) {
            // New monitor has been connected, refresh everything
            init();
        } else {
            // Disconnected
            m_connected = conn;
            m_clones.clear();
            m_heightMm = 0;
            m_widthMm = 0;
            m_type = KScreen::Output::Unknown;
            qDeleteAll(m_modes);
            m_modes.clear();
            m_preferredModes.clear();
            m_edid.clear();
        }
    } else if (conn == XCB_RANDR_CONNECTION_CONNECTED) {
        // the output changed in some way, let's update the internal
        // list of modes, as it may have changed
        XCB::OutputInfo outputInfo(m_id, XCB_TIME_CURRENT_TIME);
        if (outputInfo) {
            updateModes(outputInfo);
        }

        m_hotplugModeUpdate = XRandR::hasProperty(m_id, "hotplug_mode_update");
        m_edid.clear();
    }

    // A monitor has been enabled or disabled
    // We don't use isEnabled(), because it checks for crtc && crtc->mode(), however
    // crtc->mode may already be unset due to xcb_randr_crtc_tChangeNotify coming before
    // xcb_randr_output_tChangeNotify and reseting the CRTC mode

    if ((m_crtc == nullptr) != (crtc == XCB_NONE)) {
        if (crtc == XCB_NONE && mode == XCB_NONE) {
            // Monitor has been disabled
            m_crtc->disconectOutput(m_id);
            m_crtc = nullptr;
        } else {
            m_crtc = m_config->crtc(crtc);
            m_crtc->connectOutput(m_id);
        }
    }
}

static constexpr const char *KDE_SCREEN_INDEX = "_KDE_SCREEN_INDEX";

XRandROutput::Priority XRandROutput::outputPriorityFromProperty() const
{
    if (!isConnected()) {
        return 0;
    }

    xcb_atom_t screen_index_atom = XCB::InternAtom(/* only_if_exists */ false, strlen(KDE_SCREEN_INDEX), KDE_SCREEN_INDEX)->atom;

    auto cookie = xcb_randr_get_output_property(XCB::connection(),
                                                m_id,
                                                screen_index_atom,
                                                XCB_ATOM_INTEGER,
                                                /*offset*/ 0,
                                                /*length*/ 1,
                                                /*delete*/ false,
                                                /*pending*/ false);
    XCB::ScopedPointer<xcb_randr_get_output_property_reply_t> reply(xcb_randr_get_output_property_reply(XCB::connection(), cookie, nullptr));
    if (!reply) {
        return 0;
    }

    if (!(reply->type == XCB_ATOM_INTEGER && reply->format == PRIORITY_FORMAT && reply->num_items == 1)) {
        return 0;
    }

    const uint8_t *prop = xcb_randr_get_output_property_data(reply.data());
    const Priority priority = *reinterpret_cast<const Priority *>(prop);
    return priority;
}

void XRandROutput::setOutputPriorityToProperty(Priority priority)
{
    if (!isConnected()) {
        return;
    }

    const std::array<Priority, 1> data = {priority};

    xcb_atom_t screen_index_atom = XCB::InternAtom(/* only_if_exists */ false, strlen(KDE_SCREEN_INDEX), KDE_SCREEN_INDEX)->atom;

    xcb_randr_change_output_property(XCB::connection(), //
                                     m_id,
                                     screen_index_atom,
                                     XCB_ATOM_INTEGER,
                                     PRIORITY_FORMAT,
                                     XCB_PROP_MODE_REPLACE,
                                     data.size(),
                                     data.data());
}

void XRandROutput::setAsPrimary()
{
    if (isConnected() && isEnabled()) {
        xcb_randr_set_output_primary(XCB::connection(), XRandR::rootWindow(), m_id);
    }
}

void XRandROutput::init()
{
    XCB::OutputInfo outputInfo(m_id, XCB_TIME_CURRENT_TIME);
    Q_ASSERT(outputInfo);
    if (!outputInfo) {
        return;
    }

    m_name = QString::fromUtf8((const char *)xcb_randr_get_output_info_name(outputInfo.data()), outputInfo->name_len);
    m_type = fetchOutputType(m_id, m_name);
    m_icon = QString();
    m_connected = (xcb_randr_connection_t)outputInfo->connection;

    xcb_randr_output_t *clones = xcb_randr_get_output_info_clones(outputInfo.data());
    for (int i = 0; i < outputInfo->num_clones; ++i) {
        m_clones.append(clones[i]);
    }

    m_widthMm = outputInfo->mm_width;
    m_heightMm = outputInfo->mm_height;

    m_crtc = m_config->crtc(outputInfo->crtc);
    if (m_crtc) {
        m_crtc->connectOutput(m_id);
    }
    m_hotplugModeUpdate = XRandR::hasProperty(m_id, "hotplug_mode_update");

    updateModes(outputInfo);
}

void XRandROutput::updateModes(const XCB::OutputInfo &outputInfo)
{
    /* Init modes */
    XCB::ScopedPointer<xcb_randr_get_screen_resources_reply_t> screenResources(XRandR::screenResources());

    Q_ASSERT(screenResources);
    if (!screenResources) {
        return;
    }
    xcb_randr_mode_info_t *modes = xcb_randr_get_screen_resources_modes(screenResources.data());
    xcb_randr_mode_t *outputModes = xcb_randr_get_output_info_modes(outputInfo.data());

    m_preferredModes.clear();
    qDeleteAll(m_modes);
    m_modes.clear();
    for (int i = 0; i < outputInfo->num_modes; ++i) {
        /* Resources->modes contains all possible modes, we are only interested
         * in those listed in outputInfo->modes. */
        for (int j = 0; j < screenResources->num_modes; ++j) {
            if (modes[j].id != outputModes[i]) {
                continue;
            }

            XRandRMode *mode = new XRandRMode(modes[j], this);
            m_modes.insert(mode->id(), mode);

            if (i < outputInfo->num_preferred) {
                m_preferredModes.append(QString::number(mode->id()));
            }
            break;
        }
    }
}

KScreen::Output::Type XRandROutput::fetchOutputType(xcb_randr_output_t outputId, const QString &name)
{
    QString type = QString::fromUtf8(typeFromProperty(outputId));
    if (type.isEmpty()) {
        type = name;
    }

    return Utils::guessOutputType(type, name);
}

QByteArray XRandROutput::typeFromProperty(xcb_randr_output_t outputId)
{
    QByteArray type;

    XCB::InternAtom atomType(true, 13, "ConnectorType");
    if (!atomType) {
        return type;
    }

    auto cookie = xcb_randr_get_output_property(XCB::connection(), outputId, atomType->atom, XCB_ATOM_ANY, 0, 100, false, false);
    XCB::ScopedPointer<xcb_randr_get_output_property_reply_t> reply(xcb_randr_get_output_property_reply(XCB::connection(), cookie, nullptr));
    if (!reply) {
        return type;
    }

    if (!(reply->type == XCB_ATOM_ATOM && reply->format == 32 && reply->num_items == 1)) {
        return type;
    }

    const uint8_t *prop = xcb_randr_get_output_property_data(reply.data());
    XCB::AtomName atomName(*reinterpret_cast<const xcb_atom_t *>(prop));
    if (!atomName) {
        return type;
    }

    return QByteArray(xcb_get_atom_name_name(atomName), xcb_get_atom_name_name_length(atomName));
}

bool isScaling(const xcb_render_transform_t &tr)
{
    return tr.matrix11 != fZero && tr.matrix12 == fZero && tr.matrix13 == fZero && tr.matrix21 == fZero && tr.matrix22 != fZero && tr.matrix23 == fZero
        && tr.matrix31 == fZero && tr.matrix32 == fZero && tr.matrix33 == fOne;
}

xcb_render_transform_t zeroTransform()
{
    return {DOUBLE_TO_FIXED(0),
            DOUBLE_TO_FIXED(0),
            DOUBLE_TO_FIXED(0),
            DOUBLE_TO_FIXED(0),
            DOUBLE_TO_FIXED(0),
            DOUBLE_TO_FIXED(0),
            DOUBLE_TO_FIXED(0),
            DOUBLE_TO_FIXED(0),
            DOUBLE_TO_FIXED(0)};
}

xcb_render_transform_t unityTransform()
{
    return {DOUBLE_TO_FIXED(1),
            DOUBLE_TO_FIXED(0),
            DOUBLE_TO_FIXED(0),
            DOUBLE_TO_FIXED(0),
            DOUBLE_TO_FIXED(1),
            DOUBLE_TO_FIXED(0),
            DOUBLE_TO_FIXED(0),
            DOUBLE_TO_FIXED(0),
            DOUBLE_TO_FIXED(1)};
}

xcb_render_transform_t XRandROutput::currentTransform() const
{
    auto cookie = xcb_randr_get_crtc_transform(XCB::connection(), m_crtc->crtc());
    xcb_generic_error_t *error = nullptr;
    auto *reply = xcb_randr_get_crtc_transform_reply(XCB::connection(), cookie, &error);
    if (error) {
        return zeroTransform();
    }

    const xcb_render_transform_t transform = reply->pending_transform;
    free(reply);
    return transform;
}

QSizeF XRandROutput::logicalSize() const
{
    const QSize modeSize = size();
    if (!modeSize.isValid()) {
        return QSize();
    }

    const xcb_render_transform_t transform = currentTransform();
    const qreal width = FIXED_TO_DOUBLE(transform.matrix11) * modeSize.width();
    const qreal height = FIXED_TO_DOUBLE(transform.matrix22) * modeSize.height();

    return QSizeF(width, height);
}

void XRandROutput::updateLogicalSize(const KScreen::OutputPtr &output, XRandRCrtc *crtc)
{
    if (!crtc) {
        // TODO: This is a workaround for now when updateLogicalSize is called on enabling the
        //       output. At this point m_crtc is not yet set. Change the order in the future so
        //       that the additional argument is not necessary anymore.
        crtc = m_crtc;
    }
    const QSizeF logicalSize = output->explicitLogicalSize();
    xcb_render_transform_t transform = unityTransform();

    KScreen::ModePtr mode = output->currentMode() ? output->currentMode() : output->preferredMode();
    if (mode && logicalSize.isValid()) {
        QSize modeSize = mode->size();
        if (!output->isHorizontal()) {
            modeSize.transpose();
        }

        const qreal widthFactor = logicalSize.width() / (qreal)modeSize.width();
        const qreal heightFactor = logicalSize.height() / (qreal)modeSize.height();
        transform.matrix11 = DOUBLE_TO_FIXED(widthFactor);
        transform.matrix22 = DOUBLE_TO_FIXED(heightFactor);
    }

    QByteArray filterName(isScaling(transform) ? "bilinear" : "nearest");

    auto cookie = xcb_randr_set_crtc_transform_checked(XCB::connection(), crtc->crtc(), transform, filterName.size(), filterName.data(), 0, nullptr);
    xcb_generic_error_t *error = xcb_request_check(XCB::connection(), cookie);
    if (error) {
        qCDebug(KSCREEN_XRANDR) << "Error on logical size transformation!";
        free(error);
    }
}

KScreen::OutputPtr XRandROutput::toKScreenOutput() const
{
    KScreen::OutputPtr kscreenOutput(new KScreen::Output);

    const bool signalsBlocked = kscreenOutput->signalsBlocked();
    kscreenOutput->blockSignals(true);
    kscreenOutput->setId(m_id);
    kscreenOutput->setType(m_type);
    kscreenOutput->setSizeMm(QSize(m_widthMm, m_heightMm));
    kscreenOutput->setName(m_name);
    kscreenOutput->setIcon(m_icon);
    kscreenOutput->setPriority(priority());

    // See https://bugzilla.redhat.com/show_bug.cgi?id=1290586
    // QXL will be creating a new mode we need to jump to every time the display is resized
    kscreenOutput->setFollowPreferredMode(m_hotplugModeUpdate && m_crtc && m_crtc->isChangedFromOutside());

    kscreenOutput->setConnected(isConnected());
    if (isConnected()) {
        KScreen::ModeList kscreenModes;
        for (auto iter = m_modes.constBegin(), end = m_modes.constEnd(); iter != end; ++iter) {
            XRandRMode *mode = iter.value();
            kscreenModes.insert(QString::number(iter.key()), mode->toKScreenMode());
        }
        kscreenOutput->setModes(kscreenModes);
        kscreenOutput->setPreferredModes(m_preferredModes);
        kscreenOutput->setClones([](const QList<xcb_randr_output_t> &clones) {
            QList<int> kclones;
            kclones.reserve(clones.size());
            for (xcb_randr_output_t o : clones) {
                kclones.append(static_cast<int>(o));
            }
            return kclones;
        }(m_clones));
        kscreenOutput->setEnabled(isEnabled());
        if (isEnabled()) {
            kscreenOutput->setSize(size());
            kscreenOutput->setPos(position());
            kscreenOutput->setRotation(rotation());
            kscreenOutput->setCurrentModeId(currentModeId());
        }
        // TODO: set logical size?
    }

    kscreenOutput->blockSignals(signalsBlocked);
    return kscreenOutput;
}

#include "moc_xrandroutput.cpp"
