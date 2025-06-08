/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "emojierplugin.h"

#undef signals
#include "emojidict.h"
#include "emojiersettings.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <qqml.h>

class AbstractEmojiModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum EmojiRole { CategoryRole = Qt::UserRole + 1, AnnotationsRole };

    int rowCount(const QModelIndex &parent = {}) const override
    {
        return parent.isValid() ? 0 : m_emoji.size();
    }
    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!checkIndex(index,
                        QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid
                            | QAbstractItemModel::CheckIndexOption::DoNotUseParent)
            || index.column() != 0)
            return {};

        const auto &emoji = m_emoji[index.row()];
        switch (role) {
        case Qt::DisplayRole:
            return emoji.content;
        case Qt::ToolTipRole:
            return emoji.description;
        case CategoryRole:
            return emoji.categoryName();
        case AnnotationsRole:
            return emoji.annotations;
        }
        return {};
    }

protected:
    QList<Emoji> m_emoji;
};

class EmojiModel : public AbstractEmojiModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList categories MEMBER m_categories CONSTANT)
public:
    enum EmojiRole { CategoryRole = Qt::UserRole + 1 };

    EmojiModel()
    {
        QLocale locale;
        QVector<QString> dicts;
        const auto bcp = locale.bcp47Name();
        const QString dictName = QLatin1String{"plasma/emoji/"} + QString(bcp).replace(QLatin1Char('-'), QLatin1Char('_')) + QLatin1String{".dict"};
        const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, dictName);
        if (!path.isEmpty()) {
            dicts << path;
        }

        const auto idxSpecific = bcp.indexOf(QLatin1Char('-'));
        if (idxSpecific > 0) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            const QString genericDictName = QLatin1String{"plasma/emoji/"} + bcp.leftRef(idxSpecific) + QLatin1String{".dict"};
#else
            const QString genericDictName = QLatin1String{"plasma/emoji/"} + QStringView(bcp).left(idxSpecific) + QLatin1String{".dict"};
#endif
            const QString genericPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, genericDictName);

            if (!genericPath.isEmpty()) {
                dicts << genericPath;
            }
        }

        // Always fallback to en, because some annotation data only have minimum data.
        const QString genericPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("plasma/emoji/en.dict"));
        dicts << genericPath;

        if (dicts.isEmpty()) {
            qWarning() << "could not find emoji dictionaries." << dictName;
            return;
        }

        QSet<QString> categories;
        EmojiDict dict;
        // We load in reverse order, because we want to preserve the order in en.dict.
        // en.dict almost always gives complete set of data.
        for (auto iter = dicts.crbegin(); iter != dicts.crend(); ++iter) {
            dict.load(*iter);
        }
        m_emoji = std::move(dict.m_emojis);
        for (const auto &emoji : m_emoji) {
            categories.insert(emoji.categoryName());
        }
        m_categories = categories.values();
        m_categories.sort();
    }

    Q_SCRIPTABLE QString findFirstEmojiForCategory(const QString &category)
    {
        for (const Emoji &emoji : m_emoji) {
            if (emoji.categoryName() == category)
                return emoji.content;
        }
        return {};
    }

private:
    QStringList m_categories;
};

class RecentEmojiModel : public AbstractEmojiModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount CONSTANT)
public:
    RecentEmojiModel()
    {
        refresh();
    }

    Q_SCRIPTABLE void includeRecent(const QString &emoji, const QString &emojiDescription)
    {
        QStringList recent = m_settings.recent();
        QStringList recentDescriptions = m_settings.recentDescriptions();

        const int idx = recent.indexOf(emoji);
        if (idx >= 0) {
            recent.removeAt(idx);
            recentDescriptions.removeAt(idx);
        }
        recent.prepend(emoji);
        recent = recent.mid(0, 50);
        m_settings.setRecent(recent);

        recentDescriptions.prepend(emojiDescription);
        recentDescriptions = recentDescriptions.mid(0, 50);
        m_settings.setRecentDescriptions(recentDescriptions);
        m_settings.save();

        refresh();
    }

    Q_INVOKABLE void clearHistory()
    {
        m_settings.setRecent(QStringList());
        m_settings.setRecentDescriptions(QStringList());
        m_settings.save();

        refresh();
    }

private:
    void refresh()
    {
        beginResetModel();
        auto recent = m_settings.recent();
        auto recentDescriptions = m_settings.recentDescriptions();
        int i = 0;
        m_emoji.clear();
        for (const QString &c : recent) {
            m_emoji += {c, recentDescriptions.at(i++), 0, {}};
        }
        endResetModel();
    }

    EmojierSettings m_settings;
};

class CategoryModelFilter : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString category READ category WRITE setCategory)
public:
    QString category() const
    {
        return m_category;
    }
    void setCategory(const QString &category)
    {
        if (m_category != category) {
            m_category = category;
            invalidateFilter();
        }
    }

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override
    {
        return m_category.isEmpty() || sourceModel()->index(source_row, 0, source_parent).data(EmojiModel::CategoryRole).toString() == m_category;
    }

private:
    QString m_category;
};

class SearchModelFilter : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString search READ search WRITE setSearch)
public:
    QString search() const
    {
        return m_search;
    }
    void setSearch(const QString &search)
    {
        if (m_search != search) {
            m_search = search;
            invalidateFilter();
        }
    }

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override
    {
        const auto idx = sourceModel()->index(source_row, 0, source_parent);
        return idx.data(Qt::ToolTipRole).toString().contains(m_search, Qt::CaseInsensitive)
            || idx.data(AbstractEmojiModel::AnnotationsRole).toStringList().contains(m_search, Qt::CaseInsensitive);
    }

private:
    QString m_search;
};

class CopyHelperPrivate : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE static void copyTextToClipboard(const QString &text)
    {
        QClipboard *clipboard = qGuiApp->clipboard();
        clipboard->setText(text, QClipboard::Clipboard);
        clipboard->setText(text, QClipboard::Selection);
    }
};

void EmojierDeclarativePlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QByteArray("org.kde.plasma.emoji"));

    qmlRegisterType<EmojiModel>(uri, 1, 0, "EmojiModel");
    qmlRegisterType<CategoryModelFilter>(uri, 1, 0, "CategoryModelFilter");
    qmlRegisterType<SearchModelFilter>(uri, 1, 0, "SearchModelFilter");
    qmlRegisterType<RecentEmojiModel>(uri, 1, 0, "RecentEmojiModel");
    qmlRegisterSingletonType<CopyHelperPrivate>(uri, 1, 0, "CopyHelper", [](QQmlEngine *, QJSEngine *) -> QObject * {
        return new CopyHelperPrivate;
    });
}

#include "emojierplugin.moc"
