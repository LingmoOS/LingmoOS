/*
 * SPDX-FileCopyrightText: 2016 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef LOCATIONLISTMODEL_H
#define LOCATIONLISTMODEL_H

#include <Plasma5Support/DataEngine>
#include <Plasma5Support/DataEngineConsumer>

#include <QAbstractListModel>
#include <QList>
#include <QMap>

class WeatherValidator : public QObject
{
    Q_OBJECT
public:
    WeatherValidator(Plasma5Support::DataEngine *weatherDataengine, const QString &ionName, QObject *parent = nullptr);
    ~WeatherValidator() override;

    /**
     * @param location the name of the location to find
     */
    void validate(const QString &location);

Q_SIGNALS:
    /**
     * Emitted when an error in validation occurs
     **/
    void error(const QString &message);

    /**
     * Emitted when validation is done
     * @param sources a mapping of user-friendly names to the DataEngine source
     **/
    void finished(const QMap<QString, QString> &sources);

public Q_SLOTS: // callback for the weather dataengine
    void dataUpdated(const QString &source, const Plasma5Support::DataEngine::Data &data);

private:
    Plasma5Support::DataEngine *m_weatherDataEngine;
    QString m_ionName;
};

class LocationItem
{
public:
    LocationItem(const QString &source);

    int relativeQuality(const QString &service) const;

    QString weatherStation;
    QString weatherService;
    QString value;
    int quality = 0;
};

Q_DECLARE_METATYPE(LocationItem)
Q_DECLARE_TYPEINFO(LocationItem, Q_MOVABLE_TYPE);

class LocationListModel : public QAbstractListModel, public Plasma5Support::DataEngineConsumer
{
    Q_OBJECT
    Q_PROPERTY(bool validatingInput READ isValidatingInput NOTIFY validatingInputChanged)

public:
    explicit LocationListModel(QObject *parent = nullptr);

public: // QAbstractListModel API
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &index) const override;

public:
    bool isValidatingInput() const;

public:
    Q_INVOKABLE QString nameForListIndex(int listIndex) const;
    Q_INVOKABLE QString valueForListIndex(int listIndex) const;
    Q_INVOKABLE void searchLocations(const QString &searchString, const QStringList &services);
    Q_INVOKABLE void clear();

Q_SIGNALS:
    void validatingInputChanged(bool validatingInput);
    void locationSearchDone(bool success, const QString &searchString);

private:
    void addSources(const QMap<QString, QString> &sources);
    void validatorError(const QString &error);
    void completeSearch();

private:
    QList<LocationItem> m_locations;

    bool m_validatingInput;
    QString m_searchString;
    int m_checkedInCount;
    QList<WeatherValidator *> m_validators;

    QMap<QString, QString> m_serviceCodeToDisplayName;
};

#endif // LOCATIONLISTMODEL_H
