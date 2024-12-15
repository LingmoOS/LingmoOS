// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "appchooserdialog.h"
#include "appchoosermodel.h"
#include "appchooserdelegate.h"

#include <QListView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

AppChooserDialog::AppChooserDialog(QWidget *parent)
    : QDialog(parent)
    , m_view(new QListView(this))
    , m_cancelBtn(new QPushButton(tr("Cancel"), this))
    , m_confirmBtn(new QPushButton(tr("Confirm"), this))
{
    m_view->setFlow(QListView::LeftToRight);
    m_view->setWrapping(true);
    m_view->setResizeMode(QListView::Adjust);
    m_view->setSelectionMode(QListView::MultiSelection);
    m_view->setSelectionRectVisible(true);
    m_view->setAlternatingRowColors(true);
    m_view->setEditTriggers(QListView::NoEditTriggers);
    connect(m_view, &QListView::clicked, this, [ = ] (const QModelIndex &index) {
        if (auto model = qobject_cast<AppChooserModel *>(m_view->model()))
            model->click(index);
    });

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setAlignment(Qt::AlignRight);
    btnLayout->addWidget(m_cancelBtn);
    btnLayout->addWidget(m_confirmBtn);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_view);
    layout->addLayout(btnLayout, 0);
    m_view->setModel(new AppChooserModel);
    m_view->setItemDelegate(new AppChooserDelegate);

    connect(m_confirmBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    setMinimumSize(750, 300);
}

QStringList AppChooserDialog::selectChoices()
{
    auto model = qobject_cast<AppChooserModel *>(m_view->model());
    if (model) {
        return model->choices();
    }
    return QStringList();
}

void AppChooserDialog::updateChoices(const QStringList &choices)
{
    m_choices = choices;

    // unused
}

void AppChooserDialog::setCurrentChoice(const QString &choice)
{
    auto model = qobject_cast<AppChooserModel *>(m_view->model());
    if (model) {
        model->click(choice);
    }
}
