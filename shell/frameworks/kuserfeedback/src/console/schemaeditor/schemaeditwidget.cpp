/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "schemaeditwidget.h"
#include "ui_schemaeditwidget.h"
#include "schemaentryitemeditorfactory.h"

#include <model/schemamodel.h>
#include <rest/restapi.h>
#include <core/product.h>
#include <core/schemaentrytemplates.h>

#include <QDebug>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkReply>
#include <QStyledItemDelegate>

using namespace KUserFeedback::Console;

SchemaEditWidget::SchemaEditWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SchemaEditWidget),
    m_schemaModel(new SchemaModel(this))
{
    ui->setupUi(this);

    ui->schemaView->setModel(m_schemaModel);
    qobject_cast<QStyledItemDelegate*>(ui->schemaView->itemDelegate())->setItemEditorFactory(new SchemaEntryItemEditorFactory);
    ui->schemaView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    connect(ui->actionAddSource, &QAction::triggered, this, &SchemaEditWidget::addSource);
    connect(ui->actionAddSourceElement, &QAction::triggered, this, &SchemaEditWidget::addSourceEntry);
    connect(ui->actionDelete, &QAction::triggered, this, &SchemaEditWidget::deleteEntry);

    connect(ui->schemaView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SchemaEditWidget::updateState);
    connect(ui->schemaView, &QWidget::customContextMenuRequested, this, &SchemaEditWidget::contextMenu);

    connect(m_schemaModel, &QAbstractItemModel::dataChanged, this, [this]() {
        Q_EMIT productChanged();
    });

    addActions({ ui->actionAddSource, ui->actionAddSourceElement, ui->actionDelete });
    updateState();
}

SchemaEditWidget::~SchemaEditWidget() = default;

void SchemaEditWidget::setRESTClient(RESTClient* client)
{
    m_restClient = client;
}

Product SchemaEditWidget::product() const
{
    return m_schemaModel->product();
}

void SchemaEditWidget::setProduct(const Product& product)
{
    m_schemaModel->setProduct(product);
    ui->schemaView->expandAll();
    updateState();
}

void SchemaEditWidget::addSource()
{
    const auto name = QInputDialog::getText(this, tr("Add Source"), tr("Name:"));
    if (name.isEmpty())
        return;
    m_schemaModel->addEntry(name);
    Q_EMIT productChanged();
}

void SchemaEditWidget::addSourceEntry()
{
    const auto name = QInputDialog::getText(this, tr("Add Source Element"), tr("Name:"));
    if (name.isEmpty())
        return;
    m_schemaModel->addElement(currentSource(), name);
    ui->schemaView->expand(currentSource());
    Q_EMIT productChanged();
}

void SchemaEditWidget::deleteEntry()
{
    const auto selection = ui->schemaView->selectionModel()->selection();
    if (selection.isEmpty())
        return;

    if (currentSource().isValid()) {
        const auto r = QMessageBox::critical(this, tr("Delete Source"),
            tr("Do you really want to delete the source '%1', and all recorded data for it?").arg(currentSource().data().toString()),
            QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel);
        if (r != QMessageBox::Discard)
            return;
    } else {
        const auto r = QMessageBox::critical(this, tr("Delete Schema Entry"),
            tr("Do you really want to delete this entry, and all recorded data for it?"),
            QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel);
        if (r != QMessageBox::Discard)
            return;
    }

    const auto idx = selection.first().topLeft();
    m_schemaModel->deleteRow(idx);
    Q_EMIT productChanged();
}

void SchemaEditWidget::updateState()
{
    const auto selection = ui->schemaView->selectionModel()->selection();
    ui->actionDelete->setEnabled(!selection.isEmpty());
    ui->actionAddSource->setEnabled(product().isValid());
    ui->actionAddSourceElement->setEnabled(currentSource().isValid());
}

void SchemaEditWidget::contextMenu(QPoint pos)
{
    QMenu menu;
    menu.addActions({ ui->actionAddSource, ui->actionAddSourceElement, ui->actionDelete });
    menu.exec(ui->schemaView->viewport()->mapToGlobal(pos));
}

QModelIndex SchemaEditWidget::currentSource() const
{
    const auto selection = ui->schemaView->selectionModel()->selectedRows();
    if (selection.isEmpty())
        return {};
    const auto idx = selection.at(0);
    if (!idx.parent().isValid())
        return idx;
    return {};
}

#include "moc_schemaeditwidget.cpp"
