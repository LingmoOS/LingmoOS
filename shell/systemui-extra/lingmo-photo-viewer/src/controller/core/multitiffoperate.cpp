#include "multitiffoperate.h"

MultiTiffOperate::MultiTiffOperate(const QString &path, const QList<ProcessingBase::FlipWay> operateList)
{
    m_path = path;
    m_operateList = operateList;
}

void MultiTiffOperate::run()
{
    for (int i = 0; i < m_operateList.length(); i++) {
        m_fibilist.clear();
        FREE_IMAGE_FORMAT fif = FIF_TIFF;
        FIMULTIBITMAP *pTIF = FreeImage_OpenMultiBitmap(fif, m_path.toLocal8Bit().data(), false, true, true, 0);
        int gifImgCnt = FreeImage_GetPageCount(pTIF);
        FIBITMAP *pFrame;
        if (m_operateList.at(i) == ProcessingBase::clockwise) {
            if (gifImgCnt > 0) {
                for (int curFrame = 0; curFrame < gifImgCnt; curFrame++) {
                    pFrame = FreeImage_LockPage(pTIF, curFrame);
                    FIBITMAP *pFrame1 = FreeImage_Rotate(pFrame, -90);
                    m_fibilist.append(pFrame1);
                    FreeImage_UnlockPage(pTIF, pFrame, curFrame);
                }
            }
        } else if (m_operateList.at(i) == ProcessingBase::vertical) {
            if (gifImgCnt > 0) {
                for (int curFrame = 0; curFrame < gifImgCnt; curFrame++) {
                    pFrame = FreeImage_LockPage(pTIF, curFrame);
                    FreeImage_FlipVertical(pFrame);
                    m_fibilist.append(FreeImage_Clone(pFrame));
                    FreeImage_UnlockPage(pTIF, pFrame, curFrame);
                }
            }
        } else if (m_operateList.at(i) == ProcessingBase::horizontal) {
            if (gifImgCnt > 0) {
                for (int curFrame = 0; curFrame < gifImgCnt; curFrame++) {
                    pFrame = FreeImage_LockPage(pTIF, curFrame);
                    FreeImage_FlipHorizontal(pFrame);
                    m_fibilist.append(FreeImage_Clone(pFrame));
                    FreeImage_UnlockPage(pTIF, pFrame, curFrame);
                }
            }
        }
        FreeImage_CloseMultiBitmap(pTIF);
        if (m_fibilist.length() > 0) {
            FIMULTIBITMAP *out = FreeImage_OpenMultiBitmap(fif, m_path.toLocal8Bit().data(), false, false);
            for (int i = 0; i < m_fibilist.length(); i++) {
                FreeImage_AppendPage(out, m_fibilist.at(i));
            }
            for (int j = 0; j < m_fibilist.length(); j++) {
                FreeImage_DeletePage(out, 0);
            }
            FreeImage_CloseMultiBitmap(out);
        }
    }
    finishOperate();
}

void MultiTiffOperate::finishOperate()
{
    if (m_fibilist.length() > 0) {
        for (int i = 0; i < m_fibilist.length(); i++) {
            FreeImage_Unload(m_fibilist.at(i));
        }
    }
    Q_EMIT tiffOPerateFinish(m_path);
}
