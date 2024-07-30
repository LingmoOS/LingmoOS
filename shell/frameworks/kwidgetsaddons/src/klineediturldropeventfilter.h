/*
    SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KLINEEDITURLDROPEVENTFILTER_H
#define KLINEEDITURLDROPEVENTFILTER_H

#include <kwidgetsaddons_export.h>

#include <QObject>

/**
 * @class KLineEditUrlDropEventFilter klineediturldropeventfilter.h KLineEditUrlDropEventFilter
 *
 * This class provides an event filter that can be installed on a QLineEdit
 * or a subclass of it (KLineEdit) to make it handle URL drop events so
 * when a URL is dropped it replaces the existing content.
 *
 * Porting from KF5 to KF6:
 *
 * The class LineEditUrlDropEventFilter was renamed to KLineEditUrlDropEventFilter.
 *
 * @Since 6.0
 */
class KWIDGETSADDONS_EXPORT KLineEditUrlDropEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit KLineEditUrlDropEventFilter(QObject *parent = nullptr);
    ~KLineEditUrlDropEventFilter() override;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
};

#endif
