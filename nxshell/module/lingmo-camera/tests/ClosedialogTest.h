#ifndef VIDEOWIDGETTEST_H
#define VIDEOWIDGETTEST_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "src/closedialog.h"

class CloseDialogTest : public ::testing::Test
{
public:
    CloseDialogTest();
    virtual void SetUp()
    {
    }
    virtual void TearDown()
    {
    }
};

#endif // VIDEOWIDGETTEST_H
