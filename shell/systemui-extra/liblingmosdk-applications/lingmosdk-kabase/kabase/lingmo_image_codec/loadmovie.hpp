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

#ifndef LINGMO_IMAGE_CODEC_LOADMOVIE_H_
#define LINGMO_IMAGE_CODEC_LOADMOVIE_H_

#include <QThread>
#include <QMovie>

#include "lingmo_image_codec/lingmoimagecodec_global.hpp"

namespace kdk
{
namespace kabase
{

using namespace cv;

class LoadMovie : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void loadMovieFinish(const QString &path);

public:
    LoadMovie(QList<Mat> *list, QMovie *movie);

protected:
    void run();

private:
    QList<Mat> *m_list = nullptr;
    QMovie *m_movie = nullptr;
};

} // namespace kabase
} // namespace kdk

#endif // LOADMOVIE_H
