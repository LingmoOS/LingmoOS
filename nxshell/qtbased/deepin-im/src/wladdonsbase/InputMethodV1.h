// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INPUTMETHODV1_H
#define INPUTMETHODV1_H

#include "Listener.h"
#include "inputmethodv1/ZwpInputMethodV1.h"
#include "inputmethodv1/ZwpInputPanelV1.h"

#include <functional>

WL_ADDONS_BASE_BEGIN_NAMESPACE

class Server;
class InputMethodContextV1;

typedef std::function<void()> Cb;

class InputMethodV1 : public ZwpInputMethodV1
{
public:
    explicit InputMethodV1(Server *server);
    ~InputMethodV1() override;

    inline Server *server() const { return server_; }

    void sendActivate();
    void sendDeactivate();

    const auto &inputMethodContextV1() const { return inputMethodContextV1_; }

    std::function<void(uint32_t serial, const char *text)> inputMethodContextV1CommitCallback_;
    std::function<void(uint32_t serial, const char *text, const char *commit)>
        inputMethodContextV1PreeditCallback_;
    std::function<void(uint32_t key, uint32_t state)> inputMethodContextV1ForwardKeyCallback_;
    Cb inputPanelV1CreateCallback_;
    Cb inputPanelV1DestoryCallback_;

private:
    Server *server_;
    std::unique_ptr<ZwpInputPanelV1> inputPanelV1_ = nullptr;
    std::shared_ptr<InputMethodContextV1> inputMethodContextV1_ = nullptr;
};

WL_ADDONS_BASE_END_NAMESPACE

#endif // !INPUTMETHODV1_H
