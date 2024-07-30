/*
 *   SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ENGINEEXPLORER_H
#define ENGINEEXPLORER_H

class QStandardItemModel;
class QStandardItem;

#include <QDialog>

#include <Plasma5Support/DataEngine>

#include "ui_engineexplorer.h"

namespace Plasma5Support
{
class DataEngine;
class PluginLoader;
} // namespace Plasma

class QPushButton;

class EngineExplorer : public QDialog, public Ui::EngineExplorer
{
    Q_OBJECT

public:
    explicit EngineExplorer(QWidget *parent = nullptr);
    ~EngineExplorer() override;

    void setApp(const QString &app);
    void setEngine(const QString &engine);
    void setInterval(const int interval);
    void requestSource(const QString &source);

    static QString convertToString(const QVariant &value);

public Q_SLOTS:
    void dataUpdated(const QString &source, const Plasma5Support::DataEngine::Data &data);

private Q_SLOTS:
    void showEngine(const QString &engine);
    void addSource(const QString &source);
    void removeSource(const QString &source);
    void requestSource();
    void requestServiceForSource();
    void showDataContextMenu(const QPoint &point);
    void cleanUp();

private:
    void listEngines();
    int showData(QStandardItem *parent, Plasma5Support::DataEngine::Data data);
    void showData(QStandardItem *parent, int row, const QString &key, const QVariant &value);
    int showContainerData(QStandardItem *parent, QStandardItem *current, int row, const char *typeName, const QList<QVariantMap> &list);
    int showContainerData(QStandardItem *parent, QStandardItem *current, int row, const char *typeName, const QVariantList &list);
    int showContainerData(QStandardItem *parent, QStandardItem *current, int row, const char *typeName, const QVariantMap &map);
    void updateTitle();
    void enableButtons(bool enable);
    void removeExtraRows(QStandardItem *parent, int preserve);

    QStandardItemModel *m_dataModel;
    QString m_app;
    QString m_engineName;
    Plasma5Support::DataEngine *m_engine;
    int m_sourceCount;
    bool m_requestingSource;
    QPushButton *m_expandButton;
    QPushButton *m_collapseButton;
};

#endif // multiple inclusion guard
