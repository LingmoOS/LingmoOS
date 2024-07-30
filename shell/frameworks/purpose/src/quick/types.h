/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PURPOSE_QML_TYPES
#define PURPOSE_QML_TYPES

#include <QQmlEngine>

#include "alternativesmodel.h"
#include "configuration.h"
#include "job.h"
#include "jobcontroller.h"

struct AlternativesModelForeign {
    Q_GADGET
    QML_ELEMENT
    QML_NAMED_ELEMENT(PurposeAlternativesModel)
    QML_FOREIGN(Purpose::AlternativesModel)
};

struct JobControllerForeign {
    Q_GADGET
    QML_ELEMENT
    QML_NAMED_ELEMENT(PurposeJobController)
    QML_FOREIGN(Purpose::JobController)
};

struct JobForeign {
    Q_GADGET
    QML_ELEMENT
    QML_NAMED_ELEMENT(PurposeJob)
    QML_FOREIGN(Purpose::Job)
    QML_UNCREATABLE("You're not supposed to instantiate jobs")
};

struct ConfigurationForeign {
    Q_GADGET
    QML_ELEMENT
    QML_NAMED_ELEMENT(PurposeConfiguration)
    QML_FOREIGN(Purpose::Configuration)
    QML_UNCREATABLE("You're not supposed to instantiate configurations")
};

#endif
