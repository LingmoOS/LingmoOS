#include "lingmoimagecodec.h"

Mat LingmoImageCodec::FI2MAT(FIBITMAP *src)
{
    Mat dst;
    int bpp = FreeImage_GetBPP(src);
    FREE_IMAGE_TYPE fit = FreeImage_GetImageType(src);

    int cv_type = -1;
    int cv_cvt = -1;

    switch (fit) {
    case FIT_UINT16:
        cv_type = DataType<ushort>::type;
        break;
    case FIT_INT16:
        cv_type = DataType<short>::type;
        break;
    case FIT_UINT32:
        cv_type = CV_32S;
        break;
    case FIT_INT32:
        cv_type = DataType<int>::type;
        break;
    case FIT_FLOAT:
        cv_type = DataType<float>::type;
        break;
    case FIT_DOUBLE:
        cv_type = DataType<double>::type;
        break;
    case FIT_COMPLEX:
        cv_type = CV_64FC2;
        break;
    case FIT_RGB16:
        cv_type = CV_16UC3;
        cv_cvt = COLOR_RGB2BGR;
        break;
    case FIT_RGBA16:
        cv_type = CV_16UC4;
        cv_cvt = COLOR_RGBA2BGRA;
        break;
    case FIT_RGBF:
        cv_type = CV_32FC3;
        cv_cvt = COLOR_RGB2BGR;
        break;
    case FIT_RGBAF:
        cv_type = CV_32FC4;
        cv_cvt = COLOR_RGBA2BGRA;
        break;
    case FIT_BITMAP:
        switch (bpp) {
        case 8:
            cv_type = CV_8UC1;
            break;
        case 16:
            cv_type = CV_8UC2;
            break;
        case 24:
            cv_type = CV_8UC3;
            break;
        case 32:
            cv_type = CV_8UC4;
            break;
        default:
            cv_type = -1;
        }
        break;
    default:
        qDebug() << "freeimage转opencv失败";
        return dst;
    }

    int width = FreeImage_GetWidth(src);
    int height = FreeImage_GetHeight(src);
    int step = FreeImage_GetPitch(src);

    if (cv_type >= 0) {
        dst = Mat(height, width, cv_type, FreeImage_GetBits(src), step).clone();

        if (cv_cvt > 0) {
            cvtColor(dst, dst, cv_cvt);
        }
        flip(dst, dst, 0);
    } else {
        FIBITMAP *palletized;
        //标准位图且位图深度为1的图片
        if (bpp == 1 && fit == FIT_BITMAP) {
                palletized = FreeImage_ConvertToGreyscale(src);
            } else {
                palletized = FreeImage_ConvertTo8Bits(src);
            }
        dst = FI2MAT(palletized);
    }

    if (!dst.data) {
        qDebug() << "Mat为空";
    }

    return dst;
}

bool LingmoImageCodec::mat2fibitmap(Mat mat, FREE_IMAGE_FORMAT fi_format, QString filename)
{
    FIBITMAP *dst = NULL;

    int width = mat.size().width;
    int height = mat.size().height;

    switch (mat.type()) {
    case CV_8U: {
        dst = FreeImage_AllocateT(FIT_BITMAP, width, height, 8);
    } break; // 8  bit grayscale
    case CV_8UC4: {
        dst = FreeImage_AllocateT(FIT_BITMAP, width, height, 32);
    } break; // 8  bit grayscale
    case CV_8UC3: {
        dst = FreeImage_AllocateT(FIT_BITMAP, width, height, 24);
    } break; // 24 bit RGB
    case CV_16U: {
        dst = FreeImage_AllocateT(FIT_UINT16, width, height, 16);
    } break; // 16 bit grayscale
    case CV_16S: {
        dst = FreeImage_AllocateT(FIT_INT16, width, height, 16);
    } break;
    case CV_32FC3: {
        dst = FreeImage_AllocateT(FIT_RGBF, width, height, 96);
    } break;
    case CV_64F: {
        dst = FreeImage_AllocateT(FIT_DOUBLE, width, height, 32);
    } break;
    default:
        dst = NULL;
    }

    int flag_32 = 0;
    int flag_21 = 0;
    int flag_16 = 0;
    int flag_0 = 0;
    int srcRowBytes = width * mat.elemSize();

FLAG:
    if (dst == NULL) {
        //转换失败
        return NULL;
    }


    /* 拷贝内存数据 */
    for (int ih = 0; ih < height; ih++) {
        BYTE *ptr2Line = FreeImage_GetScanLine(dst, (height - 1) - ih);
        memcpy(ptr2Line, mat.ptr(ih), srcRowBytes);
    }

    //转换成功
    if (FreeImage_Save(fi_format, dst, filename.toLocal8Bit().data(), 0)) {
        //保存成功
        return true;
    } else {
        /* 保存失败时 , 申请其他格式的内存再次进行保存 */
        if (dst != NULL) {
            FreeImage_Unload(dst);
        }

        if (mat.type() == 24) {
            // "Waring : save image fail , mat format is 24 convert fun save...";
            switch (flag_32) {
            case 0: {
                dst = FreeImage_AllocateT(FIT_BITMAP, width, height, 32);
                ++flag_32;
            };
                goto FLAG;
            case 1: {
                dst = FreeImage_AllocateT(FIT_UINT32, width, height, 32);
                ++flag_32;
            };
                goto FLAG;
            case 2: {
                dst = FreeImage_AllocateT(FIT_INT32, width, height, 32);
                ++flag_32;
            };
                goto FLAG;
            case 3: {
                dst = FreeImage_AllocateT(FIT_FLOAT, width, height, 32);
                ++flag_32;
            };
                goto FLAG;
            case 4: {
                dst = FreeImage_AllocateT(FIT_DOUBLE, width, height, 32);
                ++flag_32;
            };
                goto FLAG;
            case 5: {
                dst = FreeImage_AllocateT(FIT_COMPLEX, width, height, 32);
                ++flag_32;
            };
                goto FLAG;
            case 6: {
                dst = FreeImage_AllocateT(FIT_RGB16, width, height, 32);
                ++flag_32;
            };
                goto FLAG;
            case 7: {
                dst = FreeImage_AllocateT(FIT_RGBA16, width, height, 32);
                ++flag_32;
            };
                goto FLAG;
            case 8: {
                dst = FreeImage_AllocateT(FIT_RGBF, width, height, 32);
                ++flag_32;
            };
                goto FLAG;
            case 9: {
                dst = FreeImage_AllocateT(FIT_RGBAF, width, height, 32);
                ++flag_32;
            };
                goto FLAG;
            default: {
                flag_32 = 0;
            }; break;
            }
        }

        if (mat.type() == 21) {
            // "Waring : save image fail , mat format is 21 convert fun save...";
            switch (flag_21) {
            case 0: {
                dst = FreeImage_AllocateT(FIT_RGBF, width, height, 96);
                ++flag_21;
            };
                goto FLAG;
            case 1: {
                dst = FreeImage_AllocateT(FIT_RGBAF, width, height, 96);
                ++flag_21;
            };
                goto FLAG;
            case 3: {
                dst = FreeImage_AllocateT(FIT_BITMAP, width, height, 96);
                ++flag_21;
            };
                goto FLAG;
            default: {
                flag_21 = 0;
            }; break;
            }
        }

        if (mat.type() == 16) {
            // "Waring : save image fail , mat format is 16 convert fun save...";
            switch (flag_16) {
            case 0: {
                dst = FreeImage_AllocateT(FIT_BITMAP, width, height, 24);
                ++flag_16;
            };
                goto FLAG;
            case 1: {
                dst = FreeImage_AllocateT(FIT_UINT32, width, height, 24);
                ++flag_16;
            };
                goto FLAG;
            case 2: {
                dst = FreeImage_AllocateT(FIT_INT32, width, height, 24);
                ++flag_16;
            };
                goto FLAG;
            case 3: {
                dst = FreeImage_AllocateT(FIT_FLOAT, width, height, 24);
                ++flag_16;
            };
                goto FLAG;
            case 4: {
                dst = FreeImage_AllocateT(FIT_DOUBLE, width, height, 24);
                ++flag_16;
            };
                goto FLAG;
            case 5: {
                dst = FreeImage_AllocateT(FIT_COMPLEX, width, height, 24);
                ++flag_16;
            };
                goto FLAG;
            case 6: {
                dst = FreeImage_AllocateT(FIT_RGB16, width, height, 24);
                ++flag_16;
            };
                goto FLAG;
            case 7: {
                dst = FreeImage_AllocateT(FIT_RGBA16, width, height, 24);
                ++flag_16;
            };
                goto FLAG;
            case 8: {
                dst = FreeImage_AllocateT(FIT_RGBF, width, height, 24);
                ++flag_16;
            };
                goto FLAG;
            case 9: {
                dst = FreeImage_AllocateT(FIT_RGBAF, width, height, 24);
                ++flag_16;
            };
                goto FLAG;
            default: {
                flag_16 = 0;
            }; break;
            }
        }

        if (mat.type() == 0) {
            //保存图像失败，mat格式为0时保存
            switch (flag_0) {
            case 0: {
                dst = FreeImage_AllocateT(FIT_BITMAP, width, height, 8);
                ++flag_0;
            };
                goto FLAG;
            case 1: {
                dst = FreeImage_AllocateT(FIT_UINT32, width, height, 8);
                ++flag_0;
            };
                goto FLAG;
            case 2: {
                dst = FreeImage_AllocateT(FIT_INT32, width, height, 8);
                ++flag_0;
            };
                goto FLAG;
            case 3: {
                dst = FreeImage_AllocateT(FIT_FLOAT, width, height, 8);
                ++flag_0;
            };
                goto FLAG;
            case 4: {
                dst = FreeImage_AllocateT(FIT_DOUBLE, width, height, 8);
                ++flag_0;
            };
                goto FLAG;
            case 5: {
                dst = FreeImage_AllocateT(FIT_COMPLEX, width, height, 8);
                ++flag_0;
            };
                goto FLAG;
            case 6: {
                dst = FreeImage_AllocateT(FIT_RGB16, width, height, 8);
                ++flag_0;
            };
                goto FLAG;
            case 7: {
                dst = FreeImage_AllocateT(FIT_RGBA16, width, height, 8);
                ++flag_0;
            };
                goto FLAG;
            case 8: {
                dst = FreeImage_AllocateT(FIT_RGBF, width, height, 8);
                ++flag_0;
            };
                goto FLAG;
            case 9: {
                dst = FreeImage_AllocateT(FIT_RGBAF, width, height, 8);
                ++flag_0;
            };
                goto FLAG;
            case 10: {
                dst = FreeImage_AllocateT(FIT_UINT16, width, height, 8);
                ++flag_0;
            };
                goto FLAG;
            case 11: {
                dst = FreeImage_AllocateT(FIT_INT16, width, height, 8);
                ++flag_0;
            };
                goto FLAG;
            default: {
                flag_0 = 0;
            }; break;
            }
        }
        qDebug() << "图片保存失败";
        return false;
    }

    return false;
}

fipImage LingmoImageCodec::mat2RgbFipImage(Mat mat)
{
    // Mat rgb = get3ChannelsMat(mat);
    Mat rgb = mat;
    fipImage result(FIT_BITMAP, rgb.cols, rgb.rows, 24);
    uchar *pRgb;
    BYTE *bits = result.accessPixels();
    unsigned pitch = result.getScanWidth();
    for (int i = 0; i < rgb.rows; i++) {
        pRgb = rgb.ptr<uchar>(i);
        BYTE *pixel = (BYTE *)bits;
        for (int j = 0; j < rgb.cols; j++) {
            pixel[FI_RGBA_BLUE] = pRgb[3 * j];
            pixel[FI_RGBA_GREEN] = pRgb[3 * j + 1];
            pixel[FI_RGBA_RED] = pRgb[3 * j + 2];
            pixel += 3;
        }
        bits += pitch;
    }
    return result;
}

QPixmap LingmoImageCodec::converFormat(const Mat &mat)
{
    switch (mat.type()) {
    case CV_8UC4: {
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return QPixmap::fromImage(image);
    }
    case CV_8UC3: {
        QImage tmp(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        QImage image = tmp.rgbSwapped();
        return QPixmap::fromImage(image);
    }
    case CV_8UC1: {
        QVector<QRgb> sColorTable;
        if (sColorTable.isEmpty()) {
            for (int i = 0; i < 256; ++i) {
                sColorTable.push_back(qRgb(i, i, i));
            }
        }
        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);
        image.setColorTable(sColorTable);
        return QPixmap::fromImage(image);
    }
    case CV_32FC3: {
        Mat tmpMat;
        mat.convertTo(tmpMat, CV_8UC3, 255.0 / 1);
        return converFormat(tmpMat);
    }
    }

    qDebug("Image format is not supported: depth=%d and %d channels\n", mat.depth(), mat.channels());
    //    qDebug()<<CV_8UC3<<CV_8SC3<<CV_16UC3<<CV_16SC3<<CV_32SC3<<CV_32FC3<<CV_64FC3<<CV_16FC3<<"||"<<mat.type();
    return QPixmap();
}
