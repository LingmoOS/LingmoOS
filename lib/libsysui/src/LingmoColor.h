#pragma once

#include <QColor>
#include <QObject>
#include <QQmlEngine>

#include "LingmoAccentColor.h"
#include "singleton.h"
#include "stdafx.h"

class LingmoColor : public QObject {
    Q_OBJECT

    Q_PROPERTY_AUTO(QColor, Transparent)
    Q_PROPERTY_AUTO(QColor, Black)
    Q_PROPERTY_AUTO(QColor, White)
    Q_PROPERTY_AUTO(QColor, Grey10)
    Q_PROPERTY_AUTO(QColor, Grey20)
    Q_PROPERTY_AUTO(QColor, Grey30)
    Q_PROPERTY_AUTO(QColor, Grey40)
    Q_PROPERTY_AUTO(QColor, Grey50)
    Q_PROPERTY_AUTO(QColor, Grey60)
    Q_PROPERTY_AUTO(QColor, Grey70)
    Q_PROPERTY_AUTO(QColor, Grey80)
    Q_PROPERTY_AUTO(QColor, Grey90)
    Q_PROPERTY_AUTO(QColor, Grey100)
    Q_PROPERTY_AUTO(QColor, Grey110)
    Q_PROPERTY_AUTO(QColor, Grey120)
    Q_PROPERTY_AUTO(QColor, Grey130)
    Q_PROPERTY_AUTO(QColor, Grey140)
    Q_PROPERTY_AUTO(QColor, Grey150)
    Q_PROPERTY_AUTO(QColor, Grey160)
    Q_PROPERTY_AUTO(QColor, Grey170)
    Q_PROPERTY_AUTO(QColor, Grey180)
    Q_PROPERTY_AUTO(QColor, Grey190)
    Q_PROPERTY_AUTO(QColor, Grey200)
    Q_PROPERTY_AUTO(QColor, Grey210)
    Q_PROPERTY_AUTO(QColor, Grey220)
    Q_PROPERTY_AUTO_P(LingmoAccentColor*, Yellow)
    Q_PROPERTY_AUTO_P(LingmoAccentColor*, Orange)
    Q_PROPERTY_AUTO_P(LingmoAccentColor*, Red)
    Q_PROPERTY_AUTO_P(LingmoAccentColor*, Magenta)
    Q_PROPERTY_AUTO_P(LingmoAccentColor*, Purple)
    Q_PROPERTY_AUTO_P(LingmoAccentColor*, Blue)
    Q_PROPERTY_AUTO_P(LingmoAccentColor*, Teal)
    Q_PROPERTY_AUTO_P(LingmoAccentColor*, Green)
    QML_NAMED_ELEMENT(LingmoColor)
    QML_SINGLETON

private:
    explicit LingmoColor(QObject* parent = nullptr);

public:
    SINGLETON(LingmoColor)

    [[maybe_unused]] Q_INVOKABLE LingmoAccentColor* createAccentColor(const QColor& primaryColor);

    static LingmoColor* create(QQmlEngine*, QJSEngine*)
    {
        return getInstance();
    }

signals:
};
