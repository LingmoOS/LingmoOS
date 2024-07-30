/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Benjamin C Meyer <ben+kdelibs at meyerhome dot net>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2020 Kevin Ottens <kevin.ottens@enioka.com>
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigdialogmanager.h"
#include "kconfigdialogmanager_p.h"
#include "kconfigwidgets_debug.h"

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QMetaObject>
#include <QMetaProperty>
#include <QRadioButton>
#include <QTimer>

#include <KConfigSkeleton>

typedef QHash<QString, QByteArray> MyHash;
Q_GLOBAL_STATIC(MyHash, s_propertyMap)
Q_GLOBAL_STATIC(MyHash, s_changedMap)

KConfigDialogManager::KConfigDialogManager(QWidget *parent, KCoreConfigSkeleton *conf)
    : QObject(parent)
    , d(new KConfigDialogManagerPrivate(this))
{
    d->m_conf = conf;
    d->m_dialog = parent;
    init(true);
}

KConfigDialogManager::~KConfigDialogManager() = default;

// KF6: Drop this and get signals only from metaObject and/or widget's dynamic properties kcfg_property/kcfg_propertyNotify
void KConfigDialogManager::initMaps()
{
    if (s_propertyMap()->isEmpty()) {
        s_propertyMap()->insert(QStringLiteral("KButtonGroup"), "current");
        s_propertyMap()->insert(QStringLiteral("KColorButton"), "color");
        s_propertyMap()->insert(QStringLiteral("KColorCombo"), "color");
        s_propertyMap()->insert(QStringLiteral("KKeySequenceWidget"), "keySequence");
    }

    if (s_changedMap()->isEmpty()) {
        // Qt
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
        s_changedMap()->insert(QStringLiteral("QCheckBox"), SIGNAL(checkStateChanged(Qt::CheckState)));
#else
        s_changedMap()->insert(QStringLiteral("QCheckBox"), SIGNAL(stateChanged(int)));
#endif
        s_changedMap()->insert(QStringLiteral("QPushButton"), SIGNAL(clicked(bool)));
        s_changedMap()->insert(QStringLiteral("QRadioButton"), SIGNAL(toggled(bool)));
        s_changedMap()->insert(QStringLiteral("QGroupBox"), SIGNAL(toggled(bool)));
        s_changedMap()->insert(QStringLiteral("QComboBox"), SIGNAL(activated(int)));
        s_changedMap()->insert(QStringLiteral("QDateEdit"), SIGNAL(dateChanged(QDate)));
        s_changedMap()->insert(QStringLiteral("QTimeEdit"), SIGNAL(timeChanged(QTime)));
        s_changedMap()->insert(QStringLiteral("QDateTimeEdit"), SIGNAL(dateTimeChanged(QDateTime)));
        s_changedMap()->insert(QStringLiteral("QDial"), SIGNAL(valueChanged(int)));
        s_changedMap()->insert(QStringLiteral("QDoubleSpinBox"), SIGNAL(valueChanged(double)));
        s_changedMap()->insert(QStringLiteral("QLineEdit"), SIGNAL(textChanged(QString)));
        s_changedMap()->insert(QStringLiteral("QSlider"), SIGNAL(valueChanged(int)));
        s_changedMap()->insert(QStringLiteral("QSpinBox"), SIGNAL(valueChanged(int)));
        s_changedMap()->insert(QStringLiteral("QTextEdit"), SIGNAL(textChanged()));
        s_changedMap()->insert(QStringLiteral("QTextBrowser"), SIGNAL(sourceChanged(QString)));
        s_changedMap()->insert(QStringLiteral("QPlainTextEdit"), SIGNAL(textChanged()));
        s_changedMap()->insert(QStringLiteral("QTabWidget"), SIGNAL(currentChanged(int)));

        // KDE
        s_changedMap()->insert(QStringLiteral("KComboBox"), SIGNAL(activated(int)));
        s_changedMap()->insert(QStringLiteral("KFontComboBox"), SIGNAL(activated(int)));
        s_changedMap()->insert(QStringLiteral("KFontRequester"), SIGNAL(fontSelected(QFont)));
        s_changedMap()->insert(QStringLiteral("KFontChooser"), SIGNAL(fontSelected(QFont)));
        s_changedMap()->insert(QStringLiteral("KColorCombo"), SIGNAL(activated(QColor)));
        s_changedMap()->insert(QStringLiteral("KColorButton"), SIGNAL(changed(QColor)));
        s_changedMap()->insert(QStringLiteral("KDatePicker"), SIGNAL(dateSelected(QDate)));
        s_changedMap()->insert(QStringLiteral("KDateWidget"), SIGNAL(changed(QDate)));
        s_changedMap()->insert(QStringLiteral("KDateTimeWidget"), SIGNAL(valueChanged(QDateTime)));
        s_changedMap()->insert(QStringLiteral("KEditListWidget"), SIGNAL(changed()));
        s_changedMap()->insert(QStringLiteral("KListWidget"), SIGNAL(itemSelectionChanged()));
        s_changedMap()->insert(QStringLiteral("KLineEdit"), SIGNAL(textChanged(QString)));
        s_changedMap()->insert(QStringLiteral("KRestrictedLine"), SIGNAL(textChanged(QString)));
        s_changedMap()->insert(QStringLiteral("KTextEdit"), SIGNAL(textChanged()));
        s_changedMap()->insert(QStringLiteral("KUrlRequester"), SIGNAL(textChanged(QString)));
        s_changedMap()->insert(QStringLiteral("KUrlComboRequester"), SIGNAL(textChanged(QString)));
        s_changedMap()->insert(QStringLiteral("KUrlComboBox"), SIGNAL(urlActivated(QUrl)));
        s_changedMap()->insert(QStringLiteral("KButtonGroup"), SIGNAL(changed(int)));
    }
}

QHash<QString, QByteArray> *KConfigDialogManager::propertyMap()
{
    initMaps();
    return s_propertyMap();
}

void KConfigDialogManager::init(bool trackChanges)
{
    initMaps();
    d->trackChanges = trackChanges;

    // Go through all of the children of the widgets and find all known widgets
    (void)parseChildren(d->m_dialog, trackChanges);
}

void KConfigDialogManager::addWidget(QWidget *widget)
{
    (void)parseChildren(widget, true);
}

void KConfigDialogManager::setupWidget(QWidget *widget, KConfigSkeletonItem *item)
{
    QVariant minValue = item->minValue();
    if (minValue.isValid()) {
        // KSelector is using this property
        if (widget->metaObject()->indexOfProperty("minValue") != -1) {
            widget->setProperty("minValue", minValue);
        }
        if (widget->metaObject()->indexOfProperty("minimum") != -1) {
            widget->setProperty("minimum", minValue);
        }
    }
    QVariant maxValue = item->maxValue();
    if (maxValue.isValid()) {
        // KSelector is using this property
        if (widget->metaObject()->indexOfProperty("maxValue") != -1) {
            widget->setProperty("maxValue", maxValue);
        }
        if (widget->metaObject()->indexOfProperty("maximum") != -1) {
            widget->setProperty("maximum", maxValue);
        }
    }

    if (widget->whatsThis().isEmpty()) {
        QString whatsThis = item->whatsThis();
        if (!whatsThis.isEmpty()) {
            widget->setWhatsThis(whatsThis);
        }
    }

    if (widget->toolTip().isEmpty()) {
        QString toolTip = item->toolTip();
        if (!toolTip.isEmpty()) {
            widget->setToolTip(toolTip);
        }
    }

    // If it is a QGroupBox with only autoExclusive buttons
    // and has no custom property and the config item type
    // is an integer, assume we want to save the index like we did with
    // KButtonGroup instead of if it is checked or not
    QGroupBox *gb = qobject_cast<QGroupBox *>(widget);
    if (gb && getCustomProperty(gb).isEmpty()) {
        const KConfigSkeletonItem *item = d->m_conf->findItem(widget->objectName().mid(5));
        if (item->property().userType() == QMetaType::Int) {
            QObjectList children = gb->children();
            children.removeAll(gb->layout());
            const QList<QAbstractButton *> buttons = gb->findChildren<QAbstractButton *>();
            bool allAutoExclusiveDirectChildren = true;
            for (QAbstractButton *button : buttons) {
                allAutoExclusiveDirectChildren = allAutoExclusiveDirectChildren && button->autoExclusive() && button->parent() == gb;
            }
            if (allAutoExclusiveDirectChildren) {
                d->allExclusiveGroupBoxes << widget;
            }
        }
    }

    if (!item->isEqual(property(widget))) {
        setProperty(widget, item->property());
    }

    d->updateWidgetIndicator(item->name(), widget);
}

bool KConfigDialogManager::parseChildren(const QWidget *widget, bool trackChanges)
{
    bool valueChanged = false;
    const QList<QObject *> listOfChildren = widget->children();
    if (listOfChildren.isEmpty()) { //?? XXX
        return valueChanged;
    }

    const QMetaMethod onWidgetModifiedSlot = metaObject()->method(metaObject()->indexOfSlot("onWidgetModified()"));
    Q_ASSERT(onWidgetModifiedSlot.isValid() && metaObject()->indexOfSlot("onWidgetModified()") >= 0);

    for (QObject *object : listOfChildren) {
        if (!object->isWidgetType()) {
            continue; // Skip non-widgets
        }

        QWidget *childWidget = static_cast<QWidget *>(object);

        QString widgetName = childWidget->objectName();
        bool bParseChildren = true;
        bool bSaveInsideGroupBox = d->insideGroupBox;

        if (widgetName.startsWith(QLatin1String("kcfg_"))) {
            // This is one of our widgets!
            QString configId = widgetName.mid(5);
            KConfigSkeletonItem *item = d->m_conf->findItem(configId);
            if (item) {
                d->knownWidget.insert(configId, childWidget);

                setupWidget(childWidget, item);

                if (trackChanges) {
                    bool changeSignalFound = false;

                    if (d->allExclusiveGroupBoxes.contains(childWidget)) {
                        const QList<QAbstractButton *> buttons = childWidget->findChildren<QAbstractButton *>();
                        for (QAbstractButton *button : buttons) {
                            connect(button, &QAbstractButton::toggled, this, [this] {
                                d->onWidgetModified();
                            });
                        }
                    }

                    QByteArray propertyChangeSignal = getCustomPropertyChangedSignal(childWidget);
                    if (propertyChangeSignal.isEmpty()) {
                        propertyChangeSignal = getUserPropertyChangedSignal(childWidget);
                    }

                    if (propertyChangeSignal.isEmpty()) {
                        // get the change signal from the meta object
                        const QMetaObject *metaObject = childWidget->metaObject();
                        QByteArray userproperty = getCustomProperty(childWidget);
                        if (userproperty.isEmpty()) {
                            userproperty = getUserProperty(childWidget);
                        }
                        if (!userproperty.isEmpty()) {
                            const int indexOfProperty = metaObject->indexOfProperty(userproperty.constData());
                            if (indexOfProperty != -1) {
                                const QMetaProperty property = metaObject->property(indexOfProperty);
                                const QMetaMethod notifySignal = property.notifySignal();
                                if (notifySignal.isValid()) {
                                    connect(childWidget, notifySignal, this, onWidgetModifiedSlot);
                                    changeSignalFound = true;
                                }
                            }
                        } else {
                            qCWarning(KCONFIG_WIDGETS_LOG) << "Don't know how to monitor widget" << childWidget->metaObject()->className() << "for changes!";
                        }
                    } else {
                        connect(childWidget, propertyChangeSignal.constData(), this, SLOT(onWidgetModified()));
                        changeSignalFound = true;
                    }

                    if (changeSignalFound) {
                        QComboBox *cb = qobject_cast<QComboBox *>(childWidget);
                        if (cb && cb->isEditable()) {
                            connect(cb, &QComboBox::editTextChanged, this, &KConfigDialogManager::widgetModified);
                        }
                    }
                }
                QGroupBox *gb = qobject_cast<QGroupBox *>(childWidget);
                if (!gb) {
                    bParseChildren = false;
                } else {
                    d->insideGroupBox = true;
                }
            } else {
                qCWarning(KCONFIG_WIDGETS_LOG) << "A widget named" << widgetName << "was found but there is no setting named" << configId;
            }
        } else if (QLabel *label = qobject_cast<QLabel *>(childWidget)) {
            QWidget *buddy = label->buddy();
            if (!buddy) {
                continue;
            }
            QString buddyName = buddy->objectName();
            if (buddyName.startsWith(QLatin1String("kcfg_"))) {
                // This is one of our widgets!
                QString configId = buddyName.mid(5);
                d->buddyWidget.insert(configId, childWidget);
            }
        }
        // kf5: commented out to reduce debug output
        // #ifndef NDEBUG
        //     else if (!widgetName.isEmpty() && trackChanges)
        //     {
        //       QHash<QString, QByteArray>::const_iterator changedIt = s_changedMap()->constFind(childWidget->metaObject()->className());
        //       if (changedIt != s_changedMap()->constEnd())
        //       {
        //         if ((!d->insideGroupBox || !qobject_cast<QRadioButton*>(childWidget)) &&
        //             !qobject_cast<QGroupBox*>(childWidget) &&!qobject_cast<QTabWidget*>(childWidget) )
        //           qCDebug(KCONFIG_WIDGETS_LOG) << "Widget '" << widgetName << "' (" << childWidget->metaObject()->className() << ") remains unmanaged.";
        //       }
        //     }
        // #endif

        if (bParseChildren) {
            // this widget is not known as something we can store.
            // Maybe we can store one of its children.
            valueChanged |= parseChildren(childWidget, trackChanges);
        }
        d->insideGroupBox = bSaveInsideGroupBox;
    }
    return valueChanged;
}

void KConfigDialogManager::updateWidgets()
{
    bool changed = false;
    bool bSignalsBlocked = signalsBlocked();
    blockSignals(true);

    QWidget *widget;
    QHashIterator<QString, QWidget *> it(d->knownWidget);
    while (it.hasNext()) {
        it.next();
        widget = it.value();

        KConfigSkeletonItem *item = d->m_conf->findItem(it.key());
        if (!item) {
            qCWarning(KCONFIG_WIDGETS_LOG) << "The setting" << it.key() << "has disappeared!";
            continue;
        }

        if (!item->isEqual(property(widget))) {
            setProperty(widget, item->property());
            // qCDebug(KCONFIG_WIDGETS_LOG) << "The setting" << it.key() << "[" << widget->className() << "] has changed";
            changed = true;
        }
        if (item->isImmutable()) {
            widget->setEnabled(false);
            QWidget *buddy = d->buddyWidget.value(it.key(), nullptr);
            if (buddy) {
                buddy->setEnabled(false);
            }
        }
    }
    blockSignals(bSignalsBlocked);

    if (changed) {
        QTimer::singleShot(0, this, &KConfigDialogManager::widgetModified);
        d->updateAllWidgetIndicators();
    }
}

void KConfigDialogManager::updateWidgetsDefault()
{
    bool bUseDefaults = d->m_conf->useDefaults(true);
    updateWidgets();
    d->m_conf->useDefaults(bUseDefaults);
    d->updateAllWidgetIndicators();
}

void KConfigDialogManager::setDefaultsIndicatorsVisible(bool enabled)
{
    d->setDefaultsIndicatorsVisible(enabled);
}

void KConfigDialogManager::updateSettings()
{
    bool changed = false;

    QWidget *widget;
    QHashIterator<QString, QWidget *> it(d->knownWidget);
    while (it.hasNext()) {
        it.next();
        widget = it.value();

        KConfigSkeletonItem *item = d->m_conf->findItem(it.key());
        if (!item) {
            qCWarning(KCONFIG_WIDGETS_LOG) << "The setting" << it.key() << "has disappeared!";
            continue;
        }

        QVariant fromWidget = property(widget);
        if (!item->isEqual(fromWidget)) {
            item->setProperty(fromWidget);
            changed = true;
        }
    }
    if (changed) {
        d->m_conf->save();
        Q_EMIT settingsChanged();
        d->updateAllWidgetIndicators();
    }
}

QByteArray KConfigDialogManager::getUserProperty(const QWidget *widget) const
{
    MyHash *map = s_propertyMap();
    const QMetaObject *metaObject = widget->metaObject();
    const QString className(QLatin1String(metaObject->className()));
    auto it = map->find(className);
    if (it == map->end()) {
        const QMetaProperty userProp = metaObject->userProperty();
        if (userProp.isValid()) {
            it = map->insert(className, userProp.name());
            // qCDebug(KCONFIG_WIDGETS_LOG) << "class name: '" << className
            //<< " 's USER property: " << metaProperty.name() << endl;
        } else {
            return QByteArray(); // no USER property
        }
    }

    const QComboBox *cb = qobject_cast<const QComboBox *>(widget);
    if (cb) {
        const char *qcomboUserPropertyName = cb->QComboBox::metaObject()->userProperty().name();
        const int qcomboUserPropertyIndex = qcomboUserPropertyName ? cb->QComboBox::metaObject()->indexOfProperty(qcomboUserPropertyName) : -1;
        const char *widgetUserPropertyName = metaObject->userProperty().name();
        const int widgetUserPropertyIndex = widgetUserPropertyName ? cb->metaObject()->indexOfProperty(widgetUserPropertyName) : -1;

        // no custom user property set on subclass of QComboBox?
        if (qcomboUserPropertyIndex == widgetUserPropertyIndex) {
            return QByteArray(); // use the q/kcombobox special code
        }
    }

    return it != map->end() ? it.value() : QByteArray{};
}

QByteArray KConfigDialogManager::getCustomProperty(const QWidget *widget) const
{
    QVariant prop(widget->property("kcfg_property"));
    if (prop.isValid()) {
        if (!prop.canConvert<QByteArray>()) {
            qCWarning(KCONFIG_WIDGETS_LOG) << "kcfg_property on" << widget->metaObject()->className() << "is not of type ByteArray";
        } else {
            return prop.toByteArray();
        }
    }
    return QByteArray();
}

QByteArray KConfigDialogManager::getUserPropertyChangedSignal(const QWidget *widget) const
{
    const QString className = QLatin1String(widget->metaObject()->className());
    auto changedIt = s_changedMap()->constFind(className);

    if (changedIt == s_changedMap()->constEnd()) {
        // If the class name of the widget wasn't in the monitored widgets map, then look for
        // it again using the super class name. This fixes a problem with using QtRuby/Korundum
        // widgets with KConfigXT where 'Qt::Widget' wasn't being seen a the real deal, even
        // though it was a 'QWidget'.
        if (widget->metaObject()->superClass()) {
            const QString parentClassName = QLatin1String(widget->metaObject()->superClass()->className());
            changedIt = s_changedMap()->constFind(parentClassName);
        }
    }

    return (changedIt == s_changedMap()->constEnd()) ? QByteArray() : *changedIt;
}

QByteArray KConfigDialogManager::getCustomPropertyChangedSignal(const QWidget *widget) const
{
    QVariant prop(widget->property("kcfg_propertyNotify"));
    if (prop.isValid()) {
        if (!prop.canConvert<QByteArray>()) {
            qCWarning(KCONFIG_WIDGETS_LOG) << "kcfg_propertyNotify on" << widget->metaObject()->className() << "is not of type ByteArray";
        } else {
            return prop.toByteArray();
        }
    }
    return QByteArray();
}

void KConfigDialogManager::setProperty(QWidget *w, const QVariant &v)
{
    if (d->allExclusiveGroupBoxes.contains(w)) {
        const QList<QAbstractButton *> buttons = w->findChildren<QAbstractButton *>();
        if (v.toInt() < buttons.count()) {
            buttons[v.toInt()]->setChecked(true);
        }
        return;
    }

    QByteArray userproperty = getCustomProperty(w);
    if (userproperty.isEmpty()) {
        userproperty = getUserProperty(w);
    }
    if (userproperty.isEmpty()) {
        QComboBox *cb = qobject_cast<QComboBox *>(w);
        if (cb) {
            if (cb->isEditable()) {
                int i = cb->findText(v.toString());
                if (i != -1) {
                    cb->setCurrentIndex(i);
                } else {
                    cb->setEditText(v.toString());
                }
            } else {
                cb->setCurrentIndex(v.toInt());
            }
            return;
        }
    }
    if (userproperty.isEmpty()) {
        qCWarning(KCONFIG_WIDGETS_LOG) << w->metaObject()->className() << "widget not handled!";
        return;
    }

    w->setProperty(userproperty.constData(), v);
}

QVariant KConfigDialogManager::property(QWidget *w) const
{
    if (d->allExclusiveGroupBoxes.contains(w)) {
        const QList<QAbstractButton *> buttons = w->findChildren<QAbstractButton *>();
        for (int i = 0; i < buttons.count(); ++i) {
            if (buttons[i]->isChecked()) {
                return i;
            }
        }
        return -1;
    }

    QByteArray userproperty = getCustomProperty(w);
    if (userproperty.isEmpty()) {
        userproperty = getUserProperty(w);
    }
    if (userproperty.isEmpty()) {
        QComboBox *cb = qobject_cast<QComboBox *>(w);
        if (cb) {
            if (cb->isEditable()) {
                return QVariant(cb->currentText());
            } else {
                return QVariant(cb->currentIndex());
            }
        }
    }
    if (userproperty.isEmpty()) {
        qCWarning(KCONFIG_WIDGETS_LOG) << w->metaObject()->className() << "widget not handled!";
        return QVariant();
    }

    return w->property(userproperty.constData());
}

bool KConfigDialogManager::hasChanged() const
{
    QWidget *widget;
    QHashIterator<QString, QWidget *> it(d->knownWidget);
    while (it.hasNext()) {
        it.next();
        widget = it.value();

        KConfigSkeletonItem *item = d->m_conf->findItem(it.key());
        if (!item) {
            qCWarning(KCONFIG_WIDGETS_LOG) << "The setting" << it.key() << "has disappeared!";
            continue;
        }

        if (!item->isEqual(property(widget))) {
            // qCDebug(KCONFIG_WIDGETS_LOG) << "Widget for '" << it.key() << "' has changed.";
            return true;
        }
    }
    return false;
}

bool KConfigDialogManager::isDefault() const
{
    QWidget *widget;
    QHashIterator<QString, QWidget *> it(d->knownWidget);
    while (it.hasNext()) {
        it.next();
        widget = it.value();

        KConfigSkeletonItem *item = d->m_conf->findItem(it.key());
        if (!item) {
            qCWarning(KCONFIG_WIDGETS_LOG) << "The setting" << it.key() << "has disappeared!";
            continue;
        }

        if (property(widget) != item->getDefault()) {
            return false;
        }
    }
    return true;
}

KConfigDialogManagerPrivate::KConfigDialogManagerPrivate(KConfigDialogManager *qq)
    : q(qq)
    , insideGroupBox(false)
    , defaultsIndicatorsVisible(false)
{
}

void KConfigDialogManagerPrivate::setDefaultsIndicatorsVisible(bool enabled)
{
    if (defaultsIndicatorsVisible != enabled) {
        defaultsIndicatorsVisible = enabled;
        updateAllWidgetIndicators();
    }
}

void KConfigDialogManagerPrivate::onWidgetModified()
{
    const auto widget = qobject_cast<QWidget *>(q->sender());
    Q_ASSERT(widget);

    const QLatin1String prefix("kcfg_");
    QString configId = widget->objectName();
    if (configId.startsWith(prefix)) {
        configId.remove(0, prefix.size());
        updateWidgetIndicator(configId, widget);
    } else {
        auto *parent = qobject_cast<QWidget *>(widget->parent());
        Q_ASSERT(parent);
        configId = parent->objectName();
        Q_ASSERT(configId.startsWith(prefix));
        configId.remove(0, prefix.size());
        updateWidgetIndicator(configId, parent);
    }

    Q_EMIT q->widgetModified();
}

void KConfigDialogManagerPrivate::updateWidgetIndicator(const QString &configId, QWidget *widget)
{
    const auto item = m_conf->findItem(configId);
    Q_ASSERT(item);

    const auto widgetValue = q->property(widget);
    const auto defaultValue = item->getDefault();

    const auto defaulted = widgetValue == defaultValue;

    if (allExclusiveGroupBoxes.contains(widget)) {
        const QList<QAbstractButton *> buttons = widget->findChildren<QAbstractButton *>();
        for (int i = 0; i < buttons.count(); i++) {
            const auto value = widgetValue.toInt() == i && !defaulted && defaultsIndicatorsVisible;
            buttons.at(i)->setProperty("_kde_highlight_neutral", value);
            buttons.at(i)->update();
        }
    } else {
        widget->setProperty("_kde_highlight_neutral", !defaulted && defaultsIndicatorsVisible);
        widget->update();
    }
}

void KConfigDialogManagerPrivate::updateAllWidgetIndicators()
{
    QHashIterator<QString, QWidget *> it(knownWidget);
    while (it.hasNext()) {
        it.next();
        updateWidgetIndicator(it.key(), it.value());
    }
}

#include "moc_kconfigdialogmanager.cpp"
