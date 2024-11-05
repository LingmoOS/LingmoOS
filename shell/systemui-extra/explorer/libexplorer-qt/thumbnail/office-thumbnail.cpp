/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: renpeijia <renpeijia@kylinos.cn>
 *
 */

#include "generic-thumbnailer.h"
#include "office-thumbnail.h"
#include "file-utils.h"
#include <QFileInfo>
#include <QDebug>
#include <QtConcurrent>
#include <QImage>
#include <QMessageAuthenticationCode>
#include <QPainter>
#include <QImageReader>
#include <qglobal.h>

OfficeThumbnail::OfficeThumbnail(const QString &uri)
{
    if (!uri.startsWith("file:///")) {
        m_url = FileUtils::getTargetUri(uri);
        qDebug()<<"target uri:"<< m_url.path();
    }
    else {
        m_url = uri;
    }

    auto fileInfo = FileInfo::fromUri(uri);
    m_modifyTime = fileInfo->modifiedTime();
}

OfficeThumbnail::~OfficeThumbnail()
{

}

/*
*函数功能：
*1、提取office文件的缩略图，利用libreoffice将文件的首页转换为jpg图片，
* 从而得到缩略图要显示的内容。
*2、md5值是为了区分同名文件的情况，以及文件的是否修改，如果修改过，重新
* 生成缩略图。
*3、将转换后的jpg图片暂时存放到/tmp目录下，该目录时内存文件系统，读写速
* 度快；只要系统重启，这些转换的图片就会清除，不需要主动删除，这样可以避免
* 重复进行图片提取，缺点就是会占用一部分内存，由于桌面系统不会长时间运行，
* 也就避免了文件堆积造成占用内存过高的情况。
*
* 性能测试（测试的内容有限，并不能够说明所有问题）：
* 1、ppt的文件转换一页最慢的需要12s左右，这个时间和文件页数关系不大，但是ppt的
* 版本对时间的影响较大；
* 2、word文件转换一页最慢的需要17s左右，这个和文件中的内容以及格式关系很大，如果
* 是纯文本文字，230页的文件转换也就是2-3s的时间，如果文件中图片较多，即便是只有5
* 页的文件，转换一页消耗的时间也要5s的时间。
* 3、excel文件暂未测试
* 4、转pdf的时间消耗，和文件的页数成正比，页数越多，时间消耗越长，时间消耗达到分钟级。
*
* 后续优化思路：
* 1、寻找office的开发api，直接操作文件，避免转换的过程
* 2、通过并发的提升性能，经过验证libreoffice是单进程处理，不可以并发
*/
QIcon OfficeThumbnail::generateThumbnail()
{
    QIcon thumbnailImage;
    QString md5Name=GenericThumbnailer::codeMd5WithModifyTime(m_url.path(), m_modifyTime);
    QString thumbnail_dir= GenericThumbnailer::thumbnaileCachDir() + "/" + md5Name;
    QString fileName = m_url.fileName();
    qint16 idx = fileName.lastIndexOf(".");
    QString fileThumbnail=thumbnail_dir + "/" + fileName.left(idx) + ".jpg";

    //优化无效流程，未安装libreoffice则直接返回
    if (! QFile::exists("/usr/bin/libreoffice"))
    {
        qDebug()<<"libreoffice not installed, return";
        return thumbnailImage;
    }

    qDebug()<<"file thumbnail:"<<fileThumbnail;
    if (!QFile::exists(fileThumbnail)) {
        //libreoffice --convert-to jpg:writer_jpg_Export test1.doc --outdir ./
        QStringList list;
        list<<"--headless"  /*headless和invisible的方式可以避免出现界面以及无用的log信息，速度更快*/
            <<"--invisible"
            <<"--convert-to"
            <<"jpg:writer_jpg_Export"     /*转换格式jpg*/
            <<m_url.path()                /*要转换的文件*/
            <<"--outdir"                  /*转换完的jpg文件存在的路径*/
            <<thumbnail_dir;
        qDebug()<<"the libreoffice cmd: " << list;

        QProcess p;
        p.start("/usr/bin/libreoffice",list);

        /*
        * 等待30s超时，30s是默认时间，可以修改
        */
        if (!p.waitForStarted()) {
            qWarning()<<"libreoffice start failed, or timeout";
            return thumbnailImage;
        }

        /*
        * 等待30s超时，30s是默认时间，可以修改
        */
        if (!p.waitForFinished()) {
            qWarning()<<"libreoffice run failed, or timeout";
            return thumbnailImage;
        }

        QString err=p.readAllStandardError();
        QString read=p.readAll();
        if (!err.isEmpty()) {
            qWarning()<<"office convert jpg error: " << err;
            return thumbnailImage;
        }
    }

    /*
     *不知道会不会出现无效的图片的情况，可能需要对这种情况做处理？
    */
    thumbnailImage = GenericThumbnailer::generateThumbnail(fileThumbnail, true);

    return thumbnailImage;
}
