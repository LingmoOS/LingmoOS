# KItemModels

Set of item models extending the Qt model-view framework

## Introduction

KItemModels provides the following models:

* KBreadcrumbSelectionModel - Selects the parents of selected items to create
  breadcrumbs
* KCheckableProxyModel - Adds a checkable capability to a source model
* KDescendantsProxyModel - Proxy Model for restructuring a Tree into a list
* KExtraColumnsProxyModel - Adds columns after existing columns
* KLinkItemSelectionModel - Share a selection in multiple views which do not
  have the same source model
* KModelIndexProxyMapper - Mapping of indexes and selections through proxy
  models
* KNumberModel - Creates a model of entries from N to M with rows at a given interval
* KRearrangeColumnsProxyModel - Can reorder and hide columns from the source model
* KRoleNames - Attached property to map between roles and role names of a model
* KSelectionProxyModel - A Proxy Model which presents a subset of its source
  model to observers
