#include "LingmoHotkey.h"

#include <QGuiApplication>

LingmoHotkey::LingmoHotkey(QObject* parent)
    : QObject { parent }
{
    _sequence = "";
    _isRegistered = false;
    connect(this, &LingmoHotkey::sequenceChanged, this, [=] {
        if (_hotkey) {
            QObject::disconnect(_hotkey.get(), &QHotkey::activated, qApp, nullptr);
            QObject::disconnect(_hotkey.get(), &QHotkey::registeredChanged, qApp, nullptr);
            _hotkey->deleteLater();
            _hotkey.reset();
        }
        _hotkey = std::make_shared<QHotkey>(QKeySequence(_sequence), true, qApp);
        this->isRegistered(_hotkey->isRegistered());
        QObject::connect(_hotkey.get(), &QHotkey::activated, qApp, [=]() { Q_EMIT this->activated(); });
        QObject::connect(_hotkey.get(), &QHotkey::registeredChanged, qApp,
            [=]() { this->isRegistered(_hotkey->isRegistered()); });
    });
}