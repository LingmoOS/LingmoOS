/*
    SPDX-FileCopyrightText: 2012-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_COLOR_TREE_WIDGET_H
#define KATE_COLOR_TREE_WIDGET_H

#include <QTreeWidget>

#include <KSyntaxHighlighting/Theme>

class KateColorItem
{
public:
    KateColorItem(KSyntaxHighlighting::Theme::EditorColorRole _role = KSyntaxHighlighting::Theme::BackgroundColor)
        : role(_role)
    {
    }

    KSyntaxHighlighting::Theme::EditorColorRole role;
    QString name; // translated name
    QString category; // translated category for tree view hierarchy
    QString whatsThis; // what's this info
    QString key; // untranslated id, used as key to save/load from KConfig
    QColor color; // user visible color
    QColor defaultColor; // used when "Default" is clicked
    bool useDefault = true; // flag whether to use the default color
};

class KateColorTreeWidget : public QTreeWidget
{
    Q_OBJECT
    friend class KateColorTreeItem;
    friend class KateColorTreeDelegate;

public:
    explicit KateColorTreeWidget(QWidget *parent = nullptr);

public:
    void addColorItem(const KateColorItem &colorItem);
    void addColorItems(const QList<KateColorItem> &colorItems);

    QList<KateColorItem> colorItems() const;

    QColor findColor(const QString &key) const;

    bool readOnly() const;
    void setReadOnly(bool readOnly);

public Q_SLOTS:
    void selectDefaults();

Q_SIGNALS:
    void changed();

protected:
    bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) override;
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;

private:
    bool m_readOnly = false;
};

#endif
