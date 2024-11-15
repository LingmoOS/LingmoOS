#pragma once

#include "stdafx.h"

#include <QObject>
#include <QQmlEngine>
#include <qqml.h>

// Forward declaration of the C++ class
class LingmoLogger : public QObject {
    Q_OBJECT
    Q_PROPERTY_AUTO(QString, loggingTag)
    QML_NAMED_ELEMENT(LingmoLogger)

public:
    explicit LingmoLogger(QObject* parent = nullptr);

    ~LingmoLogger();

    Q_INVOKABLE void lAssert(const QString& msg);
    Q_INVOKABLE void lError(const QString& msg);
    Q_INVOKABLE void lWarn(const QString& msg);
    Q_INVOKABLE void lInfo(const QString& msg);
    Q_INVOKABLE void lDebug(const QString& msg);
    Q_INVOKABLE void lVerbose(const QString& msg);

    Q_INVOKABLE void lAseertTag(const QString& tag, const QString& msg);
    Q_INVOKABLE void lErrorTag(const QString& tag, const QString& msg);
    Q_INVOKABLE void lWarnTag(const QString& tag, const QString& msg);
    Q_INVOKABLE void lInfoTag(const QString& tag, const QString& msg);
    Q_INVOKABLE void lDebugTag(const QString& tag, const QString& msg);
    Q_INVOKABLE void lVerboseTag(const QString& tag, const QString& msg);
};