// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

// FIXME: seems should be "-" not "_"
class LockdownPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Lockdown")

    Q_PROPERTY(bool disable_printing READ printing WRITE setPrint)
    inline bool printing() const { return m_printing; }
    inline void setPrint(bool enabled) { m_printing = enabled; }

    Q_PROPERTY(bool disable_save_to_disk READ disk WRITE setDisk)
    inline bool disk() const { return m_save_to_disk; }
    inline void setDisk(bool enabled) { m_save_to_disk = enabled; }

    Q_PROPERTY(bool disable_application_handlers READ handlers WRITE setHandlers)
    inline bool handlers() const { return m_application_handlers; }
    inline void setHandlers(bool enabled) { m_application_handlers = enabled; }

    Q_PROPERTY(bool disable_location READ location WRITE setLocation)
    inline bool location() const { return m_location; }
    inline void setLocation(bool enabled) { m_location = enabled; }

    Q_PROPERTY(bool disable_camera READ camera WRITE setCamera)
    inline bool camera() const { return m_camera; }
    inline void setCamera(bool enabled) { m_camera = enabled; }

    Q_PROPERTY(bool disable_microphone READ microphone WRITE setMicrophone)
    inline bool microphone() const { return m_microphone; }
    inline void setMicrophone(bool enabled) { m_microphone = enabled; }

    Q_PROPERTY(bool disable_sound_output READ soundOutput WRITE setSoundOutput)
    inline bool soundOutput() const { return m_sound_output; }
    inline void setSoundOutput(bool enabled) { m_sound_output = enabled; }

public:
    explicit LockdownPortal(QObject *parent);
    ~LockdownPortal() = default;

private:
    bool m_printing;
    bool m_save_to_disk;
    bool m_application_handlers;
    bool m_location;
    bool m_camera;
    bool m_microphone;
    bool m_sound_output;
};
