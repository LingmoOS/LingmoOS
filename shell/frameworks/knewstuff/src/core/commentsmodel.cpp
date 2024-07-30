/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "commentsmodel.h"

#include "entry.h"
#include "knewstuffcore_debug.h"

#include <KLocalizedString>

#include <QTimer>

namespace KNSCore
{
class CommentsModelPrivate
{
public:
    CommentsModelPrivate(CommentsModel *qq)
        : q(qq)
    {
    }
    CommentsModel *const q;
    EngineBase *engine = nullptr;

    Entry entry;

    QList<std::shared_ptr<KNSCore::Comment>> comments;

    enum FetchOptions {
        NoOption,
        ClearModel,
    };
    bool fetchThrottle = false;
    void fetch(FetchOptions option = NoOption)
    {
        if (fetchThrottle) {
            return;
        }
        fetchThrottle = true;
        QTimer::singleShot(1, q, [this]() {
            fetchThrottle = false;
        });
        // Sanity checks, because we need a few things to be correct before we can actually fetch comments...
        if (!engine) {
            qCWarning(KNEWSTUFFCORE) << "CommentsModel must be parented on a KNSCore::EngineBase instance to be able to fetch comments";
        }
        if (!entry.isValid()) {
            qCWarning(KNEWSTUFFCORE) << "Without an entry to fetch comments for, CommentsModel cannot fetch comments for it";
        }

        if (engine && entry.isValid()) {
            QSharedPointer<Provider> provider = engine->provider(entry.providerId());
            if (option == ClearModel) {
                q->beginResetModel();
                comments.clear();
                provider->disconnect(q);
                q->connect(provider.data(), &Provider::commentsLoaded, q, [=](const QList<std::shared_ptr<KNSCore::Comment>> &newComments) {
                    QList<std::shared_ptr<KNSCore::Comment>> actualNewComments;
                    for (const std::shared_ptr<KNSCore::Comment> &comment : newComments) {
                        bool commentIsKnown = false;
                        for (const std::shared_ptr<KNSCore::Comment> &existingComment : std::as_const(comments)) {
                            if (existingComment->id == comment->id) {
                                commentIsKnown = true;
                                break;
                            }
                        }
                        if (commentIsKnown) {
                            continue;
                        }
                        actualNewComments << comment;
                    }
                    if (actualNewComments.count() > 0) {
                        q->beginInsertRows(QModelIndex(), comments.count(), comments.count() + actualNewComments.count() - 1);
                        qCDebug(KNEWSTUFFCORE) << "Appending" << actualNewComments.count() << "new comments";
                        comments.append(actualNewComments);
                        q->endInsertRows();
                    }
                });
                q->endResetModel();
            }
            int commentsPerPage = 100;
            int pageToLoad = comments.count() / commentsPerPage;
            qCDebug(KNEWSTUFFCORE) << "Loading comments, page" << pageToLoad << "with current comment count" << comments.count() << "out of a total of"
                                   << entry.numberOfComments();
            provider->loadComments(entry, commentsPerPage, pageToLoad);
        }
    }
};
}

KNSCore::CommentsModel::CommentsModel(EngineBase *parent)
    : QAbstractListModel(parent)
    , d(new CommentsModelPrivate(this))
{
    d->engine = parent;
}

KNSCore::CommentsModel::~CommentsModel() = default;

QHash<int, QByteArray> KNSCore::CommentsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        {IdRole, "id"},
        {SubjectRole, "subject"},
        {TextRole, "text"},
        {ChildCountRole, "childCound"},
        {UsernameRole, "username"},
        {DateRole, "date"},
        {ScoreRole, "score"},
        {ParentIndexRole, "parentIndex"},
        {DepthRole, "depth"},
    };
    return roles;
}

QVariant KNSCore::CommentsModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return QVariant();
    }
    const std::shared_ptr<KNSCore::Comment> comment = d->comments[index.row()];
    switch (role) {
    case IdRole:
        return comment->id;
    case SubjectRole:
        return comment->subject;
    case TextRole:
        return comment->text;
    case ChildCountRole:
        return comment->childCount;
    case UsernameRole:
        return comment->username;
    case DateRole:
        return comment->date;
    case ScoreRole:
        return comment->score;
    case ParentIndexRole:
        return comment->parent ? d->comments.indexOf(comment->parent) : -1;
    case DepthRole: {
        int depth{0};
        if (comment->parent) {
            std::shared_ptr<KNSCore::Comment> child = comment->parent;
            while (child) {
                ++depth;
                child = child->parent;
            }
        }
        return depth;
    }
    default:
        return i18nc("The value returned for an unknown role when requesting data from the model.", "Unknown CommentsModel role");
    }
}

int KNSCore::CommentsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->comments.count();
}

bool KNSCore::CommentsModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return false;
    }
    if (d->entry.numberOfComments() > d->comments.count()) {
        return true;
    }
    return false;
}

void KNSCore::CommentsModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid()) {
        return;
    }
    d->fetch();
}

const KNSCore::Entry &KNSCore::CommentsModel::entry() const
{
    return d->entry;
}

void KNSCore::CommentsModel::setEntry(const KNSCore::Entry &newEntry)
{
    d->entry = newEntry;
    d->fetch(CommentsModelPrivate::ClearModel);
    Q_EMIT entryChanged();
}

#include "moc_commentsmodel.cpp"
