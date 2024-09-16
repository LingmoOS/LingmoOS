// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INPUTMETHODV2_H
#define INPUTMETHODV2_H

#include "wl/client/ZwpInputMethodV2.h"

#include <QObject>
#include <QString>

#include <list>

namespace wl {
namespace client {
class ZwpVirtualKeyboardV1;
}
} // namespace wl

namespace org {
namespace deepin {
namespace dim {

class Dim;

class InputMethodV2QObject : public QObject
{
    Q_OBJECT

signals:
    void activate();
    void deactivate();
    void surroundingText(const char *text, uint32_t cursor, uint32_t anchor);
    void textChangeCause(uint32_t cause);
    void contentType(uint32_t hint, uint32_t purpose);
    void done();
    void unavailable();
};

class InputMethodV2 : public wl::client::ZwpInputMethodV2
{
public:
    explicit InputMethodV2(zwp_input_method_v2 *val,
                           Dim *dim);
    ~InputMethodV2() override;

    InputMethodV2QObject *qobject() { return qobject_.get(); }

protected:
    void zwp_input_method_v2_activate() override;
    void zwp_input_method_v2_deactivate() override;
    void zwp_input_method_v2_surrounding_text(const char *text,
                                              uint32_t cursor,
                                              uint32_t anchor) override;
    void zwp_input_method_v2_text_change_cause(uint32_t cause) override;
    void zwp_input_method_v2_content_type(uint32_t hint, uint32_t purpose) override;
    void zwp_input_method_v2_done() override;
    void zwp_input_method_v2_unavailable() override;

private:
    std::unique_ptr<InputMethodV2QObject> qobject_;
};

} // namespace dim
} // namespace deepin
} // namespace org

#endif // !INPUTMETHODV2_H
