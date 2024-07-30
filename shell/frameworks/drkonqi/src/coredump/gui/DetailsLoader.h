// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#pragma once

#include <memory>

#include <QObject>
#include <QProcess>

#include "Patient.h"

Q_DECLARE_METATYPE(Patient *)
class DetailsLoader : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Patient *patient MEMBER m_patient WRITE setPatient NOTIFY patientChanged)
    Patient *m_patient = nullptr;
    void setPatient(Patient *patient);
    Q_SIGNAL void patientChanged();

public:
    using QObject::QObject;

Q_SIGNALS:
    void details(const QString &details);
    void error(const QString &error);

private:
    void load();
    std::unique_ptr<QProcess> m_LoaderProcess;
};
