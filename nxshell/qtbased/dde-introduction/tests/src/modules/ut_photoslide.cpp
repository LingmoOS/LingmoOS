// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_photoslide.h"

#define protected public
#include "photoslide.h"
#undef protected

ut_photoslide_test::ut_photoslide_test()
{

}

TEST_F(ut_photoslide_test, PhotoSlide)
{
    PhotoSlide* test_module = new PhotoSlide();
    ASSERT_TRUE(test_module);
    ASSERT_TRUE(test_module->m_currentLabel);
    ASSERT_TRUE(test_module->m_lastAni);
    ASSERT_TRUE(test_module->m_currentAni);
    test_module->deleteLater();
}

TEST_F(ut_photoslide_test, initSetImage)
{
    PhotoSlide* test_module = new PhotoSlide();
    test_module->initSetImage(DSysInfo::UosEdition::UosProfessional);
    EXPECT_EQ(test_module->m_strPhotoPath,UOSPROFESSIONAL_PHOTO_PATH);
    EXPECT_EQ(test_module->m_iHpotoTotal,UOSPROFESSIONAL_PHOTO_TOTAL);
    test_module->initSetImage(DSysInfo::UosEdition::UosCommunity);
    EXPECT_EQ(test_module->m_strPhotoPath,UOSPROFESSIONAL_PHOTO_PATH);
    EXPECT_EQ(test_module->m_iHpotoTotal,UOSPROFESSIONAL_PHOTO_TOTAL);
    test_module->initSetImage(DSysInfo::UosEdition::UosEnterprise);
    EXPECT_EQ(test_module->m_strPhotoPath,UOSENTERPRISE_PHOTO_PATH);
    EXPECT_EQ(test_module->m_iHpotoTotal,UOSENTERPRISE_PHOTO_TOTAL);
    test_module->initSetImage(DSysInfo::UosEdition::UosEnterpriseC);
    EXPECT_EQ(test_module->m_strPhotoPath,UOSENTERPRISE_PHOTO_PATH);
    EXPECT_EQ(test_module->m_iHpotoTotal,UOSENTERPRISE_PHOTO_TOTAL);
    test_module->initSetImage(DSysInfo::UosEdition::UosEuler);
    EXPECT_EQ(test_module->m_strPhotoPath,UOSEULER_PHOTO_PATH);
    EXPECT_EQ(test_module->m_iHpotoTotal,UOSEULER_PHOTO_TOTAL);
    test_module->initSetImage(DSysInfo::UosEdition::UosEditionUnknown);
    EXPECT_EQ(test_module->m_strPhotoPath,UOSPROFESSIONAL_PHOTO_PATH);
    EXPECT_EQ(test_module->m_iHpotoTotal,UOSPROFESSIONAL_PHOTO_TOTAL);

    test_module->deleteLater();
}

TEST_F(ut_photoslide_test, updateBigIcon)
{
    PhotoSlide* test_module = new PhotoSlide();
    test_module->updateBigIcon();
    EXPECT_EQ(700,test_module->width());
    EXPECT_EQ(450,test_module->height());
    EXPECT_EQ(700,test_module->m_currentLabel->width());
    EXPECT_EQ(450,test_module->m_currentLabel->height());
    test_module->deleteLater();
}

TEST_F(ut_photoslide_test, updateSmallIcon)
{
    PhotoSlide* test_module = new PhotoSlide();
    test_module->updateSmallIcon();
    EXPECT_EQ(549,test_module->width());
    EXPECT_EQ(309,test_module->height());
    EXPECT_EQ(549,test_module->m_currentLabel->width());
    EXPECT_EQ(309,test_module->m_currentLabel->height());

    test_module->deleteLater();
}

TEST_F(ut_photoslide_test, updateSelectBtnPos)
{
    PhotoSlide* test_module = new PhotoSlide();
    test_module->updateSelectBtnPos();

    test_module->deleteLater();
}

TEST_F(ut_photoslide_test, keyPressEvent)
{
    PhotoSlide* test_module = new PhotoSlide();
    QKeyEvent *event = new QKeyEvent(QEvent::Type::KeyPress, 1,Qt::KeyboardModifier::NoModifier);
    test_module->keyPressEvent(event);
     
    delete event;
    test_module->deleteLater();
}


TEST_F(ut_photoslide_test, start)
{
    PhotoSlide* test_module = new PhotoSlide();
    test_module->start(false, true, 1000);
     
    test_module->deleteLater();
}

TEST_F(ut_photoslide_test, setPhotoList)
{
    PhotoSlide* test_module = new PhotoSlide();
    QStringList list;
    for (int i = 1; i <= UOSPROFESSIONAL_PHOTO_TOTAL; i++) {
        list << QString("%1/slide_small/0%2.jpg").arg(UOSPROFESSIONAL_PHOTO_PATH).arg(i);
    }

    test_module->setPhotoList(list);
    test_module->deleteLater();
}

TEST_F(ut_photoslide_test, updateSlideImage)
{
    PhotoSlide* test_module = new PhotoSlide();
    test_module->updateSlideImage();
    test_module->m_currentAni->updateState(QPropertyAnimation::Running,QPropertyAnimation::Stopped);
    test_module->updateSlideImage();
    ASSERT_TRUE(test_module->m_currentLabel);
     
    test_module->deleteLater();
}
