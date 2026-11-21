// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/accounts/userrow.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include <unicode/unistr.h>
#include <unicode/utf16.h>

#include "base/i18n.h"
#include "base/transform.h"
#include "ui/animation.h"
#include "ui/cell_layout.h"
#include "ui/image_view.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/path.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "widgets/iconbutton.h"
#include "widgets/metrics.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

// The account's picture in a circle, or its initial, as on the lock screen,
// for an account with no picture: the icon theme has no avatar that size.
// Hovering dims it under a pencil, as the lock screen's avatar shows its
// swap arrows, since a click changes the picture.
class UserAvatar : public View
{
public:
    static constexpr double kSize = 48;

    UserAvatar()
    {
        m_initial = addChild<Label>();
        m_initial->setFontSize(22);
        // Drawn once decoded; the initial shows until then.
        m_picture = addChild<ImageView>();
        m_picture->setCornerRadius(kSize / 2);
        m_picture->onStatusChanged = [this](ImageView::Status) {
            requestLayout();
            requestPaint();
        };
        m_overlay = addChild<EditOverlay>(this);
    }

    void setUserName(const std::string& name)
    {
        // The first character, upper case, as QString::charAt(0).toUpper() gives it.
        icu::UnicodeString text = icu::UnicodeString::fromUTF8(name);
        std::string initial = "?";
        if (!text.isEmpty()) {
            icu::UnicodeString first = text.tempSubString(0, text.length() > 0 && U16_IS_LEAD(text[0]) ? 2 : 1);
            first.toUpper();
            initial.clear();
            first.toUTF8String(initial);
        }
        m_initial->setText(initial);
        requestLayout();
    }

    void setPicture(const std::string& path)
    {
        if (path == m_picture->source())
            return;
        m_picture->setSource(path);
        requestLayout();
        requestPaint();
    }

    SizeF sizeHint() const override { return {kSize, kSize}; }
    bool acceptsPointer() const override { return true; }
    CursorShape cursorShape() const override { return CursorShape::Pointer; }

    std::function<void()> onClicked;

protected:
    void layout() override
    {
        m_initial->setColor(Theme::current().disabledText);
        const SizeF size = m_initial->sizeHint();
        m_initial->setGeometry({rounding::centeredStart(kSize, size.width), rounding::centeredStart(kSize, size.height), size.width, size.height});
        m_initial->setVisible(!hasPicture());
        m_picture->setGeometry(localRect());
        m_overlay->setGeometry(localRect());
    }

    void paint(Painter& painter) override
    {
        // The account's initial, as on the lock screen, for an account with
        // no picture: the icon theme has no avatar the size of a picture.
        if (!hasPicture())
            painter.fillEllipse(localRect(), Theme::current().text.withAlpha(0.12));
    }

    void pointerRelease(const PointerEvent& event) override
    {
        if (event.button == MouseButton::Left && localRect().contains(event.position) && onClicked)
            onClicked();
    }

    void pointerEnter() override { setHovered(true); }
    void pointerLeave() override { setHovered(false); }

private:
    class EditOverlay : public View
    {
    public:
        explicit EditOverlay(UserAvatar* avatar)
            : m_avatar(avatar)
        {
        }

    protected:
        void paint(Painter& painter) override
        {
            const double t = m_avatar->m_hover;
            if (t <= 0)
                return;
            painter.fillEllipse(localRect(), Color {0, 0, 0, 255}.withAlpha(0.35 * t));
            // The pencil on its 24 by 24 grid, drawn 18 across and centred.
            constexpr double kIcon = 18;
            Path pencil;
            pencil.moveTo(13.26, 3.59997);
            pencil.lineTo(5.04997, 12.29);
            pencil.cubicTo(4.73997, 12.62, 4.43997, 13.27, 4.37997, 13.72);
            pencil.lineTo(4.00997, 16.96);
            pencil.cubicTo(3.87997, 18.13, 4.71997, 18.93, 5.87997, 18.73);
            pencil.lineTo(9.09997, 18.18);
            pencil.cubicTo(9.54997, 18.1, 10.18, 17.77, 10.49, 17.43);
            pencil.lineTo(18.7, 8.73997);
            pencil.cubicTo(20.12, 7.23997, 20.76, 5.52997, 18.55, 3.43997);
            pencil.cubicTo(16.35, 1.36997, 14.68, 2.09997, 13.26, 3.59997);
            pencil.close();
            pencil.moveTo(11.89, 5.05005);
            pencil.cubicTo(12.32, 7.81005, 14.56, 9.92005, 17.34, 10.2);
            pencil.moveTo(3, 22);
            pencil.lineTo(21, 22);
            const double scale = kIcon / 24;
            const base::AffineTransform place(scale, 0, 0, scale, rounding::centeredStart(geometry().width, kIcon),
                                        rounding::centeredStart(geometry().height, kIcon));
            painter.strokePath(pencil.transformed(place), Color {255, 255, 255, 255}.withAlpha(t), 1.5,
                               LineCap::Round, LineJoin::Round);
        }

    private:
        UserAvatar* m_avatar;
    };

    bool hasPicture() const { return m_picture->status() == ImageView::Status::Ready; }

    void setHovered(bool hovered)
    {
        Window* window = this->window();
        if (!window) {
            setHover(hovered ? 1 : 0);
            return;
        }
        if (!m_hoverAnimation) {
            m_hoverAnimation = std::make_unique<Animation>(window);
            m_hoverAnimation->setDuration(std::chrono::milliseconds(80));
            m_hoverAnimation->setEasing(Animation::Easing::OutQuad);
            m_hoverAnimation->onUpdate = [this](double value) {
                setHover(m_hoverFrom + (m_hoverTarget - m_hoverFrom) * value);
            };
        }
        // From wherever it is now, as a Behavior retargets.
        m_hoverFrom = m_hover;
        m_hoverTarget = hovered ? 1 : 0;
        m_hoverAnimation->stop();
        m_hoverAnimation->start();
    }

    void setHover(double value)
    {
        m_hover = value;
        // A picture shows through the dim; an initial would cross the pencil.
        m_initial->setOpacity(1 - value);
        m_overlay->requestPaint();
    }

    Label* m_initial;
    ImageView* m_picture;
    EditOverlay* m_overlay;
    std::unique_ptr<Animation> m_hoverAnimation;
    double m_hover = 0;
    double m_hoverFrom = 0;
    double m_hoverTarget = 0;
};

UserRow::UserRow(std::shared_ptr<accounts::UserAccount> account, int64_t loggedInUid)
    : m_account(std::move(account))
    , m_loggedInUid(loggedInUid)
{
    // As much room above and below the picture as before it.
    setMinimumRowHeight(UserAvatar::kSize + 2 * metrics::cardHorizontalPadding);
    m_avatar = setLeading<UserAvatar>();
    m_avatar->onClicked = [this] {
        if (onPictureClicked)
            onPictureClicked();
    };
    m_userName = addContent<Label>();
    m_realName = addContent<Label>();
    addContent<View>()->setLayoutHints({.fillWidth = true});
    m_loggedIn = addContent<Label>(tr("Logged in"));
    m_info = addContent<IconButton>("info", 20);
    m_info->onClicked = [this] {
        if (onInfo)
            onInfo();
    };
    onClicked = [this] {
        if (onInfo)
            onInfo();
    };

    m_changed = m_account->changed.connect([this] { update(); });
    update();
}

UserRow::~UserRow() = default;

void UserRow::showPicture(const std::string& path)
{
    m_avatar->setPicture(path);
}

void UserRow::layout()
{
    const Theme& theme = Theme::current();
    m_realName->setColor(theme.disabledText);
    m_loggedIn->setColor(theme.disabledText);
    m_info->setColor(theme.highlight);
    ListRow::layout();
}

void UserRow::update()
{
    const std::string userName = m_account->userName();
    const std::string realName = m_account->realName();
    setName("user:" + userName);
    m_avatar->setUserName(userName);
    m_avatar->setPicture(m_account->iconFileName());
    m_userName->setText(userName);
    m_realName->setText(realName);
    m_realName->setVisible(realName != userName);
    m_loggedIn->setVisible(m_account->userId() == m_loggedInUid);
    requestLayout();
}

std::vector<std::shared_ptr<accounts::UserAccount>>
loggedInFirst(const std::vector<std::shared_ptr<accounts::UserAccount>>& accounts, int64_t loggedInUid)
{
    std::vector<std::shared_ptr<accounts::UserAccount>> ordered = accounts;
    std::stable_partition(ordered.begin(), ordered.end(),
                          [loggedInUid](const auto& account) { return account->userId() == loggedInUid; });
    return ordered;
}

} // namespace settings
