/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_SAMPLE_H
#define KUSERFEEDBACK_CONSOLE_SAMPLE_H

#include <QMetaType>
#include <QSharedDataPointer>
#include <QVector>

class QDateTime;
class QString;

namespace KUserFeedback {
namespace Console {

class Product;
class SampleData;

/** One data sample reported by a client. */
class Sample
{
public:
    Sample();
    Sample(const Sample&);
    ~Sample();
    Sample& operator=(const Sample&);

    QDateTime timestamp() const;
    QVariant value(const QString &name) const;

    static QVector<Sample> fromJson(const QByteArray &json, const Product &product);
    static QByteArray toJson(const QVector<Sample> &samples, const Product &product);

private:
    QSharedDataPointer<SampleData> d;
};

}
}

Q_DECLARE_TYPEINFO(KUserFeedback::Console::Sample, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KUserFeedback::Console::Sample)

#endif // KUSERFEEDBACK_CONSOLE_SAMPLE_H
