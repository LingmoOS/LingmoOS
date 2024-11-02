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

#include "lingmo_image_codec/loadmovie.hpp"

namespace kdk
{
namespace kabase
{

LoadMovie::LoadMovie(QList<Mat> *list, QMovie *movie)
{
    m_list = list;
    m_movie = movie;
    //结束回收资源
    connect(this, &LoadMovie::finished, m_movie, &QMovie::deleteLater);
    connect(this, &LoadMovie::finished, this, &LoadMovie::deleteLater);
}

void LoadMovie::run()
{
    for (int i = 2; i < m_movie->frameCount(); ++i) {
        m_movie->jumpToFrame(i);
        QImage image = m_movie->currentImage();
        Mat mat = Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar *>(image.bits()),
                      static_cast<size_t>(image.bytesPerLine()))
                      .clone();
        //如果切换了图片，结束加载
        if (m_list->isEmpty()) {
            return;
        }
        m_list->append(mat);
    }
    //    emit loadMovieFinish(m_movie->fileName());
    Q_EMIT loadMovieFinish(m_movie->fileName());
}

} // namespace kabase
} // namespace kdk
