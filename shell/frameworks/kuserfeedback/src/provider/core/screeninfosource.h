/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_SCREENINFOSOURCE_H
#define KUSERFEEDBACK_SCREENINFOSOURCE_H

#include "kuserfeedbackcore_export.h"
#include "abstractdatasource.h"

namespace KUserFeedback {

/*! Data source for information about connected displays.
 *  This provides as array of maps containing the following properties:
 *  - height: Height of the corresponding screen in pixel.
 *  - width: Width of the corresponding screen in pixel.
 *  - dpi: Dots per inch of the corresponding screen.
 *
 *  The default telemetry mode for this source is Provider::DetailedSystemInformation.
 */
class KUSERFEEDBACKCORE_EXPORT ScreenInfoSource :  public AbstractDataSource
{
    Q_DECLARE_TR_FUNCTIONS(KUserFeedback::ScreenInfoSource)
public:
    /*! Create a new screen information source. */
    ScreenInfoSource();

    QString name() const override;
    QString description() const override;

    QVariant data() override;
};

}

#endif // KUSERFEEDBACK_SCREENINFOSOURCE_H
