/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Teo Mrnjavac <teo@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KABOUT_APPLICATION_PERSON_LIST_DELEGATE_H
#define KABOUT_APPLICATION_PERSON_LIST_DELEGATE_H

#include <KWidgetItemDelegate>

namespace KDEPrivate
{
class KAboutApplicationPersonProfile;

class KAboutApplicationPersonListDelegate : public KWidgetItemDelegate
{
    Q_OBJECT
public:
    explicit KAboutApplicationPersonListDelegate(QAbstractItemView *itemView, QObject *parent = nullptr);

    ~KAboutApplicationPersonListDelegate() override
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QList<QWidget *> createItemWidgets(const QModelIndex &index) const override;

    void updateItemWidgets(const QList<QWidget *> widgets, const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const override;

private Q_SLOTS:
    void launchUrl(QAction *action) const;

private:
    int heightForString(const QString &string, int lineWidth, const QStyleOptionViewItem &option) const;
    QString buildTextForProfile(const KAboutApplicationPersonProfile &profile) const;
    QRect widgetsRect(const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const;

    enum DelegateWidgets {
        TextLabel = 0,
        MainLinks,
        SocialLinks,
    };

    enum MainLinkActions {
        EmailAction = 0,
        HomepageAction,
        VisitProfileAction,
    };
};

} // namespace KDEPrivate

#endif // KABOUT_APPLICATION_PERSON_LIST_DELEGATE_H
