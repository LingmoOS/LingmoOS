/*
    This file is part of the proxy model test suite.

    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "descendantqmltree.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QSplitter>
#include <QTreeView>

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"
#include "kselectionproxymodel.h"

DescendantQmlTreeWidget::DescendantQmlTreeWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    QSplitter *splitter = new QSplitter(this);
    layout->addWidget(splitter);

    m_rootModel = new DynamicTreeModel(this);

    new DynamicTreeWidget(m_rootModel, splitter);

    qmlRegisterType<KSelectionProxyModel>("KF5ItemModels", 1, 0, "SelectionProxyModel");

    QQuickWidget *quickView = new QQuickWidget(splitter);

    quickView->engine()->rootContext()->setContextProperty(QStringLiteral("_model"), m_rootModel);

    quickView->setSource(QUrl::fromLocalFile(QLatin1String(SRC_DIR "/tree.qml")));
}
