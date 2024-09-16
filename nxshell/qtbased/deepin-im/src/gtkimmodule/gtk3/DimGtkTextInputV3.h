// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DIM_TEXT_INPUT_V1_H
#define DIM_TEXT_INPUT_V1_H

#include "imcontext.h"
#include "wl/client/ZwpTextInputV3.h"

class DimGtkTextInputV3 : public wl::client::ZwpTextInputV3
{
public:
    explicit DimGtkTextInputV3(struct ::zwp_text_input_v3 *text_input,
                               DimIMContextWaylandGlobal *global);
    ~DimGtkTextInputV3();

    inline void text_input_preedit(const char *text, int32_t cursor_begin, int32_t cursor_end)
    {
        return zwp_text_input_v3_preedit_string(text, cursor_begin, cursor_end);
    }

protected:
    void zwp_text_input_v3_enter(struct wl_surface *surface) override;
    void zwp_text_input_v3_leave(struct wl_surface *surface) override;
    void zwp_text_input_v3_preedit_string(const char *text,
                                          int32_t cursor_begin,
                                          int32_t cursor_end) override;
    void zwp_text_input_v3_commit_string(const char *text) override;
    void zwp_text_input_v3_delete_surrounding_text(uint32_t before_length,
                                                   uint32_t after_length) override;
    void zwp_text_input_v3_done(uint32_t serial) override;

private:
    DimIMContextWaylandGlobal *global_;
    wl_surface *m_surface = nullptr;
};

#endif // DIM_TEXT_INPUT_V1_H
