// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GTESTVIEW_H
#define GTESTVIEW_H

#

#include <QtTest/QtTest>
#include <gtest/gtest.h>

#include <DApplicationSettings>

#include "testapi.h"

#define JPEG1 "/usr/share/uosbrowser/product_logo_32.png"
#define GIF2 "/usr/share/deepin-app-store/web_dist/en-AU/assets/images/loading2.gif"
#define JPEGPATH "/usr/share/wallpapers/deepin/abc-123.jpg"
#define SVGPATH "/usr/share/deepin-image-viewer/icons/logo/deepin-image-viewer.svg"
#define DDSPATH "/data/home/lmh/0916/tu/DDS.dds"

#define test_main
#define test_module_view_contents
#define test_module_view_scen
#define test_module_view_z
#define test_service
#define test_utils
#define test_z_exit

//class BlurFrame;
class PushButton;
class ScanPathsDialog;
class VolumeMonitor;
class ScanPathsItem;
class gtestview : public ::testing::Test
{
public:
    gtestview();
    virtual void SetUp()
    {
        list.clear();
        m_SVGPath = QApplication::applicationDirPath() + "/svg.svg";
        m_JPGPath = QApplication::applicationDirPath() + "/jpg.jpg";
        m_DDSPath = QApplication::applicationDirPath() + "/dds.dds";
        m_GIFPath = QApplication::applicationDirPath() + "/gif.gif";
        m_ICOPath = QApplication::applicationDirPath() + "/ico.ico";
        m_MNGPath = QApplication::applicationDirPath() + "/mng.mng";
        m_TIFPath = QApplication::applicationDirPath() + "/tif.tif";
        m_WBMPPath = QApplication::applicationDirPath() + "/wbmp.wbmp";
        m_PNGPath = QApplication::applicationDirPath() + "/png.png";
        m_GIF2Path = QApplication::applicationDirPath() + "/gif2.gif";
        m_ErrorPicPath = QApplication::applicationDirPath() + "/errorPic.icns";
        m_TgaPath = QApplication::applicationDirPath() + "/tga.tga";
        list.push_back(m_TIFPath);
        list.push_back(m_SVGPath);
        list.push_back(m_JPGPath);
        list.push_back(m_DDSPath);
        list.push_back(m_GIFPath);
        list.push_back(m_ICOPath);
        list.push_back(m_MNGPath);
        list.push_back(m_WBMPPath);
        list.push_back(m_PNGPath);
        list.push_back(m_GIF2Path);
        list.push_back(m_ErrorPicPath);
        list.push_back(m_TgaPath);


    }

    virtual void TearDown()
    {

    }
protected:




    /*dirwatcher*/
//    ScanPathsDialog* m_ScanPathsDialog{nullptr};
    VolumeMonitor *m_VolumeMonitor{nullptr};
    ScanPathsItem *m_ScanPathsItem{nullptr};

    PushButton *m_pushbutton{nullptr};


    QStringList list;

    QString m_SVGPath{SVGPATH};
    QString m_JPGPath{JPEGPATH};
    QString m_DDSPath{DDSPATH};
    QString m_GIFPath{GIF2};
    QString m_ICOPath;
    QString m_MNGPath;
    QString m_TIFPath;
    QString m_WBMPPath;
    QString m_PNGPath;
    QString m_GIF2Path;
    QString m_ErrorPicPath;
    QString m_TgaPath;

};

#endif // GTESTVIEW_H
