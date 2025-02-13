/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2010 Thomas Fjellstrom <thomas@fjellstrom.ca>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEFILETREEMODEL_H
#define KATEFILETREEMODEL_H

#include <unordered_map>

#include <QAbstractItemModel>
#include <QBrush>
#include <QColor>

#include <ktexteditor/modificationinterface.h>
namespace KTextEditor
{
class Document;
}

class ProxyItem;
class ProxyItemDir;

QDebug operator<<(QDebug dbg, ProxyItem *item);
QDebug operator<<(QDebug dbg, ProxyItemDir *item);

class KateFileTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum {
        DocumentRole = Qt::UserRole + 1,
        PathRole,
        OpeningOrderRole,
        DocumentTreeRole,
        WidgetRole,
    };

    KateFileTreeModel(QObject *p);
    ~KateFileTreeModel() override;

    /* QAbstractItemModel implementations */
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    QMimeData *mimeData(const QModelIndexList &indexes) const override;

    Qt::DropActions supportedDropActions() const override;

    /* extra api for view */
    QModelIndex docIndex(const KTextEditor::Document *) const;

    QModelIndex widgetIndex(QWidget *) const;

    bool isDir(const QModelIndex &index) const;
    bool isWidgetDir(const QModelIndex &index) const;

    bool listMode() const;
    void setListMode(bool);

    void setShadingEnabled(bool);

    void setEditShade(const QColor &);

    void setViewShade(const QColor &);

    bool showFullPathOnRoots(void) const;
    void setShowFullPathOnRoots(bool);

    void documentsOpened(const QList<KTextEditor::Document *> &);
    /* used strictly for the item coloring */
    void documentActivated(const KTextEditor::Document *);
    void documentEdited(const KTextEditor::Document *);
    void resetHistory();

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;

public Q_SLOTS:
    void documentOpened(KTextEditor::Document *);
    void documentClosed(KTextEditor::Document *);
    void documentNameChanged(KTextEditor::Document *);
    void documentModifiedChanged(KTextEditor::Document *);
    void documentModifiedOnDisc(KTextEditor::Document *, bool, KTextEditor::ModificationInterface::ModifiedOnDiskReason);

    void addWidget(QWidget *w);
    void removeWidget(QWidget *w);

Q_SIGNALS:
    void triggerViewChangeAfterNameChange();

private:
    ProxyItemDir *findRootNode(const QString &name, const int r = 1) const;
    static ProxyItemDir *findChildNode(const ProxyItemDir *parent, const QString &name);
    void insertItemInto(ProxyItemDir *root, ProxyItem *item, bool move = false, ProxyItemDir **moveDest = nullptr);
    void handleInsert(ProxyItem *item);
    void handleNameChange(ProxyItem *item);
    void handleEmptyParents(ProxyItemDir *item);
    static void setupIcon(ProxyItem *item);
    static void updateItemPathAndHost(ProxyItem *item);
    void handleDuplicitRootDisplay(ProxyItemDir *item);

    void updateBackgrounds(bool force = false);

    void initModel();
    void clearModel();
    void connectDocument(const KTextEditor::Document *);
    ProxyItem *itemForIndex(const QModelIndex &index) const;

private:
    ProxyItemDir *m_root;
    ProxyItem *m_widgetsRoot = nullptr;
    QHash<const KTextEditor::Document *, ProxyItem *> m_docmap;

    bool m_shadingEnabled;

    std::vector<ProxyItem *> m_viewHistory;
    std::vector<ProxyItem *> m_editHistory;
    std::unordered_map<ProxyItem *, QBrush> m_brushes;

    QColor m_editShade;
    QColor m_viewShade;
    QColor m_inactiveDocColor;

    bool m_listMode;
};

Q_DECLARE_METATYPE(QList<KTextEditor::Document *>)

#endif /* KATEFILETREEMODEL_H */
