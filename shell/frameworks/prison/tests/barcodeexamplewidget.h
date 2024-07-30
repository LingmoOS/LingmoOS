/*
    SPDX-FileCopyrightText: 2010-2014 Sune Vuorela <sune@vuorela.dk>

    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_BARCODEWIDGET_H
#define PRISON_BARCODEWIDGET_H

#include <QWidget>

#include <Prison/Barcode>

/**
 * QWidget with a barcode on
 */
class BarcodeExampleWidget : public QWidget
{
public:
    BarcodeExampleWidget(std::optional<Prison::Barcode> barcode, QWidget *parent = nullptr);
    /**
     * Creates a barcode widget with 'barcode' as barcode generator
     * @param barcode The barcode generator for this widget. Takes ownership over the barcode generator
     * @param parent the parent in QWidget hierarchy
     */
    BarcodeExampleWidget(Prison::BarcodeType barcode, QWidget *parent = nullptr);
    ~BarcodeExampleWidget() override;
    /**
     * sets the data shown to data, and triggers a repaint and resize if needed
     * @param data QString holding the data to be shown
     */
    void setData(const QString &data);
    /**
     * Reimplementation
     * @return minimumSizeHint for this widget
     */
    QSize minimumSizeHint() const override;

protected:
    /**
     * paintEvent
     * @param event QPaintEvent
     */
    void paintEvent(QPaintEvent *event) override;
    /**
     * resizeEvent
     * @param event QResizeEvent
     */
    void resizeEvent(QResizeEvent *event) override;
    /**
     * enables drag from the barcodewidget
     * @param event QMouseEvent
     */
    void mousePressEvent(QMouseEvent *event) override;

private:
    std::optional<Prison::Barcode> m_barcode;
};

#endif // PRISON_BARCODEWIDGET_H
