#ifndef PROCESSING_H
#define PROCESSING_H

#include "global/variable.h"
#include <QPixmap>

#include "flip.h"

using namespace cv;

class Processing : public ProcessingBase
{
public:
    enum ProcessingType { flip };
    enum MousePosType {
        OUTSIDE = -1,
        INSIDE = 0,
        TOP = 1,
        LEFT = 2,
        LEFT_TOP = 3,
        RIGHT_BOTTOM = 4,
        LEFT_BOTTOM = 5,
        RIGHT_TOP = 6,
        BOTTOM = 7,
        RIGHT = 8
    };
    static Mat processingImage(const ProcessingType &type, const Mat &mat, const QVariant &args);
    static QPixmap converFormat(const Mat &mat);
    static QPixmap resizePix(const QPixmap &pixmap, const QSize &size);
    static QImage pictureDeepen(const QImage &img, const QSize &hightlightSize, const QPoint &point); //图片加深
    static QImage pictureCutImageDeepen(const QImage &img, const QSize &hightlightSize, const QPoint &startPoint,
                                        const QImage &imgBefore, const QSize &hightlightSizeBefore,
                                        QPoint changePoint);                   //图片加深
    static QImage pictureCutDeepen(const QImage &img, const QSize &imageSize); //裁剪-图片加深
    static QPixmap localAmplification(const QPixmap &orgPix, QSize showSize, QPoint local, QSize widSize);
    static QImage pictureRelease(const QImage &img, const QImage &originImage); //画鼠标释放的裁剪框
    static QPoint releaseStart();                                               //裁剪框起点
    static QPoint releaseEnd();                                                 //裁剪框终点
    static QImage pictureCutChange(int type, int backtype, const QImage &img, QPoint &startP, QPoint &endP,
                                   QPoint &changeP, const QPoint &backP, const QImage &origCutImageBefore);
    static QImage pictureCutSave(const QImage &img, const QPoint &startCutP,
                                 const QPoint &endCutP); //画鼠标释放的裁剪框

private:
    static inline int minNumIsZero(const int &num1, const int &num2);
    static QList<cutBoxColor> m_cutBoxChange;
    static QList<cutBoxColor> pictureColor(QPoint &backPoint, const QPoint &nextPoint,
                                           const QPoint &startPoint); //返回需要处理的界面图片裁剪矩形显示
    static cutBoxColor regionColor(const QPoint &startP, const QPoint &endP, int key); //返回需要处理的矩形设置
    static void regionBoundry(const QPoint &startP, const QPoint &changeP);            //计算框选区域
    //    static void regionBoundryAgian( QPoint &startP,  QPoint &changeP,  QPoint
    //    &endP);//计算框选区域-释放后在释放的情况
    static QList<cutBoxColor> releaseSquare(const QPoint &startP, const QPoint &endP,
                                            const QImage &img); //计算框选区域释放时的8个小方块的起始点和终点
    static QList<cutBoxColor> pictureCutPainter(const QImage &img, int type, int backtype, const QPoint &startP,
                                                QPoint &endP, QPoint &changeP,
                                                QPoint &backChangeP); //裁剪后-对图片进行改变
    static QList<cutBoxColor> pictureCutMove(const QImage &img, const QPoint &startCutP, const QPoint &endCutP,
                                             int xOffset, int yOffset); //移动鼠标释放的裁剪框
    static inline int boundaryJudg(int max, int point);                 //边界处理
    static int rectBoundaryJudg(int contrast, int target, bool isNeedMin);
};

#endif // PROCESSING_H
