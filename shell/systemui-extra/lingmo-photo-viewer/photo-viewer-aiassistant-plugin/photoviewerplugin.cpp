#include "photoviewerplugin.h"

PhotoViewerPlugin::PhotoViewerPlugin(QObject *parent) : QObject(parent)
{

}

bool PhotoViewerPlugin::isMattingImagePath(const QString &path)
{
    if (QFileInfo::exists(path)) {
        //不是支持的文件格式忽略
        QString format = path;
        format = format.split(".").last();
        const QStringList supportFormats = LingmoImageCodec::m_supportFormats;
        //const QStringList unSupportFormats = {"ico", "ras", "svg", "gif", "apng", "j2k", "wbmp", "xbm", "tga"};
        if (supportFormats.contains(format.toLower())) {
            return true;
        } else {
            qDebug() << "不支持此格式图片进行背景更换";
            return false;
        }
    }
    return false;
}

void PhotoViewerPlugin::matToTransparent()
{
    // 检查图像是否为四通道,四通道转为三通道
    if (m_origMattingMat.channels() != 3) {
        cv::cvtColor(m_origMattingMat, m_origMattingMat, cv::COLOR_BGRA2BGR);
    }
    if (m_origMattingMat.channels() != 3) {
        qDebug() << "Please input RGB image.";
        return;
    }
#if !defined(__loongarch64) && !defined(__riscv)
    // 调整图像大小为模型期望的尺寸
    int inputWidth = 512;
    int inputHeight = 512;
    const std::vector<float> mean = {0.5, 0.5, 0.5};
    const std::vector<float> std = {0.5, 0.5, 0.5};
    cv::Mat image;
    cv::cvtColor(m_origMattingMat, image, cv::COLOR_BGR2RGB);
    image.convertTo(image, CV_32FC3, 1.0f / 255.0f);
    cv::Mat mean_mat(image.size(), CV_32FC3, cv::Scalar(mean[0], mean[1], mean[2]));
    cv::subtract(image, mean_mat, image);
    cv::Mat std_mat(image.size(), CV_32FC3, cv::Scalar(std[0], std[1], std[2]));

    cv::divide(image, std_mat, image);
    cv::resize(image, image, cv::Size(inputWidth, inputHeight), cv::INTER_LINEAR);
    image = cv::dnn::blobFromImage(image);
    // 创建ORT环境
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "Test");
    // 创建会话选项
    Ort::SessionOptions sessionOptions;
    // 加载模型
    const char* model_path = "/usr/share/lingmo-photo-viewer/model/lingmo_photo_viewer_matting_model.onnx";
    Ort::Session session(env, model_path, sessionOptions);
    // 准备输入数据
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
    size_t input_tensor_size  = inputWidth * inputHeight * 3;
    std::vector<int64_t> input_shape = {1, 3, inputHeight, inputWidth};
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, (float*)image.data, input_tensor_size, input_shape.data(), 4);
    std::vector<const char*> input_names = {nullptr};
    std::vector<const char*> output_names = {nullptr};
    const char* input_name = "input";
    const char* output_name = "output";
    input_names[0] = input_name;
    output_names[0] = output_name;

    std::vector<float> output(1 * 1 * inputHeight * inputWidth);
    const std::vector<int64_t> outputShape = {1, 1, inputHeight, inputWidth};
    Ort::Value outputTensor = Ort::Value::CreateTensor<float>(
        Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeDefault),
        output.data(), output.size(), outputShape.data(), outputShape.size());

    Ort::RunOptions runOpts(nullptr);
    session.Run(runOpts, input_names.data(), &input_tensor, input_names.size(),
                output_names.data(), &outputTensor, output_names.size());

    float* output_data = outputTensor.GetTensorMutableData<float>();
    cv::Mat matte_np(inputHeight, inputWidth, CV_32F, output_data);

    // 调整灰度图大小与原始图像大小相匹配
    cv::Mat matte_resized;
    cv::resize(matte_np, matte_resized, cv::Size(m_origMattingMat.cols, m_origMattingMat.rows), 0, 0, cv::INTER_LINEAR);
    // 将抠图结果中背景部分设置为透明色
    matte_resized.convertTo(m_transparentMat, CV_8U, 255); // 将灰度图像转换为 8 位图像
#else
    return;
#endif
}

QString PhotoViewerPlugin::startMatting(QString origImagePath, uint b, uint g, uint r, uint a)
{
    m_origImagePath = origImagePath;
    m_imageRealFormat = realFormat(origImagePath);

    if(!isMattingImagePath(origImagePath)) {
        qDebug() << "输入图片格式错误";
        return "";
    }

    m_origMattingMat = LingmoImageCodec::loadImageToMat(m_origImagePath).mat;
    if (!m_origMattingMat.data) {
        qDebug() << "无法读取原图片!";
        return "";
    }
    matToTransparent();

    Mat result(m_origMattingMat.size(), CV_8UC4);
    for (int y = 0; y < m_origMattingMat.rows; ++y) {
        for (int x = 0; x < m_origMattingMat.cols; ++x) {
            if (m_transparentMat.at<uchar>(y, x) >= 200) {
                cv::Vec3b bgr = m_origMattingMat.at<cv::Vec3b>(y, x);
                result.at<cv::Vec4b>(y, x) = cv::Vec4b(bgr[0], bgr[1], bgr[2], 255); //前景
            } else {
                result.at<cv::Vec4b>(y, x) = cv::Vec4b(b, g, r, a);  //背景
            }
        }
    }
    if (b == 255 && g == 255 && r == 255 && a == 0) {
        m_flag = true;
    }
    needSaveMattingImage(result);
    return m_savePath;
}

void PhotoViewerPlugin::needSaveMattingImage(Mat mat)
{
    m_resultMat = mat.clone();
    QString picPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QString dirPath = picPath + "/" + tr("抠图");
    QDir dir;
    if (!dir.exists(dirPath)) {
        if (!dir.mkpath(dirPath)) {
            qDebug() << "创建抠图目录失败";
            return;
        } else {
            qDebug() << "创建抠图目录成功 dir path === " << dirPath;
        }
    }
    QFileInfo fileInfo(m_origImagePath);
    QString filename = fileInfo.fileName();
    QString path = dirPath + "/" +filename;
    if (m_flag) {
        int dotPos = path.lastIndexOf('.');
        //当保存透明背景图片时，都保存为png格式
        if (path.right(dotPos) != "png") {
            QString baseName = path.left(dotPos);
            path = baseName + ".png";
        }
        m_savePath = LingmoImageCodec::saveWay(path, false);
        if (cv::imwrite(m_savePath.toStdString(), m_resultMat)) {
            qDebug() << "成功-保存透明背景图片 save path == " << m_savePath;
        } else {
            qDebug() << "失败-保存透明背景图片";
        }
    } else {
        m_savePath = LingmoImageCodec::saveWay(path, false);
        cv::cvtColor(m_resultMat, m_resultMat, cv::COLOR_BGRA2BGR);
        if (LingmoImageCodec::save(m_resultMat, m_savePath, m_imageRealFormat)) {
            qDebug() << "成功-保存普通背景图片 save path == " << m_savePath;
        } else {
            qDebug() << "失败-保存普通背景图片";
        }
    }
}

bool PhotoViewerPlugin::isScanImagePath(const QString &path)
{
    if (QFileInfo::exists(path)) {
        //是否是支持的图片
        QString format = path;
        format = format.split(".").last();
        const QString supportFormats = "(*.jpg *.jpe *.jpeg);;"
                                       "(*.pbm);;"
                                       "(*.pgm *.ppm);;"
                                       "(*.tiff *.tif);;"
                                       "(*.png);;"
                                       "(*.ico);;(*.sr);;"
                                       "(*.ras);;"
                                       "(*.svg);;(*.gif);;"
                                       "(*.webp);;(*.bmp);;"
                                       "(*.jp2 *.j2k *.jng);;(*.wbmp);;"
                                       "(*.xbm *.xpm)";
//        QStringList notsupportScan = {"psd","dib","tga","exr","jfi","jif","pnm"};
        notsupportWaterMark<<"j2k"<<"psd"<<"jp2"<<"jng"<<"exr"<<"dib"<<"ras"<<"tga"<<"sr"<<"svg"<<"gif"<<"jpe";
        if (supportFormats.contains(format.toLower())) {
            return true;
        } else {
            qDebug() << "不支持此格式图片进行黑白件扫描"<<format.toLower();
            return false;
        }
    }
    return false;
}

QString PhotoViewerPlugin::startScan(const QString &origImagePath, QString watermarkText)
{
    m_origImagePath = origImagePath;
    m_imageRealFormat = realFormat(origImagePath);
    if(!isScanImagePath(origImagePath)) {
        qDebug() << "输入图片格式错误";
        return "";
    }

    Mat origImageMat = LingmoImageCodec::loadImageToMat(m_origImagePath).mat;
    if (origImageMat.empty()) {
        qDebug() << "无法读取原图片!";
        return "";
    }
    //扫描
    Mat scanMat = scanImage (origImageMat);
    if(scanMat.empty()){
        qDebug()<<"扫描失败";
        return "";
    }

    if(watermarkText != ""){
        //水印
        QPixmap watermarkPix = addWatermar(scanMat, watermarkText);
        if(watermarkPix.isNull()){
            qDebug()<<"添加水印失败";
            return "";
        }
        //水印图片保存路径
        savePath();
        if(m_dirPath != ""){
            QFileInfo fileInfo(m_origImagePath);
            QString fileName = fileInfo.fileName();
            m_savePath = m_dirPath + "/" + fileName;
            if(notsupportWaterMark.contains(m_imageRealFormat)){
                QString fileName = fileInfo.baseName() + ".png";
                m_savePath = m_savePath = m_dirPath + "/" + fileName ;
                qDebug()<<" 保存为png格式 "<< fileName;
            }
        }else{
            qDebug()<<"获取水印图片保存路径失败";
            return "";
        }
        //保存图片
        if(watermarkPix.save(m_savePath)){
            qDebug() << "水印图片保存成功 savepath = " << m_savePath;
        }else{
            qDebug() << "水印图片保存失败 ";
            return "";
        }
    }else{
        //保存图片
        needSaveScanImage(scanMat);
    }

    return m_savePath;
}

Mat PhotoViewerPlugin::scanImage(const Mat &origMat)
{
    Mat opMat = origMat.clone();
    //灰度
    Mat grayMat = opMat.clone();
    if(opMat.channels() != 1){
        cvtColor(opMat, grayMat, COLOR_BGR2GRAY);
    }
    //双边滤波-降噪
    Mat filtMat = grayMat.clone() ;
    bilateralFilter(grayMat, filtMat, 10, 10, 10);       //src和dst需不同

    //黑白二值图像
    Mat binaryMat = filtMat.clone();
    double maxValue = 255;
    cv::adaptiveThreshold(filtMat, binaryMat, maxValue,
                          cv::ADAPTIVE_THRESH_GAUSSIAN_C, //自动阈值计算
                          cv::THRESH_BINARY,
                          25,  // blockSize 必须是奇数
                          9);  // -C 用于微调阈值
    return binaryMat;
}

void PhotoViewerPlugin::needSaveScanImage(const Mat &mat)
{
    //扫描图片
    Mat m_resultMat = mat.clone();
    //扫描图片保存路径
    savePath();
    if(m_dirPath != ""){
        QFileInfo fileInfo(m_origImagePath);
        QString fileName = fileInfo.fileName();
        m_savePath = m_dirPath + "/" + fileName ;
    }else{
        qDebug()<<"获取扫描图片保存路径失败";
        m_savePath = "";
        return;
    }
    //保存扫描图片
    if (LingmoImageCodec::saveImage(m_resultMat, m_savePath, m_imageRealFormat)) {
        qDebug() << "扫描图片保存成功 savepath = " << m_savePath;
    } else {
        qDebug() << "扫描图片保存失败 ";
        m_savePath = "";
        return;
    }
}

QPixmap PhotoViewerPlugin::addWatermar(const Mat &mat, QString watermarkText)
{
    //准备绘制水印的图片、文本和设置
    QPixmap pix = LingmoImageCodec::converFormat(mat);
    qDebug()<<"开始添加水印 pix"<<pix.isNull();

    QFont font("Microsoft YaHei",14);    //设置字体大小
    font.setItalic(true);                //斜体
    QColor textColor(128, 128, 128, 51); //半透明水印文字
    //计算绘制文本字符串的宽和高
    QFontMetricsF fontMetrics(font);
    qreal font_w = fontMetrics.width(watermarkText);
    qreal font_h = fontMetrics.height();
    //旋转角度
    qreal ang = 45.0;
    //开始绘图
    QPainter painter(&pix);
    painter.setFont(font);
    painter.setPen(textColor);
    painter.translate(pix.width() / 2, -pix.width() / 2);  //平移原点
    painter.rotate(ang);
    int project_Y = abs(pix.width() * sin(ang)) + abs(pix.height() * sin(ang));//原图像Y坐标在新坐标系Y轴上的投影长度
    int project_X = abs(pix.height() * sin(ang)) + abs(pix.width() * cos(ang));//原图像x坐标在新坐标系x轴上的投影长度
    //水印间的距离
    int x_step =  2*font_w;
    int y_step = (3*font_h);
    //水印的行数与列数
    int rowCount = project_Y / y_step;
    int colCount = project_X / x_step + 2;//水印的行数  因为旋转了，如果不加2会导致水印缺少一块
    for (int r = 0; r < rowCount; r++){
        for (int c = 0; c < colCount; c++){
            painter.drawText(x_step * c, y_step * r,watermarkText);//写水印
        }
    }
    return pix;
}

void PhotoViewerPlugin::saveAsPDF(const QStringList &imagePaths)
{
    //PDF保存路径
    savePath();
    if(m_dirPath != ""){
        if(imagePaths.length() == 1){
            QFileInfo fileInfo(imagePaths[0]);
            QString baseName = fileInfo.baseName();
            m_savePath = m_dirPath + "/" + baseName + ".pdf";
        }else if(imagePaths.length() > 1){
            QString dateString = QDate::currentDate().toString("yyyyMMdd");
            m_savePath = m_dirPath + "/" + "images_pdf_" + dateString + ".pdf";
        }else{
            qDebug()<<"输入的PDF图片路径列表错误 imagePaths ="<<imagePaths;
            return;
        }
    }else{
        qDebug()<<"获取PDF文件保存路径失败";
        m_savePath = "";
        return;
    }
    //打印机设置
    printer = new QPrinter();
    printer->setOutputFormat(QPrinter::PdfFormat);
    printer->setOutputFileName(m_savePath);
    printer->setPaperSize(QPrinter::A4);
    printer->setOrientation(QPrinter::Portrait);
    printer->setResolution(72);
    QPainter painter(printer);
    for (const QString &imagePath : imagePaths) {
        Mat mat = LingmoImageCodec::loadImageToMat(imagePath).mat;
        QImage printImage = LingmoImageCodec::converFormat(mat).toImage();
        if (printImage.isNull()) {
            qDebug() << "图像加载错误" << imagePath;
            continue;
        }
        //绘制图像
        QSize pageSize = printer->pageRect().size();
        QSize imageSize = printImage.size();
        imageSize.scale(pageSize, Qt::KeepAspectRatio);
        QPointF startPoint((pageSize.width() - imageSize.width()) / 2.0, (pageSize.height() - imageSize.height()) / 2.0);
        painter.drawImage(QRectF(startPoint, imageSize), printImage);

        if (imagePath != imagePaths.last()) {
            printer->newPage(); // 创建新的PDF页面
        }
    }
    painter.end();
}

void PhotoViewerPlugin::savePath()
{
    //保存路径
    QString picPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    //m_dirPath = picPath + "/" + tr("Docs Processing");
    m_dirPath = picPath + "/" + tr("文档处理");
    QDir dir;
    if (!dir.exists(m_dirPath)) {
        if (!dir.mkpath(m_dirPath)) {
            m_dirPath = "";
            qDebug() << "创建文档处理目录失败";
        } else {
            qDebug() << "创建文档处理目录成功 dirpath = " << m_dirPath;
        }
    }
}

QString PhotoViewerPlugin::realFormat(const QString &path)
{
    m_imageRealFormat = "";
    m_imageRealFormat = LingmoImageCodec::m_formats.key(LingmoImageCodec::get_real_format(path));
    if (m_imageRealFormat == "") {
        processStart("file", QStringList() << "-i" << path);
        if (m_imageRealFormat != "") {
            if (m_imageRealFormat.contains("image/")) {
                m_imageRealFormat = m_imageRealFormat.mid(m_imageRealFormat.indexOf(":") + 2);
                m_imageRealFormat = m_imageRealFormat.left(m_imageRealFormat.indexOf(";"));
            } else {
                m_imageRealFormat = "";
            }
        }
    }
    if (m_imageRealFormat.startsWith("image/")) {
        if ("image/svg+xml" == m_imageRealFormat) {
            m_imageRealFormat = "svg";
        } else if ("image/x-sun-raster" == m_imageRealFormat) {
            m_imageRealFormat = "sr";
        } else if ("image/x-portable-anymap" == m_imageRealFormat) {
            m_imageRealFormat = "pnm";
        } else {
            qDebug() << "不支持的format";
        }
    } else {
        m_imageRealFormat = m_imageRealFormat.toLower();
    }
    //校准
    if ("" != m_imageRealFormat) {
        QFileInfo file(path);
        QString fileFormat = file.suffix().toLower();
        // jpeg
        if (LingmoImageCodec::g_jpegList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_jpegList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "jpeg";
            }
        }
        //{"bmp", "dib"};
        if (LingmoImageCodec::g_bmpList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_bmpList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "bmp";
            }
        }
        //{"png", "apng"}
        if (LingmoImageCodec::g_pngList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_pngList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "png";
            }
        }
        //{"fax", "g3"}
        if (LingmoImageCodec::g_faxList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_faxList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "fax";
            }
        }
        //{"koa", "koala"}
        if (LingmoImageCodec::g_koaList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_koaList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "koa";
            }
        }
        //{"lbm", "iff"}
        if (LingmoImageCodec::g_lbmList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_lbmList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "lbm";
            }
        }
        //{"tga", "targa"}
        if (LingmoImageCodec::g_tgaList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_tgaList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "tga";
            }
        }
        //{"tiff", "tif"}
        if (LingmoImageCodec::g_tiffList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_tiffList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "tiff";
            }
        }
        //{"pct", "pic", "pict"}
        if (LingmoImageCodec::g_pictList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_pictList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "pict";
            }
        }
        //{"ppm", "pbm", "pgm"}
        if (LingmoImageCodec::g_pbmList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_pbmList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "pbm";
            }
        }
        if (LingmoImageCodec::g_otherFormatList.contains(m_imageRealFormat)) {
            if (LingmoImageCodec::g_otherFormatList.contains(fileFormat) && fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            }
        }
    }

    return m_imageRealFormat;
}

void PhotoViewerPlugin::processStart(const QString &cmd, QStringList arguments)
{
    if (m_process == nullptr) {
        // File构造函数是在主线程调用的，m_process如果在构造函数中实例化，会报错
        m_process = new QProcess; //操作文件
        connect(m_process, &QProcess::readyReadStandardError, [=] {
            qDebug() << "*******process error*******\n"
                     << QString::fromLocal8Bit(m_process->readAllStandardError()) << "\n*******process error*******";
        });
        connect(m_process, &QProcess::readyReadStandardOutput, [=] {
            m_imageRealFormat = QString::fromLocal8Bit(m_process->readAll());
        });
        m_process->setReadChannel(QProcess::StandardOutput); //标准输出读取内容必须设置该属性
    }
    m_process->start(cmd, arguments);
    m_process->waitForStarted();
    m_process->waitForFinished();
    m_process->waitForReadyRead();
}
