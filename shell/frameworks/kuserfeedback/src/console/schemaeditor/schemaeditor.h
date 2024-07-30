/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_SCHEMAEDITOR_H
#define KUSERFEEDBACK_CONSOLE_SCHEMAEDITOR_H

#include <QWidget>

#include <memory>

class QAction;

namespace KUserFeedback {
namespace Console {

class Product;
class RESTClient;

namespace Ui {
class SchemaEditor;
}

class SchemaEditor : public QWidget
{
    Q_OBJECT
public:
    explicit SchemaEditor(QWidget *parent = nullptr);
    ~SchemaEditor() override;

    void setRESTClient(RESTClient *client);

    Product product() const;
    void setProduct(const Product &product);

    bool isDirty() const;

Q_SIGNALS:
    void productChanged(const Product &product);
    void logMessage(const QString &msg);

private:
    void save();
    void exportSchema();
    void importSchema();

    void setDirty(bool dirty = true);
    void updateState();

    std::unique_ptr<Ui::SchemaEditor> ui;
    RESTClient *m_restClient = nullptr;
    QAction *m_createFromTemplateAction;
    bool m_isDirty = false;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_SCHEMAEDITOR_H
