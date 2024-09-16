// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USER_ITEM_DELEGATE_H
#define USER_ITEM_DELEGATE_H

#include <QStyledItemDelegate>

/*!
 * \brief The UserItemDelegate class
 */
class UserItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum UserItemDataRole {
        StaticUserDataRole      = Qt::UserRole + 1, // 用户静态数据
        IsCurrentUserDataRole   = Qt::UserRole + 2, // 是否是当前用户
    };

    struct UserItemData {
        QString imagePath;
        QString displayName;
        QString name;
        QString userStrType;
        int userType;
        uid_t userId;
        bool isLogined = false;
    };

public:
    explicit UserItemDelegate(QObject *parent = nullptr);
    static int displayNameHeight();
    static int nameHeight();
    static int userTypeHeight();

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    void drawRoundImage(QPainter *painter, const QRect &rect, const QString &path) const;
    void drawLoginedState(QPainter *painter, const QRect &rect) const;
    void drawNameAndType(QPainter *painter, const UserItemData &userData, const QRect &rect) const;
    void drawCheckedState(QPainter *painter, const QRect &rect) const;

    int stringWidth(const QString &str, int fontSize, bool isBold = false) const;
    QString elidedText(const QString &originString, int width, int fontSize, bool isBold = false) const;
};

Q_DECLARE_METATYPE(UserItemDelegate::UserItemData)

#endif // USER_ITEM_DELEGATE_H
