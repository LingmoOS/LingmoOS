/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <QList>
#include <QObject>
#include <QVariant>

#include "processcore_export.h"

namespace KSysGuard
{
class Processes;
class Process;
class ProcessAttribute;

/**
 * Base class for a process plugin data
 * Plugins provide a list of additional attributes, which in turn have data about a given process
 */
class PROCESSCORE_EXPORT ProcessDataProvider : public QObject
{
    Q_OBJECT

public:
    ProcessDataProvider(QObject *parent, const QVariantList &args);
    ~ProcessDataProvider() override;

    /**
     * Accessors for process information matching
     */
    KSysGuard::Processes *processes() const;

    /**
     * Returns a new process object for a given PID
     * This will update the process list if this PID does not exist yet
     * This may return a null pointer
     */
    KSysGuard::Process *getProcess(long pid);

    /**
     * A list of all process attributes provided by this plugin
     * It is expected to remain constant through the lifespan of this class
     */
    QList<ProcessAttribute *> attributes() const;

    /**
     * Called when processes should be updated if manually polled
     * Plugins can however update at any time if enabled
     */
    virtual void update()
    {
    }

    /**
     * True when at least one attribute from this plugin is subscribed
     */
    bool enabled() const;

    virtual void handleEnabledChanged(bool enabled)
    {
        Q_UNUSED(enabled)
    }

    // for any future compatibility
    virtual void virtual_hook(int id, void *data)
    {
        Q_UNUSED(id)
        Q_UNUSED(data)
    }

protected:
    /**
     * Register a new process attribute
     * Process attributes should be created in the plugin constructor and must live for the duration the plugin
     */
    void addProcessAttribute(ProcessAttribute *attribute);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
