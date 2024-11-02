#include "lingmoimagecodec.h"

MatResult LingmoImageCodec::loadImageToMat(QString path)
{
    return loadThumbnailToMat(path, "", IMREAD_UNCHANGED, QSize(0, 0));
}

MatResult LingmoImageCodec::loadThumbnailToMat(QString path, QString realFormat, ImreadModes modes, QSize size)
{
    MatResult mr;
    QString suffix;
    QFileInfo info(path);
    if ("" == realFormat) {
        suffix = info.suffix().toLower();
    } else {
        suffix = realFormat;
    }
    // svg、gif等opencv不支持的格式
    if ((suffix == "gif" || suffix == "apng" || suffix == "png")) {
        mr = loadMovieToMat(path, modes, suffix);
    } else if (suffix == "tif" || suffix == "tiff") {
        FREE_IMAGE_FORMAT fif = FIF_TIFF;
        FIMULTIBITMAP *pTIF = FreeImage_OpenMultiBitmap(fif, path.toLocal8Bit().data(), false, true, true, 0);
        int gifImgCnt = FreeImage_GetPageCount(pTIF);
        FIBITMAP *pFrame;
        if (gifImgCnt > 0) {
            mr = loadImageToMat(path, modes, suffix, size);
        }
        if (gifImgCnt > 1) {
            mr.matList = new QList<Mat>;
            mr.fps = 1000;
            mr.matList->append(mr.mat);
            for (int curFrame = 1; curFrame < gifImgCnt; curFrame++) {
                pFrame = FreeImage_LockPage(pTIF, curFrame);
                Mat mat = FI2MAT(pFrame);
                mr.matList->append(mat);
                FreeImage_UnlockPage(pTIF, pFrame, curFrame);
            }
        }
        FreeImage_CloseMultiBitmap(pTIF);
    } else {
        mr = loadImageToMat(path, modes, suffix, size);
    }
    mr.info = info;
    Mat mat = mr.mat;
    //判断图像是否有效
    if (!mat.data) {
        mr.openSuccess = false;
    }
    mr.mat = mat;
    return mr;
}

MatResult LingmoImageCodec::loadImageToMat(QString path, ImreadModes modes, QString suffix, QSize size)
{
    MatResult mr;
    Mat mat;
    if (suffix == "svg") {
        // 将路径转换为本地编码
        QString nativePath = QDir::toNativeSeparators(path);
        const char* nativePathChars = nativePath.toLocal8Bit().constData();
        // 使用本地编码的路径加载 SVG 文件
        RsvgHandle *handle = rsvg_handle_new_from_file(nativePathChars, NULL);
        if (handle == NULL) {
            qDebug() << "加载svg图片失败！";
            return mr;
        }

        RsvgDimensionData dimensions;
        rsvg_handle_get_dimensions(handle, &dimensions);
        int width = dimensions.width;
        int height = dimensions.height;

        //创建指定格式和尺寸的图像
        cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
        if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
            qDebug() << "创建图像失败！";
            rsvg_handle_free(handle);
            return mr;
        }

        //渲染图像
        cairo_t *cr = cairo_create(surface);
        rsvg_handle_render_cairo(handle, cr);

        QImage image(cairo_image_surface_get_data(surface),
                     width,
                     height,
                     QImage::Format_ARGB32_Premultiplied);
        mat = Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar *>(image.bits()),
                  static_cast<size_t>(image.bytesPerLine()))
                  .clone();

        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        rsvg_handle_free(handle);

    } else if (suffix == "tga") {
        int w, h, a;
        unsigned char *data = stbi_load(path.toLocal8Bit().data(), &w, &h, &a, 0);
        if (!data) {
            return mr;
        }
        Mat tmpMat;
        if (3 == a) {
            tmpMat = Mat(h, w, CV_8UC3, const_cast<uchar *>(data)).clone();
            cvtColor(tmpMat, mat, cv::COLOR_BGR2RGB);
        } else if (4 == a) {
            tmpMat = Mat(h, w, CV_8UC4, const_cast<uchar *>(data)).clone();
            cvtColor(tmpMat, mat, cv::COLOR_BGRA2RGBA);
        }

        stbi_image_free(data);

    } else if (suffix == "ico") {
        QImage image(path);
        mat = Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar *>(image.bits()),
                  static_cast<size_t>(image.bytesPerLine()))
                  .clone();
    } else if (suffix == "xpm") {
        QImage image(path);
        image = image.convertToFormat(QImage::Format_RGB888);
        image = image.rgbSwapped();
        mat = Mat(image.height(), image.width(), CV_8UC3, const_cast<uchar *>(image.bits()),
                  static_cast<size_t>(image.bytesPerLine()))
                  .clone();
    } else {
        mat = loadFreeimageFormat(path.toLocal8Bit().data());
    }
    // 1通道的bmp图片且freeimage库加载失败，opencv读取时会闪退在库中，所以直接返回显示为无法读取的图。
    if (suffix == "bmp") {
        if (mat.channels() == 1) {
            if (!mat.data) {
                mr.mat = mat;
                return mr;
            } else {
                mat = imread(path.toLocal8Bit().data(), IMREAD_COLOR);
            }
        }
        //加载三通道bmp格式图片时，某些图有可能会按照4通道去加载，导致图片完全透明
        if (modes == IMREAD_UNCHANGED && mat.channels() == 4) {
            //以任何可能的颜色格式返回
            mat = imread(path.toLocal8Bit().data(), IMREAD_ANYCOLOR);
        } else {
            //否则按原样返回图像格式
            mat = imread(path.toLocal8Bit().data(), modes);
        }
    }
    //其他情况下尝试正常读取图像
    if (!mat.data) {
        mat = imread(path.toLocal8Bit().data(), modes);
        //加载三通道bmp格式图片时，某些图有可能会按照4通道去加载，导致图片完全透明
        if (modes == IMREAD_UNCHANGED && suffix == "bmp") {
            if (mat.channels() == 4) {
                mat = imread(path.toLocal8Bit().data(), IMREAD_ANYCOLOR);
            }
        } else if (modes == IMREAD_UNCHANGED && suffix == "jpeg") { // 4通道的jpeg图片会读取不出来,单独处理
            if (mat.channels() == 4) {
                mat = imread(path.toLocal8Bit().data(), IMREAD_COLOR);
            }
        }
        //通用加载方法的缩略图尺寸优化
        if (mat.cols < size.width() && mat.rows < size.height() && modes == IMREAD_REDUCED_COLOR_8) {
            mat = imread(path.toLocal8Bit().data());
        }
    }
    mr.mat = mat;
    return mr;
}

MatResult LingmoImageCodec::loadMovieToMat(QString path, ImreadModes modes, QString suffix)
{
    MatResult mr;
    Mat mat;
    //如果是缩略图节则省资源
    if (modes == IMREAD_REDUCED_COLOR_8) {
        QImage img(path, "apng");
        mat = Mat(img.height(), img.width(), CV_8UC4, const_cast<uchar *>(img.bits()),
                  static_cast<size_t>(img.bytesPerLine()))
                  .clone();
        mr.mat = mat;
        return mr;
    }
    if (suffix == "png") {
        mat = imread(path.toLocal8Bit().data(), modes).clone();
        mr.mat = mat;
        return mr;
    }
    auto *tmpMovie = new QMovie(path, "apng");
    mr.maxFrame = tmpMovie->frameCount();
    tmpMovie->jumpToFrame(0);
    QImage image = tmpMovie->currentImage();
    mat = Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar *>(image.bits()),
              static_cast<size_t>(image.bytesPerLine()))
              .clone();
    mr.mat = mat;
    //如果大于一帧
    if (tmpMovie->frameCount() > 1) {
        mr.matList = new QList<Mat>;
        //获取帧率
        mr.fps = getDelay(path, suffix);
        //获取第二帧
        tmpMovie->jumpToFrame(1);
        image = tmpMovie->currentImage();
        Mat secondMat = Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar *>(image.bits()),
                            static_cast<size_t>(image.bytesPerLine()))
                            .clone();
        //存放前两帧
        mr.matList->append(mat);
        mr.matList->append(secondMat);
        //从第三帧开始，在线程中加载
        LoadMovie *loadMovie = new LoadMovie(mr.matList, tmpMovie);
        connect(loadMovie, &LoadMovie::loadMovieFinish, getSignalObj(), &LingmoImageCodecSignals::loadMovieFinish);
        loadMovie->start();
    }
    //临界值为2时，在线程中回收
    if (tmpMovie->frameCount() < 2) {
        tmpMovie->deleteLater();
        Q_EMIT getSignalObj()->loadMovieFinish(path);
    }
    return mr;
}

int LingmoImageCodec::gifDelay(const QString &path)
{
    int j = 0;
    int frameDelay = 0;
    SavedImage *
        frame; //这个是系统中保存gif的扩展块的信息，不太了解gif的文件结构的可以看一下：http://blog.csdn.net/wzy198852/article/details/17266507
    ExtensionBlock *ext = nullptr; //这个保存gif延时代码块的结构体

    int err = 0;
    GifFileType *gifFileType = DGifOpenFileName(path.toLocal8Bit().data(), &err);
    if (err != 0) {
        DGifCloseFile(gifFileType, &err);
        return 0;
    }
    DGifSlurp(gifFileType);
    frame = &gifFileType->SavedImages[0]; //拿到第一个图片相关信息
    for (j = 0; j < frame->ExtensionBlockCount; j++) {
        //找到含有延迟时间的代码块
        if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
            ext = &(frame->ExtensionBlocks[j]);
        }
        //拿到延迟时间
        if (ext) {
            //延迟时间1-->10ms
            frameDelay = 10 * (ext->Bytes[2] << 8 | ext->Bytes[1]); //拿到延迟时间
            break;
        }
    }
    DGifCloseFile(gifFileType, &err);
    return frameDelay;
}

int LingmoImageCodec::getDelay(const QString &path, const QString &suffix)
{
    int fps = 0;

    if (suffix == "gif") {
        fps = gifDelay(path);
    }

    //    else if (suffix == "apng" || suffix == "png") {
    //    qDebug()<<"===此处待补充===";
    //}

    if (fps != 0) {
        return fps;
    }

    //以上都没获取到fps的保底机制
    // QMovie没有现成的方法，为求稳定，每次打开新动图的时候循环5次取最大值，此处待优化
    QMovie tmpMovie(path);
    if (fps == 0) {
        for (int i = 0; i < 5; i++) {
            tmpMovie.start();
            tmpMovie.stop();
            fps = tmpMovie.nextFrameDelay() > fps ? tmpMovie.nextFrameDelay() : fps;
        }
    }

    return fps == 0 ? 100 : fps;
}

Mat LingmoImageCodec::loadFreeimageFormat(const char *filename)
{
    Mat mat;
    FIBITMAP *dib = NULL;

    QFileInfo fi(QString::fromUtf8(filename));
    QString suffix = fi.suffix();
    QString file_suffix_lower = suffix.toLower();
    QImageReader reader;
    QImage res_qt;

    reader.setFileName(QString(filename));
    reader.setFormat(file_suffix_lower.toLatin1());
    reader.setAutoTransform(true);

    if(reader.imageCount() >= 0){
        res_qt = reader.read();
        if (res_qt.isNull()) {
            qDebug()<<"获得图片格式失败或不支持此格式图片的读取";
            return mat;
        }
    }
    /* 获取文件格式 */
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    fif = get_real_format(QString(filename));

    if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
        dib = FreeImage_Load(fif, filename, 0);
        if (dib != NULL) {
                mat = FI2MAT(dib);
        }
        FreeImage_Unload(dib);
    } else {
        qDebug() << "图像加载失败";
    }

    return mat;
}

FREE_IMAGE_FORMAT LingmoImageCodec::get_real_format(const QString &path)
{
    /* 获取后缀 */
    QFileInfo file_info(path);
    QString file_suffix_upper = file_info.suffix().toUpper();

    /* 用库获取文件格式 */
    QByteArray temp_path;
    temp_path.append(path.toUtf8());
    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(temp_path.data());

    /* 修复后缀 , 以库解析出来的为准 */
    int formatCode;
    if (m_formats.contains(file_suffix_upper)) {
        formatCode = m_formats.value(file_suffix_upper);
    }
    if (format != FIF_UNKNOWN && format != formatCode) {
        file_suffix_upper = m_formats.key(format);
    }
    if (format == FIF_TIFF) {
        file_suffix_upper = "TIFF";
    }
    /* 不支持格式 , 重新解析 , 防止后缀错误 */
    if (!m_formats.contains(file_suffix_upper)) {
        file_suffix_upper.clear();
    }

    /* 从文件数据解析文件格式 */
    if (file_suffix_upper.isEmpty()) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return FIF_UNKNOWN;
        }
        const QByteArray data = file.read(64);

        /* Check bmp file */
        if (data.startsWith("BM")) {
            return FIF_BMP;
        }

        /* Check dds file */
        if (data.startsWith("DDS")) {
            return FIF_DDS;
        }

        /* Check gif file */
        if (data.startsWith("GIF8")) {
            return FIF_GIF;
        }

        /* Check Max OS icons file */
        if (data.startsWith("icns")) {
            return FIF_UNKNOWN;
        }

        /* Check jpeg file */
        if (data.startsWith("\xff\xd8")) {
            return FIF_JPEG;
        }

        /* Check mng file */
        if (data.startsWith("\x8a\x4d\x4e\x47\x0d\x0a\x1a\x0a")) {
            return FIF_MNG;
        }

        //        /* Check net pbm file (BitMap) */
        if (data.startsWith("P1") || data.startsWith("P4")) {
            return FIF_PBM;
        }

        //        /* Check pgm file (GrayMap) */
        if (data.startsWith("P2") || data.startsWith("P5")) {
            return FIF_PGM;
        }

        //        /* Check ppm file (PixMap) */
        if (data.startsWith("P3") || data.startsWith("P6")) {
            return FIF_PPM;
        }

        //        /* Check png file */
        if (data.startsWith("\x89PNG\x0d\x0a\x1a\x0a")) {
            return FIF_PNG;
        }

        /* Check svg file */
        if (data.indexOf("<svg") > -1) {
            return FIF_UNKNOWN;
        }

        /* Check tiff file */
        if (data.startsWith("MM\x00\x2a") || data.startsWith("II\x2a\x00")) {
            // big-endian, little-endian.
            return FIF_TIFF;
        }

        /* Check webp file */
        if (data.startsWith("RIFFr\x00\x00\x00WEBPVP")) {
            return FIF_WEBP;
        }

        /* Check xbm file */
        if (data.indexOf("#define max_width ") > -1 && data.indexOf("#define max_height ") > -1) {
            return FIF_XBM;
        }

        /* Check xpm file */
        if (data.startsWith("/* XPM */")) {
            return FIF_XPM;
        }
        return FIF_UNKNOWN;
    }

    /* 解决为空时 , 三目运算返回错误问题 */
    if (file_suffix_upper.isEmpty()) {
        return FIF_UNKNOWN;
    }

    /* 根据后缀获取对应的格式 */
    int formatCode1 = m_formats.value(file_suffix_upper);
    format = static_cast<FREE_IMAGE_FORMAT>(formatCode1);


    return format >= 0 ? format : FIF_UNKNOWN;
}
