/*
 *   SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>
 *   SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MODELVIEWER
#define MODELVIEWER

#include <QAbstractItemDelegate>
#include <QDialog>
#include <QStyleOptionViewItem>

class QAbstractItemModel;
class QTreeView;

namespace Plasma5Support
{
class DataEngine;
} // namespace Plasma5Support

class Delegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit Delegate(QObject *parent = nullptr);
    ~Delegate() override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

protected:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class ModelViewer : public QDialog
{
    Q_OBJECT

public:
    ModelViewer(Plasma5Support::DataEngine *engine, const QString &m_source, QWidget *parent = nullptr);
    ~ModelViewer() override;

private Q_SLOTS:
    void engineDestroyed();

private:
    Plasma5Support::DataEngine *m_engine;
    QString m_source;
    QAbstractItemModel *m_model;
    QTreeView *m_view;
};

#endif
