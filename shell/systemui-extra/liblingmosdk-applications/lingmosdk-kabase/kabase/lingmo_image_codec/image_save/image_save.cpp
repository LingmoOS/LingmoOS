/*
 * liblingmosdk-base's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include "lingmo_image_codec/lingmoimagecodec.hpp"

namespace kdk
{
namespace kabase
{

QStringList *LingmoImageCodec::m_list = new QStringList;
QString LingmoImageCodec::m_savePath = "";
bool LingmoImageCodec::saveImage(const Mat &mat, const QString &savepath, bool replace)
{
    return save(mat, saveWay(savepath, replace), QFileInfo(savepath).suffix());
}

bool LingmoImageCodec::saveImage(QList<Mat> *list, const int &fps, const QString &savepath, bool replace)
{
    return save(list, fps, saveWay(savepath, replace));
}

QString LingmoImageCodec::saveWay(const QString &savepath, bool replace)
{
    QFileInfo tmpFileInfo(savepath);
    //覆盖保存
    if (replace) {
        return savepath;
    }

    //备份保存
    QString tmpPath = savepath;
    int num = 1;
    while (QFileInfo::exists(tmpPath)) {
        tmpPath = tmpFileInfo.absolutePath() + "/" + tmpFileInfo.completeBaseName() + "(" + QString::number(num) + ")"
                  + "." + tmpFileInfo.completeSuffix();
        num++;
    }
    return tmpPath;
}

bool LingmoImageCodec::save(const Mat &mat, const QString &savepath, const QString &type)
{
    m_savePath = savepath;
    if (type == "svg") {
        QPixmap pix = converFormat(mat);
        QPainter paint;
        QSvgGenerator svg;
        svg.setFileName(savepath);
        svg.setViewBox(QRect(QPoint(0, 0), pix.size()));
        paint.begin(&svg);
        paint.drawImage(QPoint(0, 0), pix.toImage());
        paint.end();
        return true;
    }
    if (type == "tga") {
        Mat tmpMat;
        if (mat.type() == 4) {
            cvtColor(mat, tmpMat, cv::COLOR_RGBA2BGRA);
        } else if (mat.type() == 3) {
            cvtColor(mat, tmpMat, cv::COLOR_RGB2BGR);
        }
        return stbi_write_tga(savepath.toLocal8Bit().data(), tmpMat.cols, tmpMat.rows, 4, tmpMat.data);
    }
    if (type == "ico" || type == "xpm") {
        QPixmap pix = converFormat(mat);
        return pix.save(savepath);
    }
    //    if(type == "hdr"){
    //        Mat tmpMat;
    //        cvtColor(mat,tmpMat,cv::COLOR_RGB2BGR);
    //        auto img_array = tmpMat.data;
    //        float *img_pt = new float[tmpMat.rows*tmpMat.cols];
    //        for (int i = 0; i < tmpMat.rows*tmpMat.cols; i++)
    //            img_pt[i] = (float)img_array[i];
    //        stbi_write_hdr(savepath.toLocal8Bit().data(),tmpMat.cols,tmpMat.rows,3,img_pt);
    //        return true;
    //    }
    //单帧gif的场景
    if (type == "gif") {
        QList<Mat> *list = new QList<Mat>;
        list->append(mat);
        return save(list, 0, savepath);
    }
    if (type == "pbm") {
        QPixmap pix = converFormat(mat);
        return pix.save(savepath);
    }
    if (type == "ppm") {
        QPixmap pix = converFormat(mat);
        return pix.save(savepath);
    }
    //如果是freeimage库的图片
    if (m_freeimageSupportFormats.contains(type)) {
        if (type == "xpm" || type == "xbm" || type == "wbmp") {
            QPixmap pix = converFormat(mat);
            return pix.save(savepath);
        }
        FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
        fif = get_real_format(QString(savepath));
        return saveFreeImage(savepath, mat, fif);
    }

    //非特殊情况
    return imwrite(savepath.toStdString(), mat);
}

bool LingmoImageCodec::save(QList<Mat> *list, const int &fps, const QString &savepath)
{
    if (list->length() < 1) {
        return false;
    }

    return saveMovie(list, fps, savepath);
}

bool LingmoImageCodec::saveMovie(QList<Mat> *list, const int &fps, const QString &savepath)
{
    SaveMovie *saveMovie = new SaveMovie(list, fps, savepath, m_list);
    connect(saveMovie, &SaveMovie::saveMovieFinish, getSignalObj(), &LingmoImageCodecSignals::saveMovieFinish);
    saveMovie->start();
    //    Gif_H m_Gif;
    //    Gif_H::GifWriter m_GifWriter;
    //    for (int i=0; i<list->length(); ++i){
    //        Mat tmpMat;
    //        cvtColor(list->at(i),tmpMat,cv::COLOR_RGBA2BGRA);
    //        if(i == 0)
    //            if (!m_Gif.GifBegin(&m_GifWriter, savepath.toLocal8Bit().data(), tmpMat.cols, tmpMat.rows, fps))
    //                return false;
    //        // 写入图片信息
    //        m_Gif.GifWriteFrame(&m_GifWriter, tmpMat.data, tmpMat.cols, tmpMat.rows, fps);
    //    }
    //    m_Gif.GifEnd(&m_GifWriter);
    return true;
}

bool LingmoImageCodec::saveFreeImage(const QString &path, Mat mat, FREE_IMAGE_FORMAT fi_format)
{
    return mat2fibitmap(mat, fi_format, path);
}

bool LingmoImageCodec::isSaving(const QString &path)
{
    return m_list->contains(path);
}

bool LingmoImageCodec::allSaveFinish()
{
    return m_list->isEmpty();
}

QString LingmoImageCodec::savePath()
{
    return m_savePath;
}

} // namespace kabase
} // namespace kdk
