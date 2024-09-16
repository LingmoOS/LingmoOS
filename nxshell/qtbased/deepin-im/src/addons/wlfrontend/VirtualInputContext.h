// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALINPUTCONTEXT_H
#define VIRTUALINPUTCONTEXT_H

#include <dimcore/InputContext.h>

#include <QPoint>

namespace org::deepin::dim {

class VirtualInputContextGlue;

class VirtualInputContext : public InputContext
{
    Q_OBJECT

public:
    VirtualInputContext(VirtualInputContextGlue *parentIC, Dim *dim);
    ~VirtualInputContext();

    VirtualInputContextGlue *parentIC() { return parentIC_; }

    void setWindowPos(const QPoint &leftTop);

    const QPoint &windowPos() { return leftTop_; }

protected:
    void updatePreeditImpl(const QString &text, int32_t cursorBegin, int32_t cursorEnd) override;
    void commitStringImpl(const QString &text) override;
    void commitImpl() override;
    void forwardKeyImpl(uint32_t keycode, bool pressed) override;

protected:
    QPoint leftTop_;

private:
    VirtualInputContextGlue *parentIC_;
};

} // namespace org::deepin::dim

#endif // !VIRTUALINPUTCONTEXT_H
