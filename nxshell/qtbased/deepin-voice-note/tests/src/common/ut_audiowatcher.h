// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_AUDIOWATCHER_H
#define UT_AUDIOWATCHER_H

#include "gtest/gtest.h"
class AudioWatcher;

class UT_AudioWatcher : public ::testing::Test
{
public:
    UT_AudioWatcher();

    // Test interface
protected:
    virtual void SetUp() override;
    virtual void TearDown() override;

private:
    AudioWatcher *m_AudioWatcher {nullptr};
};

#endif // UT_AUDIOWATCHER_H
