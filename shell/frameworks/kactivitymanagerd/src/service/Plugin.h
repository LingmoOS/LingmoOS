/*
 *   SPDX-FileCopyrightText: 2011-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "kactivitymanagerd_plugin_export.h"

// Qt
#include <QMetaObject>
#include <QObject>

// KDE
#include <kconfiggroup.h>
#include <kpluginfactory.h>

// Utils
#include <utils/d_ptr.h>

// Local
#include "Event.h"
#include "Module.h"

/**
 *
 */
class KACTIVITYMANAGERD_PLUGIN_EXPORT Plugin : public Module
{
    Q_OBJECT

public:
    explicit Plugin(QObject *parent);
    ~Plugin() override;

    /**
     * Initializes the plugin.
     * @arg modules Activities, Resources and Features manager objects
     * @returns the plugin needs to return whether it has
     *      successfully been initialized
     */
    virtual bool init(QHash<QString, QObject *> &modules) = 0;

    /**
     * Returns the config group for the plugin.
     * In order to use it, you need to set the plugin name.
     */
    KConfigGroup config() const;
    QString name() const;

    /**
     * Convenience meta-method to provide prettier invocation of QMetaObject::invokeMethod
     */
    // template <typename ReturnType>
    // inline static ReturnType retrieve(QObject *object, const char *method,
    //                                   const char *returnTypeName)
    // {
    //     ReturnType result;
    //
    //     QMetaObject::invokeMethod(
    //         object, method, Qt::DirectConnection,
    //         QReturnArgument<ReturnType>(returnTypeName, result));
    //
    //     return result;
    // }

    template<typename ReturnType, typename... Args>
    inline static ReturnType retrieve(QObject *object, const char *method, Args... args)
    {
        ReturnType result;

        QMetaObject::invokeMethod(object, method, Qt::DirectConnection, Q_RETURN_ARG(ReturnType, result), args...);

        return result;
    }

    /**
     * Convenience meta-method to provide prettier invocation of QMetaObject::invokeMethod
     */
    // template <Qt::ConnectionType connection = Qt::QueuedConnection>
    // inline static void invoke(QObject *object, const char *method,
    //                            const char *returnTypeName)
    // {
    //     Q_UNUSED(returnTypeName);
    //     QMetaObject::invokeMethod(object, method, connection);
    // }

    template<Qt::ConnectionType connection, typename... Args>
    inline static void invoke(QObject *object, const char *method, Args... args)
    {
        QMetaObject::invokeMethod(object, method, connection, args...);
    }

protected:
    void setName(const QString &name);

private:
    D_PTR;
};
