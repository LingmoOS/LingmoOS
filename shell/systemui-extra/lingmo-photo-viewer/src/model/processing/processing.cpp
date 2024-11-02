#include "processing.h"
//无法类内初始化
static QPoint m_backPoint = QPoint(-1, -1);
static QPoint m_startP = QPoint(-1, -1);       //记录起点
static QPoint m_nextP = QPoint(-1, -1);        //记录变化的点
static QPoint m_regionStartP = QPoint(-1, -1); //记录框选起点
static QPoint m_regionEndP = QPoint(-1, -1);   //记录框选终点
static bool m_isReleasePaint = true;           //释放后翻转
static int m_xOffset;
static int m_yOffset;
static QPoint m_endPointCorrect =
    QPoint(-1, -1); //发现终点值一直不对，目前没有找到具体是哪里调整了这个值，每次变化在4相似以内
static int m_currentType = -1; //默认-1
Mat Processing::processingImage(const ProcessingType &type, const Mat &mat, const QVariant &args)
{
    switch (type) {
    case flip:
        Flip f;
        return f.processing(mat, args);
    }

    return Mat();
}

QPixmap Processing::converFormat(const Mat &mat)
{
    return LingmoImageCodec::converFormat(mat);
}

QPixmap Processing::resizePix(const QPixmap &pixmap, const QSize &size)
{
    return pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation); // 按比例缩放
}

QPixmap Processing::localAmplification(const QPixmap &orgPix, QSize showSize, QPoint local, QSize widSize)
{
    // orgPix——原图   showSize——目标大小   local——基于showSize的起始坐标   widSize——窗口尺寸
    double proportion = double(showSize.width()) / double(orgPix.width());
    QPoint locaPoint(local.rx() / proportion, local.ry() / proportion);
    QSize locaSize(widSize.width() / proportion, widSize.height() / proportion);
    QPixmap localPix = orgPix.copy(locaPoint.rx(), locaPoint.ry(), locaSize.width(), locaSize.height());

    return Processing::resizePix(localPix, widSize);
}
//绘制框选区域
QImage Processing::pictureRelease(const QImage &img, const QImage &originImage)
{
    //鼠标释放，重置第一次绘制裁剪时变量的值
    m_backPoint.setX(-1);
    m_backPoint.setY(-1);
    //终点值偏差，还未查出具体原因，做此方法规避一下
    if (m_endPointCorrect.x() != -1) {
        if (m_endPointCorrect.x() != m_regionEndP.x()) {
            m_regionEndP.setX(m_endPointCorrect.x());
        }
    }
    QImage image = img.copy();
    //描高亮区域边框
    for (int i = m_regionStartP.x(); i < m_regionEndP.x(); i++) {
        QColor colorDown(img.pixel(i, m_regionEndP.y()));
        colorDown.setRed(Variable::DEFAULT_WHITE_COLOR);
        colorDown.setGreen(Variable::DEFAULT_WHITE_COLOR);
        colorDown.setBlue(Variable::DEFAULT_WHITE_COLOR);
        image.setPixel(i, m_regionEndP.y(), colorDown.rgb());
        QColor colorTop(img.pixel(i, m_regionStartP.y()));
        colorTop.setRed(Variable::DEFAULT_WHITE_COLOR);
        colorTop.setGreen(Variable::DEFAULT_WHITE_COLOR);
        colorTop.setBlue(Variable::DEFAULT_WHITE_COLOR);
        image.setPixel(i, m_regionStartP.y(), colorTop.rgb());
    }
    for (int k = m_regionStartP.y(); k < m_regionEndP.y(); k++) {
        QColor colorLeft(img.pixel(m_regionStartP.x(), k));
        colorLeft.setRed(Variable::DEFAULT_WHITE_COLOR);
        colorLeft.setGreen(Variable::DEFAULT_WHITE_COLOR);
        colorLeft.setBlue(Variable::DEFAULT_WHITE_COLOR);
        image.setPixel(m_regionStartP.x(), k, colorLeft.rgb());
        QColor colorRight(img.pixel(m_regionEndP.x(), k));
        colorRight.setRed(Variable::DEFAULT_WHITE_COLOR);
        colorRight.setGreen(Variable::DEFAULT_WHITE_COLOR);
        colorRight.setBlue(Variable::DEFAULT_WHITE_COLOR);
        image.setPixel(m_regionEndP.x(), k, colorRight.rgb());
    }
    QList<cutBoxColor> cutSquare = releaseSquare(m_regionStartP, m_regionEndP, img);
    if (cutSquare.length() > 0) {
        //遍历列表，改变图层颜色
        for (int i = 0; i < cutSquare.length(); i++) {
            //加黑色描边
            for (int j = cutSquare.at(i).startPostion.y(); j < cutSquare.at(i).endPostion.y(); j++) {
                for (int k = cutSquare.at(i).startPostion.x(); k < cutSquare.at(i).endPostion.x(); k++) {
                    QColor color(img.pixel(k, j));
                    color.setRed(minNumIsZero(Variable::DEFAULT_BLACK_COLOR, 0));
                    color.setGreen(minNumIsZero(Variable::DEFAULT_BLACK_COLOR, 0));
                    color.setBlue(minNumIsZero(Variable::DEFAULT_BLACK_COLOR, 0));
                    image.setPixel(k, j, color.rgb());
                }
            }
            //填充白色
            for (int j = cutSquare.at(i).startPostion.y() + 1; j < cutSquare.at(i).endPostion.y() - 1; j++) {
                for (int k = cutSquare.at(i).startPostion.x() + 1; k < cutSquare.at(i).endPostion.x() - 1; k++) {
                    QColor color(img.pixel(k, j));
                    color.setRed(minNumIsZero(Variable::DEFAULT_WHITE_COLOR, 0));
                    color.setGreen(minNumIsZero(Variable::DEFAULT_WHITE_COLOR, 0));
                    color.setBlue(minNumIsZero(Variable::DEFAULT_WHITE_COLOR, 0));
                    image.setPixel(k, j, color.rgb());
                }
            }
        }
    }

    m_endPointCorrect = QPoint(-1, -1);
    return image;
}

QPoint Processing::releaseStart()
{
    m_isReleasePaint = false;
    return m_regionStartP;
}

QPoint Processing::releaseEnd()
{
    return m_regionEndP;
}
QImage Processing::pictureCutChange(int type, int backtype, const QImage &img, QPoint &startP, QPoint &endP,
                                    QPoint &changeP, const QPoint &backP, const QImage &origCutImageBefore)
{
    QList<cutBoxColor> cutBoxSet;
    QImage image = img.copy();
    QPoint startPoint = startP;
    QPoint endPoint = endP;
    QPoint changePoint = changeP;
    QPoint backChangeP = backP;

    //移动
    if (type == INSIDE) {
        if (backChangeP.x() == -1 && backChangeP.y() == -1) {
            backChangeP = changeP;
        }
        int xOffsetTemp = changeP.x() - backChangeP.x();
        int yOffsetTemp = changeP.y() - backChangeP.y();

        cutBoxSet = pictureCutMove(img, startP, endP, xOffsetTemp, yOffsetTemp);
        //更新起点终点
        startP = QPoint(startP.x() + xOffsetTemp, startP.y() + yOffsetTemp);
        endP = QPoint(endP.x() + xOffsetTemp, endP.y() + yOffsetTemp);
        //边界值处理
        startP.setX(boundaryJudg(img.width(), startP.x()));
        startP.setY(boundaryJudg(img.height(), startP.y()));
        endP.setX(boundaryJudg(img.width(), endP.x()));
        endP.setY(boundaryJudg(img.height(), endP.y()));
        //确定边框
        regionBoundry(startP, endP);
        //在框外
    } else if (type == OUTSIDE) {
        return image;
    } else { //其余情况
        //边界值处理
        changeP.setX(boundaryJudg(img.width(), changeP.x()));
        changeP.setY(boundaryJudg(img.height(), changeP.y()));

        cutBoxSet = pictureCutPainter(img, type, backtype, startPoint, endPoint, changePoint, backChangeP);
        //边界值处理
        startP.setX(boundaryJudg(img.width(), startP.x()));
        startP.setY(boundaryJudg(img.height(), startP.y()));
        endP.setX(boundaryJudg(img.width(), endP.x()));
        endP.setY(boundaryJudg(img.height(), endP.y()));
    }
    //反黑反白--cutBoxSet.at(i).key的值进行判断,key = 0;原色；key = 50;蒙层
    if (cutBoxSet.length() > 0) {
        //遍历列表，改变图层颜色
        for (int i = 0; i < cutBoxSet.length(); i++) {
            for (int j = cutBoxSet.at(i).startPostion.y(); j < cutBoxSet.at(i).endPostion.y(); j++) {
                for (int k = cutBoxSet.at(i).startPostion.x(); k < cutBoxSet.at(i).endPostion.x(); k++) {
                    QColor color(origCutImageBefore.pixelColor(k, j));
                    if (!color.alphaF()) {
                        if (!cutBoxSet.at(i).key) {
                            color.setAlphaF(0);
                        } else {
                            color.setAlphaF(0.4);
                        }
                        image.setPixel(k, j, color.rgba());
                        continue;
                    }
                    color.setRed(minNumIsZero(color.red(), cutBoxSet.at(i).key));
                    color.setGreen(minNumIsZero(color.green(), cutBoxSet.at(i).key));
                    color.setBlue(minNumIsZero(color.blue(), cutBoxSet.at(i).key));
                    image.setPixel(k, j, color.rgb());
                }
            }
        }
    }
    return image;
}


QImage Processing::pictureCutSave(const QImage &img, const QPoint &startCutP, const QPoint &endCutP)
{
    QImage image;
    for (int j = startCutP.y(); j < endCutP.y(); ++j) {
        for (int i = startCutP.x(); i < endCutP.x(); ++i) {
            QColor color(img.pixel(i, j));
            color.setRed(minNumIsZero(color.red(), 0));
            color.setGreen(minNumIsZero(color.green(), 0));
            color.setBlue(minNumIsZero(color.blue(), 0));
            image.setPixel(i, j, color.rgb());
        }
    }
    return image;
}

QImage Processing::pictureDeepen(const QImage &img, const QSize &hightlightSize, const QPoint &point)
{
    QImage image = img.copy();
    int key = Variable::PICTURE_DEEPEN_KEY;
    int left = point.x();
    int right = point.x() + hightlightSize.width();
    int top = point.y();
    //边界值限定
    int bottom = point.y() + hightlightSize.height();
    right = boundaryJudg(img.width(), right);
    bottom = boundaryJudg(img.height(), bottom);

    for (int j = 0; j < image.height(); ++j) {
        for (int i = 0; i < image.width(); ++i) {
            if (i >= left && i < right && j >= top && j < bottom) {
                continue; //高亮区域不处理
            }
            QColor color(image.pixelColor(i, j));
            if (!color.alphaF()) { //透明区域40%透明度的黑色
                color.setAlphaF(0.4);
                image.setPixel(i, j, color.rgba());
                continue;
            }
            color.setRed(minNumIsZero(color.red(), key));
            color.setGreen(minNumIsZero(color.green(), key));
            color.setBlue(minNumIsZero(color.blue(), key));
            image.setPixel(i, j, color.rgb());
        }
    }
    //绘制边框-画内边界
    for (int i = left; i < right; ++i) {
        i = boundaryJudg(right, i);
        QColor colorDown(img.pixel(i, top));
        colorDown.setRed(Variable::DEFAULT_WHITE_COLOR);
        colorDown.setGreen(Variable::DEFAULT_WHITE_COLOR);
        colorDown.setBlue(Variable::DEFAULT_WHITE_COLOR);
        image.setPixel(i, top, colorDown.rgb());
        QColor colorTop(img.pixel(i, bottom - 1));
        colorTop.setRed(Variable::DEFAULT_WHITE_COLOR);
        colorTop.setGreen(Variable::DEFAULT_WHITE_COLOR);
        colorTop.setBlue(Variable::DEFAULT_WHITE_COLOR);
        image.setPixel(i, bottom - 1, colorTop.rgb());
    }
    for (int k = top; k < bottom; ++k) {
        k = boundaryJudg(bottom, k);
        QColor colorLeft(img.pixel(left, k));
        colorLeft.setRed(Variable::DEFAULT_WHITE_COLOR);
        colorLeft.setGreen(Variable::DEFAULT_WHITE_COLOR);
        colorLeft.setBlue(Variable::DEFAULT_WHITE_COLOR);
        image.setPixel(left, k, colorLeft.rgb());
        QColor colorRight(img.pixel(right - 1, k));
        colorRight.setRed(Variable::DEFAULT_WHITE_COLOR);
        colorRight.setGreen(Variable::DEFAULT_WHITE_COLOR);
        colorRight.setBlue(Variable::DEFAULT_WHITE_COLOR);
        image.setPixel(right - 1, k, colorRight.rgb());
    }
    return image;
}

QImage Processing::pictureCutDeepen(const QImage &img, const QSize &imageSize)
{
    QImage image = img.copy();
    int key = Variable::PICTURE_DEEPEN_KEY;

    for (int j = 0; j < image.height(); ++j) {
        for (int i = 0; i < image.width(); ++i) {
            QColor color(image.pixelColor(i, j));
            if (!color.alpha()) { //透明区域40%透明度的黑色
                color.setAlphaF(0.4);
                image.setPixel(i, j, color.rgba());
                continue;
            }

            color.setRed(minNumIsZero(color.red(), key));
            color.setGreen(minNumIsZero(color.green(), key));
            color.setBlue(minNumIsZero(color.blue(), key));
            image.setPixel(i, j, color.rgb());
        }
    }
    return image;
}
QImage Processing::pictureCutImageDeepen(const QImage &img, const QSize &hightlightSize, const QPoint &startPoint,
                                         const QImage &imgBefore, const QSize &hightlightSizeBefore, QPoint changePoint)
{
    Q_UNUSED(hightlightSize);
    Q_UNUSED(hightlightSizeBefore);
    m_isReleasePaint = false;
    QList<cutBoxColor> cutBoxSet;
    QImage image = img;
    int yMax = image.height();
    int xMax = image.width();
    changePoint.setX(boundaryJudg(xMax, changePoint.x()));
    changePoint.setY(boundaryJudg(yMax, changePoint.y()));
    m_backPoint.setX(boundaryJudg(xMax, m_backPoint.x()));
    m_backPoint.setY(boundaryJudg(yMax, m_backPoint.y()));
    //返回-最多两个需要变化颜色的矩形大小，位置，颜色
    cutBoxSet = pictureColor(m_backPoint, changePoint, startPoint);
    m_startP = startPoint;
    m_nextP = changePoint;

    //反黑反白--cutBoxSet.at(i).key的值进行判断,key = 0;原色；key = 50;蒙层
    if (cutBoxSet.length() > 0) {
        //遍历列表，改变图层颜色
        for (int i = 0; i < cutBoxSet.length(); i++) {
            for (int j = cutBoxSet.at(i).startPostion.y(); j < cutBoxSet.at(i).endPostion.y() && j < yMax; j++) {
                for (int k = cutBoxSet.at(i).startPostion.x(); k < cutBoxSet.at(i).endPostion.x() && k < xMax; k++) {
                    QColor color(imgBefore.pixelColor(k, j));
                    if (!color.alphaF()) {
                        if (!cutBoxSet.at(i).key) {
                            color.setAlphaF(0);
                        } else {
                            color.setAlphaF(0.4);
                        }
                        image.setPixel(k, j, color.rgba());
                        continue;
                    }
                    color.setRed(minNumIsZero(color.red(), cutBoxSet.at(i).key));
                    color.setGreen(minNumIsZero(color.green(), cutBoxSet.at(i).key));
                    color.setBlue(minNumIsZero(color.blue(), cutBoxSet.at(i).key));
                    image.setPixel(k, j, color.rgb());
                }
            }
        }
    }
    //记录上一个点
    m_backPoint = changePoint;
    regionBoundry(m_startP, m_nextP);
    return image;
}
//返回-最多两个裁剪的矩形框信息
QList<cutBoxColor> Processing::pictureColor(QPoint &backPoint, const QPoint &nextPoint, const QPoint &startPoint)
{
    QList<cutBoxColor> cutBoxChange;

    if (backPoint.x() == -1 && backPoint.y() == -1) {
        backPoint = startPoint;
    }
    int widthDiff = nextPoint.x() - backPoint.x();
    int heightDiff = nextPoint.y() - backPoint.y();
    //在轴线
    if (backPoint.x() == startPoint.x() && nextPoint.x() == startPoint.x()) {
        if (backPoint.x() >= nextPoint.x()) {
            cutBoxChange.append(regionColor(nextPoint, backPoint, 50));
        } else {
            cutBoxChange.append(regionColor(backPoint, nextPoint, 50));
        }
        return cutBoxChange;
    }
    if (backPoint.y() == startPoint.y() && nextPoint.y() == startPoint.y()) {
        if (backPoint.y() >= nextPoint.y()) {
            cutBoxChange.append(regionColor(nextPoint, backPoint, 50));
        } else {
            cutBoxChange.append(regionColor(backPoint, nextPoint, 50));
        };
        return cutBoxChange;
    }
    if (backPoint.y() <= startPoint.y() && nextPoint.y() <= startPoint.y()) {
        //左上到右上
        if (backPoint.x() <= startPoint.x() && nextPoint.x() >= startPoint.x()) {
            cutBoxChange.append(regionColor(backPoint, startPoint, 50));
            cutBoxChange.append(
                regionColor(QPoint(startPoint.x(), nextPoint.y()), QPoint(nextPoint.x(), startPoint.y()), 0));
            return cutBoxChange;
        }
        //右上到左上
        if ((backPoint.x() >= startPoint.x()) && (nextPoint.x() <= startPoint.x())) {
            cutBoxChange.append(regionColor(nextPoint, startPoint, 0));
            cutBoxChange.append(
                regionColor(QPoint(startPoint.x(), backPoint.y()), QPoint(backPoint.x(), startPoint.y()), 50));
            return cutBoxChange;
        }
    }
    if (backPoint.y() >= startPoint.y() && nextPoint.y() >= startPoint.y()) {
        //右下到左下
        if (backPoint.x() >= startPoint.x() && nextPoint.x() <= startPoint.x()) {
            cutBoxChange.append(regionColor(startPoint, backPoint, 50));
            cutBoxChange.append(
                regionColor(QPoint(nextPoint.x(), startPoint.y()), QPoint(startPoint.x(), nextPoint.y()), 0));
            return cutBoxChange;
        }
        //左下到右下
        if (backPoint.x() <= startPoint.x() && nextPoint.x() >= startPoint.x()) {
            cutBoxChange.append(
                regionColor(QPoint(backPoint.x(), startPoint.y()), QPoint(startPoint.x(), backPoint.y()), 50));
            cutBoxChange.append(regionColor(startPoint, nextPoint, 0));
            return cutBoxChange;
        }
    }
    if (backPoint.x() >= startPoint.x() && nextPoint.x() >= startPoint.x()) {
        //右上到右下
        if (backPoint.y() <= startPoint.y() && nextPoint.y() >= startPoint.y()) {
            cutBoxChange.append(
                regionColor(QPoint(startPoint.x(), backPoint.y()), QPoint(backPoint.x(), startPoint.y()), 50));
            cutBoxChange.append(regionColor(startPoint, nextPoint, 0));
            return cutBoxChange;
        }
        //右下到右上
        if (backPoint.y() >= startPoint.y() && nextPoint.y() <= startPoint.y()) {
            cutBoxChange.append(
                regionColor(QPoint(startPoint.x(), nextPoint.y()), QPoint(nextPoint.x(), startPoint.y()), 0));
            cutBoxChange.append(regionColor(startPoint, backPoint, 50));
            return cutBoxChange;
        }
    }
    if (backPoint.x() <= startPoint.x() && nextPoint.x() <= startPoint.x()) {
        //左下到左上
        if (backPoint.y() >= startPoint.y() && nextPoint.y() <= startPoint.y()) {
            cutBoxChange.append(
                regionColor(QPoint(backPoint.x(), startPoint.y()), QPoint(startPoint.x(), backPoint.y()), 50));
            cutBoxChange.append(regionColor(nextPoint, startPoint, 0));
            return cutBoxChange;
        }
        //左上到左下
        if (backPoint.y() <= startPoint.y() && nextPoint.y() >= startPoint.y()) {
            cutBoxChange.append(regionColor(backPoint, startPoint, 50));
            cutBoxChange.append(
                regionColor(QPoint(nextPoint.x(), startPoint.y()), QPoint(startPoint.x(), nextPoint.y()), 0));
            return cutBoxChange;
        }
    }
    //由左上翻转到右下
    if ((backPoint.x() <= startPoint.x() && backPoint.y() <= startPoint.y())
        && (nextPoint.x() >= startPoint.x() && nextPoint.y() >= startPoint.y())) {
        cutBoxChange.append(regionColor(backPoint, startPoint, 50));
        cutBoxChange.append(regionColor(startPoint, nextPoint, 0));
        return cutBoxChange;
    }
    //由右下翻转到左上
    if ((nextPoint.x() <= startPoint.x() && nextPoint.y() <= startPoint.y())
        && (backPoint.x() >= startPoint.x() && backPoint.y() >= startPoint.y())) {
        cutBoxChange.append(regionColor(nextPoint, startPoint, 0));
        cutBoxChange.append(regionColor(startPoint, backPoint, 50));
        return cutBoxChange;
    }
    //由左下翻转到右上
    if ((backPoint.x() <= startPoint.x() && backPoint.y() <= startPoint.y())
        && (nextPoint.x() >= startPoint.x() && nextPoint.y() <= startPoint.y())) {
        cutBoxChange.append(
            regionColor(QPoint(backPoint.x(), startPoint.y()), QPoint(startPoint.x(), backPoint.y()), 50));
        cutBoxChange.append(
            regionColor(QPoint(startPoint.x(), nextPoint.y()), QPoint(nextPoint.x(), startPoint.y()), 0));
        return cutBoxChange;
    }
    //由右上翻转到左下
    if ((nextPoint.x() <= startPoint.x() && nextPoint.y() <= startPoint.y())
        && (backPoint.x() >= startPoint.x() && backPoint.y() <= startPoint.y())) {
        cutBoxChange.append(
            regionColor(QPoint(nextPoint.x(), startPoint.y()), QPoint(startPoint.x(), nextPoint.y()), 0));
        cutBoxChange.append(
            regionColor(QPoint(startPoint.x(), backPoint.y()), QPoint(backPoint.x(), startPoint.y()), 50));
        return cutBoxChange;
    }
    //对于点在左上的情况
    if ((startPoint.x() >= nextPoint.x()) && (startPoint.y() >= nextPoint.y())) {
        //对于后来的位置一直大于等于前一个的情况--全变白
        if ((widthDiff <= 0) && (heightDiff <= 0)) {

            cutBoxChange.append(regionColor(nextPoint, QPoint(startPoint.x(), backPoint.y()), 0));
            cutBoxChange.append(
                regionColor(QPoint(nextPoint.x(), backPoint.y()), QPoint(backPoint.x(), startPoint.y()), 0));
            return cutBoxChange;
        } else {
            //全黑，右黑低白，右白底黑
            if ((widthDiff >= 0) && (heightDiff >= 0)) {

                cutBoxChange.append(regionColor(backPoint, QPoint(startPoint.x(), nextPoint.y()), 50));
                cutBoxChange.append(
                    regionColor(QPoint(backPoint.x(), nextPoint.y()), QPoint(nextPoint.x(), startPoint.y()), 50));
                return cutBoxChange;
            } else if ((widthDiff >= 0)) {

                cutBoxChange.append(regionColor(nextPoint, QPoint(startPoint.x(), backPoint.y()), 0));
                cutBoxChange.append(regionColor(backPoint, QPoint(nextPoint.x(), startPoint.y()), 50));
                return cutBoxChange;
            } else if ((heightDiff >= 0)) {

                cutBoxChange.append(regionColor(nextPoint, QPoint(backPoint.x(), startPoint.y()), 0));
                cutBoxChange.append(regionColor(backPoint, QPoint(startPoint.x(), nextPoint.y()), 50));
                return cutBoxChange;
            }
        }
    }
    //左下
    if ((startPoint.x() >= nextPoint.x()) && (startPoint.y() <= nextPoint.y())) {
        //对于后来的位置一直大于等于前一个的情况--全变白
        if ((widthDiff <= 0) && (heightDiff >= 0)) {

            cutBoxChange.append(
                regionColor(QPoint(nextPoint.x(), startPoint.y()), QPoint(backPoint.x(), nextPoint.y()), 0));
            cutBoxChange.append(regionColor(backPoint, QPoint(startPoint.x(), nextPoint.y()), 0));
            return cutBoxChange;
        } else {
            //全黑，右黑低白，右白底黑
            if ((widthDiff >= 0) && (heightDiff <= 0)) {

                cutBoxChange.append(
                    regionColor(QPoint(backPoint.x(), startPoint.y()), QPoint(nextPoint.x(), backPoint.y()), 50));
                cutBoxChange.append(regionColor(nextPoint, QPoint(startPoint.x(), backPoint.y()), 50));
                return cutBoxChange;
            } else if ((widthDiff >= 0)) {

                cutBoxChange.append(
                    regionColor(QPoint(backPoint.x(), startPoint.y()), QPoint(nextPoint.x(), backPoint.y()), 50));
                cutBoxChange.append(
                    regionColor(QPoint(nextPoint.x(), backPoint.y()), QPoint(startPoint.x(), nextPoint.y()), 0));
                return cutBoxChange;
            } else if ((heightDiff <= 0)) {

                cutBoxChange.append(
                    regionColor(QPoint(nextPoint.x(), startPoint.y()), QPoint(backPoint.x(), nextPoint.y()), 0));
                cutBoxChange.append(
                    regionColor(QPoint(backPoint.x(), nextPoint.y()), QPoint(startPoint.x(), backPoint.y()), 50));
                return cutBoxChange;
            }
        }
    }
    //右上
    if ((startPoint.x() <= nextPoint.x()) && (startPoint.y() >= nextPoint.y())) {
        //对于后来的位置一直大于等于前一个的情况--全变白
        if ((widthDiff >= 0) && (heightDiff <= 0)) {

            cutBoxChange.append(
                regionColor(QPoint(startPoint.x(), nextPoint.y()), QPoint(nextPoint.x(), backPoint.y()), 0));
            cutBoxChange.append(regionColor(backPoint, QPoint(nextPoint.x(), startPoint.y()), 0));
            return cutBoxChange;
        } else {
            //全黑，右黑低白，右白底黑
            if ((widthDiff <= 0) && (heightDiff >= 0)) {
                cutBoxChange.append(
                    regionColor(QPoint(startPoint.x(), backPoint.y()), QPoint(backPoint.x(), nextPoint.y()), 50));
                cutBoxChange.append(regionColor(nextPoint, QPoint(backPoint.x(), startPoint.y()), 50));
                return cutBoxChange;
            } else if ((widthDiff <= 0)) {

                cutBoxChange.append(
                    regionColor(QPoint(startPoint.x(), nextPoint.y()), QPoint(nextPoint.x(), backPoint.y()), 0));
                cutBoxChange.append(
                    regionColor(QPoint(nextPoint.x(), backPoint.y()), QPoint(backPoint.x(), startPoint.y()), 50));
                return cutBoxChange;
            } else if ((heightDiff >= 0)) {
                cutBoxChange.append(
                    regionColor(QPoint(startPoint.x(), backPoint.y()), QPoint(backPoint.x(), nextPoint.y()), 50));
                cutBoxChange.append(
                    regionColor(QPoint(backPoint.x(), nextPoint.y()), QPoint(nextPoint.x(), startPoint.y()), 0));
                return cutBoxChange;
            }
        }
    }
    //右下
    if ((startPoint.x() <= nextPoint.x()) && (startPoint.y() <= nextPoint.y())) {
        //对于后来的位置一直大于等于前一个的情况--全变白
        if ((widthDiff >= 0) && (heightDiff >= 0)) {
            cutBoxChange.append(regionColor(QPoint(backPoint.x(), startPoint.y()), nextPoint, 0));
            cutBoxChange.append(
                regionColor(QPoint(startPoint.x(), backPoint.y()), QPoint(backPoint.x(), nextPoint.y()), 0));
            return cutBoxChange;
        } else {
            //全黑，右黑低白，右白底黑
            if ((widthDiff <= 0) && (heightDiff <= 0)) {
                cutBoxChange.append(regionColor(QPoint(nextPoint.x(), startPoint.y()), backPoint, 50));
                cutBoxChange.append(
                    regionColor(QPoint(startPoint.x(), nextPoint.y()), QPoint(nextPoint.x(), backPoint.y()), 50));
                return cutBoxChange;
            } else if ((widthDiff <= 0)) {
                cutBoxChange.append(
                    regionColor(QPoint(nextPoint.x(), startPoint.y()), QPoint(backPoint.x(), nextPoint.y()), 50));
                cutBoxChange.append(regionColor(QPoint(startPoint.x(), backPoint.y()), nextPoint, 0));
                return cutBoxChange;
            } else if ((heightDiff <= 0)) {
                cutBoxChange.append(regionColor(QPoint(backPoint.x(), startPoint.y()), nextPoint, 0));
                cutBoxChange.append(regionColor(QPoint(startPoint.x(), nextPoint.y()), backPoint, 50));
                return cutBoxChange;
            }
        }
    }
}
//返回需要处理的矩形-起始位置和颜色设置
cutBoxColor Processing::regionColor(const QPoint &startP, const QPoint &endP, int key)
{
    cutBoxColor cutRecord;
    cutRecord.startPostion = startP;
    cutRecord.endPostion = endP;

    cutRecord.key = key;
    if (key == 0) {
        cutRecord.colorChange = false;
    } else {
        cutRecord.colorChange = true;
    }
    return cutRecord;
}
//计算区域边框
void Processing::regionBoundry(const QPoint &startP, const QPoint &changeP)
{

    if (startP.x() < changeP.x()) {
        m_regionStartP.setX(startP.x());
        m_regionEndP.setX(changeP.x());
    } else {
        m_regionStartP.setX(changeP.x());
        m_regionEndP.setX(startP.x());
    }
    if (startP.y() < changeP.y()) {
        m_regionStartP.setY(startP.y());
        m_regionEndP.setY(changeP.y());
    } else {
        m_regionStartP.setY(changeP.y());
        m_regionEndP.setY(startP.y());
    }
}

//记录8个边角
QList<cutBoxColor> Processing::releaseSquare(const QPoint &startP, const QPoint &endP, const QImage &img)
{
    QList<cutBoxColor> m_squarePoint; //每一个的方框
    QPoint startPoint = startP;
    QPoint endPoint = endP;

    // 8个角
    cutBoxColor colorSquare1;
    QPoint leftUpStart;
    QPoint leftUpEnd;
    leftUpStart.setX(rectBoundaryJudg(startPoint.x() - 4, 0, true));
    leftUpStart.setY(rectBoundaryJudg(startPoint.y() - 4, 0, true));
    leftUpEnd.setX(leftUpStart.x() + 8);
    leftUpEnd.setY(leftUpStart.y() + 8);
    colorSquare1.startPostion = leftUpStart;
    colorSquare1.endPostion = leftUpEnd;

    cutBoxColor colorSquare2;
    QPoint rightBottomStart;
    QPoint rightBottomEnd;
    rightBottomEnd.setX(rectBoundaryJudg(endPoint.x() + 4, img.width(), false));
    rightBottomEnd.setY(rectBoundaryJudg(endPoint.y() + 4, img.height(), false));
    rightBottomStart.setX(rightBottomEnd.x() - 8);
    rightBottomStart.setY(rightBottomEnd.y() - 8);
    colorSquare2.startPostion = rightBottomStart;
    colorSquare2.endPostion = rightBottomEnd;

    cutBoxColor colorSquare3;
    QPoint leftBottomStart;
    QPoint leftBottomEnd;
    leftBottomStart.setX(rectBoundaryJudg(startPoint.x() - 4, 0, true));
    leftBottomEnd.setY(rectBoundaryJudg(endPoint.y() + 4, img.height(), false));
    leftBottomEnd.setX(leftBottomStart.x() + 8);
    leftBottomStart.setY(leftBottomEnd.y() - 8);
    colorSquare3.startPostion = leftBottomStart;
    colorSquare3.endPostion = leftBottomEnd;

    cutBoxColor colorSquare4;
    QPoint rightUpStart;
    QPoint rightUpEnd;
    rightUpEnd.setX(rectBoundaryJudg(endPoint.x() + 4, img.width(), false));
    rightUpStart.setY(rectBoundaryJudg(startPoint.y() - 4, 0, true));
    rightUpStart.setX(rightUpEnd.x() - 8);
    rightUpEnd.setY(rightUpStart.y() + 8);
    colorSquare4.startPostion = rightUpStart;
    colorSquare4.endPostion = rightUpEnd;

    cutBoxColor colorSquare5;
    QPoint upMiddleStart;
    QPoint upMiddleEnd;

    upMiddleStart.setY(rectBoundaryJudg(startPoint.y() - 4, 0, true));
    upMiddleEnd.setY(upMiddleStart.y() + 8);
    upMiddleStart.setX((endPoint.x() + startPoint.x()) / 2 - 4);
    upMiddleEnd.setX((endPoint.x() + startPoint.x()) / 2 + 4);
    colorSquare5.startPostion = upMiddleStart;
    colorSquare5.endPostion = upMiddleEnd;

    cutBoxColor colorSquare6;
    QPoint bottomMiddleStart;
    QPoint bottomMiddleEnd;
    bottomMiddleEnd.setY(rectBoundaryJudg(endPoint.y() + 4, img.height(), false));
    bottomMiddleStart.setY(bottomMiddleEnd.y() - 8);
    bottomMiddleStart.setX((endPoint.x() + startPoint.x()) / 2 - 4);
    bottomMiddleEnd.setX((endPoint.x() + startPoint.x()) / 2 + 4);
    colorSquare6.startPostion = bottomMiddleStart;
    colorSquare6.endPostion = bottomMiddleEnd;

    cutBoxColor colorSquare7;
    QPoint leftMiddleStart;
    QPoint leftMiddleEnd;
    leftMiddleStart.setX(rectBoundaryJudg(startPoint.x() - 4, 0, true));
    leftMiddleEnd.setX(leftMiddleStart.x() + 8);
    leftMiddleStart.setY((endPoint.y() + startPoint.y()) / 2 - 4);
    leftMiddleEnd.setY((endPoint.y() + startPoint.y()) / 2 + 4);
    colorSquare7.startPostion = leftMiddleStart;
    colorSquare7.endPostion = leftMiddleEnd;

    cutBoxColor colorSquare8;
    QPoint rightMiddleStart;
    QPoint rightMiddleEnd;
    rightMiddleEnd.setX(rectBoundaryJudg(endPoint.x() + 4, img.width(), false));
    rightMiddleStart.setX(rightMiddleEnd.x() - 8);
    rightMiddleStart.setY((endPoint.y() + startPoint.y()) / 2 - 4);
    rightMiddleEnd.setY((endPoint.y() + startPoint.y()) / 2 + 4);
    colorSquare8.startPostion = rightMiddleStart;
    colorSquare8.endPostion = rightMiddleEnd;

    m_squarePoint.append(colorSquare1);
    m_squarePoint.append(colorSquare2);
    m_squarePoint.append(colorSquare3);
    m_squarePoint.append(colorSquare4);
    m_squarePoint.append(colorSquare5);
    m_squarePoint.append(colorSquare6);
    m_squarePoint.append(colorSquare7);
    m_squarePoint.append(colorSquare8);

    return m_squarePoint;
}

QList<cutBoxColor> Processing::pictureCutPainter(const QImage &img, int type, int backtype, const QPoint &startP,
                                                 QPoint &endP, QPoint &changeP, QPoint &backChangeP)
{
    QList<cutBoxColor> cutBoxChange;
    //上边
    if (type == 1) {
        // backChangP的起始点要等于鼠标起始位置的点
        if (backChangeP.x() == -1 && backChangeP.y() == -1) {
            backChangeP = QPoint(endP.x(), startP.y());
            //处理偏移量
            //            m_yOffset = changeP.y() - startP.y();

        } else {
            backChangeP.setX(boundaryJudg(img.width(), backChangeP.x()));
            backChangeP.setY(boundaryJudg(img.height(), backChangeP.y()));
        }
        QPoint back = QPoint(endP.x(), backChangeP.y());
        cutBoxChange = pictureColor(back, QPoint(endP.x(), changeP.y()), QPoint(startP.x(), endP.y()));
        regionBoundry(QPoint(startP.x(), changeP.y()), endP);
        //右上角
        return cutBoxChange;
    }
    if (type == 7) {
        // backChangP的起始点要等于鼠标起始位置的点
        if (backChangeP.x() == -1 && backChangeP.y() == -1) {
            backChangeP = endP;
        } else {
            backChangeP.setX(boundaryJudg(img.width(), backChangeP.x()));
            backChangeP.setY(boundaryJudg(img.height(), backChangeP.y()));
        }
        QPoint back = QPoint(endP.x(), backChangeP.y());
        cutBoxChange = pictureColor(back, QPoint(endP.x(), changeP.y()), startP);
        regionBoundry(startP, QPoint(endP.x(), changeP.y()));
        //终点
        return cutBoxChange;
    }
    if (type == 2) {
        // backChangP的起始点要等于鼠标起始位置的点
        if (backChangeP.x() == -1 && backChangeP.y() == -1) {
            backChangeP = QPoint(startP.x(), endP.y());
            //处理偏移量
            //            m_xOffset = changeP.x() - startP.x();
        } else {
            backChangeP.setX(boundaryJudg(img.width(), backChangeP.x()));
            backChangeP.setY(boundaryJudg(img.height(), backChangeP.y()));
        }
        QPoint back = QPoint(backChangeP.x(), endP.y());
        cutBoxChange = pictureColor(back, QPoint(changeP.x(), endP.y()), QPoint(endP.x(), startP.y()));
        if (changeP.x() >= endP.x()) {
            m_endPointCorrect.setX(changeP.x());
        } else {
            m_endPointCorrect.setX(endP.x());
        }
        regionBoundry(QPoint(endP.x(), startP.y()), QPoint(changeP.x(), endP.y()));
        //左下角
        return cutBoxChange;
    }
    if (type == 8) {
        // backChangP的起始点要等于鼠标起始位置的点
        if (backChangeP.x() == -1 && backChangeP.y() == -1) {
            backChangeP = endP;
        } else {
            backChangeP.setX(boundaryJudg(img.width(), backChangeP.x()));
            backChangeP.setY(boundaryJudg(img.height(), backChangeP.y()));
        }
        QPoint back = QPoint(backChangeP.x(), endP.y());
        cutBoxChange = pictureColor(back, QPoint(changeP.x(), endP.y()), startP);
        if (changeP.x() >= startP.x()) {
            m_endPointCorrect.setX(changeP.x());
        } else {
            m_endPointCorrect.setX(startP.x());
        }
        regionBoundry(startP, QPoint(changeP.x(), endP.y()));
        //终点
        return cutBoxChange;
    }

    if (type == 3) {
        // backChangP的起始点要等于鼠标起始位置的点
        if (backChangeP.x() == -1 && backChangeP.y() == -1) {
            backChangeP = startP;
        } else {
            backChangeP.setX(boundaryJudg(img.width(), backChangeP.x()));
            backChangeP.setY(boundaryJudg(img.height(), backChangeP.y()));
        }
        cutBoxChange = pictureColor(backChangeP, QPoint(changeP.x(), changeP.y()), endP);
        if (changeP.x() >= endP.x()) {
            m_endPointCorrect.setX(changeP.x());
        } else {
            m_endPointCorrect.setX(endP.x());
        }
        regionBoundry(endP, QPoint(changeP.x(), changeP.y()));
        //起点
        return cutBoxChange;
    }
    if (type == 4) {
        // backChangP的起始点要等于鼠标起始位置的点
        if (backChangeP.x() == -1 && backChangeP.y() == -1) {
            backChangeP = endP;
            //处理偏移量
            //            m_yOffset = changeP.y() - endP.y();
            //            m_xOffset = changeP.x() - endP.x();
        } else {
            backChangeP.setX(boundaryJudg(img.width(), backChangeP.x()));
            backChangeP.setY(boundaryJudg(img.height(), backChangeP.y()));
        }
        cutBoxChange = pictureColor(backChangeP, QPoint(changeP.x(), changeP.y()), startP);
        if (changeP.x() >= startP.x()) {
            m_endPointCorrect.setX(changeP.x());
        } else {
            m_endPointCorrect.setX(startP.x());
        }
        regionBoundry(startP, QPoint(changeP.x(), changeP.y()));
        //终点
        return cutBoxChange;
    }
    if (type == 5) {
        // backChangP的起始点要等于鼠标起始位置的点
        if (backChangeP.x() == -1 && backChangeP.y() == -1) {
            backChangeP = QPoint(startP.x(), endP.y());
            //处理偏移量
            //            m_yOffset = changeP.y() - endP.y();
            //            m_xOffset = changeP.x() - startP.x();
        } else {
            backChangeP.setX(boundaryJudg(img.width(), backChangeP.x()));
            backChangeP.setY(boundaryJudg(img.height(), backChangeP.y()));
        }
        cutBoxChange = pictureColor(backChangeP, QPoint(changeP.x(), changeP.y()), QPoint(endP.x(), startP.y()));
        if (changeP.x() >= endP.x()) {
            m_endPointCorrect.setX(changeP.x());
        } else {
            m_endPointCorrect.setX(endP.x());
        }
        regionBoundry(QPoint(endP.x(), startP.y()), QPoint(changeP.x(), changeP.y()));
        //左下角
        return cutBoxChange;
    }
    if (type == 6) {
        // backChangP的起始点要等于鼠标起始位置的点
        if (backChangeP.x() == -1 && backChangeP.y() == -1) {
            backChangeP = QPoint(endP.x(), startP.y());
            //处理偏移量
            //            m_yOffset = changeP.y() - startP.y();
            //            m_xOffset = changeP.x() - endP.x();
        } else {
            backChangeP.setX(boundaryJudg(img.width(), backChangeP.x()));
            backChangeP.setY(boundaryJudg(img.height(), backChangeP.y()));
        }
        cutBoxChange = pictureColor(backChangeP, QPoint(changeP.x(), changeP.y()), QPoint(startP.x(), endP.y()));

        if (changeP.x() >= startP.x()) {
            m_endPointCorrect.setX(changeP.x());
        } else {
            m_endPointCorrect.setX(startP.x());
        }
        regionBoundry(QPoint(startP.x(), endP.y()), QPoint(changeP.x(), changeP.y()));
        //右上角
        return cutBoxChange;
    }
}
//返回需要处理的矩形框--用于移动裁剪框
QList<cutBoxColor> Processing::pictureCutMove(const QImage &img, const QPoint &startCutP, const QPoint &endCutP,
                                              int xOffset, int yOffset)
{
    QList<cutBoxColor> cutBoxChange;
    int xStart = startCutP.x() + xOffset;
    int yStart = startCutP.y() + yOffset;
    int xEnd = endCutP.x() + xOffset;
    int yEnd = endCutP.y() + yOffset;
    //边界值处理
    xStart = boundaryJudg(img.width(), xStart);
    xEnd = boundaryJudg(img.width(), xEnd);
    yStart = boundaryJudg(img.height(), yStart);
    yEnd = boundaryJudg(img.height(), yEnd);
    //处理两个矩形块无重叠部分（快速移动）的情况
    if (abs(xOffset) >= abs(endCutP.x() - startCutP.x()) || abs(yOffset) >= abs(endCutP.y() - startCutP.y())) {
        cutBoxChange.append(regionColor(startCutP, endCutP, 50));
        cutBoxChange.append(regionColor(QPoint(xStart, yStart), QPoint(xEnd, yEnd), 0));
        return cutBoxChange;
    }
    //处理有重叠区域的情况
    //左上
    if (xOffset <= 0 && yOffset <= 0) {
        //左亮
        cutBoxChange.append(regionColor(QPoint(xStart, yStart), QPoint(startCutP.x(), yEnd), 0));
        //上亮
        cutBoxChange.append(regionColor(QPoint(xStart, yStart), QPoint(xEnd, startCutP.y()), 0));
        //下暗
        cutBoxChange.append(regionColor(QPoint(startCutP.x(), yEnd), endCutP, 50));
        //左暗
        cutBoxChange.append(regionColor(QPoint(xEnd, startCutP.y()), endCutP, 50));

        return cutBoxChange;
    }
    //左下
    if (xOffset <= 0 && yOffset >= 0) {
        //左亮
        cutBoxChange.append(regionColor(QPoint(xStart, yStart), QPoint(startCutP.x(), yEnd), 0));
        //下亮
        cutBoxChange.append(regionColor(QPoint(startCutP.x(), endCutP.y()), QPoint(xEnd, yEnd), 0));
        //右暗
        cutBoxChange.append(regionColor(QPoint(xEnd, startCutP.y()), endCutP, 50));
        //上暗
        cutBoxChange.append(regionColor(startCutP, QPoint(endCutP.x(), yStart), 50));

        return cutBoxChange;
    }
    //右下
    if (xOffset >= 0 && yOffset >= 0) {
        //下亮
        cutBoxChange.append(regionColor(QPoint(xStart, endCutP.y()), QPoint(xEnd, yEnd), 0));
        //右亮
        cutBoxChange.append(regionColor(QPoint(endCutP.x(), yStart), QPoint(xEnd, yEnd), 0));
        //左暗
        cutBoxChange.append(regionColor(startCutP, QPoint(xStart, endCutP.y()), 50));
        //上暗
        cutBoxChange.append(regionColor(startCutP, QPoint(endCutP.x(), yStart), 50));

        return cutBoxChange;
    }
    //右上
    if (xOffset >= 0 && yOffset <= 0) {
        //上亮
        cutBoxChange.append(regionColor(QPoint(xStart, yStart), QPoint(xEnd, startCutP.y()), 0));
        //右亮
        cutBoxChange.append(regionColor(QPoint(endCutP.x(), yStart), QPoint(xEnd, yEnd), 0));
        //左暗
        cutBoxChange.append(regionColor(startCutP, QPoint(xStart, endCutP.y()), 50));
        //下暗
        cutBoxChange.append(regionColor(QPoint(startCutP.x(), yEnd), endCutP, 50));

        return cutBoxChange;
    }
}
//处理边界值
int Processing::boundaryJudg(int max, int point)
{
    if (point < 0) {
        point = 0;
    }
    if (point > max) {
        point = max;
    }
    return point;
}

int Processing::rectBoundaryJudg(int contrast, int target, bool isNeedMin)
{
    int tmp = contrast;
    if (isNeedMin) {
        if (tmp < target) {
            tmp = 0;
        }
    } else {
        if (tmp > target) {
            tmp = target;
        }
    }
    return tmp;
}

int Processing::minNumIsZero(const int &num1, const int &num2)
{
    int num = num1 - num2;
    if (num < 0) {
        return 0;
    }
    return num;
}
