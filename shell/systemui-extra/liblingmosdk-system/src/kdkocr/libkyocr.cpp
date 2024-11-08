/*
 * liblingmosdk-system's Library
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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "libkyocr.hpp"

kdk::TOcrResult::TOcrResult() : imagePath(""),
                            hasGotRect(false),
                            textRects(),
                            texts()
{
}

void kdk::TOcrResult::reset()
{
    imagePath = "";
    hasGotRect = false;
//    textRects.clear();
//    texts.clear();
    std::vector<std::vector<std::vector<int>>>().swap(textRects);
    std::vector<std::string>().swap(texts);
}

bool kdk::TOcrResult::isNewPath(const std::string &strImagePath) const
{
    return imagePath != strImagePath;
}

kdk::kdkOCR::kdkOCR() : objOcrRet() {

}


#ifdef LOONGARCH64


#include <iostream>
#include <string>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>



std::vector<std::vector<std::vector<int>>> kdk::kdkOCR::getRect(const std::string &imagePath)
{
    if (imagePath.empty())
    {
        return std::vector<std::vector<std::vector<int>>>();
    }

    handleImage(imagePath);
    
    return objOcrRet.textRects;
}

std::vector<std::string> kdk::kdkOCR::getCls(const std::string &imagePath, int nums)
{
    if (imagePath.empty())
    {
        return std::vector<std::string>();
    }

    handleImage(imagePath);
    return objOcrRet.texts;
}

void kdk::kdkOCR::handleImage(const std::string &imagePath)
{
    if (imagePath.empty())
    {
        return;
    }
    if (!objOcrRet.isNewPath(imagePath))
    {
        if (objOcrRet.hasGotRect)
        {
            return;
        }
    }

    objOcrRet.reset();

    // tesseract识别
    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    if (!api->Init(NULL, "chi_sim"))
    {
        Pix *image = pixRead(imagePath.c_str());
        if (image)
        {
            api->SetImage(image);

            Boxa *boxes = api->GetComponentImages(tesseract::RIL_TEXTLINE, true, NULL, NULL);
            //std::cout << "boxes cnt : " << boxes->n << std::endl;
            for (int i = 0; i < boxes->n; ++i)
            {
                BOX *box = boxaGetBox(boxes, i, L_CLONE);

                // 文字位置收集
                std::vector<int> posLeftTop;
                posLeftTop.push_back(box->x);
                posLeftTop.push_back(box->y);
                std::vector<int> posRightTop;
                posRightTop.push_back(box->x + box->w);
                posRightTop.push_back(box->y);
                std::vector<int> posRightBottom;
                posRightBottom.push_back(box->x + box->w);
                posRightBottom.push_back(box->y + box->h);
                std::vector<int> posLeftBottom;
                posLeftBottom.push_back(box->x);
                posLeftBottom.push_back(box->y + box->h);

                std::vector<std::vector<int>> rcItem;
                rcItem.push_back(posLeftTop);
                rcItem.push_back(posRightTop);
                rcItem.push_back(posRightBottom);
                rcItem.push_back(posLeftBottom);

                objOcrRet.textRects.push_back(rcItem);

                // 文字识别
                api->SetRectangle(box->x, box->y-1, box->w, box->h+1);
                char* ocrResult = api->GetUTF8Text();
                objOcrRet.texts.push_back(std::string(ocrResult));
                delete [] ocrResult;

                boxDestroy(&box);
            }

            boxDestroy(boxes->box);
            delete boxes;

            pixDestroy(&image);
        }

        api->End();
        delete api;
    }

    objOcrRet.hasGotRect = true;
    objOcrRet.imagePath = imagePath;
}

#else

#include <string>
#include "ocr_main.h"
#include <gflags/gflags.h>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/highgui.hpp>
using namespace std;
DEFINE_bool(use_gpu, false, "Infering with GPU or CPU.");
DEFINE_int32(gpu_id, 0, "Device id of GPU to execute.");
DEFINE_int32(gpu_mem, 4000, "GPU id when infering with GPU.");
DEFINE_int32(cpu_threads, 4, "Num of threads with CPU.");
DEFINE_bool(enable_mkldnn, false, "Whether use mkldnn with CPU.");
DEFINE_bool(use_tensorrt, false, "Whether use tensorrt.");
DEFINE_string(precision, "fp32", "Precision be one of fp32/fp16/int8");
//DEFINE_bool(benchmark, false, "Whether use benchmark.");
DEFINE_string(save_log_path, "./log_output/", "Save benchmark log path.");
// detection related
DEFINE_string(image_dir, "", "Dir of input image.");
DEFINE_string(det_model_dir, "/etc/kdkocr/det", "Path of det inference model.");
DEFINE_int32(max_side_len, 960, "max_side_len of input image.");
DEFINE_double(det_db_thresh, 0.3, "Threshold of det_db_thresh.");
DEFINE_double(det_db_box_thresh, 0.5, "Threshold of det_db_box_thresh.");
DEFINE_double(det_db_unclip_ratio, 1.6, "Threshold of det_db_unclip_ratio.");
DEFINE_bool(use_polygon_score, false, "Whether use polygon score.");
DEFINE_bool(visualize, false, "Whether show the detection results.");
// classification related
DEFINE_bool(use_angle_cls, false, "Whether use use_angle_cls.");
DEFINE_string(cls_model_dir, "/etc/kdkocr/cls", "Path of cls inference model.");
DEFINE_double(cls_thresh, 0.9, "Threshold of cls_thresh.");
// recognition related
DEFINE_string(rec_model_dir, "/etc/kdkocr/rec", "Path of rec inference model.");
DEFINE_int32(rec_batch_num, 6, "rec_batch_num.");
DEFINE_string(char_list_file, "/etc/kdkocr/ppocr_keys_v1.txt", "Path of dictionary.");
using namespace kdkocr_infer;

std::vector<std::vector<std::vector<int>>> kdk::kdkOCR::getRect(const std::string &imagePath) {
    kdkocrDetInfer det(FLAGS_det_model_dir, FLAGS_use_gpu, FLAGS_gpu_id,
                       FLAGS_gpu_mem, FLAGS_cpu_threads,
                       FLAGS_enable_mkldnn, FLAGS_max_side_len, FLAGS_det_db_thresh,
                       FLAGS_det_db_box_thresh, FLAGS_det_db_unclip_ratio,
                       FLAGS_use_polygon_score, FLAGS_visualize,
                       FLAGS_use_tensorrt, FLAGS_precision);
    std::vector<std::vector<std::vector<int>>> boxes;
    det.getRect(imagePath,boxes);
    //std::cout << "total cnt : " << boxes.size() << std::endl;
    return boxes;



}

vector<string> kdk::kdkOCR::getCls(const std::string &imagePath, int nums){
    if (nums<=0) nums=4;
    std::vector<cv::String> cv_all_img_names;
    cv::glob(imagePath, cv_all_img_names);

    std::string char_list_file = FLAGS_char_list_file;
    std::vector<double> time_info_det = {0, 0, 0};
    std::vector<double> time_info_rec = {0, 0, 0};

    kdkocrDetInfer det(FLAGS_det_model_dir, FLAGS_use_gpu, FLAGS_gpu_id,
                   FLAGS_gpu_mem, FLAGS_cpu_threads,
                   FLAGS_enable_mkldnn, FLAGS_max_side_len, FLAGS_det_db_thresh,
                   FLAGS_det_db_box_thresh, FLAGS_det_db_unclip_ratio,
                   FLAGS_use_polygon_score, FLAGS_visualize,
                   FLAGS_use_tensorrt, FLAGS_precision);

    kdkocrGetInfer rec(FLAGS_rec_model_dir, FLAGS_use_gpu, FLAGS_gpu_id,
                       FLAGS_gpu_mem, FLAGS_cpu_threads,
                       FLAGS_enable_mkldnn, char_list_file,
                       FLAGS_use_tensorrt, FLAGS_precision, FLAGS_rec_batch_num);

    vector<string> result;

    for (int i = 0; i < cv_all_img_names.size(); ++i) {

        cv::Mat srcimg = cv::imread(cv_all_img_names[i], cv::IMREAD_COLOR);
        if (!srcimg.data) {
            std::cerr << "[ERROR] image read failed! image path: " << cv_all_img_names[i] << endl;
            exit(1);
        }
        std::vector<std::vector<std::vector<int>>> boxes;
        std::vector<double> det_times;
        std::vector<double> rec_times;

        det.Run(srcimg, boxes, &det_times);
        std::vector<cv::Mat> img_list;
        int length = boxes.size();
        for (int j = 0; j < length; j++) {

            cv::Mat crop_img;
            crop_img = Utility::GetRotateCropImage(srcimg, boxes[j]);
            img_list.push_back(crop_img);
            if (j % nums == 0 || j == length - 1){
                rec.Run(img_list, &rec_times,result);

            }
            if (j % nums == 0) {
                img_list.clear();
            }

        }

    }
    return result;

}
#endif
