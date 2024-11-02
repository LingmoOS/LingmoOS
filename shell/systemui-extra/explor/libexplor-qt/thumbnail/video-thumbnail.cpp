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
#include "video-thumbnail.h"
#include "file-utils.h"
#include <QFileInfo>
#include <QDebug>
#include <QtConcurrent>
#include <QImage>
#include <QMessageAuthenticationCode>
#include <QPainter>
#include <QImageReader>
#include <qglobal.h>

VideoThumbnail::VideoThumbnail(const QString &uri)
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

VideoThumbnail::~VideoThumbnail()
{

}

/*
*获取要显示的图片的位置，因为视频文件的前几帧有可能没有图像是黑的
*/
QMap<QString, QString> VideoThumbnail::videoInfo()
{
    QMap<QString, QString> map;
    QStringList list2;
    list2<<"-hwaccel"<<"auto";
    list2<<"-i"<< m_url.path();
    QString ret="5.0";
    QString time=QString();
    map["Pos"]=ret;
    map["Time"]=time;
    QProcess p;

    p.start("/usr/bin/ffmpeg",list2);
    if (!p.waitForStarted()) {
        return map ;
    }

    if (!p.waitForFinished()) {
        return map;
    }

    QString error=p.readAllStandardError();

    if(error.isEmpty())
        return map;

    QStringList list=error.split("\n");

    for (QString s:list) {
        if (s.trimmed().startsWith("Duration")) {
            s=s.remove("Duration:");
            QString name=s.section(",",0,0);

            QStringList listtime=name.trimmed().split(":");
            if (listtime.count()>=3) {
                QString h=listtime.at(0);
                QString m=listtime.at(1);
                QString s=listtime.at(2);

                if (h.toFloat()>0) {
                    ret= "15.0"; time+=h+":";
                }
                else if (m.toFloat()>0) {
                    ret=  "7.0";
                }
                else if (s.toFloat()<=1) {
                    ret=  "0.1";
                }
                else if (s.toFloat()<=5) {
                    ret=  "1.0";
                }
                else if (s.toFloat()<=10) {
                    ret=  "3.0";
                }
                else if (s.toFloat()>10 ) {
                    ret=    ret;
                }

                time+=m+":";
                time+=s.leftRef(2);

                qDebug()<<"info"<<ret+"|"+time;;

                map["Pos"]=ret;
                map["Time"]=time;
                return map;
            }
        }
    }

    return map;
}

/*
* 函数功能：
* 通过ffmpeg从视频文件中提取出缩略图显示的图片，该图片存放到tmp目录下，不会主动删除，
* tmp是内存文件系统，系统重启会自动清除图片
*
* 性能测试：
* 转化性能和文件大小以及视频文件格式有关。在V10上面测试ffmpeg不支持mpeg格式的视频文件
* 这个可能和解码器的配置有关，可以通过视频格式转换后,再提取图片，效率很低，暂时未实现。
*
* 后续优化方式：
* 通过调用ffpmeg的api实现视频格式转换和图片文件提取，需要验证性能的提升情况。
*/
QIcon VideoThumbnail::generateThumbnail()
{
    QIcon thumbnailImage;
    QString thumbnail= GenericThumbnailer::thumbnaileCachDir();
    QString md5Name=GenericThumbnailer::codeMd5WithModifyTime(m_url.path(), m_modifyTime);
    QString fileThumbnail=thumbnail+"/"+md5Name;

    if (!QFile::exists(fileThumbnail)) {
        static bool isWayland = qApp->property("isWayland").toBool();
        QStringList list;
        if (isWayland) {
            QMap<QString, QString> map=  videoInfo();
            QString pos=map.value("Pos");

            //ffmpeg -i ./kofar-bi-amirica.mp4 -y -ss 10.0 -vframes 1 -f image2 -s 128x128 thumbnail
            list<<"-hwaccel"<<"auto"    /*try using hardware accel*/
               <<"-i"<<m_url.path()     /*Input File Name*/
               <<"-y"                    /*Overwrite*/
               <<"-ss"<<pos              /* seeks in this position*/
               <<"-vframes"<<"1"         /* Num Frames */
               <<"-f"<<"image2"          /* file format.  */
              // <<"-s"<<"128x128"         /*<<"-vf"<<scal*/
               <<"-s"<<"640x640"         /*<<"-vf"<<scal*/
               <<fileThumbnail; /*output file Name */
            qDebug()<<"the ffmpeg cmd: " << list;
        } else {
            //ffmpeg -i ./kofar-bi-amirica.mp4 -y -ss 10.0 -vframes 1 -f image2 -s 128x128 thumbnail
            list<<"-i"<<m_url.path()     /*Input File Name*/
               <<"-y"                    /*Overwrite*/
               <<"-ss"<<"10.0"              /* seeks in this position*/
               <<"-vframes"<<"1"         /* Num Frames */
               <<"-f"<<"image2"          /* file format.  */
              // <<"-s"<<"128x128"         /*<<"-vf"<<scal*/
               <<"-s"<<"640x640"         /*<<"-vf"<<scal*/
               <<fileThumbnail; /*output file Name */
            qDebug()<<"the ffmpeg cmd: " << list;
        }

        QProcess p;
        p.start("/usr/bin/ffmpeg",list);

        if (!p.waitForStarted()) {
            qWarning()<<"start get video image failed.";
            return thumbnailImage;
        }

        if (!p.waitForFinished(40000)) {
            qWarning()<<"wait video image too long time.";
            return thumbnailImage;
        }

        QString err=p.readAllStandardError();
        QString read=p.readAll();
        if (err.contains("not contain any stream")) {
            qWarning()<<"get video image failed.";
            return thumbnailImage;
        }
    }

    /*
     *不知道会不会出现无效的图片的情况，可能需要对这种情况做处理
    */
    thumbnailImage = GenericThumbnailer::generateThumbnail(fileThumbnail, true);

    return thumbnailImage;
}
