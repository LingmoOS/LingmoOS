/*
 * SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include <gtest/gtest.h>
#include <QPluginLoader>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDir>

struct PluginInfo {
    QString path;
    QString metaFilePath;
};

class TestPluginLoading : public testing::Test, public testing::WithParamInterface<PluginInfo>
{
public:
    static void SetUpTestSuite()
    {
        // Assume build directory is in project root directory
#ifdef UT_PLATFORMPLUGIN_PATH
        pluginBaseDir = UT_PLATFORMPLUGIN_PATH;
#else
        pluginBaseDir = "../plugins";
#endif
#ifdef META_FILE_BASE_DIR
        metaFileBaseDir = META_FILE_BASE_DIR;
#else
        metaFileBaseDir = "../..";
#endif
    }

private:
    static QString pluginBaseDir;
    static QString metaFileBaseDir;
};

QString TestPluginLoading::pluginBaseDir = "";
QString TestPluginLoading::metaFileBaseDir = "";

TEST_P(TestPluginLoading, load)
{
    auto info = GetParam();
    QPluginLoader loader(info.path);
    loader.load();
    // Test if loaded successfully
    EXPECT_TRUE(loader.isLoaded());
    EXPECT_NE(nullptr, loader.instance());
    // Test if the meta data match, especially key
    QFile metaFile{QDir(metaFileBaseDir).filePath(info.metaFilePath)};
    auto success = metaFile.open(QFile::ReadOnly);
    ASSERT_TRUE(success);
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(metaFile.readAll(), &err);
    ASSERT_EQ(err.error, QJsonParseError::NoError);
    ASSERT_FALSE(doc.isNull());
    ASSERT_FALSE(doc.isEmpty());
    ASSERT_TRUE(doc.isObject());
    auto root = doc.object();
    auto meta = loader.metaData();
    // meta info must include Keys field
    auto fileKeys = root.value("Keys").toArray();
    auto metaKeys = meta.value("MetaData").toObject().value("Keys").toArray();
    ASSERT_FALSE(fileKeys.isEmpty());
    ASSERT_FALSE(metaKeys.isEmpty());
    ASSERT_EQ(fileKeys.count(), metaKeys.count());
    for (int i = 0; i < fileKeys.count(); ++i) {
        EXPECT_EQ(fileKeys.at(i).toString(), metaKeys.at(i).toString());
    }
    loader.unload();
}

INSTANTIATE_TEST_SUITE_P(Default, TestPluginLoading,
    testing::Values(
        PluginInfo{"iconengines/libdicon.so", "iconengineplugins/diconengine/diconengine.json"},
        PluginInfo{"iconengines/libdsvgicon.so", "iconengineplugins/svgiconengine/qsvgiconengine.json"},
        PluginInfo{"imageformats/libdci.so", "imageformatplugins/dci/dci.json"},
        PluginInfo{"imageformats/libdsvg.so", "imageformatplugins/svg/svg.json"},
        PluginInfo{"platformthemes/libqdeepin.so", "platformthemeplugin/deepin.json"},
        PluginInfo{"styles/libchameleon.so", "styleplugins/chameleon/chameleon.json"}
    )
);
