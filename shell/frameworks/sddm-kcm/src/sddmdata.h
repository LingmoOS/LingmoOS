/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef SDDM_DATA_H
#define SDDM_DATA_H

#include <KCModuleData>

class SddmSettings;

class SddmData : public KCModuleData
{
    Q_OBJECT
public:
    SddmData(QObject *parent);
    SddmSettings *sddmSettings() const;

private:
    SddmSettings *m_settings;
};

#endif
