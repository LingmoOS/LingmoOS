/*
    SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "modelcommanderwidget.h"

#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "dynamictreemodel.h"
#include "modelcommander.h"
#include <QMetaMethod>

ModelCommanderWidget::ModelCommanderWidget(DynamicTreeModel *dynamicTreeModel, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_dynamicTreeModel(dynamicTreeModel)
    , m_modelCommander(new ModelCommander(m_dynamicTreeModel, this))
    , m_treeWidget(new QTreeWidget)
    , m_executeButton(new QPushButton(QStringLiteral("Execute")))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_treeWidget);
    layout->addWidget(m_executeButton);

    init();

    connect(m_treeWidget, &QTreeWidget::currentItemChanged, this, &ModelCommanderWidget::currentItemChanged);

    connect(m_executeButton, &QPushButton::clicked, this, &ModelCommanderWidget::executeCurrentTest);
}

void ModelCommanderWidget::init()
{
    const QMetaObject *mo = m_modelCommander->metaObject();
    QMetaMethod mm;
    for (int i = 0; i < mo->methodCount(); ++i) {
        mm = mo->method(i);
        QString signature = mm.methodSignature();
        if (signature.startsWith(QLatin1String("init_")) && signature.endsWith(QLatin1String("(QString)"))) {
            QTreeWidgetItem *testFunctionItem = new QTreeWidgetItem(m_treeWidget, QStringList() << signature.mid(5, signature.length() - 14));
            m_treeWidget->addTopLevelItem(testFunctionItem);

            QStringList testData;
            QMetaObject::invokeMethod(m_modelCommander,
                                      QByteArray("execute_" + testFunctionItem->text(0).toLatin1()).constData(),
                                      Q_RETURN_ARG(QStringList, testData),
                                      Q_ARG(QString, QString()));

            for (const QString &testRun : std::as_const(testData)) {
                new QTreeWidgetItem(testFunctionItem, QStringList() << testRun);
            }
        }
    }
}

void ModelCommanderWidget::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous);
    initTest(current);
}

void ModelCommanderWidget::executeCurrentTest()
{
    executeTest(m_treeWidget->currentItem());

    disconnect(m_executeButton, &QPushButton::clicked, this, &ModelCommanderWidget::executeCurrentTest);
    m_executeButton->setText(QStringLiteral("Reset"));
    connect(m_executeButton, &QPushButton::clicked, this, &ModelCommanderWidget::resetCurrentTest);
}

void ModelCommanderWidget::resetCurrentTest()
{
    initTest(m_treeWidget->currentItem());

    disconnect(m_executeButton, &QPushButton::clicked, this, &ModelCommanderWidget::resetCurrentTest);
    m_executeButton->setText(QStringLiteral("Execute"));
    connect(m_executeButton, &QPushButton::clicked, this, &ModelCommanderWidget::executeCurrentTest);
}

void ModelCommanderWidget::initTest(QTreeWidgetItem *item)
{
    if (!item->parent()) {
        return; // m_dynamicTreeModel->clear();
    }
    m_dynamicTreeModel->clear();
    bool success =
        QMetaObject::invokeMethod(m_modelCommander, QByteArray("init_" + item->parent()->text(0).toLatin1()).constData(), Q_ARG(QString, item->text(0)));
    Q_ASSERT(success);
}

void ModelCommanderWidget::executeTest(QTreeWidgetItem *item)
{
    if (!item->parent()) {
        return;
    }

    bool success =
        QMetaObject::invokeMethod(m_modelCommander, QByteArray("execute_" + item->parent()->text(0).toLatin1()).constData(), Q_ARG(QString, item->text(0)));
    Q_ASSERT(success);
}

#include "moc_modelcommanderwidget.cpp"
