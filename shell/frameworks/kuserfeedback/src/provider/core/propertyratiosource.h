/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_PROPERTYRATIOSOURCE_H
#define KUSERFEEDBACK_PROPERTYRATIOSOURCE_H

#include "kuserfeedbackcore_export.h"
#include "abstractdatasource.h"

namespace KUserFeedback {

class PropertyRatioSourcePrivate;

/*! Records the time ratio a given QObject property has a specific value.
 *
 *  An example use-case would be the usage ratio of a applications
 *  views/modes selected by a QTabWidget or QRadioButton.
 *
 *  The default telemetry mode for this source is Provider::DetailedUsageStatistics.
 */
class KUSERFEEDBACKCORE_EXPORT PropertyRatioSource :  public AbstractDataSource
{
public:
    /*! Create a new property ratio data source.
     * @param obj the QObject of which a property should be monitored.
     * @param propertyName The name of the property to monitor.
     * This property must have a change notification signal. The value must have
     * comparison operators registered.
     * @param sampleName This is the name of the database field this data source is
     * associated with.
     */
    explicit PropertyRatioSource(QObject *obj, const char* propertyName, const QString &sampleName);

    /*! Returns the monitored object. */
    QObject* object() const;
    /*! Sets the monitoried object. */
    void setObject(QObject *object);

    /*! Returns the property name. */
    QString propertyName() const;
    /*! Sets the property name that should be monitored. */
    void setPropertyName(const QString &name);

    /*! Map property value @p value to @p str for sending to the server.
     *  This is useful to map internal identifiers to portable and persistable values,
     *  such as turning pointers or indexes into meaningful descriptions.
     *  @param value The property value to map.
     *  @param str The string the property value @p value should be mapped to.
     */
    void addValueMapping(const QVariant &value, const QString &str);

    QString name() const override;
    /*! Set human readable name.
     *  @see AbstractDataSource::name()
     */
    void setName(const QString &name);

    QString description() const override;

    /*! Set human-readable and translated description of the data provided by this source.
     *  @note This must be set before adding this source, sources without description are
     *  discarded.
     *  @param desc The description.
     */
    void setDescription(const QString &desc);

    QVariant data() override;

    void loadImpl(QSettings *settings) override;
    void storeImpl(QSettings *settings) override;
    void resetImpl(QSettings *settings) override;

    using AbstractDataSource::setId;
private:
    Q_DECLARE_PRIVATE(PropertyRatioSource)
};

}

#endif // KUSERFEEDBACK_PROPERTYRATIOSOURCE_H
