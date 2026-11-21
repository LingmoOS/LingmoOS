// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
// Runtime provider for the notification host. Keep the module loaded while any
// SDK views or asynchronous callbacks created by it remain alive.
#include "desktop/widgets/widgetlayer.h"
#include "desktop/widgets/widget.h"
#include "core/config.h"
#include "base/dbus.h"
#include "base/i18n.h"
#include "base/settings.h"
#include "ui/button.h"
#include "ui/label.h"
#include "ui/window.h"
#include <algorithm>
namespace fm {
namespace {
const char* shelfTr(const char* text) { return Lingmo::i18n::dtr("lingmo-filemanager", text); }
class WidgetShelf final : public ui::View {
public:
    WidgetShelf() {
        setName("notification-widgets");
        m_title=addChild<ui::Label>(shelfTr("Widgets"));m_title->setFontSize(16);m_title->setWeight(600);
        m_edit=addChild<ui::Button>(shelfTr("Edit Widgets…"));m_edit->setName("notification-widgets:edit");
        m_edit->onClicked=[] { base::dbus::Proxy(base::dbus::Connection::session(),"org.lingmo.FileManager","/FileManager","org.lingmo.FileManager").send("EditNotificationWidgets"); };
        m_empty=addChild<ui::Label>(shelfTr("Add widgets to Notification Center"));m_empty->setWrap(true);m_empty->setFontSize(12);
        m_layer=addChild<WidgetLayer>();m_layer->setFlowLayout(true);
        m_layer->onEditWidgets=m_edit->onClicked;
        m_layer->onChanged=[this] {
            m_layer->layout();
            base::Settings settings(settingsFile());settings.setValue("NotificationWidgets/layout.v1",serializeWidgets(m_layer->placements()));settings.sync();
            m_empty->setVisible(m_layer->widgets().empty());requestLayout();if(parent())parent()->requestLayout();
        };
    }
    void setActive(bool active) {
        m_active=active;
        base::Settings settings(settingsFile());
        setVisible(active && settings.boolValue("NotificationWidgets/visible",true));
        m_layer->setPlacements(active && isVisible() ? parseWidgets(settings.stringValue("NotificationWidgets/layout.v1")) : std::vector<WidgetPlacement>{});
        // The notification centre is foreground UI; automatic stays colourful.
        m_layer->setAppearance(true,int(settings.intValue("DesktopWidgets/colorMode",0)),true);
        m_layer->setEditing(false);
        m_empty->setVisible(m_layer->widgets().empty());
        m_title->setText(shelfTr("Widgets"));m_edit->setText(shelfTr("Edit Widgets…"));
        m_layer->layout();requestLayout();if(parent())parent()->requestLayout();
    }
    double heightForWidth(double width) const override {
        if(!m_active || !isVisible())return 0;
        double x=0,y=0,row=0;
        for(auto* widget:m_layer->widgets()){const auto size=widgetSizeSpec(widget->size());if(x>0 && x+size.width>width+.5){x=0;y+=row+kWidgetGap;row=0;}x+=size.width+kWidgetGap;row=std::max(row,size.height);}
        return 56 + (m_layer->widgets().empty()?44:y+row);
    }
    ui::SizeF sizeHint() const override{return {368,heightForWidth(368)};}
    void layout() override {
        const double width=geometry().width;
        m_title->setGeometry({0,0,std::max(0.0,width-146),32});
        m_edit->setGeometry({std::max(0.0,width-140),0,140,32});
        m_empty->setGeometry({0,48,width,40});
        m_empty->setColor(ui::Theme::current().text.withAlpha(.6));
        m_layer->setDesktopWindow(window());
        m_layer->setGeometry({0,48,width,std::max(0.0,heightForWidth(width)-48)});
        m_layer->setWorkArea({0,0,width,100000});m_layer->layout();
    }
private:
    WidgetLayer* m_layer;ui::Label* m_title;ui::Label* m_empty;ui::Button* m_edit;bool m_active=false;
};
}
}
// The symbol version includes the SDK View ownership contract. The caller owns
// the returned view and must retain this library until all its views are gone.
extern "C" __attribute__((visibility("default"))) Lingmo::ui::View* lingmo_notification_widgets_create_v1()
{
    Lingmo::i18n::bindDomain("lingmo-filemanager");
    return new fm::WidgetShelf;
}
extern "C" __attribute__((visibility("default"))) void lingmo_notification_widgets_active_v1(Lingmo::ui::View* view, bool active)
{
    if(auto* shelf=dynamic_cast<fm::WidgetShelf*>(view))shelf->setActive(active);
}
