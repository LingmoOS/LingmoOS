// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_ADDONS_BASE_TYPE_H
#define WL_ADDONS_BASE_TYPE_H

#include "../Global.h"
#include "Resource.h"
#include "common/common.h"
#include "wl/Type.h"

#include <wayland-server-core.h>

#include <map>
#include <memory>
#include <optional>
#include <utility>

#include <assert.h>

WL_ADDONS_BASE_BEGIN_NAMESPACE

template<auto F>
struct GlobalCallbackWrapper;

template<typename C, typename R, typename Arg, typename... Args, R (C::*F)(Arg, Args...)>
struct GlobalCallbackWrapper<F>
{
    static R func(Arg farg, void *userdata, Args... args)
    {
        auto *p = reinterpret_cast<C *>(userdata);
        return (p->*F)(farg, args...);
    }
};

template<typename C, typename T>
class Type : public wl::Type<T>
{
public:
    Type() = default;

    virtual ~Type() = default;

    void global(struct wl_display *display)
    {
        global_.reset(wl_global_create(display,
                                       this->wl_interface,
                                       this->wl_interface->version,
                                       this,
                                       GlobalCallbackWrapper<&Type::bindCb>::func));
    }

    void init(struct wl_client *client, uint32_t id)
    {
        assert(this->wl_interface != nullptr);
        auto resource = bind(client, this->wl_interface->version, id);
        resource_ = resource;
    }

    static C *fromResource(struct wl_resource *resource)
    {
        auto *r = static_cast<Resource *>(wl_resource_get_user_data(resource));
        auto *t = static_cast<Type *>(r->object());
        return dynamic_cast<C *>(t);
    }

    const auto getResource() const { return resource_; }

protected:
    virtual void resource_bind(Resource *resource) { }

    virtual void resource_destroy(Resource *resource) { }

private:
    std::unique_ptr<struct wl_global, Deleter<wl_global_destroy>> global_;
    std::shared_ptr<Resource> resource_;

    std::shared_ptr<Resource> bind(struct wl_client *client, uint32_t version, uint32_t id)
    {
        auto resource = std::make_shared<Resource>(client, this->wl_interface, version, id);
        resource->setImplementation(&C::impl,
                                    this,
                                    ResourceDestroyWrapper<&Type::resourceDestroyCb>::func);

        resource_bind(resource.get());

        return resource;
    }

    void bindCb(struct wl_client *client, uint32_t version, uint32_t id)
    {
        auto resource = bind(client, version, id);
        resource_ = resource;
    }

    void resourceDestroyCb(Resource *resource) { }
};

WL_ADDONS_BASE_END_NAMESPACE

#endif // !WL_ADDONS_BASE_TYPE_H
