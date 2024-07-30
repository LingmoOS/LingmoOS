/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "analyticsview.h"
#include "ui_analyticsview.h"

#include "aggregator.h"
#include "categoryaggregator.h"
#include "chartexportdialog.h"
#include "chartutil.h"
#include "numericaggregator.h"
#include "ratiosetaggregator.h"
#include "totalaggregator.h"

#include <model/aggregateddatamodel.h>
#include <model/datamodel.h>
#include <model/timeaggregationmodel.h>
#include <rest/restapi.h>
#include <core/aggregation.h>
#include <core/sample.h>

#include <QtCharts/QChart>

#include <QActionGroup>
#include <QClipboard>
#include <QFile>
#include <QFileDialog>
#include <QImage>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkReply>
#include <QPdfWriter>
#include <QSettings>
#include <QSvgGenerator>

using namespace KUserFeedback::Console;

AnalyticsView::AnalyticsView(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::AnalyticsView),
    m_dataModel(new DataModel(this)),
    m_timeAggregationModel(new TimeAggregationModel(this)),
    m_aggregatedDataModel(new AggregatedDataModel(this)),
    m_nullSingularChart(new QChart),
    m_nullTimelineChart(new QChart)
{
    ui->setupUi(this);

    ChartUtil::applyTheme(m_nullSingularChart.get());
    ChartUtil::applyTheme(m_nullTimelineChart.get());
    ui->singularChartView->setChart(m_nullSingularChart.get());
    ui->timelineChartView->setChart(m_nullTimelineChart.get());

    ui->dataView->setModel(m_dataModel);
    ui->aggregatedDataView->setModel(m_aggregatedDataModel);

    m_timeAggregationModel->setSourceModel(m_dataModel);
    connect(m_timeAggregationModel, &QAbstractItemModel::modelReset, this, &AnalyticsView::updateTimeSliderRange);

    ui->actionAggregateYear->setData(TimeAggregationModel::AggregateYear);
    ui->actionAggregateMonth->setData(TimeAggregationModel::AggregateMonth);
    ui->actionAggregateWeek->setData(TimeAggregationModel::AggregateWeek);
    ui->actionAggregateDay->setData(TimeAggregationModel::AggregateDay);
    auto aggrGroup = new QActionGroup(this);
    aggrGroup->addAction(ui->actionAggregateYear);
    aggrGroup->addAction(ui->actionAggregateMonth);
    aggrGroup->addAction(ui->actionAggregateWeek);
    aggrGroup->addAction(ui->actionAggregateDay);
    aggrGroup->setExclusive(true);
    connect(aggrGroup, &QActionGroup::triggered, this, [this, aggrGroup]() {
        m_timeAggregationModel->setAggregationMode(static_cast<TimeAggregationModel::AggregationMode>(aggrGroup->checkedAction()->data().toInt()));
    });

    auto timeAggrMenu = new QMenu(tr("&Time Interval"), this);
    timeAggrMenu->addAction(ui->actionAggregateDay);
    timeAggrMenu->addAction(ui->actionAggregateWeek);
    timeAggrMenu->addAction(ui->actionAggregateMonth);
    timeAggrMenu->addAction(ui->actionAggregateYear);

    auto chartModeGroup = new QActionGroup(this);
    chartModeGroup->addAction(ui->actionSingularChart);
    chartModeGroup->addAction(ui->actionTimelineChart);
    connect(chartModeGroup, &QActionGroup::triggered, this, &AnalyticsView::updateChart);

    auto chartMode = new QMenu(tr("&Chart Mode"), this);
    chartMode->addAction(ui->actionSingularChart);
    chartMode->addAction(ui->actionTimelineChart);

    ui->actionReload->setShortcut(QKeySequence::Refresh);
    connect(ui->actionReload, &QAction::triggered, m_dataModel, &DataModel::reload);
    connect(ui->actionExportChart, &QAction::triggered, this, &AnalyticsView::exportChart);
    connect(ui->actionExportData, &QAction::triggered, this, &AnalyticsView::exportData);
    connect(ui->actionImportData, &QAction::triggered, this, &AnalyticsView::importData);

    addActions({
        timeAggrMenu->menuAction(),
        chartMode->menuAction(),
        ui->actionReload,
        ui->actionExportChart,
        ui->actionExportData,
        ui->actionImportData
    });

    QSettings settings;
    settings.beginGroup(QStringLiteral("Analytics"));
    const auto aggrSetting = settings.value(QStringLiteral("TimeAggregationMode"), TimeAggregationModel::AggregateMonth).toInt();
    foreach (auto act, aggrGroup->actions())
        act->setChecked(act->data().toInt() == aggrSetting);
    m_timeAggregationModel->setAggregationMode(static_cast<TimeAggregationModel::AggregationMode>(aggrSetting));
    settings.endGroup();

    connect(ui->chartType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AnalyticsView::chartSelected);
    connect(ui->timeSlider, &QSlider::valueChanged, this, [this](int value) {
        auto aggr = ui->chartType->currentData().value<Aggregator*>();
        if (!aggr)
            return;
        aggr->setSingularTime(value);
        ui->timeLabel->setText(aggr->singularAggregationModel()->index(0, 0).data(TimeAggregationModel::TimeDisplayRole).toString());
    });

    connect(ui->dataView, &QWidget::customContextMenuRequested, this, [this](QPoint pos) {
        QMenu menu;
        menu.addAction(QIcon::fromTheme(QStringLiteral("edit-copy")), tr("Copy"), [this, pos]() {
            const auto idx = ui->dataView->indexAt(pos);
            QGuiApplication::clipboard()->setText(idx.data().toString());
        });
        menu.exec(ui->dataView->viewport()->mapToGlobal(pos));
    });
}

AnalyticsView::~AnalyticsView()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("Analytics"));
    settings.setValue(QStringLiteral("TimeAggregationMode"), m_timeAggregationModel->aggregationMode());
    settings.endGroup();

    // the chart views can't handle null or deleted charts, so set them to something safe
    ui->singularChartView->setChart(m_nullSingularChart.get());
    ui->timelineChartView->setChart(m_nullTimelineChart.get());
    qDeleteAll(m_aggregators);
}

void AnalyticsView::setRESTClient(RESTClient* client)
{
    m_client = client;
    m_dataModel->setRESTClient(client);
}

void AnalyticsView::setProduct(const Product& product)
{
    // the chart views can't handle null or deleted charts, so set them to something safe
    ui->singularChartView->setChart(m_nullSingularChart.get());
    ui->timelineChartView->setChart(m_nullTimelineChart.get());

    m_dataModel->setProduct(product);

    ui->chartType->clear();
    m_aggregatedDataModel->clear();

    qDeleteAll(m_aggregators);
    m_aggregators.clear();

    m_aggregatedDataModel->addSourceModel(m_timeAggregationModel);
    auto totalsAggr = new TotalAggregator;
    m_aggregators.push_back(totalsAggr);
    totalsAggr->setSourceModel(m_timeAggregationModel);
    ui->chartType->addItem(totalsAggr->displayName(), QVariant::fromValue<Aggregator*>(totalsAggr));

    foreach (const auto &aggr, product.aggregations()) {
        auto aggregator = createAggregator(aggr);
        if (!aggregator)
            continue;
        m_aggregators.push_back(aggregator);
        if (auto model = aggregator->timeAggregationModel()) {
            m_aggregatedDataModel->addSourceModel(model, aggregator->displayName());
        }
        if (aggregator->chartModes() != Aggregator::None)
            ui->chartType->addItem(aggregator->displayName(), QVariant::fromValue(aggregator));
    }
}

void AnalyticsView::chartSelected()
{
    auto aggr = ui->chartType->currentData().value<Aggregator*>();
    if (!aggr)
        return;

    const auto chartMode = aggr->chartModes();
    ui->actionSingularChart->setEnabled(chartMode & Aggregator::Singular);
    ui->actionTimelineChart->setEnabled(chartMode & Aggregator::Timeline);
    if (chartMode != (Aggregator::Timeline | Aggregator::Singular)) {
        ui->actionSingularChart->setChecked(chartMode & Aggregator::Singular);
        ui->actionTimelineChart->setChecked(chartMode & Aggregator::Timeline);
    }

    updateChart();
}

void AnalyticsView::updateChart()
{
    auto aggr = ui->chartType->currentData().value<Aggregator*>();
    if (!aggr)
        return;

    if (ui->actionTimelineChart->isChecked()) {
        ui->timelineChartView->setChart(aggr->timelineChart());
        ui->chartStack->setCurrentWidget(ui->timelinePage);
    } else if (ui->actionSingularChart->isChecked()) {
        ui->singularChartView->setChart(aggr->singlularChart());
        ui->chartStack->setCurrentWidget(ui->singularPage);
        aggr->setSingularTime(ui->timeSlider->value());
    }
}

void AnalyticsView::updateTimeSliderRange()
{
    if (m_timeAggregationModel->rowCount() > 0) {
        ui->timeSlider->setRange(0, m_timeAggregationModel->rowCount() - 1);
        ui->timeLabel->setText(m_timeAggregationModel->index(ui->timeSlider->value(), 0).data(TimeAggregationModel::TimeDisplayRole).toString());
        auto aggr = ui->chartType->currentData().value<Aggregator*>();
        if (aggr)
            aggr->setSingularTime(ui->timeSlider->value());
    }
}

Aggregator* AnalyticsView::createAggregator(const Aggregation& aggr) const
{
    Aggregator *aggregator = nullptr;

    switch (aggr.type()) {
        case Aggregation::None:
            break;
        case Aggregation::Category:
            aggregator = new CategoryAggregator;
            break;
        case Aggregation::Numeric:
            aggregator = new NumericAggregator;
            break;
        case Aggregation::RatioSet:
            aggregator = new RatioSetAggregator;
            break;
    }

    if (!aggregator)
        return nullptr;

    aggregator->setAggregation(aggr);
    aggregator->setSourceModel(m_timeAggregationModel);
    return aggregator;
}

void AnalyticsView::exportData()
{
    const auto fileName = QFileDialog::getSaveFileName(this, tr("Export Data"));
    if (fileName.isEmpty())
        return;

    QFile f(fileName);
    if (!f.open(QFile::WriteOnly)) {
        QMessageBox::critical(this, tr("Export Failed"), tr("Could not open file: %1").arg(f.errorString()));
        return;
    }

    const auto samples = m_dataModel->index(0, 0).data(DataModel::AllSamplesRole).value<QVector<Sample>>();
    f.write(Sample::toJson(samples, m_dataModel->product()));
    Q_EMIT logMessage(tr("Data samples of %1 exported to %2.").arg(m_dataModel->product().name(), f.fileName()));
}

void AnalyticsView::importData()
{
    const auto fileName = QFileDialog::getOpenFileName(this, tr("Import Data"));
    if (fileName.isEmpty())
        return;

    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        QMessageBox::critical(this, tr("Import Failed"), tr("Could not open file: %1").arg(f.errorString()));
        return;
    }
    const auto samples = Sample::fromJson(f.readAll(), m_dataModel->product());
    if (samples.isEmpty()) {
        QMessageBox::critical(this, tr("Import Failed"), tr("Selected file contains no valid data."));
        return;
    }

    auto reply = RESTApi::addSamples(m_client, m_dataModel->product(), samples);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            Q_EMIT logMessage(tr("Data samples imported."));
            m_dataModel->reload();
        }
    });
}

void AnalyticsView::exportChart()
{
    ChartExportDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    QChart *chart = nullptr;
    QGraphicsScene *scene = nullptr;
    if (ui->actionTimelineChart->isChecked()) {
        chart = ui->timelineChartView->chart();
        scene = ui->timelineChartView->scene();
    } else if (ui->actionSingularChart->isChecked()) {
        chart = ui->singularChartView->chart();
        scene = ui->singularChartView->scene();
    }
    Q_ASSERT(chart);
    Q_ASSERT(scene);

    chart->setTheme(QChart::ChartThemeLight);

    switch (dlg.type()) {
        case ChartExportDialog::Image:
        {
            QImage img(dlg.size(), QImage::Format_ARGB32_Premultiplied);
            img.fill(Qt::transparent);
            QPainter p(&img);
            p.setRenderHint(QPainter::Antialiasing);
            scene->render(&p, QRectF(QPoint(), dlg.size()), scene->sceneRect());
            img.save(dlg.filename());
            break;
        }
        case ChartExportDialog::SVG:
        {
            QSvgGenerator svg;
            svg.setFileName(dlg.filename());
            svg.setSize(scene->sceneRect().size().toSize());
            svg.setViewBox(scene->sceneRect());
            svg.setTitle(ui->chartType->currentText());

            QPainter p(&svg);
            p.setRenderHint(QPainter::Antialiasing);
            scene->render(&p);
            break;
        }
        case ChartExportDialog::PDF:
        {
            QPdfWriter pdf(dlg.filename());
            pdf.setCreator(QStringLiteral("UserFeedbackConsole"));
            pdf.setTitle(ui->chartType->currentText());
            if (scene->sceneRect().width() > scene->sceneRect().height())
                pdf.setPageOrientation(QPageLayout::Landscape);

            QPainter p(&pdf);
            p.setRenderHint(QPainter::Antialiasing);
            scene->render(&p);
            break;
        }
    }

    ChartUtil::applyTheme(chart);
}

#include "moc_analyticsview.cpp"
