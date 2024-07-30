/*  This file is part of the KDE project
 *    SPDX-FileCopyrightText: 2023 Quang Ngô <ngoquang2708@gmail.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-only
 *
 */

#pragma once

#include <QMutex>
#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>

#ifdef WITH_DDCUTIL
#include <ddcutil_c_api.h>
#include <ddcutil_macros.h> // for DDCUTIL_V{MAJOR,MINOR,MICRO}
#include <ddcutil_status_codes.h>

#define DDCUTIL_VERSION QT_VERSION_CHECK(DDCUTIL_VMAJOR, DDCUTIL_VMINOR, DDCUTIL_VMICRO)
#endif

#include "displaybrightness.h"

class BrightnessWorker;

class DDCutilDisplay : public DisplayBrightness
{
    Q_OBJECT

    friend class BrightnessWorker;

public:
#ifdef WITH_DDCUTIL
    DDCutilDisplay(DDCA_Display_Ref, QMutex *openDisplayMutex);
    DDCA_IO_Path ioPath() const;
    static QString generatePathId(const DDCA_IO_Path &displayPath);
#endif
    ~DDCutilDisplay();

    QString id() const override;
    QString label() const override;
    int knownSafeMinBrightness() const override;
    int maxBrightness() const override;
    int brightness() const override;
    void setBrightness(int value) override;
    bool supportsBrightness() const;
    void resumeWorker();
    void pauseWorker();
    std::optional<QByteArray> edidData() const override;

Q_SIGNALS:
    void supportsBrightnessChanged(bool supportsBrightness);
    void ddcBrightnessChangeRequested(int value, DDCutilDisplay *display);

private Q_SLOTS:
    void ddcBrightnessChangeFinished(bool isSuccessful);
    void onTimeout();

private:
#ifdef WITH_DDCUTIL
    DDCA_Display_Ref m_displayRef;
    DDCA_IO_Path m_ioPath;
#endif
    QString m_id;
    QString m_label;
    BrightnessWorker *m_brightnessWorker;
    QThread m_brightnessWorkerThread;
    QTimer *m_timer;
    QMutex *m_openDisplayMutex;
    int m_brightness;
    int m_maxBrightness;
    bool m_supportsBrightness;
    QByteArray m_edidData;
};

class BrightnessWorker : public QObject
{
    Q_OBJECT
    friend class DDCutilDisplay;

private Q_SLOTS:
    void ddcSetBrightness(int value, DDCutilDisplay *display);
Q_SIGNALS:
    void ddcBrightnessChangeApplied(bool isSuccessful);
};
