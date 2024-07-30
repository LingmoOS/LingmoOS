/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007, 2009 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QApplication>
#include <QIcon>
#include <QMainWindow>
#include <QStringListModel>

#include <kcategorizedsortfilterproxymodel.h>
#include <kcategorizedview.h>
#include <kcategorydrawer.h>

QStringList icons;

class MyModel : public QStringListModel
{
public:
    QVariant data(const QModelIndex &index, int role) const override
    {
        switch (role) {
        case KCategorizedSortFilterProxyModel::CategoryDisplayRole: {
            return QString::number(index.row() / 10);
        }
        case KCategorizedSortFilterProxyModel::CategorySortRole: {
            return index.row() / 10;
        }
        case Qt::DecorationRole:
            return QIcon::fromTheme(icons[index.row() % 4]).pixmap(QSize(48, 48));
        default:
            break;
        }
        return QStringListModel::data(index, role);
    }
};

int main(int argc, char **argv)
{
    icons << QStringLiteral("konqueror");
    icons << QStringLiteral("okular");
    icons << QStringLiteral("plasma");
    icons << QStringLiteral("system-file-manager");

    QApplication app(argc, argv);

    QMainWindow *mainWindow = new QMainWindow();
    mainWindow->setMinimumSize(640, 480);
    KCategorizedView *listView = new KCategorizedView();
    listView->setCategoryDrawer(new KCategoryDrawer(listView));
    listView->setViewMode(QListView::IconMode);
    MyModel *model = new MyModel();

    model->insertRows(0, 100);
    for (int i = 0; i < 100; ++i) {
        model->setData(model->index(i, 0), QString::number(i), Qt::DisplayRole);
    }

    KCategorizedSortFilterProxyModel *proxyModel = new KCategorizedSortFilterProxyModel();
    proxyModel->setCategorizedModel(true);
    proxyModel->setSourceModel(model);

    listView->setModel(proxyModel);

    mainWindow->setCentralWidget(listView);

    mainWindow->show();

    return app.exec();
}
