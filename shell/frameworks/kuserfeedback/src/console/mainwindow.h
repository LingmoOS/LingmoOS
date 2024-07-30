/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_MAINWINDOW_H
#define KUSERFEEDBACK_CONSOLE_MAINWINDOW_H

#include <QMainWindow>
#include <QVector>

#include <memory>

class QAbstractItemModel;
class QNetworkAccessManager;

namespace KUserFeedback {

class Provider;

namespace Console {

namespace Ui
{
class MainWindow;
}

class Product;
class ProductModel;
class ServerInfo;
class RESTClient;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void connectToServer(const ServerInfo &info);

    void createProduct();
    void deleteProduct();
    void productSelected();
    void importProduct();
    void exportProduct();

    void logMessage(const QString &msg);
    void logError(const QString &msg);

    Product selectedProduct() const;

    void updateActions();

    template <typename T> void addView(T *view, QMenu *menu);

    std::unique_ptr<Ui::MainWindow> ui;
    RESTClient *m_restClient;
    ProductModel *m_productModel;

    Provider *m_feedbackProvider;
};
}
}

#endif // KUSERFEEDBACK_CONSOLE_MAINWINDOW_H
