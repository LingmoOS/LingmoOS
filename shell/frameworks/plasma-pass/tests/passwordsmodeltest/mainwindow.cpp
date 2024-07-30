// SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "mainwindow.h"
#include "passwordfiltermodel.h"
#include "passwordprovider.h"
#include "passwordsmodel.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QProgressBar>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QTreeView>
#include <QVBoxLayout>

using namespace PlasmaPass;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(900, 350); // NOLINT(cppcoreguidelines-avoid-magic-numbers)

    auto w = new QWidget;
    setCentralWidget(w);

    auto h = new QHBoxLayout(w);

    auto splitter = new QSplitter;
    h->addWidget(splitter);

    w = new QWidget;
    splitter->addWidget(w);

    auto v = new QVBoxLayout(w);

    auto input = new QLineEdit;
    input->setClearButtonEnabled(true);
    input->setPlaceholderText(QStringLiteral("Search ..."));
    connect(input, &QLineEdit::textChanged, this, &MainWindow::onSearchChanged);
    v->addWidget(input);

    mStack = new QStackedWidget;
    v->addWidget(mStack);

    auto treeView = new QTreeView;
    treeView->setHeaderHidden(true);
    treeView->setModel(new PasswordsModel(this));
    connect(treeView, &QTreeView::clicked, this, &MainWindow::onPasswordClicked);
    mStack->addWidget(treeView);

    auto listView = new QListView;
    mFilterModel = new PasswordFilterModel(listView);
    mFilterModel->setSourceModel(treeView->model());
    listView->setModel(mFilterModel);
    connect(listView, &QListView::clicked, this, &MainWindow::onPasswordClicked);
    mStack->addWidget(listView);

    mStack->setCurrentIndex(0);

    w = new QWidget;
    splitter->addWidget(w);

    v = new QVBoxLayout(w);

    v->addWidget(mTitle = new QLabel);
    auto font = mTitle->font();
    font.setBold(true);
    mTitle->setFont(font);

    auto g = new QFormLayout;
    v->addLayout(g);
    g->addRow(QStringLiteral("Path:"), mPath = new QLabel());
    g->addRow(QStringLiteral("Type:"), mType = new QLabel());
    g->addRow(QStringLiteral("Password:"), mPassword = new QLabel());
    g->addRow(QStringLiteral("Expiration:"), mPassProgress = new QProgressBar());
    g->addRow(QStringLiteral("Error:"), mError = new QLabel());
    mPassProgress->setTextVisible(false);

    v->addWidget(mPassBtn = new QPushButton(QStringLiteral("Display Password")));
    connect(mPassBtn, &QPushButton::clicked, this, [this]() {
        setProvider(mCurrent.data(PasswordsModel::PasswordRole).value<PasswordProvider *>());
    });

    v->addStretch(2.0); // NOLINT(cppcoreguidelines-avoid-magic-numbers)

    onPasswordClicked({});
}

MainWindow::~MainWindow() = default;

void MainWindow::setProvider(ProviderBase *provider)
{
    mProvider = provider;
    if (provider->isValid()) {
        mPassBtn->setVisible(false);
        mPassword->setVisible(true);
        mPassword->setText(provider->secret());
    }
    if (provider->hasError()) {
        mError->setVisible(true);
        mError->setText(provider->error());
    }

    connect(provider, &ProviderBase::secretChanged, this, [this, provider]() {
        const auto pass = provider->secret();
        if (!pass.isEmpty()) {
            mPassword->setVisible(true);
            mPassword->setText(provider->secret());
        } else {
            onPasswordClicked(mCurrent);
        }
    });
    connect(provider, &ProviderBase::timeoutChanged, this, [this, provider]() {
        mPassProgress->setVisible(true);
        mPassProgress->setMaximum(provider->defaultTimeout());
        mPassProgress->setValue(provider->timeout());
    });
    connect(provider, &ProviderBase::errorChanged, this, [this, provider]() {
        mError->setVisible(true);
        mError->setText(provider->error());
    });
}

void MainWindow::onPasswordClicked(const QModelIndex &idx)
{
    if (mProvider != nullptr) {
        mProvider->disconnect(this);
    }
    mCurrent = idx;
    mTitle->setText(idx.data(PasswordsModel::NameRole).toString());
    mPath->setText(idx.data(PasswordsModel::PathRole).toString());
    const auto type = idx.isValid() ? static_cast<PasswordsModel::EntryType>(idx.data(PasswordsModel::EntryTypeRole).toInt()) : PasswordsModel::FolderEntry;
    mType->setText(type == PasswordsModel::PasswordEntry ? QStringLiteral("Password") : QStringLiteral("Folder"));
    mPassword->setVisible(false);
    mPassword->clear();
    mPassBtn->setEnabled(type == PasswordsModel::PasswordEntry);
    mPassBtn->setVisible(true);
    mPassProgress->setVisible(false);
    mError->clear();
    mError->setVisible(false);

    const auto hasProvider = mCurrent.data(PasswordsModel::HasPasswordRole).toBool();
    if (hasProvider) {
        setProvider(mCurrent.data(PasswordsModel::PasswordRole).value<PasswordProvider*>());
    }
}

void MainWindow::onSearchChanged(const QString &text)
{
    mStack->setCurrentIndex(text.isEmpty() ? 0 : 1);
    mFilterModel->setPasswordFilter(text);
}

#include "moc_mainwindow.cpp"
