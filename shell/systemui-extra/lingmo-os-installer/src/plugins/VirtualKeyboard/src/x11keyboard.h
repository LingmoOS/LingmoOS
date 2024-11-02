/**
 * Copyright (C) 2018 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#ifndef X11KEYBOARD_H
#define X11KEYBOARD_H

#include <QObject>
#include <QMetaEnum>

class Modifier : public QObject
{
    Q_OBJECT
public:
    Modifier(){}

    enum MOD{
        UNKNOWN = -1,
        CTRL,
        SHIFT,
        ALT,
        SUPER
    };
    Q_ENUM(MOD)

    static QString getModifierName(int mod)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<Modifier::MOD>();
        const char* modName = metaEnum.valueToKey(mod);
        QString result = QString(modName).toLower();
        return result;
    }
    static MOD getModifier(const QString &modName)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<Modifier::MOD>();
        MOD mod = (MOD)metaEnum.keyToValue(modName.toUpper().toLocal8Bit().data());
        return mod;
    }
};

class FuncKey : public QObject
{
    Q_OBJECT
public:
    FuncKey(){}

    enum FUNCKEY {
        UNKNOWN = -1,
        SPACE = 0,
        BACKSPACE,
        ENTER,
        HOME,
        END,
        PGUP,
        PGDN,
        INSERT,
        DELETE,
        UP,
        DOWN,
        LEFT,
        RIGHT
    };
    Q_ENUM(FUNCKEY)
    static QString getKeyName(int key)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<FuncKey::FUNCKEY>();
        const char* keyName = metaEnum.valueToKey(key);
        QString result = QString(keyName).toLower();
        return result;
    }
    static FUNCKEY getKey(const QString &keyName)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<FuncKey::FUNCKEY>();
        FUNCKEY key = (FUNCKEY)metaEnum.keyToValue(keyName.toUpper().toLocal8Bit().data());
        return key;
    }
};

class X11Keyboard : public QObject
{
    Q_OBJECT
public:
    explicit X11Keyboard(QObject *parent = nullptr);
    ~X11Keyboard();
    void addModifier(Modifier::MOD mod);
    void removeModifier(Modifier::MOD mod);
    bool hasModifier(Modifier::MOD mod);
    QList<Modifier::MOD> getAllModifier();
    void clearModifier();

public Q_SLOTS:
    void onKeyPressed(QChar c);
    void onKeyPressed(FuncKey::FUNCKEY key);

private:
    void sendKey(unsigned int keyCode);

private:
    QList<Modifier::MOD> modList;
};

#endif // X11KEYBOARD_H
