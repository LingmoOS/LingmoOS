#pragma once

#include <QColor>
#include <QObject>
#include <QQmlEngine>

#include "stdafx.h"

/**
 * @brief The LingmoAccentColor class
 */
class LingmoAccentColor : public QObject {
    Q_OBJECT

    Q_PROPERTY_AUTO(QColor, darkest)
    Q_PROPERTY_AUTO(QColor, darker)
    Q_PROPERTY_AUTO(QColor, dark)
    Q_PROPERTY_AUTO(QColor, normal)
    Q_PROPERTY_AUTO(QColor, light)
    Q_PROPERTY_AUTO(QColor, lighter)
    Q_PROPERTY_AUTO(QColor, lightest)
    QML_NAMED_ELEMENT(LingmoAccentColor)
public:
    explicit LingmoAccentColor(QObject* parent = nullptr);
};
