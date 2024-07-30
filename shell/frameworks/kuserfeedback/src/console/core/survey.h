/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_SURVEY_H
#define KUSERFEEDBACK_CONSOLE_SURVEY_H

#include <QMetaType>
#include <QSharedDataPointer>
#include <QVector>

class QString;
class QUrl;
class QUuid;

namespace KUserFeedback {
namespace Console {

class SurveyData;

/** Data for one survey. */
class Survey
{
public:
    Survey();
    Survey(const Survey&);
    ~Survey();
    Survey& operator=(const Survey&);

    bool operator==(const Survey &other) const;
    bool operator!=(const Survey &other) const;

    QUuid uuid() const;
    void setUuid(const QUuid &id);

    QString name() const;
    void setName(const QString& name);

    QUrl url() const;
    void setUrl(const QUrl& url);

    bool isActive() const;
    void setActive(bool enabled);

    QString target() const;
    void setTarget(const QString &target);

    QByteArray toJson() const;
    static QVector<Survey> fromJson(const QByteArray &data);
private:
    QSharedDataPointer<SurveyData> d;
};

}
}

Q_DECLARE_TYPEINFO(KUserFeedback::Console::Survey, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KUserFeedback::Console::Survey)

#endif // KUSERFEEDBACK_CONSOLE_SURVEY_H
