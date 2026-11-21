#include "applicationscope.h"

#include <gtest/gtest.h>

TEST(ApplicationScope, namesFollowTheDesktopEntrySpecification)
{
    EXPECT_EQ(ApplicationScope::unitName("com.obsproject.Studio", 4242),
              "app-lingmo-com.obsproject.Studio-4242.scope");
    // Dashes must not be mistaken for the launcher and random separators.
    EXPECT_EQ(ApplicationScope::unitName("lingmo-filemanager", 7),
              "app-lingmo\\x2dfilemanager-7.scope");
    EXPECT_EQ(ApplicationScope::unitName(".hidden app", 1),
              "app-lingmo-\\x2ehidden\\x20app-1.scope");
    EXPECT_EQ(ApplicationScope::unitName("org.gnome.Evolution-alarm-notify", 2437),
              "app-lingmo-org.gnome.Evolution\\x2dalarm\\x2dnotify-2437.scope");
}
