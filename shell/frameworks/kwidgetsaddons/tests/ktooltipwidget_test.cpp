/*
    SPDX-FileCopyrightText: 2017 Elvis Angelaccio <elvis.angelaccio@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "ktooltipwidget_test.h"

#include <QApplication>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

// Doxygen will generate code snippets from this file.
// We can't use i18n() here, but we want it to show up in the apidox.
#define i18n QStringLiteral

TestDialog::TestDialog(QWidget *parent)
    : QDialog(parent)
{
    m_view.setColumnCount(1);
    m_view.setRootIsDecorated(false);
    m_view.setMouseTracking(true);

    auto item = new QTreeWidgetItem(&m_view);
    item->setText(0, QStringLiteral("Hover me! (first item)"));
    auto item2 = new QTreeWidgetItem(&m_view);
    item2->setText(0, QStringLiteral("Hover me! (second item)"));

    auto spinBox = new QSpinBox(this);
    spinBox->setPrefix(QStringLiteral("Hide delay (ms): "));
    spinBox->setMaximum(5000);
    spinBox->setValue(500);
    connect(spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), &m_tooltipWidget, &KToolTipWidget::setHideDelay);
    auto layout = new QVBoxLayout(this);
    layout->addWidget(spinBox);
    layout->addWidget(&m_view);

    //! [show_tooltip_widget]
    connect(&m_view, &QAbstractItemView::entered, this, [this](const QModelIndex &index) {
        auto rect = m_view.visualRect(index);
        const auto pos = m_view.viewport()->mapToGlobal(rect.topLeft());
        rect.moveTo(pos);
        auto button = new QPushButton(i18n("Push me (row %1)").arg(index.row()), this);
        m_tooltipWidget.showBelow(rect, button, m_view.nativeParentWidget()->windowHandle());
    });
    //! [show_tooltip_widget]

    connect(&m_view, &QAbstractItemView::viewportEntered, &m_tooltipWidget, &KToolTipWidget::hideLater);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TestDialog dialog;
    QObject::connect(&dialog, &QDialog::finished, &app, &QCoreApplication::quit);
    dialog.show();

    return app.exec();
}

#include "moc_ktooltipwidget_test.cpp"
