// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest/QtTest>
#include <gtest/gtest.h>

#include "service/aimodelservice.h"
#include "service/aimodelservice_p.h"

//class TestAIModelService : public testing::Test
//{
//public:
//    TestAIModelService() = default;
//    ~TestAIModelService() = default;

//protected:
//    static AIModelService *ins;
//};

//AIModelService *TestAIModelService::ins = AIModelService::instance();

//TEST_F(TestAIModelService, TemporaryFile_Contains_Pass)
//{
//    ins->dptr->enhanceCache.clear();
//    EnhancePtr ptr(new EnhanceInfo("source", "output", "model"));
//    ins->dptr->enhanceCache.insert("output", ptr);

//    EXPECT_FALSE(ins->isTemporaryFile("source"));
//    EXPECT_TRUE(ins->isTemporaryFile("output"));
//    EXPECT_EQ(QString("source"), ins->sourceFilePath("output"));

//    ins->dptr->enhanceCache.clear();
//}

//TEST_F(TestAIModelService, ImageProcessing_ErrorFile_Failed)
//{
//    ModelPtr mptr(new ModelInfo);
//    mptr->modelID = 0;
//    mptr->model = "model";
//    ins->dptr->mapModelInfo.insert(mptr->modelID, mptr);

//    QString outfile = ins->imageProcessing("source", mptr->modelID, QImage());
//    // 等待处理结束,涉及条件变量等待！
//    ins->dptr->enhanceWatcher.waitForFinished();

//    auto enhancePtr = ins->dptr->enhanceWatcher.result();
//    EXPECT_EQ(enhancePtr->source, QString("source"));
//    EXPECT_EQ(enhancePtr->output, outfile);

//    EXPECT_FALSE(QFile::exists(outfile));
//    EXPECT_FALSE(ins->dptr->enhanceCache.isEmpty());
//}

//TEST_F(TestAIModelService, OnDBusEnhanceEnd_NotExist_Failed)
//{
//    QString source;
//    QString output;
//    AIModelService::State state;
//    auto conn =
//        QObject::connect(ins, &AIModelService::enhanceEnd, [&](const QString &src, const QString &out, AIModelService::State s) {
//            source = src;
//            output = out;
//            state = s;
//        });

//    ins->dptr->enhanceCache.clear();
//    ins->onDBusEnhanceEnd("output", 0);
//    EXPECT_TRUE(source.isEmpty());

//    EnhancePtr ptr(new EnhanceInfo("source", "output", "model"));
//    ptr->state.store(AIModelService::LoadSucc);
//    ins->dptr->enhanceCache.insert("output", ptr);
//    ins->onDBusEnhanceEnd("output", 0);
//    EXPECT_EQ(AIModelService::LoadFailed, ptr->state.loadAcquire());

//    EXPECT_EQ(source, QString("source"));
//    EXPECT_EQ(output, QString("output"));
//    EXPECT_EQ(state, ptr->state.loadAcquire());

//    QObject::disconnect(conn);
//}

//TEST_F(TestAIModelService, CheckConvertFile_NewImage_Pass)
//{
//    ins->dptr->convertCache.clear();
//    QImage nullImage;
//    QString tmpImage = ins->checkConvertFile("localtest.png", nullImage);
//    EXPECT_TRUE(tmpImage.isEmpty());

//    QImage contentImage(20, 20, QImage::Format_ARGB32);
//    contentImage.fill(Qt::red);
//    tmpImage = ins->checkConvertFile("localtest.png", contentImage);
//    EXPECT_FALSE(tmpImage.isEmpty());
//    EXPECT_TRUE(QFile::exists(tmpImage));
//    EXPECT_TRUE(ins->dptr->convertCache.contains("localtest.png"));
//    EXPECT_EQ(tmpImage, ins->dptr->convertCache.value("localtest.png"));

//    ins->dptr->convertCache.clear();
//}
