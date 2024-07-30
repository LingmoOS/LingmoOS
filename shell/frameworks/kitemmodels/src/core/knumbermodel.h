/*
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNUMBERMODEL_H
#define KNUMBERMODEL_H

#include <QAbstractListModel>
#include <QLocale>

#include "kitemmodels_export.h"

#include <memory>

class KNumberModelPrivate;

/**
 * @class KNumberModel knumbermodel.h KNumberModel
 *
 * Creates a model of entries from N to M with rows at a given interval
 *
 * The model contains two roles:
 * @li display - the number represented as a string
 * @li value - the actual value as a number
 *
 * @since 5.65
 */
class KITEMMODELS_EXPORT KNumberModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * The minimum value for the model
     *
     * The default value is @c 1.0.
     */
    Q_PROPERTY(qreal minimumValue READ minimumValue WRITE setMinimumValue NOTIFY minimumValueChanged)
    /**
     * The maximum value for the model
     *
     * The default value is @c 1.0.
     *
     * @note  If @c maximumValue is a multiple of @c stepSize added to @c minimumValue
     * it will be included. Otherwise it will not be reached.
     * E.g. in a model with a @c minimumValue of 0.0, a @c maximumValue of 1.0 and a @c stepSize of 0.3, the final row will be 0.9.
     */
    Q_PROPERTY(qreal maximumValue READ maximumValue WRITE setMaximumValue NOTIFY maximumValueChanged)
    /**
     * Step between listed entries
     *
     * The default value is @c 1.0.
     */
    Q_PROPERTY(qreal stepSize READ stepSize WRITE setStepSize NOTIFY stepSizeChanged)
    /**
     * Defines the string representation of the number,
     * e.g. "1,000" or "1000".
     *
     * Default is @c QLocale::Default.
     */
    Q_PROPERTY(QLocale::NumberOptions formattingOptions READ formattingOptions WRITE setFormattingOptions NOTIFY formattingOptionsChanged)

public:
    explicit KNumberModel(QObject *parent = nullptr);
    ~KNumberModel() override;

    enum Roles {
        DisplayRole = Qt::DisplayRole,
        ValueRole = Qt::UserRole,
    };

    void setMinimumValue(qreal minimumValue);
    qreal minimumValue() const;

    void setMaximumValue(qreal maximumValue);
    qreal maximumValue() const;

    void setStepSize(qreal stepSize);
    qreal stepSize() const;

    void setFormattingOptions(QLocale::NumberOptions options);
    QLocale::NumberOptions formattingOptions() const;

    /**
     * Returns the value represented at the given index.
     */
    qreal value(const QModelIndex &index) const;

    int rowCount(const QModelIndex &index = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void minimumValueChanged();
    void maximumValueChanged();
    void stepSizeChanged();
    void formattingOptionsChanged();

private:
    std::unique_ptr<KNumberModelPrivate> const d;
};

#endif
