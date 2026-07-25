#ifndef LINGMOAPP_H
#define LINGMOAPP_H

#include <QObject>
#include <QQmlEngine>
#include <QJsonArray>
#include <QTranslator>

#include "singleton.h"
#include "stdafx.h"

#include "LingmoIconDef.h"

/**
 * @brief The LingmoApp class. Used in all apps.
 * @since LingmoUI 3.0
 */
class LingmoApp : public QObject {
    Q_OBJECT
    QML_SINGLETON
    QML_NAMED_ELEMENT(LingmoApp)

    Q_PROPERTY_AUTO(QString, windowIcon)
    Q_PROPERTY_AUTO(QLocale, locale)
    /**
     * @brief launcher points to the object by which LingmoApp is initialized.
     */
    Q_PROPERTY_AUTO_P(QObject*, launcher)
    Q_PROPERTY_AUTO(bool, useSystemAppBar)

private:
    explicit LingmoApp(QObject* parent = nullptr);

    ~LingmoApp() override = default;

public:
    SINGLETON(LingmoApp)

    static LingmoApp* create(QQmlEngine*, QJSEngine*) { return getInstance(); }

    Q_INVOKABLE void init(QObject* launcher, QLocale locale = QLocale::system());

    [[maybe_unused]] Q_INVOKABLE static QJsonArray iconData(const QString &keyword = "");

private:
    QQmlEngine* _engine {};
    QTranslator* _translator = nullptr;
};

#endif // LINGMOAPP_H
