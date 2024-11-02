#ifndef PHOTOVIEWERPLUGIN_H
#define PHOTOVIEWERPLUGIN_H

#include <QObject>
#include <QDebug>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include <QPdfWriter>
#include <QPrinter>
#include <QDate>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#if !defined(__loongarch64) && !defined(__riscv)
#include <onnxruntime/onnxruntime_cxx_api.h>
#endif
#include "lingmoimagecodec.h"
using namespace cv;
using namespace std;

class PhotoViewerPlugin : public QObject
{
    Q_OBJECT
public:
    explicit PhotoViewerPlugin(QObject *parent = nullptr);
    //抠图
    QString startMatting(QString origImagePath, uint b = 255, uint g = 255, uint r = 255, uint a = 0);
    //扫描
    QString startScan(const QString &origImagePath, QString watermarkText = "");
    //转PDF
    QString exportPDF(const QStringList &imagePaths);

//private:
    //抠图
    Mat m_origMattingMat;
    Mat m_transparentMat;
    Mat m_resultMat;
    bool m_flag = false;                                           //判断是否更换透明背景
    bool isMattingImagePath(const QString &path);                  //判断输入参数是否为有效图片路径
    void matToTransparent();                                       //将图片前景背景分割，背景置为透明
    void needSaveMattingImage(Mat mat);                            //保存图片
    //扫描黑白件
    QString m_dirPath = "";                                        //扫描件保存路径
    QString watermarkText;                                         //水印文字
    QPrinter * printer;                                            //pdf打印
    QStringList notsupportWaterMark;                               //不支持水印保存的格式（修改为png格式）
    bool isScanImagePath(const QString &path);                     //判断输入参数是否为有效图片路径
    Mat scanImage(const Mat &origMat);                             //扫描为黑白图片
    void needSaveScanImage(const Mat &mat);                        //保存为图片
    QPixmap addWatermar(const Mat &mat, QString watermarkText);    //添加水印
    void saveAsPDF(const QStringList &imagePaths);                 //保存为pdf
    void savePath();                                               //返回扫描件保存路径:图片/文档处理

    QProcess *m_process = nullptr;
    QString m_savePath;                                            //完成保存路径
    QString m_origImagePath;                                       //输入原图路径
    QString m_imageRealFormat;                                     //输入原图图片真实格式
    QString realFormat(const QString &path);                       //获取图片真实格式
    void processStart(const QString &cmd, QStringList arguments);
};

#endif // PHOTOVIEWERPLUGIN_H
