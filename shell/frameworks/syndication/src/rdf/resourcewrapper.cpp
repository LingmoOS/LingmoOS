/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "resourcewrapper.h"
#include "model.h"
#include "resource.h"

namespace Syndication
{
namespace RDF
{
class SYNDICATION_NO_EXPORT ResourceWrapper::ResourceWrapperPrivate
{
public:
    ResourcePtr resource;
    Model model;
};

ResourceWrapper::ResourceWrapper()
    : d(new ResourceWrapperPrivate)
{
    d->resource = ResourcePtr(new Resource());
}

ResourceWrapper::ResourceWrapper(const ResourceWrapper &other)
{
    *this = other;
}

ResourceWrapper::ResourceWrapper(ResourcePtr resource)
    : d(new ResourceWrapperPrivate)
{
    // if a null pointer is passed, we create a null resource
    d->resource = !resource ? ResourcePtr(new Resource()) : resource;
    d->model = d->resource->model();
}

ResourceWrapper::~ResourceWrapper()
{
}

ResourceWrapper &ResourceWrapper::operator=(const ResourceWrapper &other)
{
    d = other.d;
    return *this;
}

bool ResourceWrapper::operator==(const ResourceWrapper &other) const
{
    return *(d->resource) == *(other.d->resource);
}

bool ResourceWrapper::isNull() const
{
    return d->resource->isNull();
}

ResourcePtr ResourceWrapper::resource() const
{
    return d->resource;
}

} // namespace RDF
} // namespace Syndication
