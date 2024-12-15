// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _DIM_IBUSTYPES_H
#define _DIM_IBUSTYPES_H

#include <QDBusArgument>
#include <QInputMethodEvent>
#include <QList>
#include <QTextCharFormat>

QT_BEGIN_NAMESPACE

class IBusSerializable
{
public:
    IBusSerializable();

    void serializeTo(QDBusArgument &argument) const;
    void deserializeFrom(const QDBusArgument &argument);

    QString name;
    QHash<QString, QDBusArgument> attachments;
};

class IBusAttribute : private IBusSerializable
{
public:
    enum Type {
        Invalid = 0,
        Underline = 1,
        Foreground = 2,
        Background = 3,
    };

    enum Underline {
        UnderlineNone = 0,
        UnderlineSingle = 1,
        UnderlineDouble = 2,
        UnderlineLow = 3,
        UnderlineError = 4,
    };

    IBusAttribute();

    QTextCharFormat format() const;

    void serializeTo(QDBusArgument &argument) const;
    void deserializeFrom(const QDBusArgument &argument);

    Type type;
    quint32 value;
    quint32 start;
    quint32 end;
};

Q_DECLARE_TYPEINFO(IBusAttribute, Q_RELOCATABLE_TYPE);

class IBusAttributeList : private IBusSerializable
{
public:
    IBusAttributeList();

    QList<QInputMethodEvent::Attribute> imAttributes() const;

    void serializeTo(QDBusArgument &argument) const;
    void deserializeFrom(const QDBusArgument &argument);

    QList<IBusAttribute> attributes;
};

Q_DECLARE_TYPEINFO(IBusAttributeList, Q_RELOCATABLE_TYPE);

class IBusText : private IBusSerializable
{
public:
    IBusText();

    void serializeTo(QDBusArgument &argument) const;
    void deserializeFrom(const QDBusArgument &argument);

    QString text;
    IBusAttributeList attributes;
};

Q_DECLARE_TYPEINFO(IBusText, Q_RELOCATABLE_TYPE);

class IBusEngineDesc : private IBusSerializable
{
public:
    IBusEngineDesc();

    void serializeTo(QDBusArgument &argument) const;
    void deserializeFrom(const QDBusArgument &argument);

    QString engine_name;
    QString longname;
    QString description;
    QString language;
    QString license;
    QString author;
    QString icon;
    QString layout;
    unsigned int rank;
    QString hotkeys;
    QString symbol;
    QString setup;
    QString layout_variant;
    QString layout_option;
    QString version;
    QString textdomain;
    QString iconpropkey;
};

Q_DECLARE_TYPEINFO(IBusEngineDesc, Q_RELOCATABLE_TYPE);

inline QDBusArgument &operator<<(QDBusArgument &argument, const IBusAttribute &attribute)
{
    attribute.serializeTo(argument);
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, IBusAttribute &attribute)
{
    attribute.deserializeFrom(argument);
    return argument;
}

inline QDBusArgument &operator<<(QDBusArgument &argument, const IBusAttributeList &attributeList)
{
    attributeList.serializeTo(argument);
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument,
                                       IBusAttributeList &attributeList)
{
    attributeList.deserializeFrom(argument);
    return argument;
}

inline QDBusArgument &operator<<(QDBusArgument &argument, const IBusText &text)
{
    text.serializeTo(argument);
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, IBusText &text)
{
    text.deserializeFrom(argument);
    return argument;
}

inline QDBusArgument &operator<<(QDBusArgument &argument, const IBusEngineDesc &desc)
{
    desc.serializeTo(argument);
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, IBusEngineDesc &desc)
{
    desc.deserializeFrom(argument);
    return argument;
}

QT_END_NAMESPACE

Q_DECLARE_METATYPE(IBusAttribute)
Q_DECLARE_METATYPE(IBusAttributeList)
Q_DECLARE_METATYPE(IBusText)
Q_DECLARE_METATYPE(IBusEngineDesc)

#endif
