#include "lingmoimagecodec.h"

QStringList *LingmoImageCodec::m_list = new QStringList;
QString LingmoImageCodec::m_savePath = "";
bool LingmoImageCodec::saveImage(const Mat &mat, const QString &savepath, const QString &realFormat, bool replace)
{
    return save(mat, saveWay(savepath, replace), realFormat);
}

bool LingmoImageCodec::saveImage(QList<Mat> *list, const int &fps, const QString &savepath, const QString &realFormat,
                                bool replace)
{
    return save(list, fps, saveWay(savepath, replace), realFormat);
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
        tmpPath = tmpPath.mid(0, tmpPath.lastIndexOf(".")) + "(" + QString::number(num) + ")"
                  + tmpPath.mid(tmpPath.lastIndexOf("."), -1);
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
        if (mat.type() == CV_8UC4) {
            cvtColor(mat, tmpMat, cv::COLOR_RGBA2BGRA);
        } else if (mat.type() == CV_8UC3) {
            cvtColor(mat, tmpMat, cv::COLOR_RGB2BGR);
        }
        return stbi_write_tga(savepath.toLocal8Bit().data(), tmpMat.cols, tmpMat.rows, tmpMat.channels(), tmpMat.data);
    }
    if (type == "ico" || type == "xpm" || type == "xbm") {
        QPixmap pix = converFormat(mat);
        return pix.save(savepath, type.toLocal8Bit());
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
        return save(list, 0, savepath, type);
    }
    if (type == "pbm") {
        QPixmap pix = converFormat(mat);
        return pix.save(savepath, type.toLocal8Bit());
    }
    if (type == "ppm") {
        QPixmap pix = converFormat(mat);
        return pix.save(savepath, type.toLocal8Bit());
    }
    if (type == "dib") {
        return imwrite(savepath.toStdString(), mat);
    }
    //如果是freeimage库的图片
    if (m_freeimageSupportFormats.contains(type) && !g_needSaveAs) {
        if (type == "xpm" || type == "xbm" || type == "wbmp" || type == "webp") {
            QPixmap pix = converFormat(mat);
            return pix.save(savepath, type.toLocal8Bit());
        }
        FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
        fif = get_real_format(QString(savepath));
        int formatCode;
        if (m_formats.contains(type.toUpper())) {
            formatCode = m_formats.value(type.toUpper());
            fif = FREE_IMAGE_FORMAT(formatCode);
            //调整颜色通道的顺序，确保颜色通道的顺序是RGB
            if (fif == FIF_EXR) {
                Mat tmpMat;
                cvtColor(mat, tmpMat, cv::COLOR_BGR2RGB);
                return saveFreeImage(savepath, tmpMat, fif);
            }
        } else {
            if (fif == FIF_UNKNOWN) {
                fif = get_real_format(QString(savepath) + "." + type);
            }
        }
        return saveFreeImage(savepath, mat, fif);
    }

    QFileInfo fileInfo(savepath);
    if (m_supportFormats.contains(fileInfo.suffix().toLower())) {
        if (g_needSaveAs) {
            g_needSaveAs = false;
            return imwrite(savepath.toStdString(), mat);
        }
        if (type == fileInfo.suffix().toLower()) {
            return imwrite(savepath.toStdString(), mat);
        } else if (type == "ras" && fileInfo.suffix().toLower() == "sr") {
            return imwrite(savepath.toStdString(), mat);
        } else {
            qDebug() << "保存失败";
            return false;
        }
    }
    qDebug() << "不支持该图片的保存";
    return false;
}

bool LingmoImageCodec::save(QList<Mat> *list, const int &fps, const QString &savepath, const QString &realFormat)
{
    if (list->length() < 1) {
        return false;
    }

    return saveMovie(list, fps, savepath, realFormat);
}

bool LingmoImageCodec::saveMovie(QList<Mat> *list, const int &fps, const QString &savepath, const QString &realFormat)
{
    SaveMovie *saveMovie = new SaveMovie(list, fps, savepath, realFormat, m_list);
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
