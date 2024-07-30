/*
 * SPDX-FileCopyrightText: 2007, 2008 Petri Damstén <damu@iki.fi>
 * SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CONVERTERRUNNER_H
#define CONVERTERRUNNER_H

#include <KRunner/AbstractRunner>
#include <KRunner/Action>
#include <KUnitConversion/Converter>
#include <KUnitConversion/UnitCategory>
#include <QLocale>
#include <QRegularExpression>
#include <QTimer>

using namespace KRunner;

/**
 * This class converts values to different units.
 */
class ConverterRunner : public AbstractRunner
{
    Q_OBJECT

public:
    ConverterRunner(QObject *parent, const KPluginMetaData &metaData);
    void init() override;
    ~ConverterRunner() override;

    void match(RunnerContext &context) override;
    void run(const RunnerContext &context, const QueryMatch &match) override;
    QMimeData *mimeDataForMatch(const QueryMatch &match) override;

private Q_SLOTS:
    void updateCompatibleUnits();

private:
    std::unique_ptr<KUnitConversion::Converter> converter;
    const QLocale locale;
    QRegularExpression valueRegex;
    QRegularExpression unitSeperatorRegex;
    /** To convert currency symbols back to ISO string and handle case sensitive units */
    QMap<QString, QString> compatibleUnits;

    QTimer m_currencyUpdateTimer;

    const KRunner::Actions actionList;
    QPair<bool, double> stringToDouble(const QStringView &value);
    QPair<bool, double> getValidatedNumberValue(const QString &value);
    QList<KUnitConversion::Unit> createResultUnits(QString &outputUnitString, const KUnitConversion::UnitCategory &category);
};

#endif
