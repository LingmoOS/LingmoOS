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

//
// Created by lingmo on 2022/4/7.
//

#ifndef KDKOCR_OCR_MAIN_H
#define KDKOCR_OCR_MAIN_H

#endif //KDKOCR_OCR_MAIN_H
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "include/paddle_inference_api.h"
#include "include/paddle_api.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <vector>

#include "postprocess_op.h"
#include "preprocess_op.h"

#include <cstring>
#include <fstream>
#include <numeric>


using namespace paddle;
using namespace PaddleOCR;
using namespace paddle_infer;

namespace kdkocr_infer {
    class kdkocrGetInfer {
    public:
        explicit kdkocrGetInfer(const std::string &model_dir, const bool &use_gpu,
                             const int &gpu_id, const int &gpu_mem,
                             const int &cpu_math_library_num_threads,
                             const bool &use_mkldnn, const string &label_path,
                             const bool &use_tensorrt, const std::string &precision,
                             const int &rec_batch_num) {
            this->use_gpu_ = use_gpu;
            this->gpu_id_ = gpu_id;
            this->gpu_mem_ = gpu_mem;
            this->cpu_math_library_num_threads_ = cpu_math_library_num_threads;
            this->use_mkldnn_ = use_mkldnn;
            this->use_tensorrt_ = use_tensorrt;
            this->precision_ = precision;
            this->rec_batch_num_ = rec_batch_num;

            this->label_list_ = Utility::ReadDict(label_path);
            this->label_list_.insert(this->label_list_.begin(),
                                     "#"); // blank char for ctc
            this->label_list_.push_back(" ");

            LoadModel(model_dir);
        }
        void Run(std::vector<cv::Mat> img_list, std::vector<double> *times,std::vector<std::string> &ressult);
        void LoadModel(const std::string &model_dir);
        void getText(const std::string &imagePath,std::vector<std::vector<std::string>> &str_res);


    private:

        std::shared_ptr<Predictor> predictor_;

        bool use_gpu_ = false;
        int gpu_id_ = 0;
        int gpu_mem_ = 4000;
        int cpu_math_library_num_threads_ = 4;
        bool use_mkldnn_ = false;

        std::vector<std::string> label_list_;

        std::vector<float> mean_ = {0.5f, 0.5f, 0.5f};
        std::vector<float> scale_ = {1 / 0.5f, 1 / 0.5f, 1 / 0.5f};
        bool is_scale_ = true;
        bool use_tensorrt_ = false;
        std::string precision_ = "fp32";
        int rec_batch_num_ = 6;

        // pre-process
        CrnnResizeImg resize_op_;
        Normalize normalize_op_;
        PermuteBatch permute_op_;

        // post-process
        PostProcessor post_processor_;

    };
    class kdkocrDetInfer{
    public:
        explicit kdkocrDetInfer(const std::string &model_dir, const bool &use_gpu,
                            const int &gpu_id, const int &gpu_mem,
                            const int &cpu_math_library_num_threads,
                            const bool &use_mkldnn, const int &max_side_len,
                            const double &det_db_thresh,
                            const double &det_db_box_thresh,
                            const double &det_db_unclip_ratio,
                            const bool &use_polygon_score, const bool &visualize,
                            const bool &use_tensorrt, const std::string &precision) {
            this->use_gpu_ = use_gpu;
            this->gpu_id_ = gpu_id;
            this->gpu_mem_ = gpu_mem;
            this->cpu_math_library_num_threads_ = cpu_math_library_num_threads;
            this->use_mkldnn_ = use_mkldnn;

            this->max_side_len_ = max_side_len;

            this->det_db_thresh_ = det_db_thresh;
            this->det_db_box_thresh_ = det_db_box_thresh;
            this->det_db_unclip_ratio_ = det_db_unclip_ratio;
            this->use_polygon_score_ = use_polygon_score;

            this->visualize_ = visualize;
            this->use_tensorrt_ = use_tensorrt;
            this->precision_ = precision;

            LoadModel(model_dir);
        }
        void LoadModel(const std::string &model_dir);

        // Run predictor
        void getRect(const std::string &imagePath,std::vector<std::vector<std::vector<int>>> &boxes);
        void Run(cv::Mat &img,
        std::vector<std::vector<std::vector<int>>> &boxes,
                std::vector<double> *times);

    private:
        std::shared_ptr<Predictor> predictor_;

        bool use_gpu_ = false;
        int gpu_id_ = 0;
        int gpu_mem_ = 4000;
        int cpu_math_library_num_threads_ = 4;
        bool use_mkldnn_ = false;

        int max_side_len_ = 960;

        double det_db_thresh_ = 0.3;
        double det_db_box_thresh_ = 0.5;
        double det_db_unclip_ratio_ = 2.0;
        bool use_polygon_score_ = false;

        bool visualize_ = true;
        bool use_tensorrt_ = false;
        std::string precision_ = "fp32";

        std::vector<float> mean_ = {0.485f, 0.456f, 0.406f};
        std::vector<float> scale_ = {1 / 0.229f, 1 / 0.224f, 1 / 0.225f};
        bool is_scale_ = true;

        // pre-process
        ResizeImgType0 resize_op_;
        Normalize normalize_op_;
        Permute permute_op_;

        // post-process
        PostProcessor post_processor_;
    };
    class kdkocrClsInfer{
    public:
        explicit kdkocrClsInfer(const std::string &model_dir, const bool &use_gpu,
                            const int &gpu_id, const int &gpu_mem,
                            const int &cpu_math_library_num_threads,
                            const bool &use_mkldnn, const double &cls_thresh,
                            const bool &use_tensorrt, const std::string &precision) {
            this->use_gpu_ = use_gpu;
            this->gpu_id_ = gpu_id;
            this->gpu_mem_ = gpu_mem;
            this->cpu_math_library_num_threads_ = cpu_math_library_num_threads;
            this->use_mkldnn_ = use_mkldnn;

            this->cls_thresh = cls_thresh;
            this->use_tensorrt_ = use_tensorrt;
            this->precision_ = precision;

            LoadModel(model_dir);
        }

        // Load Paddle inference model
        void LoadModel(const std::string &model_dir);

        cv::Mat Run(cv::Mat &img);

    private:
        std::shared_ptr<Predictor> predictor_;

        bool use_gpu_ = false;
        int gpu_id_ = 0;
        int gpu_mem_ = 4000;
        int cpu_math_library_num_threads_ = 4;
        bool use_mkldnn_ = false;
        double cls_thresh = 0.5;

        std::vector<float> mean_ = {0.5f, 0.5f, 0.5f};
        std::vector<float> scale_ = {1 / 0.5f, 1 / 0.5f, 1 / 0.5f};
        bool is_scale_ = true;
        bool use_tensorrt_ = false;
        std::string precision_ = "fp32";
        // pre-process
        ClsResizeImg resize_op_;
        Normalize normalize_op_;
        Permute permute_op_;
    };
};
