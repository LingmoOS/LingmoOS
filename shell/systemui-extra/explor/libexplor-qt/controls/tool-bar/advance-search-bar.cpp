/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Meihong He <hemeihong@kylinos.cn>
 *
 */

#include "advance-search-bar.h"
#include "fm-window.h"
#include "search-vfs-uri-parser.h"
#include "file-utils.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QStringListModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>

#include <QDebug>

using namespace Peony;

AdvanceSearchBar::AdvanceSearchBar(FMWindowIface *window, QWidget *parent) : QScrollArea(parent)
{
    m_top_window = window;
    init();
}

void AdvanceSearchBar::init()
{
    //add multiple filter page
    //this->setBackgroundRole(QPalette::Light);
    m_filter = new QWidget(this);
    m_filter->setContentsMargins(0, 0, 0, 0);

    QLabel *keyLabel = new QLabel(tr("Key Words"), m_filter);
    m_advanced_key = new QLineEdit(m_filter);
    keyLabel->setBuddy(m_advanced_key);
    m_advanced_key->setPlaceholderText(tr("input key words..."));
    QLabel *searchLocation = new QLabel(tr("Search Location"), m_filter);
    m_search_path = new QLineEdit(m_filter);
    m_search_path->setPlaceholderText(tr("choose search path..."));
    QString uri = m_top_window->getCurrentUri();
    //FIXME: replace BLOCKING api in ui thread.
    m_search_path->setText(FileUtils::getFileDisplayName(uri));
    m_advance_target_path = uri;
    m_choosed_paths.push_back(uri);

    QPushButton *m_browse_button = new QPushButton(tr("browse"), nullptr);
    QLabel *fileType = new QLabel(tr("File Type"), m_filter);
    typeViewCombox = new QComboBox(m_filter);
    typeViewCombox->setToolTip(tr("Choose File Type"));
    auto model = new QStringListModel(m_filter);
    model->setStringList(m_file_type_list);
    typeViewCombox->setModel(model);

    QLabel *modifyTime = new QLabel(tr("Modify Time"), m_filter);
    timeViewCombox = new QComboBox(m_filter);
    timeViewCombox->setToolTip(tr("Choose Modify Time"));
    auto time_model = new QStringListModel(m_filter);
    time_model->setStringList(m_file_mtime_list);
    timeViewCombox->setModel(time_model);

    QLabel *fileSize = new QLabel(tr("File Size"), m_filter);
    sizeViewCombox = new QComboBox(m_filter);
    sizeViewCombox->setToolTip(tr("Choose file size"));
    auto size_model = new QStringListModel(m_filter);
    size_model->setStringList(m_file_size_list);
    sizeViewCombox->setModel(size_model);

    QPushButton *m_show_hidden_button = new QPushButton(tr("show hidden file"), nullptr);
    QPushButton *m_go_back = new QPushButton(tr("go back"), nullptr);
    m_go_back->setToolTip(tr("hidden advance search page"));

    QCheckBox *file_name = new QCheckBox(tr("file name"), nullptr);
    QCheckBox *file_content = new QCheckBox(tr("content"), nullptr);
    file_name->setChecked(true);
    m_search_content = false;
    m_search_name = true;

    QPushButton *m_filter_button = new QPushButton(tr("search"), nullptr);
    m_filter_button->setToolTip(tr("start search"));

    QFormLayout *topLayout = new QFormLayout();
    topLayout->setContentsMargins(10, 10, 10, 10);
    QWidget *b1 = new QWidget(m_filter);
    QHBoxLayout *middleLayout = new QHBoxLayout(b1);
    b1->setLayout(middleLayout);
    QWidget *b2 = new QWidget(m_filter);
    QHBoxLayout *bottomLayout = new QHBoxLayout(b2);
    b2->setLayout(bottomLayout);
    QVBoxLayout *mainLayout = new QVBoxLayout(m_filter);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(middleLayout);
    mainLayout->addLayout(bottomLayout);

    topLayout->addWidget(keyLabel);
    topLayout->addWidget(m_advanced_key);
    topLayout->addWidget(searchLocation);
    topLayout->addWidget(m_search_path);
    middleLayout->addWidget(m_browse_button, Qt::AlignCenter);
    middleLayout->setContentsMargins(10,10,10,10);
    topLayout->addWidget(b1);
    topLayout->addWidget(fileType);
    topLayout->addWidget(typeViewCombox);
    topLayout->addWidget(modifyTime);
    topLayout->addWidget(timeViewCombox);
    topLayout->addWidget(fileSize);
    topLayout->addWidget(sizeViewCombox);
    topLayout->addWidget(m_show_hidden_button);
    topLayout->addWidget(m_go_back);
    topLayout->addWidget(file_name);
    topLayout->addWidget(file_content);
    bottomLayout->setContentsMargins(10,20,10,10);
    bottomLayout->addWidget(m_filter_button, Qt::AlignCenter);
    topLayout->addWidget(b2);

    m_filter->setLayout(mainLayout);
    this->setWidget(m_filter);
    //end multiple filter

    //don't show HorizontalScroll
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(m_browse_button, &QPushButton::clicked, this, &AdvanceSearchBar::browsePath);
    connect(m_filter_button, &QPushButton::clicked, this, &AdvanceSearchBar::searchFilter);
    connect(m_show_hidden_button, &QPushButton::clicked, this, &AdvanceSearchBar::setShowHidden);
    connect(m_search_path, &QLineEdit::textChanged, this, &AdvanceSearchBar::pathChanged);
    connect(typeViewCombox, &QComboBox::currentTextChanged,this, &AdvanceSearchBar::filterUpdate);
    connect(timeViewCombox, &QComboBox::currentTextChanged,this, &AdvanceSearchBar::filterUpdate);
    connect(sizeViewCombox, &QComboBox::currentTextChanged,this, &AdvanceSearchBar::filterUpdate);
    connect(file_name, &QCheckBox::clicked, this, [=]() {
        m_search_name = file_name->isChecked();
        qDebug()<<"search name"<<m_search_name;
    });
    connect(file_content, &QCheckBox::clicked, this, [=]() {
        m_search_content = file_content->isChecked();
        qDebug()<<"search content"<<m_search_content;
    });

    //go back hidden this page
    connect(m_go_back, &QPushButton::clicked, [=]() {
        m_top_window->advanceSearch();
    });
}

void AdvanceSearchBar::setdefaultpath(QString path)
{
    m_choosed_paths.clear();
    m_choosed_paths.push_back(path);
}

void AdvanceSearchBar::browsePath()
{
    QString target_path = QFileDialog::getExistingDirectory(this, tr("Select path"), m_top_window->getCurrentUri(), QFileDialog::ShowDirsOnly);
    //qDebug()<<"browsePath Opened:"<<target_path;
    //add root prefix
    if (! target_path.contains("file://"))
        target_path = "file://" + target_path;

    if (! m_choosed_paths.contains(target_path))
        m_choosed_paths.push_back(target_path);

    //qDebug()<<"selected path:"<<m_choosed_paths;
    updateLocation();
}

void AdvanceSearchBar::searchFilter()
{
    qDebug()<<"searchFilter clicked"<<m_advanced_key->text()<<"path:"<<m_advance_target_path;
    if (m_advanced_key->text() == nullptr || m_advance_target_path == nullptr) //must have key words and target path
    {
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setWindowTitle(tr("Operate Tips"));
        msgBox->setText(tr("Have no key words or search location!"));
        msgBox->exec();
        return;
    }

    if (! m_search_name && !m_search_content)
    {
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setWindowTitle(tr("Operate Tips"));
        msgBox->setText(tr("Search file name or content at least choose one!"));
        msgBox->exec();
        return;
    }

    m_top_window->searchFilter(m_advance_target_path, m_advanced_key->text(), m_search_name, m_search_content);
}

void AdvanceSearchBar::filterUpdate()
{
    m_top_window->filterUpdate(typeViewCombox->currentIndex(), timeViewCombox->currentIndex(), sizeViewCombox->currentIndex());
}

void AdvanceSearchBar::setShowHidden()
{
    m_top_window->setShowHidden(!m_top_window->getWindowShowHidden());
    clearData();
}

void AdvanceSearchBar::pathChanged()
{
    //input space, just return
    if (m_last_show_name == m_search_path->text().trimmed())
        return;
    QList<QString> names = m_search_path->text().split(",");
    //check deleted path,remove from selected list
    for(auto path : m_choosed_paths)
    {
        //FIXME: replace BLOCKING api in ui thread.
        QString show = FileUtils::getFileDisplayName(path);
        if (! names.contains(show))
        {
            m_choosed_paths.removeOne(path);
        }
    }

    qDebug()<<"pathChanged m_choosed_paths:"<<m_choosed_paths;
    updateLocation();
}

void AdvanceSearchBar::updateLocation()
{
    QString show = nullptr;
    for(auto path : m_choosed_paths) {
        if (show == nullptr) {
            //FIXME: replace BLOCKING api in ui thread.
            show = FileUtils::getFileDisplayName(path);
            m_advance_target_path = path;
        }

        else {
            //FIXME: replace BLOCKING api in ui thread.
            show += "," + FileUtils::getFileDisplayName(path);
            m_advance_target_path += "," + path;
        }
    }

    if (m_choosed_paths.size() == 0)
        m_advance_target_path = "";

    m_last_show_name = show;
    m_search_path->setText(show);
}

void AdvanceSearchBar::clearData()
{
    //m_advanced_key->setText("");
    typeViewCombox->setCurrentIndex(0);
    timeViewCombox->setCurrentIndex(0);
    sizeViewCombox->setCurrentIndex(0);
    m_choosed_paths.clear();
}
