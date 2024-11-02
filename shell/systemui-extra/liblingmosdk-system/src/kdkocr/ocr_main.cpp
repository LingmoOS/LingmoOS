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

#include "ocr_main.h"
#include "math.h"
#include "utility.h"
#include <sys/stat.h>
using namespace PaddleOCR;
using namespace paddle;
namespace kdkocr_infer {


    void kdkocrGetInfer::getText(const std::string &imagePath,std::vector<std::vector<std::string>> &str_res) {
        //TODO:这里需要做异常判断
        if (imagePath.empty()){
            return;
        }
        std::vector<double> time_info = {0, 0, 0};
        std::vector<cv::String> cv_all_img_names;
        cv::glob(imagePath, cv_all_img_names);

        std::vector<cv::Mat> img_list;
        for (int i = 0; i < cv_all_img_names.size(); ++i) {

            cv::Mat srcimg = cv::imread(cv_all_img_names[i], cv::IMREAD_COLOR);
            if (!srcimg.data) {
                std::cerr << "[ERROR] image read failed! image path: " << cv_all_img_names[i] << std::endl;
                exit(1);
            }
            img_list.push_back(srcimg);
        }
        std::vector<double> rec_times;
        //Run(img_list, &rec_times, str_res);
        time_info[0] += rec_times[0];
        time_info[1] += rec_times[1];
        time_info[2] += rec_times[2];


    }
    void kdkocrGetInfer::Run(std::vector<cv::Mat> img_list, std::vector<double> *times,std::vector<std::string> &result) {
        std::chrono::duration<float> preprocess_diff = std::chrono::steady_clock::now() - std::chrono::steady_clock::now();
        std::chrono::duration<float> inference_diff = std::chrono::steady_clock::now() - std::chrono::steady_clock::now();
        std::chrono::duration<float> postprocess_diff = std::chrono::steady_clock::now() - std::chrono::steady_clock::now();

        int img_num = img_list.size();
        std::vector<float> width_list;
        for (int i = 0; i < img_num; i++) {
            width_list.push_back(float(img_list[i].cols) / img_list[i].rows);
        }
        std::vector<int> indices = Utility::argsort(width_list);

        for (int beg_img_no = 0; beg_img_no < img_num; beg_img_no += this->rec_batch_num_) {
            auto preprocess_start = std::chrono::steady_clock::now();
            int end_img_no = min(img_num, beg_img_no + this->rec_batch_num_);
            float max_wh_ratio = 0;
            for (int ino = beg_img_no; ino < end_img_no; ino ++) {
                int h = img_list[indices[ino]].rows;
                int w = img_list[indices[ino]].cols;
                float wh_ratio = w * 1.0 / h;
                max_wh_ratio = max(max_wh_ratio, wh_ratio);
            }
            std::vector<cv::Mat> norm_img_batch;
            for (int ino = beg_img_no; ino < end_img_no; ino ++) {
                cv::Mat srcimg;
                img_list[indices[ino]].copyTo(srcimg);
                cv::Mat resize_img;
                this->resize_op_.Run(srcimg, resize_img, max_wh_ratio, this->use_tensorrt_);
                this->normalize_op_.Run(&resize_img, this->mean_, this->scale_, this->is_scale_);
                norm_img_batch.push_back(resize_img);
            }

            int batch_width = int(ceilf(32 * max_wh_ratio)) - 1;
            std::vector<float> input(this->rec_batch_num_ * 3 * 32 * batch_width, 0.0f);
            this->permute_op_.Run(norm_img_batch, input.data());
            auto preprocess_end = std::chrono::steady_clock::now();
            preprocess_diff += preprocess_end - preprocess_start;

            // Inference.
            auto input_names = this->predictor_->GetInputNames();
            auto input_t = this->predictor_->GetInputHandle(input_names[0]);
            input_t->Reshape({this->rec_batch_num_, 3, 32, batch_width});
            auto inference_start = std::chrono::steady_clock::now();
            input_t->CopyFromCpu(input.data());
            this->predictor_->Run();

            std::vector<float> predict_batch;
            auto output_names = this->predictor_->GetOutputNames();
            auto output_t = this->predictor_->GetOutputHandle(output_names[0]);
            auto predict_shape = output_t->shape();

            int out_num = std::accumulate(predict_shape.begin(), predict_shape.end(), 1,
                                          std::multiplies<int>());
            predict_batch.resize(out_num);

            output_t->CopyToCpu(predict_batch.data());
            auto inference_end = std::chrono::steady_clock::now();
            inference_diff += inference_end - inference_start;

            // ctc decode
            auto postprocess_start = std::chrono::steady_clock::now();
            for (int m = 0; m < predict_shape[0]; m++) {
                std::string  string_res;
                std::vector<std::string> str_res;
                str_res.reserve(60000);
                int argmax_idx;
                int last_index = 0;
                float score = 0.f;
                int count = 0;
                float max_value = 0.0f;

                for (int n = 0; n < predict_shape[1]; n++) {
                    argmax_idx =
                            int(Utility::argmax(&predict_batch[(m * predict_shape[1] + n) * predict_shape[2]],
                                                &predict_batch[(m * predict_shape[1] + n + 1) * predict_shape[2]]));
                    max_value =
                            float(*std::max_element(&predict_batch[(m * predict_shape[1] + n) * predict_shape[2]],
                                                    &predict_batch[(m * predict_shape[1] + n + 1) * predict_shape[2]]));

                    if (argmax_idx > 0 && (!(n > 0 && argmax_idx == last_index))) {
                        score += max_value;
                        count += 1;
                        //str_res.push_back(label_list_[argmax_idx]);
                        string_res += label_list_[argmax_idx];
//                        string_res.push_back(label_list_[argmax_idx]);
//                        str_res.push_back(label_list_[argmax_idx]);
                    }
                    last_index = argmax_idx;
                }
                score /= count;

                if (isnan(score))
                    continue;
               result.push_back(string_res);
//                for (int i = 0; i < str_res.size(); i++) {
//                    std::cout << str_res[i] << std::endl;
//                }
//                std::cout << "\tscore: " << score << std::endl;
            }
            auto postprocess_end = std::chrono::steady_clock::now();
            postprocess_diff += postprocess_end - postprocess_start;
        }
        times->push_back(double(preprocess_diff.count() * 1000));
        times->push_back(double(inference_diff.count() * 1000));
        times->push_back(double(postprocess_diff.count() * 1000));
    }

    void kdkocrGetInfer::LoadModel(const std::string &model_dir) {
        //   AnalysisConfig config;
        paddle_infer::Config config;
        //取消日志
        config.DisableGlogInfo();
        config.SetModel(model_dir + "/inference.pdmodel",
                        model_dir + "/inference.pdiparams");

        if (this->use_gpu_) {
            config.EnableUseGpu(this->gpu_mem_, this->gpu_id_);
            if (this->use_tensorrt_) {
                auto precision = paddle_infer::Config::Precision::kFloat32;
                if (this->precision_ == "fp16") {
                    precision = paddle_infer::Config::Precision::kHalf;
                }
                if (this->precision_ == "int8") {
                    precision = paddle_infer::Config::Precision::kInt8;
                }
                config.EnableTensorRtEngine(
                        1 << 20, 10, 3,
                        precision,
                        false, false);

                std::map<std::string, std::vector<int>> min_input_shape = {
                        {"x",            {1,  3, 32, 10}},
                        {"lstm_0.tmp_0", {10, 1, 96}}};
                std::map<std::string, std::vector<int>> max_input_shape = {
                        {"x",            {1,    3, 32, 2000}},
                        {"lstm_0.tmp_0", {1000, 1, 96}}};
                std::map<std::string, std::vector<int>> opt_input_shape = {
                        {"x",            {1,  3, 32, 320}},
                        {"lstm_0.tmp_0", {25, 1, 96}}};

                config.SetTRTDynamicShapeInfo(min_input_shape, max_input_shape,
                                              opt_input_shape);
            }
        } else {
            config.DisableGpu();
            if (this->use_mkldnn_) {
                config.EnableMKLDNN();
                // cache 10 different shapes for mkldnn to avoid memory leak
                config.SetMkldnnCacheCapacity(10);
            }
            config.SetCpuMathLibraryNumThreads(this->cpu_math_library_num_threads_);
        }

        config.SwitchUseFeedFetchOps(false);
        // true for multiple input
        config.SwitchSpecifyInputNames(true);

        config.SwitchIrOptim(true);

        config.EnableMemoryOptim();
//   config.DisableGlogInfo();

        this->predictor_ = CreatePredictor(config);
    }


    void kdkocrDetInfer::getRect(const std::string &imagePath,std::vector<std::vector<std::vector<int>>> &boxes) {
        std::vector<cv::String> cv_all_img_names;
        cv::glob(imagePath, cv_all_img_names);
        std::vector<double> time_info = {0, 0, 0};
        for (int i = 0; i < cv_all_img_names.size(); ++i) {
//       LOG(INFO) << "The predict img: " << cv_all_img_names[i];
            cv::Mat srcimg = cv::imread(cv_all_img_names[i], cv::IMREAD_COLOR);
            if (!srcimg.data) {
                std::cerr << "[ERROR] image read failed! image path: " << cv_all_img_names[i] << endl;
                exit(1);
            }

            std::vector<double> det_times;

            Run(srcimg, boxes, &det_times);

            time_info[0] += det_times[0];
            time_info[1] += det_times[1];
            time_info[2] += det_times[2];

        }

    }

    void kdkocrDetInfer::LoadModel(const string &model_dir) {
        paddle_infer::Config config;
        //取消log
        config.DisableGlogInfo();
        config.SetModel(model_dir + "/inference.pdmodel",
                        model_dir + "/inference.pdiparams");

        if (this->use_gpu_) {
            config.EnableUseGpu(this->gpu_mem_, this->gpu_id_);
            if (this->use_tensorrt_) {
                auto precision = paddle_infer::Config::Precision::kFloat32;
                if (this->precision_ == "fp16") {
                    precision = paddle_infer::Config::Precision::kHalf;
                }
                if (this->precision_ == "int8") {
                    precision = paddle_infer::Config::Precision::kInt8;
                }
                config.EnableTensorRtEngine(
                        1 << 20, 10, 3,
                        precision,
                        false, false);
                std::map<std::string, std::vector<int>> min_input_shape = {
                        {"x",                         {1, 3,  50, 50}},
                        {"conv2d_92.tmp_0",           {1, 96, 20, 20}},
                        {"conv2d_91.tmp_0",           {1, 96, 10, 10}},
                        {"nearest_interp_v2_1.tmp_0", {1, 96, 10, 10}},
                        {"nearest_interp_v2_2.tmp_0", {1, 96, 20, 20}},
                        {"nearest_interp_v2_3.tmp_0", {1, 24, 20, 20}},
                        {"nearest_interp_v2_4.tmp_0", {1, 24, 20, 20}},
                        {"nearest_interp_v2_5.tmp_0", {1, 24, 20, 20}},
                        {"elementwise_add_7",         {1, 56, 2,  2}},
                        {"nearest_interp_v2_0.tmp_0", {1, 96, 2,  2}}};
                std::map<std::string, std::vector<int>> max_input_shape = {
                        {"x",                         {1, 3,  this->max_side_len_, this->max_side_len_}},
                        {"conv2d_92.tmp_0",           {1, 96, 400,                 400}},
                        {"conv2d_91.tmp_0",           {1, 96, 200,                 200}},
                        {"nearest_interp_v2_1.tmp_0", {1, 96, 200,                 200}},
                        {"nearest_interp_v2_2.tmp_0", {1, 96, 400,                 400}},
                        {"nearest_interp_v2_3.tmp_0", {1, 24, 400,                 400}},
                        {"nearest_interp_v2_4.tmp_0", {1, 24, 400,                 400}},
                        {"nearest_interp_v2_5.tmp_0", {1, 24, 400,                 400}},
                        {"elementwise_add_7",         {1, 56, 400,                 400}},
                        {"nearest_interp_v2_0.tmp_0", {1, 96, 400,                 400}}};
                std::map<std::string, std::vector<int>> opt_input_shape = {
                        {"x",                         {1, 3,  640, 640}},
                        {"conv2d_92.tmp_0",           {1, 96, 160, 160}},
                        {"conv2d_91.tmp_0",           {1, 96, 80,  80}},
                        {"nearest_interp_v2_1.tmp_0", {1, 96, 80,  80}},
                        {"nearest_interp_v2_2.tmp_0", {1, 96, 160, 160}},
                        {"nearest_interp_v2_3.tmp_0", {1, 24, 160, 160}},
                        {"nearest_interp_v2_4.tmp_0", {1, 24, 160, 160}},
                        {"nearest_interp_v2_5.tmp_0", {1, 24, 160, 160}},
                        {"elementwise_add_7",         {1, 56, 40,  40}},
                        {"nearest_interp_v2_0.tmp_0", {1, 96, 40,  40}}};

                config.SetTRTDynamicShapeInfo(min_input_shape, max_input_shape,
                                              opt_input_shape);
            }
        } else {
            config.DisableGpu();
            if (this->use_mkldnn_) {
                config.EnableMKLDNN();
                // cache 10 different shapes for mkldnn to avoid memory leak
                config.SetMkldnnCacheCapacity(10);
            }
           // config.SetCpuMathLibraryNumThreads(this->cpu_math_library_num_threads_);
            config.SetCpuMathLibraryNumThreads(2);
        }
        // use zero_copy_run as default
        config.SwitchUseFeedFetchOps(false);
        // true for multiple input
        config.SwitchSpecifyInputNames(true);

        config.SwitchIrOptim(true);

        config.EnableMemoryOptim();
        // config.DisableGlogInfo();

        this->predictor_ = CreatePredictor(config);
    }

    void kdkocrDetInfer::Run(cv::Mat &img, vector<std::vector<std::vector<int>>> &boxes, std::vector<double> *times) {
        float ratio_h{};
        float ratio_w{};

        cv::Mat srcimg;
        cv::Mat resize_img;
        img.copyTo(srcimg);

        auto preprocess_start = std::chrono::steady_clock::now();
        this->resize_op_.Run(img, resize_img, this->max_side_len_, ratio_h, ratio_w,
                             this->use_tensorrt_);

        this->normalize_op_.Run(&resize_img, this->mean_, this->scale_,
                                this->is_scale_);

        std::vector<float> input(1 * 3 * resize_img.rows * resize_img.cols, 0.0f);
        this->permute_op_.Run(&resize_img, input.data());
        auto preprocess_end = std::chrono::steady_clock::now();

        // Inference.
        auto input_names = this->predictor_->GetInputNames();
        auto input_t = this->predictor_->GetInputHandle(input_names[0]);
        input_t->Reshape({1, 3, resize_img.rows, resize_img.cols});
        auto inference_start = std::chrono::steady_clock::now();
        input_t->CopyFromCpu(input.data());

        this->predictor_->Run();

        std::vector<float> out_data;
        auto output_names = this->predictor_->GetOutputNames();
        auto output_t = this->predictor_->GetOutputHandle(output_names[0]);
        std::vector<int> output_shape = output_t->shape();
        int out_num = std::accumulate(output_shape.begin(), output_shape.end(), 1,
                                      std::multiplies<int>());

        out_data.resize(out_num);
        output_t->CopyToCpu(out_data.data());
        auto inference_end = std::chrono::steady_clock::now();

        auto postprocess_start = std::chrono::steady_clock::now();
        int n2 = output_shape[2];
        int n3 = output_shape[3];
        int n = n2 * n3;

        std::vector<float> pred(n, 0.0);
        std::vector<unsigned char> cbuf(n, ' ');

        for (int i = 0; i < n; i++) {
            pred[i] = float(out_data[i]);
            cbuf[i] = (unsigned char)((out_data[i]) * 255);
        }

        cv::Mat cbuf_map(n2, n3, CV_8UC1, (unsigned char *)cbuf.data());
        cv::Mat pred_map(n2, n3, CV_32F, (float *)pred.data());

        const double threshold = this->det_db_thresh_ * 255;
        const double maxvalue = 255;
        cv::Mat bit_map;
        cv::threshold(cbuf_map, bit_map, threshold, maxvalue, cv::THRESH_BINARY);
        cv::Mat dilation_map;
        cv::Mat dila_ele = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
        cv::dilate(bit_map, dilation_map, dila_ele);
        boxes = post_processor_.BoxesFromBitmap(
                pred_map, dilation_map, this->det_db_box_thresh_,
                this->det_db_unclip_ratio_, this->use_polygon_score_);

        boxes = post_processor_.FilterTagDetRes(boxes, ratio_h, ratio_w, srcimg);
        auto postprocess_end = std::chrono::steady_clock::now();
        //std::cout << "Detected boxes num: " << boxes.size() << endl;

        std::chrono::duration<float> preprocess_diff = preprocess_end - preprocess_start;
        times->push_back(double(preprocess_diff.count() * 1000));
        std::chrono::duration<float> inference_diff = inference_end - inference_start;
        times->push_back(double(inference_diff.count() * 1000));
        std::chrono::duration<float> postprocess_diff = postprocess_end - postprocess_start;
        times->push_back(double(postprocess_diff.count() * 1000));

        // visualization
        Utility::VisualizeBboxes(srcimg, boxes);
        if (this->visualize_) {
            Utility::VisualizeBboxes(srcimg, boxes);
        }

    }


    void kdkocrClsInfer::LoadModel(const string &model_dir) {
        AnalysisConfig config;
        //cancel log
        config.DisableGlogInfo();
        config.SetModel(model_dir + "/inference.pdmodel",
                        model_dir + "/inference.pdiparams");

        if (this->use_gpu_) {
            config.EnableUseGpu(this->gpu_mem_, this->gpu_id_);
            if (this->use_tensorrt_) {
                auto precision = paddle_infer::Config::Precision::kFloat32;
                if (this->precision_ == "fp16") {
                    precision = paddle_infer::Config::Precision::kHalf;
                }
                if (this->precision_ == "int8") {
                    precision = paddle_infer::Config::Precision::kInt8;
                }
                config.EnableTensorRtEngine(
                        1 << 20, 10, 3,
                        precision,
                        false, false);
            }
        } else {
            config.DisableGpu();
            if (this->use_mkldnn_) {
                config.EnableMKLDNN();
            }
            config.SetCpuMathLibraryNumThreads(this->cpu_math_library_num_threads_);
        }

        // false for zero copy tensor
        config.SwitchUseFeedFetchOps(false);
        // true for multiple input
        config.SwitchSpecifyInputNames(true);

        config.SwitchIrOptim(true);

        config.EnableMemoryOptim();
        config.DisableGlogInfo();

        this->predictor_ = CreatePredictor(config);
    }

    cv::Mat kdkocrClsInfer::Run(cv::Mat &img) {
        cv::Mat src_img;
        img.copyTo(src_img);
        cv::Mat resize_img;

        std::vector<int> cls_image_shape = {3, 48, 192};
        int index = 0;
        float wh_ratio = float(img.cols) / float(img.rows);

        this->resize_op_.Run(img, resize_img, this->use_tensorrt_, cls_image_shape);

        this->normalize_op_.Run(&resize_img, this->mean_, this->scale_,
                                this->is_scale_);

        std::vector<float> input(1 * 3 * resize_img.rows * resize_img.cols, 0.0f);

        this->permute_op_.Run(&resize_img, input.data());

        // Inference.
        auto input_names = this->predictor_->GetInputNames();
        auto input_t = this->predictor_->GetInputHandle(input_names[0]);
        input_t->Reshape({1, 3, resize_img.rows, resize_img.cols});
        input_t->CopyFromCpu(input.data());
        this->predictor_->Run();

        std::vector<float> softmax_out;
        std::vector<int64_t> label_out;
        auto output_names = this->predictor_->GetOutputNames();
        auto softmax_out_t = this->predictor_->GetOutputHandle(output_names[0]);
        auto softmax_shape_out = softmax_out_t->shape();

        int softmax_out_num =
                std::accumulate(softmax_shape_out.begin(), softmax_shape_out.end(), 1,
                                std::multiplies<int>());

        softmax_out.resize(softmax_out_num);

        softmax_out_t->CopyToCpu(softmax_out.data());

        float score = 0;
        int label = 0;
        for (int i = 0; i < softmax_out_num; i++) {
            if (softmax_out[i] > score) {
                score = softmax_out[i];
                label = i;
            }
        }
        if (label % 2 == 1 && score > this->cls_thresh) {
            cv::rotate(src_img, src_img, 1);
        }
        return src_img;
    }
}
