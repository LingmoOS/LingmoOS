/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigloader.h"
#include "kconfigloader_p.h"
#include "kconfigloaderhandler_p.h"

#include <QColor>
#include <QFont>
#include <QHash>
#include <QUrl>

#include <QDebug>

void ConfigLoaderPrivate::parse(KConfigLoader *loader, QIODevice *xml)
{
    clearData();
    loader->clearItems();

    if (xml) {
        ConfigLoaderHandler handler(loader, this);
        handler.parse(xml);
    }
}

ConfigLoaderHandler::ConfigLoaderHandler(KConfigLoader *config, ConfigLoaderPrivate *d)
    : m_config(config)
    , d(d)
{
    resetState();
}

bool ConfigLoaderHandler::parse(QIODevice *input)
{
    if (!input->open(QIODevice::ReadOnly)) {
        qWarning() << "Impossible to open device";
        return false;
    }
    QXmlStreamReader reader(input);

    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.hasError()) {
            return false;
        }

        switch (reader.tokenType()) {
        case QXmlStreamReader::StartElement:
            startElement(reader.name(), reader.attributes());
            break;
        case QXmlStreamReader::EndElement:
            endElement(reader.name());
            break;
        case QXmlStreamReader::Characters:
            if (!reader.isWhitespace() && !reader.text().trimmed().isEmpty()) {
                m_cdata.append(reader.text());
            }
            break;
        default:
            break;
        }
    }

    if (!reader.isEndDocument()) {
        return false;
    }

    return true;
}

static bool caseInsensitiveCompare(const QStringView a, const QLatin1String b)
{
    return a.compare(b, Qt::CaseInsensitive) == 0;
}

void ConfigLoaderHandler::startElement(const QStringView localName, const QXmlStreamAttributes &attrs)
{
    // qDebug() << "ConfigLoaderHandler::startElement(" << localName << qName;
    if (caseInsensitiveCompare(localName, QLatin1String("group"))) {
        QString group;
        for (const auto &attr : attrs) {
            const auto attrName = attr.name();
            if (caseInsensitiveCompare(attrName, QLatin1String("name"))) {
                // qDebug() << "set group to" << attrs.value(i);
                group = attr.value().toString();
            }
        }
        if (group.isEmpty()) {
            group = d->baseGroup;
        } else {
            d->groups.append(group);
            if (!d->baseGroup.isEmpty()) {
                group = d->baseGroup + QLatin1Char('\x1d') + group;
            }
        }

        if (m_config) {
            m_config->setCurrentGroup(group);
        }
    } else if (caseInsensitiveCompare(localName, QLatin1String("entry"))) {
        for (const auto &attr : attrs) {
            const auto attrName = attr.name();
            if (caseInsensitiveCompare(attrName, QLatin1String("name"))) {
                m_name = attr.value().trimmed().toString();
            } else if (caseInsensitiveCompare(attrName, QLatin1String("type"))) {
                m_type = attr.value().toString().toLower();
            } else if (caseInsensitiveCompare(attrName, QLatin1String("key"))) {
                m_key = attr.value().trimmed().toString();
            }
        }
    } else if (caseInsensitiveCompare(localName, QLatin1String("choice"))) {
        m_choice.name.clear();
        m_choice.label.clear();
        m_choice.whatsThis.clear();
        for (const auto &attr : attrs) {
            const auto attrName = attr.name();
            if (caseInsensitiveCompare(attrName, QLatin1String("name"))) {
                m_choice.name = attr.value().toString();
            }
        }
        m_inChoice = true;
    }
}

void ConfigLoaderHandler::endElement(const QStringView localName)
{
    //     qDebug() << "ConfigLoaderHandler::endElement(" << localName << qName;
    if (caseInsensitiveCompare(localName, QLatin1String("entry"))) {
        addItem();
        resetState();
    } else if (caseInsensitiveCompare(localName, QLatin1String("label"))) {
        if (m_inChoice) {
            m_choice.label = std::move(m_cdata).trimmed();
        } else {
            m_label = std::move(m_cdata).trimmed();
        }
    } else if (caseInsensitiveCompare(localName, QLatin1String("whatsthis"))) {
        if (m_inChoice) {
            m_choice.whatsThis = std::move(m_cdata).trimmed();
        } else {
            m_whatsThis = std::move(m_cdata).trimmed();
        }
    } else if (caseInsensitiveCompare(localName, QLatin1String("default"))) {
        m_default = std::move(m_cdata).trimmed();
    } else if (caseInsensitiveCompare(localName, QLatin1String("min"))) {
        m_min = m_cdata.toInt(&m_haveMin);
    } else if (caseInsensitiveCompare(localName, QLatin1String("max"))) {
        m_max = m_cdata.toInt(&m_haveMax);
    } else if (caseInsensitiveCompare(localName, QLatin1String("choice"))) {
        m_enumChoices.append(m_choice);
        m_inChoice = false;
    }

    m_cdata.clear();
}

void ConfigLoaderHandler::addItem()
{
    if (m_name.isEmpty()) {
        if (m_key.isEmpty()) {
            return;
        }

        m_name = m_key;
    }

    m_name.remove(QLatin1Char(' '));

    KConfigSkeletonItem *item = nullptr;

    if (m_type == QLatin1String("bool")) {
        const bool defaultValue = caseInsensitiveCompare(m_default, QLatin1String("true"));
        item = m_config->addItemBool(m_name, *d->newBool(), defaultValue, m_key);
    } else if (m_type == QLatin1String("color")) {
        item = m_config->addItemColor(m_name, *d->newColor(), QColor(m_default), m_key);
    } else if (m_type == QLatin1String("datetime")) {
        item = m_config->addItemDateTime(m_name, *d->newDateTime(), QDateTime::fromString(m_default), m_key);
    } else if (m_type == QLatin1String("enum")) {
        m_key = (m_key.isEmpty()) ? m_name : m_key;

        bool ok = false;
        int defaultValue = m_default.toInt(&ok);
        if (!ok) {
            for (int i = 0; i < m_enumChoices.size(); i++) {
                if (m_default == m_enumChoices[i].name) {
                    defaultValue = i;
                    break;
                }
            }
        }

        KConfigSkeleton::ItemEnum *enumItem = new KConfigSkeleton::ItemEnum(m_config->currentGroup(), m_key, *d->newInt(), m_enumChoices, defaultValue);
        m_config->addItem(enumItem, m_name);
        item = enumItem;
    } else if (m_type == QLatin1String("font")) {
        item = m_config->addItemFont(m_name, *d->newFont(), QFont(m_default), m_key);
    } else if (m_type == QLatin1String("int")) {
        KConfigSkeleton::ItemInt *intItem = m_config->addItemInt(m_name, *d->newInt(), m_default.toInt(), m_key);

        if (m_haveMin) {
            intItem->setMinValue(m_min);
        }

        if (m_haveMax) {
            intItem->setMaxValue(m_max);
        }

        item = intItem;
    } else if (m_type == QLatin1String("password")) {
        item = m_config->addItemPassword(m_name, *d->newString(), m_default, m_key);
    } else if (m_type == QLatin1String("path")) {
        item = m_config->addItemPath(m_name, *d->newString(), m_default, m_key);
    } else if (m_type == QLatin1String("string")) {
        item = m_config->addItemString(m_name, *d->newString(), m_default, m_key);
    } else if (m_type == QLatin1String("stringlist")) {
        // FIXME: the split() is naive and will break on lists with ,'s in them
        // empty parts are not wanted in this case
        item = m_config->addItemStringList(m_name, *d->newStringList(), m_default.split(QLatin1Char(','), Qt::SkipEmptyParts), m_key);
    } else if (m_type == QLatin1String("uint")) {
        KConfigSkeleton::ItemUInt *uintItem = m_config->addItemUInt(m_name, *d->newUint(), m_default.toUInt(), m_key);
        if (m_haveMin) {
            uintItem->setMinValue(m_min);
        }
        if (m_haveMax) {
            uintItem->setMaxValue(m_max);
        }
        item = uintItem;
    } else if (m_type == QLatin1String("url")) {
        m_key = (m_key.isEmpty()) ? m_name : m_key;
        KConfigSkeleton::ItemUrl *urlItem = new KConfigSkeleton::ItemUrl(m_config->currentGroup(), m_key, *d->newUrl(), QUrl::fromUserInput(m_default));
        m_config->addItem(urlItem, m_name);
        item = urlItem;
    } else if (m_type == QLatin1String("double")) {
        KConfigSkeleton::ItemDouble *doubleItem = m_config->addItemDouble(m_name, *d->newDouble(), m_default.toDouble(), m_key);
        if (m_haveMin) {
            doubleItem->setMinValue(m_min);
        }
        if (m_haveMax) {
            doubleItem->setMaxValue(m_max);
        }
        item = doubleItem;
    } else if (m_type == QLatin1String("intlist")) {
        QList<int> defaultList;
        const QList<QStringView> tmpList = QStringView(m_default).split(QLatin1Char(','), Qt::SkipEmptyParts);
        for (const QStringView tmp : tmpList) {
            defaultList.append(tmp.toInt());
        }
        item = m_config->addItemIntList(m_name, *d->newIntList(), defaultList, m_key);
    } else if (m_type == QLatin1String("longlong")) {
        KConfigSkeleton::ItemLongLong *longlongItem = m_config->addItemLongLong(m_name, *d->newLongLong(), m_default.toLongLong(), m_key);
        if (m_haveMin) {
            longlongItem->setMinValue(m_min);
        }
        if (m_haveMax) {
            longlongItem->setMaxValue(m_max);
        }
        item = longlongItem;
        /* No addItemPathList in KConfigSkeleton ?
        } else if (m_type == "PathList") {
            //FIXME: the split() is naive and will break on lists with ,'s in them
            item = m_config->addItemPathList(m_name, *d->newStringList(), m_default.split(","), m_key);
        */
    } else if (m_type == QLatin1String("point")) {
        QPoint defaultPoint;
        const QList<QStringView> tmpList = QStringView(m_default).split(QLatin1Char(','));
        if (tmpList.size() >= 2) {
            defaultPoint.setX(tmpList[0].toInt());
            defaultPoint.setY(tmpList[1].toInt());
        }
        item = m_config->addItemPoint(m_name, *d->newPoint(), defaultPoint, m_key);
    } else if (m_type == QLatin1String("pointf")) {
        QPointF defaultPointF;
        const auto tmpList = QStringView(m_default).split(u',');
        if (tmpList.size() >= 2) {
            defaultPointF.setX(tmpList[0].toDouble());
            defaultPointF.setY(tmpList[1].toDouble());
        }
        item = m_config->addItemPointF(m_name, *d->newPointF(), defaultPointF, m_key);
    } else if (m_type == QLatin1String("rect")) {
        QRect defaultRect;
        const QList<QStringView> tmpList = QStringView(m_default).split(QLatin1Char(','));
        if (tmpList.size() >= 4) {
            defaultRect.setCoords(tmpList[0].toInt(), tmpList[1].toInt(), tmpList[2].toInt(), tmpList[3].toInt());
        }
        item = m_config->addItemRect(m_name, *d->newRect(), defaultRect, m_key);
    } else if (m_type == QLatin1String("rectf")) {
        QRectF defaultRectF;
        const auto tmpList = QStringView(m_default).split(u',');
        if (tmpList.size() >= 4) {
            defaultRectF.setCoords(tmpList[0].toDouble(), tmpList[1].toDouble(), tmpList[2].toDouble(), tmpList[3].toDouble());
        }
        item = m_config->addItemRectF(m_name, *d->newRectF(), defaultRectF, m_key);
    } else if (m_type == QLatin1String("size")) {
        QSize defaultSize;
        const QList<QStringView> tmpList = QStringView(m_default).split(QLatin1Char(','));
        if (tmpList.size() >= 2) {
            defaultSize.setWidth(tmpList[0].toInt());
            defaultSize.setHeight(tmpList[1].toInt());
        }
        item = m_config->addItemSize(m_name, *d->newSize(), defaultSize, m_key);
    } else if (m_type == QLatin1String("sizef")) {
        QSizeF defaultSizeF;
        const auto tmpList = QStringView(m_default).split(u',');
        if (tmpList.size() >= 2) {
            defaultSizeF.setWidth(tmpList[0].toDouble());
            defaultSizeF.setHeight(tmpList[1].toDouble());
        }
        item = m_config->addItemSizeF(m_name, *d->newSizeF(), defaultSizeF, m_key);
    } else if (m_type == QLatin1String("ulonglong")) {
        KConfigSkeleton::ItemULongLong *ulonglongItem = m_config->addItemULongLong(m_name, *d->newULongLong(), m_default.toULongLong(), m_key);
        if (m_haveMin) {
            ulonglongItem->setMinValue(m_min);
        }
        if (m_haveMax) {
            ulonglongItem->setMaxValue(m_max);
        }
        item = ulonglongItem;
        /* No addItemUrlList in KConfigSkeleton ?
        } else if (m_type == "urllist") {
            //FIXME: the split() is naive and will break on lists with ,'s in them
            QStringList tmpList = m_default.split(",");
            QList<QUrl> defaultList;
            foreach (const QString& tmp, tmpList) {
                defaultList.append(QUrl(tmp));
            }
            item = m_config->addItemUrlList(m_name, *d->newUrlList(), defaultList, m_key);*/
    }

    if (item) {
        item->setLabel(m_label);
        item->setWhatsThis(m_whatsThis);
        d->keysToNames.insert(item->group() + item->key(), item->name());
    }
}

void ConfigLoaderHandler::resetState()
{
    m_haveMin = false;
    m_min = 0;
    m_haveMax = false;
    m_max = 0;
    m_name.clear();
    m_type.clear();
    m_label.clear();
    m_default.clear();
    m_key.clear();
    m_whatsThis.clear();
    m_enumChoices.clear();
    m_inChoice = false;
}

KConfigLoader::KConfigLoader(const QString &configFile, QIODevice *xml, QObject *parent)
    : KConfigSkeleton(configFile, parent)
    , d(new ConfigLoaderPrivate)
{
    d->parse(this, xml);
}

KConfigLoader::KConfigLoader(KSharedConfigPtr config, QIODevice *xml, QObject *parent)
    : KConfigSkeleton(std::move(config), parent)
    , d(new ConfigLoaderPrivate)
{
    d->parse(this, xml);
}

// FIXME: obviously this is broken and should be using the group as the root,
//       but KConfigSkeleton does not currently support this. it will eventually though,
//       at which point this can be addressed properly
KConfigLoader::KConfigLoader(const KConfigGroup &config, QIODevice *xml, QObject *parent)
    : KConfigSkeleton(KSharedConfig::openConfig(config.config()->name(), config.config()->openFlags(), config.config()->locationType()), parent)
    , d(new ConfigLoaderPrivate)
{
    KConfigGroup group = config.parent();
    d->baseGroup = config.name();
    while (group.isValid() && group.name() != QLatin1String("<default>")) {
        d->baseGroup = group.name() + QLatin1Char('\x1d') + d->baseGroup;
        group = group.parent();
    }
    d->parse(this, xml);
}

KConfigLoader::~KConfigLoader()
{
    delete d;
}

KConfigSkeletonItem *KConfigLoader::findItem(const QString &group, const QString &key) const
{
    return KConfigSkeleton::findItem(d->keysToNames[group + key]);
}

KConfigSkeletonItem *KConfigLoader::findItemByName(const QString &name) const
{
    return KConfigSkeleton::findItem(name);
}

QVariant KConfigLoader::property(const QString &name) const
{
    KConfigSkeletonItem *item = KConfigSkeleton::findItem(name);

    if (item) {
        return item->property();
    }

    return QVariant();
}

bool KConfigLoader::hasGroup(const QString &group) const
{
    return d->groups.contains(group);
}

QStringList KConfigLoader::groupList() const
{
    return d->groups;
}

bool KConfigLoader::usrSave()
{
    if (d->saveDefaults) {
        const auto listItems = items();
        for (const auto &item : listItems) {
            config()->group(item->group()).writeEntry(item->key(), "");
        }
    }
    return true;
}
