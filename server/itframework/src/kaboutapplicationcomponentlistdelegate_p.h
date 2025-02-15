/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Julius Künzel <jk.kdedev@smartlab.uber.space>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KABOUT_APPLICATION_COMPONENT_LIST_DELEGATE_H
#define KABOUT_APPLICATION_COMPONENT_LIST_DELEGATE_H

#include <KWidgetItemDelegate>

namespace KDEPrivate
{
class KAboutApplicationComponentProfile;

class KAboutApplicationComponentListDelegate : public KWidgetItemDelegate
{
    Q_OBJECT
public:
    explicit KAboutApplicationComponentListDelegate(QAbstractItemView *itemView, QObject *parent = nullptr);

    ~KAboutApplicationComponentListDelegate() override
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QList<QWidget *> createItemWidgets(const QModelIndex &index) const override;

    void updateItemWidgets(const QList<QWidget *> widgets, const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const override;

private Q_SLOTS:
    void launchUrl(QAction *action) const;

private:
    int heightForString(const QString &string, int lineWidth, const QStyleOptionViewItem &option) const;
    QString buildTextForProfile(const KAboutApplicationComponentProfile &profile) const;
    QRect widgetsRect(const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const;

    enum DelegateWidgets { TextLabel = 0, MainLinks };

    enum MainLinkActions {
        HomepageAction = 0,
    };
};

} // namespace KDEPrivate

#endif // KABOUT_APPLICATION_COMPONENT_LIST_DELEGATE_H
