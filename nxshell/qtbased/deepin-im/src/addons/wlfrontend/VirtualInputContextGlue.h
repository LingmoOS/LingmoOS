// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALINPUTCONTEXTGLUE_H
#define VIRTUALINPUTCONTEXTGLUE_H

#include "VirtualInputContext.h"

namespace org::deepin::dim {

class VirtualInputContextManager;

class VirtualInputContextGlue : public VirtualInputContext
{
    friend class VirtualInputContext;

    Q_OBJECT

public:
    using VirtualInputContext::VirtualInputContext;

    ~VirtualInputContextGlue() override;

    void setVirtualInputContextManager(VirtualInputContextManager *manager)
    {
        virtualICManager_ = manager;
    }

    VirtualInputContext *delegatedInputContext();

    bool realFocus() const
    {
        if (virtualICManager_) {
            return realFocus_;
        }
        return hasFocus();
    }

    void setRealFocus(bool focus) { realFocus_ = focus; }

    void focusInWrapper();
    void focusOutWrapper();
    void updateContentTypeWrapper();
    void updateSurroundingTextWrapper();
    // void setCapabilityFlagsWrapper(CapabilityFlags flags);

protected:
    void updatePreeditImpl(const QString &text, int32_t cursorBegin, int32_t cursorEnd) override;
    void commitStringImpl(const QString &text) override;
    void forwardKeyImpl(uint32_t keycode, bool pressed) override;

    virtual void updatePreeditDelegate(InputContext *ic,
                                       const QString &text,
                                       int32_t cursorBegin,
                                       int32_t cursorEnd) const = 0;
    virtual void commitStringDelegate(InputContext *, const QString &text) const = 0;
    virtual void forwardKeyDelegate(InputContext *, uint32_t keycode, bool pressed) const = 0;
    virtual void commitDelegate() const = 0;
    virtual void deleteSurroundingTextDelegate(InputContext *ic,
                                               int offset,
                                               unsigned int size) const = 0;

private:
    VirtualInputContextManager *virtualICManager_ = nullptr;

    bool realFocus_ = false;
};

} // namespace org::deepin::dim

#endif // !VIRTUALINPUTCONTEXTGLUE_H
