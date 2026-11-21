// SPDX-License-Identifier: GPL-3.0-or-later
#include <gtest/gtest.h>
#include "pagetest.h"
#include "input_test_support.h"
#include "base/settings.h"
#include "base/paths.h"
#include "widgets/choicerow.h"
#include "widgets/imageselectionitem.h"
#include "ui/test_input.h"
using namespace Lingmo::ui;
using settings::testing::findView;
namespace {
void checkChoices(View* view){
 if(auto* row=dynamic_cast<settings::ChoiceRow*>(view)){
  const auto box=row->options()->geometry();EXPECT_GE(box.x,0);EXPECT_LE(box.right(),row->geometry().width+.5);
  for(const auto& child:row->options()->children()){EXPECT_GT(child->geometry().width,0);EXPECT_LE(child->geometry().right(),box.width+.5);}
 }
 for(const auto& child:view->children())checkChoices(child.get());
}
}
TEST(AppearanceLayout, ChoicesFitNarrowAndWideWindows){
 PageFixture page("appearance");
 for(double width:{640.0,1000.0}){page.window->window().resize({width,820});page.window->window().grab();checkChoices(page.window->window().contentView());}
}
TEST(AppearanceLayout, WidgetStyleWritesSharedDesktopSetting){
 PageFixture page("appearance");auto& window=page.window->window();
 auto* mono=findView<settings::ImageSelectionItem>(window.contentView(),"appearance:widget-style-1");ASSERT_NE(mono,nullptr);
 mono->onClicked();window.grab();EXPECT_TRUE(mono->isChecked());
 EXPECT_EQ(Lingmo::base::Settings(Lingmo::base::paths::lingmoConfig("filemanager")).intValue("DesktopWidgets/colorMode",0),1);
 auto* full=findView<settings::ImageSelectionItem>(window.contentView(),"appearance:widget-style-2");ASSERT_NE(full,nullptr);full->onClicked();window.grab();EXPECT_TRUE(full->isChecked());EXPECT_FALSE(mono->isChecked());
}
