// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "blacklistwidget.h"
#include "blacklistview/blacklistview.h"
#include "blacklistview/deletedialog.h"
#include "blacklistview/blacklistmodel.h"
#include "tipslabel.h"

#include <DWidget>
#include <DFontSizeManager>
#include <DPushButton>

#include <QVBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QItemSelection>

DWIDGET_USE_NAMESPACE

using namespace GrandSearch;

BlackListWidget::BlackListWidget(QWidget *parent)
    :DWidget(parent)
{
    m_groupLabel = new QLabel(tr("Excluded path"), this);
    m_groupLabel->adjustSize();

    m_mainLayout = new QVBoxLayout();
    setLayout(m_mainLayout);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_childHLayout = new QHBoxLayout();
    m_childHLayout->addWidget(m_groupLabel);

    m_contentLabel = new TipsLabel(tr("Add paths to the exclusion list to prevent searching in them."), this);
    m_contentLabel->setWordWrap(true);
    m_childHLayout->addStretch();

    m_deleteButton = new DIconButton(this);
    m_deleteButton->setFixedSize(36, 36);
    DStyle style;
    m_deleteButton->setIcon(style.standardIcon(DStyle::SP_DecreaseElement));
    m_deleteButton->setEnabled(false);
    m_addButton = new DIconButton(this);
    m_addButton->setFixedSize(36, 36);
    m_addButton->setIcon(style.standardIcon(DStyle::SP_IncreaseElement));
    m_childHLayout->addWidget(m_deleteButton);
    m_childHLayout->addSpacerItem(new QSpacerItem(10, 10));
    m_childHLayout->addWidget(m_addButton);
    m_mainLayout->addLayout(m_childHLayout);

    m_mainLayout->addWidget(m_contentLabel);

    m_listWrapper = new BlackListWrapper(this);
    m_mainLayout->addSpacerItem(new QSpacerItem(10, 10));
    m_mainLayout->addWidget(m_listWrapper);

    connect(m_addButton, &DIconButton::clicked, this, &BlackListWidget::addButtonClicked);
    connect(m_deleteButton, &DIconButton::clicked, this, &BlackListWidget::deleteButtonClicked);
    connect(m_listWrapper, &BlackListWrapper::selectedChanged, this, &BlackListWidget::selectedChanged);
}

BlackListWidget::~BlackListWidget()
{

}

void BlackListWidget::addButtonClicked()
{
    QFileDialog fileDialog;
    auto url = fileDialog.getExistingDirectoryUrl(this, QString("")
                                                  , QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    QFileInfo info(url.toLocalFile());
    m_listWrapper->clearSelection();
    if (!url.isEmpty() && !info.isSymLink() && info.exists()) {
        m_listWrapper->addRow(info.absoluteFilePath());
    } else {
        qInfo() << "add path failed";
    }
}

void BlackListWidget::deleteButtonClicked()
{
    DeleteDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        auto selectedRows = m_listWrapper->selectionModel()->selectedRows();
        QList<int> rowList;
        for (auto index : selectedRows) {
            rowList << index.row();
        }
        qStableSort(rowList.begin(), rowList.end(), qGreater<int>());
        for (auto index : rowList) {
            m_listWrapper->removeRows(index, 1);
        }
    }
}

void BlackListWidget::selectedChanged(const QItemSelection &selected, const QItemSelection &deselected) const
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)
    m_deleteButton->setEnabled(!m_listWrapper->selectionModel()->selectedRows().isEmpty());
}
