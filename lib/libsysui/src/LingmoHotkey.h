#pragma once

#include "QHotkey"
#include "stdafx.h"

#include <QObject>
#include <QQuickItem>

#include <memory>


class LingmoHotkey : public QObject {

    Q_OBJECT
    Q_PROPERTY_AUTO(QString, sequence)
    Q_PROPERTY_AUTO(QString, name)
    Q_PROPERTY_READONLY_AUTO(bool, isRegistered)
    QML_NAMED_ELEMENT(LingmoHotkey)

public:
    explicit LingmoHotkey(QObject* parent = nullptr);
    ~LingmoHotkey() = default;
    
    Q_SIGNAL void activated();

private:
    std::shared_ptr<QHotkey> _hotkey;
};
