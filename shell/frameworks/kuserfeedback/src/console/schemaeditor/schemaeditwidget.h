/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_SCHEMAEDITWIDGET_H
#define KUSERFEEDBACK_CONSOLE_SCHEMAEDITWIDGET_H

#include <QWidget>

#include <memory>

namespace KUserFeedback {
namespace Console {

namespace Ui
{
class SchemaEditWidget;
}

class Product;
class RESTClient;
class SchemaModel;

class SchemaEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SchemaEditWidget(QWidget *parent = nullptr);
    ~SchemaEditWidget() override;

    void setRESTClient(RESTClient *client);
    Product product() const;
    void setProduct(const Product &product);

Q_SIGNALS:
    void productChanged();
    void logMessage(const QString &msg);

private:
    void addSource();
    void addSourceEntry();
    void deleteEntry();
    void contextMenu(QPoint pos);

    void updateState();

    QModelIndex currentSource() const;

    std::unique_ptr<Ui::SchemaEditWidget> ui;
    RESTClient *m_restClient;
    SchemaModel *m_schemaModel;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_SCHEMAEDITWIDGET_H
