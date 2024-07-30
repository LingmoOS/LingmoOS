# KActivitiesStats

Library to access the usage statistics data collected by the KDE activity manager.

## Introduction

The KActivitiesStats library provides a querying mechanism for the data
that the activity manager collects - which documents hae been opened by
which applications, and what documents have been linked to which activity.
The activity manager also keeps the score for each document which gets
higher when a particular document has been often accessed or kept open
for longer periods of time. This score is also available through the
querying mechanism.

## Usage

The library provides the following important classes:

- `KActivities::Stats::ResultSet` is a low level class that provides a forward iterator to the
  list of results that match the specified query
- `KActivities::Stats::ResultWatcher` provides signals when a new resource that matches a query
  arrives, or when an existing one is gone (usage statistics cleared or some
  for other reason)
- `KActivities::Stats::ResultModel` provides a Qt data model that shows the resources that
  match the specified query. This model should be subclassed to teach it
  to handle the different resource types that you want to show as the results.

Queries are defined by the `KActivities::Stats::Query` class using a simple range-like syntax.

