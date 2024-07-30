/*
    SPDX-FileCopyrightText: 1999-2003 Hans Petter Bieker <bieker@kde.org>
    SPDX-FileCopyrightText: 2007 David Jarvie <djarvie@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "klanguagebutton.h"

#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QLocale>
#include <QMenu>
#include <QPushButton>

#include <KConfig>
#include <KConfigGroup>

static void checkInsertPos(QMenu *popup, const QString &str, int &index)
{
    if (index != -1) {
        return;
    }

    int a = 0;
    const QList<QAction *> actions = popup->actions();
    int b = actions.count();

    while (a < b) {
        int w = (a + b) / 2;
        QAction *ac = actions[w];
        int j = str.localeAwareCompare(ac->text());
        if (j > 0) {
            a = w + 1;
        } else {
            b = w;
        }
    }

    index = a; // it doesn't really matter ... a == b here.

    Q_ASSERT(a == b);
}

class KLanguageButtonPrivate
{
public:
    explicit KLanguageButtonPrivate(KLanguageButton *parent);
    ~KLanguageButtonPrivate()
    {
        delete button;
        delete popup;
    }
    void setCurrentItem(QAction *);
    void clear();
    QAction *findAction(const QString &data) const;

    QPushButton *button = nullptr;
    QStringList ids;
    QMenu *popup = nullptr;
    QString current;
    QString locale;
    bool staticText : 1;
    bool showCodes : 1;
};

KLanguageButton::KLanguageButton(QWidget *parent)
    : QWidget(parent)
    , d(new KLanguageButtonPrivate(this))
{
}

KLanguageButton::KLanguageButton(const QString &text, QWidget *parent)
    : QWidget(parent)
    , d(new KLanguageButtonPrivate(this))
{
    setText(text);
}

KLanguageButtonPrivate::KLanguageButtonPrivate(KLanguageButton *parent)
    : button(new QPushButton(parent))
    , popup(new QMenu(parent))
    , locale(QLocale::system().name())
    , staticText(false)
    , showCodes(false)
{
    QHBoxLayout *layout = new QHBoxLayout(parent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(button);

    parent->setFocusProxy(button);
    parent->setFocusPolicy(button->focusPolicy());

    button->setMenu(popup);

    QObject::connect(popup, &QMenu::triggered, parent, &KLanguageButton::slotTriggered);
    QObject::connect(popup, &QMenu::hovered, parent, &KLanguageButton::slotHovered);
}

KLanguageButton::~KLanguageButton() = default;

void KLanguageButton::setText(const QString &text)
{
    d->staticText = true;
    d->button->setText(text);
}

void KLanguageButton::setLocale(const QString &locale)
{
    d->locale = locale;
}

void KLanguageButton::showLanguageCodes(bool show)
{
    d->showCodes = show;
}

static QString nameFromEntryFile(const QString &entryFile)
{
    const KConfig entry(entryFile, KConfig::SimpleConfig);
    const KConfigGroup group(&entry, QStringLiteral("KCM Locale"));
    return group.readEntry("Name", QString());
}

void KLanguageButton::insertLanguage(const QString &languageCode, const QString &name, int index)
{
    QString text;
    bool showCodes = d->showCodes;
    if (name.isEmpty()) {
        const QString entryFile =
            QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("locale/") + languageCode + QLatin1String("/kf6_entry.desktop"));
        if (QFile::exists(entryFile)) {
            text = nameFromEntryFile(entryFile);
        }

        if (text.isEmpty()) {
            text = languageCode;
            QLocale locale(languageCode);
            if (locale != QLocale::c()) {
                text = locale.nativeLanguageName();
                // For some languages the native name might be empty.
                // In this case use the non native language name as fallback.
                // See: QTBUG-51323
                text = text.isEmpty() ? QLocale::languageToString(locale.language()) : text;
            } else {
                showCodes = false;
            }
        }
    } else {
        text = name;
    }
    if (showCodes) {
        text += QLatin1String(" (") + languageCode + QLatin1Char(')');
    }

    checkInsertPos(d->popup, text, index);
    QAction *a = new QAction(QIcon(), text, this);
    a->setData(languageCode);
    if (index >= 0 && index < d->popup->actions().count() - 1) {
        d->popup->insertAction(d->popup->actions()[index], a);
    } else {
        d->popup->addAction(a);
    }
    d->ids.append(languageCode);
}

void KLanguageButton::insertSeparator(int index)
{
    if (index >= 0 && index < d->popup->actions().count() - 1) {
        d->popup->insertSeparator(d->popup->actions()[index]);
    } else {
        d->popup->addSeparator();
    }
}

void KLanguageButton::loadAllLanguages()
{
    const QStringList localeDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("locale"), QStandardPaths::LocateDirectory);
    for (const QString &localeDir : localeDirs) {
        const QStringList entries = QDir(localeDir).entryList(QDir::Dirs, QDir::Name);
        for (const QString &d : entries) {
            const QString entryFile = localeDir + QLatin1Char('/') + d + QStringLiteral("/kf6_entry.desktop");
            if (QFile::exists(entryFile)) {
                insertLanguage(d);
            }
        }
    }

    d->ids.removeDuplicates();
    setCurrentItem(d->locale);
}

void KLanguageButton::slotTriggered(QAction *a)
{
    // qCDebug(KCONFIG_WIDGETS_LOG) << "slotTriggered" << index;
    if (!a) {
        return;
    }

    d->setCurrentItem(a);

    // Forward event from popup menu as if it was emitted from this widget:
    Q_EMIT activated(d->current);
}

void KLanguageButton::slotHovered(QAction *a)
{
    // qCDebug(KCONFIG_WIDGETS_LOG) << "slotHovered" << index;

    Q_EMIT highlighted(a->data().toString());
}

int KLanguageButton::count() const
{
    return d->ids.count();
}

void KLanguageButton::clear()
{
    d->clear();
}

void KLanguageButtonPrivate::clear()
{
    ids.clear();
    popup->clear();

    if (!staticText) {
        button->setText(QString());
    }
}

bool KLanguageButton::contains(const QString &languageCode) const
{
    return d->ids.contains(languageCode);
}

QString KLanguageButton::current() const
{
    return d->current.isEmpty() ? QStringLiteral("en") : d->current;
}

QAction *KLanguageButtonPrivate::findAction(const QString &data) const
{
    const auto listActions = popup->actions();
    for (QAction *a : listActions) {
        if (!a->data().toString().compare(data)) {
            return a;
        }
    }
    return nullptr;
}

void KLanguageButton::setCurrentItem(const QString &languageCode)
{
    if (d->ids.isEmpty()) {
        return;
    }
    QAction *a;
    if (d->ids.indexOf(languageCode) < 0) {
        a = d->findAction(d->ids[0]);
    } else {
        a = d->findAction(languageCode);
    }
    if (a) {
        d->setCurrentItem(a);
    }
}

void KLanguageButtonPrivate::setCurrentItem(QAction *a)
{
    if (!a->data().isValid()) {
        return;
    }
    current = a->data().toString();

    if (!staticText) {
        button->setText(a->text());
    }
}

#include "moc_klanguagebutton.cpp"
