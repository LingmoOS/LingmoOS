#ifndef LINGMOTEXTSTYLE_H
#define LINGMOTEXTSTYLE_H

#include "singleton.h"
#include "stdafx.h"
#include <QFont>
#include <QObject>
#include <QtQml/qqml.h>

/**
 * @brief The LingmoTextStyle class
 */
class LingmoTextStyle : public QObject {
    Q_OBJECT
    Q_PROPERTY_AUTO(QString, family)
    Q_PROPERTY_AUTO(QFont, Caption)
    Q_PROPERTY_AUTO(QFont, Body)
    Q_PROPERTY_AUTO(QFont, BodyStrong)
    Q_PROPERTY_AUTO(QFont, Subtitle)
    Q_PROPERTY_AUTO(QFont, Title)
    Q_PROPERTY_AUTO(QFont, TitleLarge)
    Q_PROPERTY_AUTO(QFont, Display)
    QML_NAMED_ELEMENT(LingmoTextStyle)
    QML_SINGLETON

private:
    explicit LingmoTextStyle(QObject* parent = nullptr);

public:
    SINGLETON(LingmoTextStyle)

    static LingmoTextStyle* create(QQmlEngine*, QJSEngine*)
    {
        return getInstance();
    }
};

#endif // LINGMOTEXTSTYLE_H
