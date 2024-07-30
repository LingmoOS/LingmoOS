/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PURPOSEMENU_H
#define PURPOSEMENU_H

#include "purposewidgets_export.h"

#include <QMenu>

namespace Purpose
{
class MenuPrivate;

class AlternativesModel;

class PURPOSEWIDGETS_EXPORT Menu : public QMenu
{
    Q_OBJECT
public:
    explicit Menu(QWidget *parent = nullptr);

    /**
     * Exposes the used AlternativesModel so that it can be configured to suit
     * the application needs
     */
    AlternativesModel *model() const;

    /**
     * Forces a reload of the menu in case something changed in the model
     */
    void reload();

Q_SIGNALS:

    /**
     * Emitted just before triggered, just before starting the share operation
     * Can be used to do adjustments to the configuration, such as setting a new Url
     * @since 6.2
     */
    void aboutToShare();

    /**
     * Emitted when a triggered job finishes
     *
     * @p output contains the information offered by the plugin. The information offered will depend on the plugin type.
     * @p error will be 0 if the execution was successful, KIO::ERR_USER_CANCELED if cancelled by the user, otherwise it will provide an error message
     * @p errorMessage the error message
     *
     * @see Purpose::Job
     */
    void finished(const QJsonObject &output, int error, const QString &errorMessage);

private:
    Q_DECLARE_PRIVATE(Menu)
    MenuPrivate *const d_ptr;
};

}

#endif
