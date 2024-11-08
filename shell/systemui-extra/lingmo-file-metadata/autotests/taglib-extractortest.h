/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: baijunjie <baijunjie@kylinos.cn>
 * Modified by: baijunjie <baijunjie@kylinos.cn>
 *
 */

#ifndef LINGMO_FILE_METADATA_TAGLIBEXTRACTORTEST_H
#define LINGMO_FILE_METADATA_TAGLIBEXTRACTORTEST_H

#include <QObject>
#include <QMimeDatabase>

class TaglibExtractorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testNoExtraction();
    void testPropertyTypes();
    void testImageData_data();
    void testImageData();
    void testCommonData_data();
    void testCommonData();
    void testVorbisComment_data();
    void testVorbisComment();
    void testVorbisCommentMultivalue_data();
    void testVorbisCommentMultivalue();
    void testId3_data();
    void testId3();
    void testApe_data();
    void testApe();
    void testMp4_data();
    void testMp4();
    void testAax_data();
    void testAax();
    void testAsf_data();
    void testAsf();
    void testId3Rating_data();
    void testId3Rating();
    void testWmaRating_data();
    void testWmaRating();
    void testNoMetadata_data();
    void testNoMetadata();
    void testRobustness_data();
    void testRobustness();
private:
    QMimeDatabase mimeDb;
};


#endif //LINGMO_FILE_METADATA_TAGLIBEXTRACTORTEST_H
