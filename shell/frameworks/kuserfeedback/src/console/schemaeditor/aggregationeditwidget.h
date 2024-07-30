/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_AGGREGATIONEDITWIDGET_H
#define KUSERFEEDBACK_CONSOLE_AGGREGATIONEDITWIDGET_H

#include <QWidget>

#include <memory>

namespace KUserFeedback {
namespace Console {

class Aggregation;
class AggregationEditorModel;
class AggregationElementEditModel;
class Product;
class SchemaEntryItemEditorFactory;

namespace Ui
{
class AggregationEditWidget;
}

class AggregationEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AggregationEditWidget(QWidget *parent = nullptr);
    ~AggregationEditWidget() override;

    Product product() const;
    void setProduct(const Product &product);

Q_SIGNALS:
    void productChanged();

private:
    Aggregation currentAggregation() const;
    void addAggregation();
    void deleteAggregation();
    void addElement();
    void deleteElement();

    void updateState();
    void contextMenu(QPoint pos);
    void selectionChanged();

    std::unique_ptr<Ui::AggregationEditWidget> ui;
    AggregationEditorModel *m_model;
    std::unique_ptr<SchemaEntryItemEditorFactory> m_editorFactory;
    AggregationElementEditModel *m_elementModel;
};
}
}

#endif // KUSERFEEDBACK_CONSOLE_AGGREGATIONEDITWIDGET_H
