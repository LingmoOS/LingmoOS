// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INPUTPOPUPSURFACEV2_H
#define INPUTPOPUPSURFACEV2_H

#include "wl/client/ZwpInputPopupSurfaceV2.h"

#include <QObject>
#include <QString>

namespace org {
namespace deepin {
namespace dim {

class InputPopupSurfaceV2QObj : public QObject
{
    Q_OBJECT

public:
    InputPopupSurfaceV2QObj();
    ~InputPopupSurfaceV2QObj();

signals:
    void textInputRectangle(int32_t x, int32_t y, int32_t width, int32_t height);
};

class InputPopupSurfaceV2 : public wl::client::ZwpInputPopupSurfaceV2
{
public:
    explicit InputPopupSurfaceV2(zwp_input_popup_surface_v2 *val);
    virtual ~InputPopupSurfaceV2();

    inline InputPopupSurfaceV2QObj *getQObject() const { return qObject_.get(); }

protected:
    void zwp_input_popup_surface_v2_text_input_rectangle(int32_t x,
                                                         int32_t y,
                                                         int32_t width,
                                                         int32_t height) override;

private:
    std::unique_ptr<InputPopupSurfaceV2QObj> qObject_;
};

} // namespace dim
} // namespace deepin
} // namespace org

#endif // !INPUTPOPUPSURFACEV2_H
