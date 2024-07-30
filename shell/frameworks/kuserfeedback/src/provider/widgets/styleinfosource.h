/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_STYLEINFOSOURCE_H
#define KUSERFEEDBACK_STYLEINFOSOURCE_H

#include "kuserfeedbackwidgets_export.h"

#include <KUserFeedback/AbstractDataSource>

namespace KUserFeedback {

/*! Data source the widget style and color scheme used by the application. */
class KUSERFEEDBACKWIDGETS_EXPORT StyleInfoSource : public AbstractDataSource
{
public:
    Q_DECLARE_TR_FUNCTIONS(KUserFeedback::StyleInfoSource)
public:
    StyleInfoSource();

    QString name() const override;
    QString description() const override;

    QVariant data() override;
};

}

#endif // KUSERFEEDBACK_STYLEINFOSOURCE_H
