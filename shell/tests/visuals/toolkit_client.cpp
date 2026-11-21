// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <chrono>
#include <cmath>
#include <memory>
#include <string>

#include "base/event_loop.h"
#include "ui/application.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/view.h"
#include "ui/window.h"

using namespace Lingmo::Ui;

// A frameless client-decorated toolkit window, as the session's applications
// are: its size centred in white, and with --scroll a list that scrolls by 9
// px every 50 ms, so every frame carries a small damage region.
class Content : public View
{
public:
    explicit Content(bool scrolling)
        : m_scrolling(scrolling)
        , m_size(addChild<Label>())
    {
        m_size->setTextAlignment(Label::TextAlignment::Center);
    }

    void scrollBy(double distance)
    {
        m_contentY = std::fmod(m_contentY + distance, 1000);
        requestPaint();
    }

protected:
    void layout() override
    {
        const RectF bounds = localRect();
        m_size->setText(std::to_string(int(bounds.width)) + " x " + std::to_string(int(bounds.height)));
        const SizeF hint = m_size->sizeHint();
        m_size->setGeometry({(bounds.width - hint.width) / 2, (bounds.height - hint.height) / 2,
                             hint.width, hint.height});
    }

    void paint(Painter &painter) override
    {
        painter.fillRect(localRect(), Color::rgb(0xFFFFFF));
        if (!m_scrolling)
            return;
        const RectF list(10, 60, localRect().width - 20, localRect().height - 70);
        painter.save();
        painter.clip(list);
        for (int row = 0; row < 50; ++row)
            painter.fillRect({list.x, list.y + row * 40 - m_contentY, list.width, 20},
                             Color::rgb(0x2468AC));
        painter.restore();
    }

private:
    bool m_scrolling;
    Label *m_size;
    double m_contentY = 0;
};

int main(int argc, char **argv)
{
    Application application(argc, argv);
    bool scrolling = false;
    for (const std::string &argument : application.arguments()) {
        if (argument == "--scroll")
            scrolling = true;
    }

    Window window;
    window.setTitle("toolkit-resize-test");
    window.resize({480, 360});
    auto content = std::make_unique<Content>(scrolling);
    Content *view = content.get();
    window.setContentView(std::move(content));
    window.show();

    if (scrolling)
        application.loop().startTimer(std::chrono::milliseconds(50), true, [view] { view->scrollBy(9); });
    application.loop().startTimer(std::chrono::seconds(15), false, [&application] { application.quit(); });
    return application.run();
}
