/*
    SPDX-FileCopyrightText: 2010-2016 Sune Vuorela <sune@vuorela.dk>

    SPDX-License-Identifier: MIT
*/

#include "prisontest.h"

#include "barcodeexamplewidget.h"
// Prison
#include <Prison/Barcode>
// Qt
#include <QDebug>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>

void main_window::data_changed()
{
    QString result = m_lineedit->text();
    m_dmw->setData(result);
    m_qrw->setData(result);
    m_39w->setData(result);
    m_93w->setData(result);
    m_dmcolor->setData(result);
    m_qrcolor->setData(result);
    m_39color->setData(result);
    m_93color->setData(result);
    m_nullw->setData(result);
}

main_window::main_window()
{
    QHBoxLayout *lay = new QHBoxLayout();
    m_lineedit = new QLineEdit(this);
    QPushButton *but = new QPushButton(this);
    connect(but, &QPushButton::clicked, this, &main_window::data_changed);
    lay->addWidget(m_lineedit);
    lay->addWidget(but);

    QVBoxLayout *mainlay = new QVBoxLayout(this);

    m_dmw = new BarcodeExampleWidget(Prison::DataMatrix, this);
    m_qrw = new BarcodeExampleWidget(Prison::QRCode, this);
    m_39w = new BarcodeExampleWidget(Prison::Code39, this);
    m_93w = new BarcodeExampleWidget(Prison::Code93, this);
    {
        auto dmcolorcode = Prison::Barcode::create(Prison::DataMatrix);
        if (dmcolorcode) {
            dmcolorcode->setForegroundColor(Qt::red);
            dmcolorcode->setBackgroundColor(Qt::darkBlue);
            m_dmcolor = new BarcodeExampleWidget(std::move(dmcolorcode), this);
        }
    }
    {
        auto qrcolorcode = Prison::Barcode::create(Prison::QRCode);
        if (qrcolorcode) {
            qrcolorcode->setForegroundColor(Qt::red);
            qrcolorcode->setBackgroundColor(Qt::darkBlue);
        }
        m_qrcolor = new BarcodeExampleWidget(std::move(qrcolorcode), this);
    }
    {
        auto c39colorcode = Prison::Barcode::create(Prison::Code39);
        if (c39colorcode) {
            c39colorcode->setForegroundColor(Qt::red);
            c39colorcode->setBackgroundColor(Qt::darkBlue);
        }
        m_39color = new BarcodeExampleWidget(std::move(c39colorcode), this);
    }
    {
        auto c93colorcode = Prison::Barcode::create(Prison::Code93);
        if (c93colorcode) {
            c93colorcode->setForegroundColor(Qt::red);
            c93colorcode->setBackgroundColor(Qt::darkBlue);
        }
        m_93color = new BarcodeExampleWidget(std::move(c93colorcode), this);
    }

    m_nullw = new BarcodeExampleWidget(std::nullopt, this);

    QSplitter *splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(m_dmw);
    splitter->addWidget(m_qrw);
    splitter->addWidget(m_39w);
    splitter->addWidget(m_93w);
    splitter->addWidget(m_dmcolor);
    splitter->addWidget(m_qrcolor);
    splitter->addWidget(m_39color);
    splitter->addWidget(m_93color);
    splitter->addWidget(m_nullw);

    mainlay->addLayout(lay);
    mainlay->addWidget(splitter);

    m_lineedit->setText(QStringLiteral("AOEUIAOEUIAOEUI"));
    data_changed();
}

#include "moc_prisontest.cpp"
