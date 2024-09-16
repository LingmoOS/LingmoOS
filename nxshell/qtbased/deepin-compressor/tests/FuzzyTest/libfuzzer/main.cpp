// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCoreApplication>
#include <QDebug>
#include <stdio.h>
#include <stdlib.h>
#include <chardet.h>

int ChartDet_DetectingTextCoding(const char *str, QString &encoding, float &confidence)
{
    DetectObj *obj = detect_obj_init();

    if (obj == nullptr) {
        //qDebug() << "Memory Allocation failed\n";
        return CHARDET_MEM_ALLOCATED_FAIL;
    }

    // before 1.0.5. This API is deprecated on 1.0.5
    switch (detect(str, &obj)) {
    case CHARDET_OUT_OF_MEMORY:
        //    qDebug() << "On handle processing, occured out of memory\n";
        detect_obj_free(&obj);
        return CHARDET_OUT_OF_MEMORY;
    case CHARDET_NULL_OBJECT:
        //      qDebug() << "2st argument of chardet() is must memory allocation with detect_obj_init API\n";
        return CHARDET_NULL_OBJECT;
    }

    encoding = obj->encoding;
    confidence = obj->confidence;
    detect_obj_free(&obj);

    return CHARDET_SUCCESS;
}
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    QString encoding;
    float confidence;

    ChartDet_DetectingTextCoding((char *)data, encoding, confidence);
    return 0;
}
