# KQuickCharts

A QtQuick module providing high-performance charts.

## Introduction

The Quick Charts module provides a set of charts that can be used from QtQuick
applications. They are intended to be used for both simple display of data as
well as continuous display of high-volume data (often referred to as plotters).
The charts use a system called distance fields for their accelerated rendering,
which provides ways of using the GPU for rendering 2D shapes without loss of
quality.

### Usage Example

The following piece of code will render a simple line chart containing three
lines:

\snippet snippets/minimal.qml example

## Concepts

There are three main concepts to consider when using this module: charts, data
sources and decorations.

### Charts

These are the main items of the module. They process data and render it in a
certain way. Currently there are three main types of charts: pie charts, line
charts and bar charts. All charts inherit the [Chart] class, which provides the
most basic chart interface. On top of that is the [XYChart] base class, which
provides an interface for charts that are based on an X/Y grid.

* [PieChart](\ref PieChart)
* [LineChart](\ref LineChart)
* [BarChart](\ref BarChart)

[Chart]: \ref Chart
[XYChart]: \ref XYChart

### Data Sources

Data sources are objects that provide data to charts. These objects act as
adapters to other objects, like Qt's models. All data source objects inherit
[ChartDataSource], which represents the basic data source interface.

* [SingleValueSource](\ref SingleValueSource)
* [ArraySource](\ref ArraySource)
* [ModelSource](\ref ModelSource)
* [ValueHistorySource](\ref ValueHistorySource)
* [ModelHistorySource](\ref ModelHistorySource)
* [ColorGradientSource](\ref ColorGradientSource)
* [ChartAxisSource](\ref ChartAxisSource)

[ChartDataSource]: \ref ChartDataSource

### Decorations

Decorations are items that provide extra information about a chart. These are
things like a legend or axis labels. They usually work with multiple types of
charts, though some may be limited to X/Y charts.

* [AxisLabels](\ref AxisLabels)
* [GridLines](\ref GridLines)
* [LegendModel](\ref LegendModel)

## Controls

A submodule is provided that contains pre-made controls that are provided as a
convenience layer. These controls may be more restrictive in their data sources
or how they display things. They are composed of Charts items along with some
QtQuick Controls items. Some of the controls may use style-specific theming.

* [Legend](\ref org::kde::quickcharts::controls::Legend)
* [LegendDelegate](\ref org::kde::quickcharts::controls::LegendDelegate)
* [LineChartControl](\ref org::kde::quickcharts::controls::LineChartControl)
* [PieChartControl](\ref org::kde::quickcharts::controls::PieChartControl)

## Supporting Code

There are a number of classes and other pieces of code that are not considered
public API and thus not part of the public API documentation. Primarily, these
are all the QtQuick Scene Graph related classes located in `src/scenegraph/`,
in addition to the shaders that are used for rendering charts.
