// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ibustypes.h"

#include <QHash>

QT_BEGIN_NAMESPACE

IBusSerializable::IBusSerializable()
{
}

void IBusSerializable::deserializeFrom(const QDBusArgument &argument)
{
    argument >> name;

    argument.beginMap();
    while (!argument.atEnd()) {
        argument.beginMapEntry();
        QString key;
        QDBusVariant value;
        argument >> key;
        argument >> value;
        argument.endMapEntry();
        attachments[key] = qvariant_cast<QDBusArgument>(value.variant());
    }
    argument.endMap();
}

void IBusSerializable::serializeTo(QDBusArgument &argument) const
{
    argument << name;

    argument.beginMap(qMetaTypeId<QString>(), qMetaTypeId<QDBusVariant>());

    for (auto i = attachments.begin(), end = attachments.end(); i != end; ++i) {
        argument.beginMapEntry();
        argument << i.key();

        QDBusVariant variant(i.value().asVariant());

        argument << variant;
        argument.endMapEntry();
    }
    argument.endMap();
}

IBusAttribute::IBusAttribute()
    : type(Invalid),
      value(0),
      start(0),
      end(0)
{
    name = "IBusAttribute";
}

void IBusAttribute::serializeTo(QDBusArgument &argument) const
{
    argument.beginStructure();

    IBusSerializable::serializeTo(argument);

    quint32 t = (quint32) type;
    argument << t;
    argument << value;
    argument << start;
    argument << end;

    argument.endStructure();
}

void IBusAttribute::deserializeFrom(const QDBusArgument &argument)
{
    argument.beginStructure();

    IBusSerializable::deserializeFrom(argument);

    quint32 t;
    argument >> t;
    type = (IBusAttribute::Type) t;
    argument >> value;
    argument >> start;
    argument >> end;

    argument.endStructure();
}

QTextCharFormat IBusAttribute::format() const
{
    QTextCharFormat fmt;
    switch (type) {
    case Invalid:
        break;
    case Underline: {
        QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline;

        switch (value) {
        case UnderlineNone:
            break;
        case UnderlineSingle:
            style = QTextCharFormat::SingleUnderline;
            break;
        case UnderlineDouble:
            style = QTextCharFormat::DashUnderline;
            break;
        case UnderlineLow:
            style = QTextCharFormat::DashDotLine;
            break;
        case UnderlineError:
            style = QTextCharFormat::WaveUnderline;
            fmt.setUnderlineColor(Qt::red);
            break;
        }

        fmt.setUnderlineStyle(style);
        break;
    }
    case Foreground:
        fmt.setForeground(QColor(value));
        break;
    case Background:
        fmt.setBackground(QColor(value));
        break;
    }
    return fmt;
}

IBusAttributeList::IBusAttributeList()
{
    name = "IBusAttrList";
}

void IBusAttributeList::serializeTo(QDBusArgument &argument) const
{
    argument.beginStructure();

    IBusSerializable::serializeTo(argument);

    argument.beginArray(qMetaTypeId<QDBusVariant>());
    for (int i = 0; i < attributes.size(); ++i) {
        QVariant variant;
        variant.setValue(attributes.at(i));
        argument << QDBusVariant (variant);
    }
    argument.endArray();

    argument.endStructure();
}

void IBusAttributeList::deserializeFrom(const QDBusArgument &arg)
{
    arg.beginStructure();

    IBusSerializable::deserializeFrom(arg);

    arg.beginArray();
    while (!arg.atEnd()) {
        QDBusVariant var;
        arg >> var;

        IBusAttribute attr;
        qvariant_cast<QDBusArgument>(var.variant()) >> attr;
        attributes.append(std::move(attr));
    }
    arg.endArray();

    arg.endStructure();
}

QList<QInputMethodEvent::Attribute> IBusAttributeList::imAttributes() const
{
    QHash<QPair<int, int>, QTextCharFormat> rangeAttrs;
    const int numAttributes = attributes.size();

    // Merge text fomats for identical ranges into a single QTextFormat.
    for (int i = 0; i < numAttributes; ++i) {
        const IBusAttribute &attr = attributes.at(i);
        const QTextCharFormat &format = attr.format();

        if (format.isValid()) {
            const QPair<int, int> range(attr.start, attr.end);
            rangeAttrs[range].merge(format);
        }
    }

    // Assemble list in original attribute order.
    QList<QInputMethodEvent::Attribute> imAttrs;
    imAttrs.reserve(numAttributes);

    for (int i = 0; i < numAttributes; ++i) {
        const IBusAttribute &attr = attributes.at(i);
        const QTextFormat &format = attr.format();

        imAttrs += QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat,
            attr.start,
            attr.end - attr.start,
            format.isValid() ? rangeAttrs[QPair<int, int>(attr.start, attr.end)] : format);
    }

    return imAttrs;
}

IBusText::IBusText()
{
    name = "IBusText";
}

void IBusText::serializeTo(QDBusArgument &argument) const
{
    argument.beginStructure();

    IBusSerializable::serializeTo(argument);

    argument << text << attributes;
    argument.endStructure();
}

void IBusText::deserializeFrom(const QDBusArgument &argument)
{
    argument.beginStructure();

    IBusSerializable::deserializeFrom(argument);

    argument >> text;
    QDBusVariant variant;
    argument >> variant;
    qvariant_cast<QDBusArgument>(variant.variant()) >> attributes;

    argument.endStructure();
}

IBusEngineDesc::IBusEngineDesc()
    : rank(0)
{
    name = "IBusEngineDesc";
}

void IBusEngineDesc::serializeTo(QDBusArgument &argument) const
{
    argument.beginStructure();

    IBusSerializable::serializeTo(argument);

    argument << engine_name;
    argument << longname;
    argument << description;
    argument << language;
    argument << license;
    argument << author;
    argument << icon;
    argument << layout;
    argument << rank;
    argument << hotkeys;
    argument << symbol;
    argument << setup;
    argument << layout_variant;
    argument << layout_option;
    argument << version;
    argument << textdomain;
    argument << iconpropkey;

    argument.endStructure();
}

void IBusEngineDesc::deserializeFrom(const QDBusArgument &argument)
{
    argument.beginStructure();

    IBusSerializable::deserializeFrom(argument);

    argument >> engine_name;
    argument >> longname;
    argument >> description;
    argument >> language;
    argument >> license;
    argument >> author;
    argument >> icon;
    argument >> layout;
    argument >> rank;
    argument >> hotkeys;
    argument >> symbol;
    argument >> setup;
    // Previous IBusEngineDesc supports the arguments between engine_name
    // and setup.
    if (argument.currentSignature() == "")
        goto olderThanV2;
    argument >> layout_variant;
    argument >> layout_option;
    // Previous IBusEngineDesc supports the arguments between engine_name
    // and layout_option.
    if (argument.currentSignature() == "")
        goto olderThanV3;
    argument >> version;
    if (argument.currentSignature() == "")
        goto olderThanV4;
    argument >> textdomain;
    if (argument.currentSignature() == "")
        goto olderThanV5;
    argument >> iconpropkey;
    // <-- insert new member streaming here (1/2)
    goto newest;
olderThanV2:
    layout_variant.clear();
    layout_option.clear();
olderThanV3:
    version.clear();
olderThanV4:
    textdomain.clear();
olderThanV5:
    iconpropkey.clear();
    // <-- insert new members here (2/2)
newest:
    argument.endStructure();
}

QT_END_NAMESPACE
