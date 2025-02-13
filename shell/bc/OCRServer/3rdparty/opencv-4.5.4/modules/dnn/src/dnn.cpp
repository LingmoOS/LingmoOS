/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2013, OpenCV Foundation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "precomp.hpp"
#include "op_halide.hpp"
#include "op_inf_engine.hpp"
#include "ie_ngraph.hpp"
#include "op_vkcom.hpp"
#include "op_cuda.hpp"

#ifdef HAVE_CUDA
#include "cuda4dnn/init.hpp"
#include "cuda4dnn/primitives/eltwise.hpp" // required by fuseLayers
#endif

#include "halide_scheduler.hpp"

#include <set>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <numeric>
#include <memory>
#include <opencv2/dnn/shape_utils.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn/layer_reg.private.hpp>

#include <opencv2/core/utils/configuration.private.hpp>
#include <opencv2/core/utils/logger.hpp>

namespace cv {
namespace dnn {
CV__DNN_INLINE_NS_BEGIN

static size_t DNN_NETWORK_DUMP = utils::getConfigurationParameterSizeT("OPENCV_DNN_NETWORK_DUMP", 0);

// this option is useful to run valgrind memory errors detection
static bool DNN_DISABLE_MEMORY_OPTIMIZATIONS = utils::getConfigurationParameterBool("OPENCV_DNN_DISABLE_MEMORY_OPTIMIZATIONS", false);

#ifdef HAVE_OPENCL
static bool DNN_OPENCL_ALLOW_ALL_DEVICES = utils::getConfigurationParameterBool("OPENCV_DNN_OPENCL_ALLOW_ALL_DEVICES", false);
#endif

static int PARAM_DNN_BACKEND_DEFAULT = (int)utils::getConfigurationParameterSizeT("OPENCV_DNN_BACKEND_DEFAULT",
#ifdef HAVE_INF_ENGINE
    (size_t)DNN_BACKEND_INFERENCE_ENGINE
#else
    (size_t)DNN_BACKEND_OPENCV
#endif
);

// Additional checks (slowdowns execution!)
static bool DNN_CHECK_NAN_INF = utils::getConfigurationParameterBool("OPENCV_DNN_CHECK_NAN_INF", false);
static bool DNN_CHECK_NAN_INF_DUMP = utils::getConfigurationParameterBool("OPENCV_DNN_CHECK_NAN_INF_DUMP", false);
static bool DNN_CHECK_NAN_INF_RAISE_ERROR = utils::getConfigurationParameterBool("OPENCV_DNN_CHECK_NAN_INF_RAISE_ERROR", false);

using std::vector;
using std::map;
using std::make_pair;
using std::set;
using std::string;

//==================================================================================================

class BackendRegistry
{
public:
    typedef std::vector< std::pair<Backend, Target> > BackendsList;
    const BackendsList & getBackends() const { return backends; }
    static BackendRegistry & getRegistry()
    {
        static BackendRegistry impl;
        return impl;
    }

#ifdef HAVE_INF_ENGINE
    static inline bool checkIETarget(Target target)
    {
#if INF_ENGINE_VER_MAJOR_GE(INF_ENGINE_RELEASE_2019R3)
        // Lightweight detection
        const std::vector<std::string> devices = getCore("").GetAvailableDevices();
        for (std::vector<std::string>::const_iterator i = devices.begin(); i != devices.end(); ++i)
        {
            if (std::string::npos != i->find("MYRIAD") && target == DNN_TARGET_MYRIAD)
                return true;
            if (std::string::npos != i->find("HDDL") && target == DNN_TARGET_HDDL)
                return true;
            else if (std::string::npos != i->find("FPGA") && target == DNN_TARGET_FPGA)
                return true;
            else if (std::string::npos != i->find("CPU") && target == DNN_TARGET_CPU)
                return true;
            else if (std::string::npos != i->find("GPU") && (target == DNN_TARGET_OPENCL || target == DNN_TARGET_OPENCL_FP16))
                return true;
        }
        return false;
#else
        cv::dnn::Net net;
        cv::dnn::LayerParams lp;
        lp.set("kernel_size", 1);
        lp.set("num_output", 1);
        lp.set("bias_term", false);
        lp.type = "Convolution";
        lp.name = "testLayer";
        lp.blobs.push_back(Mat({1, 2, 1, 1}, CV_32F, Scalar(1)));
        net.addLayerToPrev(lp.name, lp.type, lp);
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_INFERENCE_ENGINE);
        net.setPreferableTarget(target);
        static int inpDims[] = {1, 2, 3, 4};
        net.setInput(cv::Mat(4, &inpDims[0], CV_32FC1, cv::Scalar(0)));
        try
        {
            net.forward();
        }
        catch(const std::exception& e)
        {
            CV_LOG_INFO(NULL, "checkIETarget(" << (int)target << ") has failed with message: " << e.what());
            return false;
        }
        return true;
#endif
    }
#endif

private:
    BackendRegistry()
    {
#ifdef HAVE_HALIDE
        backends.push_back(std::make_pair(DNN_BACKEND_HALIDE, DNN_TARGET_CPU));
#  ifdef HAVE_OPENCL
        if (cv::ocl::useOpenCL())
            backends.push_back(std::make_pair(DNN_BACKEND_HALIDE, DNN_TARGET_OPENCL));
#  endif
#endif // HAVE_HALIDE

#ifdef HAVE_INF_ENGINE
        if (checkIETarget(DNN_TARGET_CPU)) {
#ifdef HAVE_DNN_IE_NN_BUILDER_2019
            backends.push_back(std::make_pair(DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019, DNN_TARGET_CPU));
#endif
#ifdef HAVE_DNN_NGRAPH
            backends.push_back(std::make_pair(DNN_BACKEND_INFERENCE_ENGINE_NGRAPH, DNN_TARGET_CPU));
#endif
        }
        if (checkIETarget(DNN_TARGET_MYRIAD)) {
#ifdef HAVE_DNN_IE_NN_BUILDER_2019
            backends.push_back(std::make_pair(DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019, DNN_TARGET_MYRIAD));
#endif
#ifdef HAVE_DNN_NGRAPH
            backends.push_back(std::make_pair(DNN_BACKEND_INFERENCE_ENGINE_NGRAPH, DNN_TARGET_MYRIAD));
#endif
        }
        if (checkIETarget(DNN_TARGET_HDDL)) {
#ifdef HAVE_DNN_IE_NN_BUILDER_2019
            backends.push_back(std::make_pair(DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019, DNN_TARGET_HDDL));
#endif
#ifdef HAVE_DNN_NGRAPH
            backends.push_back(std::make_pair(DNN_BACKEND_INFERENCE_ENGINE_NGRAPH, DNN_TARGET_HDDL));
#endif
        }
#ifdef HAVE_DNN_IE_NN_BUILDER_2019
        if (checkIETarget(DNN_TARGET_FPGA))
            backends.push_back(std::make_pair(DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019, DNN_TARGET_FPGA));
#endif
#ifdef HAVE_OPENCL
        if (cv::ocl::useOpenCL() && ocl::Device::getDefault().isIntel())
        {
            if (checkIETarget(DNN_TARGET_OPENCL)) {
#ifdef HAVE_DNN_IE_NN_BUILDER_2019
                backends.push_back(std::make_pair(DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019, DNN_TARGET_OPENCL));
#endif
#ifdef HAVE_DNN_NGRAPH
                backends.push_back(std::make_pair(DNN_BACKEND_INFERENCE_ENGINE_NGRAPH, DNN_TARGET_OPENCL));
#endif
            }
            if (checkIETarget(DNN_TARGET_OPENCL_FP16)) {
#ifdef HAVE_DNN_IE_NN_BUILDER_2019
                backends.push_back(std::make_pair(DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019, DNN_TARGET_OPENCL_FP16));
#endif
#ifdef HAVE_DNN_NGRAPH
                backends.push_back(std::make_pair(DNN_BACKEND_INFERENCE_ENGINE_NGRAPH, DNN_TARGET_OPENCL_FP16));
#endif
            }
        }
#endif
#endif // HAVE_INF_ENGINE

#ifdef HAVE_OPENCL
        if (cv::ocl::useOpenCL())
        {
            backends.push_back(std::make_pair(DNN_BACKEND_OPENCV, DNN_TARGET_OPENCL));
            backends.push_back(std::make_pair(DNN_BACKEND_OPENCV, DNN_TARGET_OPENCL_FP16));
        }
#endif

        backends.push_back(std::make_pair(DNN_BACKEND_OPENCV, DNN_TARGET_CPU));

#ifdef HAVE_VULKAN
        if (haveVulkan())
            backends.push_back(std::make_pair(DNN_BACKEND_VKCOM, DNN_TARGET_VULKAN));
#endif

#ifdef HAVE_CUDA
        if (haveCUDA())
        {
            backends.push_back(std::make_pair(DNN_BACKEND_CUDA, DNN_TARGET_CUDA));
            backends.push_back(std::make_pair(DNN_BACKEND_CUDA, DNN_TARGET_CUDA_FP16));
        }
#endif
    }

    BackendsList backends;
};


std::vector< std::pair<Backend, Target> > getAvailableBackends()
{
    return BackendRegistry::getRegistry().getBackends();
}

std::vector<Target> getAvailableTargets(Backend be)
{
    if (be == DNN_BACKEND_DEFAULT)
        be = (Backend)PARAM_DNN_BACKEND_DEFAULT;
#ifdef HAVE_INF_ENGINE
    if (be == DNN_BACKEND_INFERENCE_ENGINE)
        be = getInferenceEngineBackendTypeParam();
#endif

    std::vector<Target> result;
    const BackendRegistry::BackendsList all_backends = getAvailableBackends();
    for(BackendRegistry::BackendsList::const_iterator i = all_backends.begin(); i != all_backends.end(); ++i )
    {
        if (i->first == be)
            result.push_back(i->second);
    }
    return result;
}

//==================================================================================================

namespace
{
    typedef std::vector<MatShape> ShapesVec;

    struct LayerShapes
    {
        ShapesVec in, out, internal;
        // No guarantees that layer which support in-place computations
        // will be computed in-place (input.data_ptr == output.data_ptr).
        // If layer said that it could work in-place and layers after it
        // no longer use input blob, we'll set output = input.
        bool supportInPlace;
        LayerShapes() {supportInPlace = false;}
    };
}

Mat blobFromImage(InputArray image, double scalefactor, const Size& size,
                  const Scalar& mean, bool swapRB, bool crop, int ddepth)
{
    CV_TRACE_FUNCTION();
    Mat blob;
    blobFromImage(image, blob, scalefactor, size, mean, swapRB, crop, ddepth);
    return blob;
}

void blobFromImage(InputArray image, OutputArray blob, double scalefactor,
                   const Size& size, const Scalar& mean, bool swapRB, bool crop, int ddepth)
{
    CV_TRACE_FUNCTION();
    std::vector<Mat> images(1, image.getMat());
    blobFromImages(images, blob, scalefactor, size, mean, swapRB, crop, ddepth);
}

Mat blobFromImages(InputArrayOfArrays images, double scalefactor, Size size,
                   const Scalar& mean, bool swapRB, bool crop, int ddepth)
{
    CV_TRACE_FUNCTION();
    Mat blob;
    blobFromImages(images, blob, scalefactor, size, mean, swapRB, crop, ddepth);
    return blob;
}

void blobFromImages(InputArrayOfArrays images_, OutputArray blob_, double scalefactor,
                    Size size, const Scalar& mean_, bool swapRB, bool crop, int ddepth)
{
    CV_TRACE_FUNCTION();
    CV_CheckType(ddepth, ddepth == CV_32F || ddepth == CV_8U, "Blob depth should be CV_32F or CV_8U");
    if (ddepth == CV_8U)
    {
        CV_CheckEQ(scalefactor, 1.0, "Scaling is not supported for CV_8U blob depth");
        CV_Assert(mean_ == Scalar() && "Mean subtraction is not supported for CV_8U blob depth");
    }

    std::vector<Mat> images;
    images_.getMatVector(images);
    CV_Assert(!images.empty());
    for (size_t i = 0; i < images.size(); i++)
    {
        Size imgSize = images[i].size();
        if (size == Size())
            size = imgSize;
        if (size != imgSize)
        {
            if(crop)
            {
              float resizeFactor = std::max(size.width / (float)imgSize.width,
                                            size.height / (float)imgSize.height);
              resize(images[i], images[i], Size(), resizeFactor, resizeFactor, INTER_LINEAR);
              Rect crop(Point(0.5 * (images[i].cols - size.width),
                              0.5 * (images[i].rows - size.height)),
                        size);
              images[i] = images[i](crop);
            }
            else
              resize(images[i], images[i], size, 0, 0, INTER_LINEAR);
        }
        if(images[i].depth() == CV_8U && ddepth == CV_32F)
            images[i].convertTo(images[i], CV_32F);
        Scalar mean = mean_;
        if (swapRB)
            std::swap(mean[0], mean[2]);

        images[i] -= mean;
        images[i] *= scalefactor;
    }

    size_t nimages = images.size();
    Mat image0 = images[0];
    int nch = image0.channels();
    CV_Assert(image0.dims == 2);
    if (nch == 3 || nch == 4)
    {
        int sz[] = { (int)nimages, nch, image0.rows, image0.cols };
        blob_.create(4, sz, ddepth);
        Mat blob = blob_.getMat();
        Mat ch[4];

        for(size_t i = 0; i < nimages; i++ )
        {
            const Mat& image = images[i];
            CV_Assert(image.depth() == blob_.depth());
            nch = image.channels();
            CV_Assert(image.dims == 2 && (nch == 3 || nch == 4));
            CV_Assert(image.size() == image0.size());

            for( int j = 0; j < nch; j++ )
                ch[j] = Mat(image.rows, image.cols, ddepth, blob.ptr((int)i, j));
            if(swapRB)
                std::swap(ch[0], ch[2]);
            split(image, ch);
        }
    }
    else
    {
       CV_Assert(nch == 1);
       int sz[] = { (int)nimages, 1, image0.rows, image0.cols };
       blob_.create(4, sz, ddepth);
       Mat blob = blob_.getMat();

       for(size_t i = 0; i < nimages; i++ )
       {
           const Mat& image = images[i];
           CV_Assert(image.depth() == blob_.depth());
           nch = image.channels();
           CV_Assert(image.dims == 2 && (nch == 1));
           CV_Assert(image.size() == image0.size());

           image.copyTo(Mat(image.rows, image.cols, ddepth, blob.ptr((int)i, 0)));
       }
    }
}

void imagesFromBlob(const cv::Mat& blob_, OutputArrayOfArrays images_)
{
    CV_TRACE_FUNCTION();

    //A blob is a 4 dimensional matrix in floating point precision
    //blob_[0] = batchSize = nbOfImages
    //blob_[1] = nbOfChannels
    //blob_[2] = height
    //blob_[3] = width
    CV_Assert(blob_.depth() == CV_32F);
    CV_Assert(blob_.dims == 4);

    images_.create(cv::Size(1, blob_.size[0]), blob_.depth());

    std::vector<Mat> vectorOfChannels(blob_.size[1]);
    for (int n = 0; n <  blob_.size[0]; ++n)
    {
        for (int c = 0; c < blob_.size[1]; ++c)
        {
            vectorOfChannels[c] = getPlane(blob_, n, c);
        }
        cv::merge(vectorOfChannels, images_.getMatRef(n));
    }
}

#ifdef HAVE_OPENCL
class OpenCLBackendWrapper : public BackendWrapper
{
public:
    OpenCLBackendWrapper(Mat& m) : BackendWrapper(DNN_BACKEND_OPENCV, DNN_TARGET_OPENCL)
    {
        m.copyTo(umat);
        host = &m;
        hostDirty = false;
    }

    OpenCLBackendWrapper(const Ptr<BackendWrapper>& baseBuffer, Mat& m)
        : BackendWrapper(DNN_BACKEND_OPENCV, DNN_TARGET_OPENCL)
    {
        Ptr<OpenCLBackendWrapper> base = baseBuffer.dynamicCast<OpenCLBackendWrapper>();
        CV_Assert(!base.empty());

        host = &m;

        int shape[] = {1, (int)base->umat.total()};
        umat = base->umat.reshape(1, 2, &shape[0])
                         .colRange(0, host->total())
                         .reshape(1, host->dims, &host->size[0]);
        hostDirty = false;
    }

    static Ptr<BackendWrapper> create(Mat& m)
    {
        return Ptr<BackendWrapper>(new OpenCLBackendWrapper(m));
    }

    static Ptr<BackendWrapper> create(const Ptr<BackendWrapper>& baseBuffer, Mat& m)
    {
        return Ptr<BackendWrapper>(new OpenCLBackendWrapper(baseBuffer, m));
    }

    static std::vector<UMat> getUMatVector(const std::vector<Ptr<BackendWrapper> >& wrappers)
    {
        const int numWrappers = wrappers.size();
        std::vector<UMat> mats(wrappers.size());
        for (int i = 0; i < numWrappers; ++i)
        {
            Ptr<OpenCLBackendWrapper> umatWrapper = wrappers[i].dynamicCast<OpenCLBackendWrapper>();
            CV_Assert(!umatWrapper.empty());
            umatWrapper->copyToDevice();
            mats[i] = umatWrapper->umat;
        }
        return mats;
    }

    // Replaces all umats in wrappers to specific ones.
    static void update(const std::vector<Ptr<BackendWrapper> >& wrappers,
                       const std::vector<UMat>& umats)
    {
        CV_Assert(wrappers.size() == umats.size());
        for (int i = 0, n = umats.size(); i < n; ++i)
        {
            Ptr<OpenCLBackendWrapper> umatWrapper = wrappers[i].dynamicCast<OpenCLBackendWrapper>();
            CV_Assert(!umatWrapper.empty());
            umatWrapper->umat = umats[i];
        }
    }

    ~OpenCLBackendWrapper() {}

    // Copies data from device to a host memory.
    virtual void copyToHost() CV_OVERRIDE
    {
        umat.copyTo(*host);
    }

    virtual void setHostDirty() CV_OVERRIDE
    {
        hostDirty = true;
    };

    void copyToDevice()
    {
        if (hostDirty)
        {
            host->copyTo(umat);
            hostDirty = false;
        }
    }

private:
    UMat umat;
    Mat* host;
    bool hostDirty;
};
#endif

struct LayerPin
{
    int lid;
    int oid;

    LayerPin(int layerId = -1, int outputId = -1)
        : lid(layerId), oid(outputId) {}

    bool valid() const
    {
        return (lid >= 0 && oid >= 0);
    }

    bool equal(const LayerPin &r) const
    {
        return (lid == r.lid && oid == r.oid);
    }

    bool operator<(const LayerPin &r) const
    {
        return lid < r.lid || (lid == r.lid && oid < r.oid);
    }

    bool operator ==(const LayerPin &r) const
    {
        return lid == r.lid && oid == r.oid;
    }
};

struct LayerData
{
    LayerData() : id(-1), dtype(CV_32F), skip(false), flag(0) {}
    LayerData(int _id, const String &_name, const String &_type, const int &_dtype, LayerParams &_params)
        : id(_id), name(_name), type(_type), dtype(_dtype), params(_params), skip(false), flag(0)
    {
        CV_TRACE_FUNCTION();

        //add logging info
        params.name = name;
        params.type = type;
    }

    int id;
    String name;
    String type;
    int dtype; // Datatype of output blobs.
    LayerParams params;

    std::vector<LayerPin> inputBlobsId;
    std::set<int> inputLayersId;
    std::set<int> requiredOutputs;
    std::vector<LayerPin> consumers;
    std::vector<Ptr<BackendWrapper> > outputBlobsWrappers;
    std::vector<Ptr<BackendWrapper> > inputBlobsWrappers;
    std::vector<Ptr<BackendWrapper> > internalBlobsWrappers;

#ifdef HAVE_CUDA
    /* output ids which must be transferred to the host in the background
     * after the completion of the forward pass of the layer
     */
    std::vector<int> cudaD2HBackgroundTransfers;
#endif

    Ptr<Layer> layerInstance;
    std::vector<Mat> outputBlobs;
    std::vector<Mat*> inputBlobs;
    std::vector<Mat> internals;
    // Computation nodes of implemented backends (except DEFAULT).
    std::map<int, Ptr<BackendNode> > backendNodes;
    // Flag for skip layer computation for specific backend.
    bool skip;

    int flag;

    Ptr<Layer> getLayerInstance()
    {
        CV_TRACE_FUNCTION();
        CV_TRACE_ARG_VALUE(type, "type", type.c_str());

        if (layerInstance)
            return layerInstance;

        layerInstance = LayerFactory::createLayerInstance(type, params);
        if (!layerInstance)
        {
            CV_Error(Error::StsError, "Can't create layer \"" + name + "\" of type \"" + type + "\"");
        }

        return layerInstance;
    }
};

//fake layer containing network input blobs
struct DataLayer : public Layer
{
    DataLayer() : Layer()
    {
        skip = false;
    }

    virtual bool supportBackend(int backendId) CV_OVERRIDE
    {
        return backendId == DNN_BACKEND_OPENCV ||
               (backendId == DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019 && inputsData.size() == 1);
    }

    void forward(InputArrayOfArrays inputs_arr, OutputArrayOfArrays outputs_arr, OutputArrayOfArrays internals_arr) CV_OVERRIDE
    {
        CV_TRACE_FUNCTION();
        CV_TRACE_ARG_VALUE(name, "name", name.c_str());

        CV_OCL_RUN(IS_DNN_OPENCL_TARGET(preferableTarget),
                   forward_ocl(inputs_arr, outputs_arr, internals_arr))

        if (outputs_arr.depth() == CV_16S)
        {
            forward_fallback(inputs_arr, outputs_arr, internals_arr);
            return;
        }

        std::vector<Mat> outputs, internals;
        outputs_arr.getMatVector(outputs);
        internals_arr.getMatVector(internals);

        // Supported modes:
        // | Input type | Output type |
        // |       fp32 |        fp32 |
        // |      uint8 |        fp32 |
        for (int i = 0; i < inputsData.size(); ++i)
        {
            double scale = scaleFactors[i];
            Scalar& mean = means[i];
            CV_Assert(mean == Scalar() || inputsData[i].size[1] <= 4);
            CV_CheckTypeEQ(outputs[i].type(), CV_32FC1, "");

            bool singleMean = true;
            for (int j = 1; j < std::min(4, inputsData[i].size[1]) && singleMean; ++j)
            {
                singleMean = mean[j] == mean[j - 1];
            }

            if (singleMean)
            {
                inputsData[i].convertTo(outputs[i], CV_32F, scale, -mean[0] * scale);
            }
            else
            {
                for (int n = 0; n < inputsData[i].size[0]; ++n)
                    for (int c = 0; c < inputsData[i].size[1]; ++c)
                    {
                        Mat inp = getPlane(inputsData[i], n, c);
                        Mat out = getPlane(outputs[i], n, c);
                        inp.convertTo(out, CV_32F, scale, -mean[c] * scale);
                    }
            }
        }
    }

#ifdef HAVE_OPENCL
    std::vector<Mat> tmp_expressions;
    bool forward_ocl(InputArrayOfArrays, OutputArrayOfArrays outputs_, OutputArrayOfArrays internals_)
    {
        // Supported modes:
        // | Input type | Output type |
        // |       fp32 |        fp32 |
        // |       fp32 |        fp16 |
        // |      uint8 |        fp32 |
        std::vector<UMat> outputs;
        outputs_.getUMatVector(outputs);

        tmp_expressions.clear();
        for (int i = 0; i < inputsData.size(); ++i)
        {
            Mat inputData = inputsData[i];

            double scale = scaleFactors[i];
            Scalar& mean = means[i];

            CV_Assert(mean == Scalar() || inputsData[i].size[1] <= 4);
            bool singleMean = true;
            for (int j = 1; j < std::min(4, inputsData[i].size[1]) && singleMean; ++j)
            {
                singleMean = mean[j] == mean[j - 1];
            }

            if (outputs_.depth() == CV_16S)
            {
                if (singleMean)
                {
                    tmp_expressions.push_back(Mat(scale * (inputsData[i] - mean[0])));
                    convertFp16(tmp_expressions.back(), outputs[i]);
                }
                else
                {
                    for (int n = 0; n < inputsData[i].size[0]; ++n)
                        for (int c = 0; c < inputsData[i].size[1]; ++c)
                        {
                            Mat inp = getPlane(inputsData[i], n, c);

                            std::vector<cv::Range> plane(4, Range::all());
                            plane[0] = Range(n, n + 1);
                            plane[1] = Range(c, c + 1);
                            UMat out = outputs[i](plane).reshape(1, inp.dims, inp.size);

                            tmp_expressions.push_back(scale * (inp - mean[c]));
                            convertFp16(tmp_expressions.back(), out);
                        }
                }
            }
            else
            {
                CV_Assert(outputs_.depth() == CV_32F);
                if (singleMean)
                {
                    inputsData[i].convertTo(outputs[i], CV_32F, scale, -mean[0] * scale);
                }
                else
                {
                    for (int n = 0; n < inputsData[i].size[0]; ++n)
                        for (int c = 0; c < inputsData[i].size[1]; ++c)
                        {
                            Mat inp = getPlane(inputsData[i], n, c);

                            std::vector<cv::Range> plane(4, Range::all());
                            plane[0] = Range(n, n + 1);
                            plane[1] = Range(c, c + 1);
                            UMat out = outputs[i](plane).reshape(1, inp.dims, inp.size);

                            inp.convertTo(out, CV_32F, scale, -mean[c] * scale);
                        }
                }
            }
        }
        return true;
    }
#endif

    int outputNameToIndex(const String& tgtName) CV_OVERRIDE
    {
        int idx = (int)(std::find(outNames.begin(), outNames.end(), tgtName) - outNames.begin());
        return (idx < (int)outNames.size()) ? idx : -1;
    }

    void setNames(const std::vector<String> &names)
    {
        outNames.assign(names.begin(), names.end());
        shapes.clear(); shapes.resize(outNames.size());
    }

    void setInputShape(const String& tgtName, const MatShape& shape)
    {
        std::vector<String>::const_iterator it = std::find(outNames.begin(), outNames.end(), tgtName);
        CV_Check(tgtName, it != outNames.end(), "Unknown input");
        int idx = (int)(it - outNames.begin());

        CV_Assert(idx < (int)shapes.size());
        CV_Check(tgtName, shapes[idx].empty(), "Input shape redefinition is not allowed");
        shapes[idx] = shape;
    }

    bool getMemoryShapes(const std::vector<MatShape> &inputs,
                         const int requiredOutputs,
                         std::vector<MatShape> &outputs,
                         std::vector<MatShape> &internals) const CV_OVERRIDE
    {
        CV_Assert(inputs.size() == requiredOutputs);
        outputs.assign(inputs.begin(), inputs.end());
        return false;
    }

    virtual void finalize(InputArrayOfArrays, OutputArrayOfArrays outputs_arr) CV_OVERRIDE
    {
        std::vector<Mat> outputs;
        outputs_arr.getMatVector(outputs);

        CV_Assert_N(outputs.size() == scaleFactors.size(), outputs.size() == means.size(),
                  inputsData.size() == outputs.size());
        skip = true;
        for (int i = 0; skip && i < inputsData.size(); ++i)
        {
            if (inputsData[i].data != outputs[i].data || scaleFactors[i] != 1.0 || means[i] != Scalar())
                skip = false;
        }
    }

#ifdef HAVE_DNN_IE_NN_BUILDER_2019
    virtual Ptr<BackendNode> initInfEngine(const std::vector<Ptr<BackendWrapper> >&) CV_OVERRIDE
    {
        CV_CheckEQ(inputsData.size(), (size_t)1, "");
        CV_CheckEQ(inputsData[0].dims, 4, "");
        const size_t numChannels = inputsData[0].size[1];
        CV_Assert(numChannels <= 4);

        // Scale
        InferenceEngine::TensorDesc td(InferenceEngine::Precision::FP32, {numChannels},
                                       InferenceEngine::Layout::C);
        auto weights = InferenceEngine::make_shared_blob<float>(td);
        weights->allocate();

        float* weight_buf = weights->buffer().as<float*>();
        std::fill(weight_buf, weight_buf + numChannels, scaleFactors[0]);

        // Mean subtraction
        auto biases = InferenceEngine::make_shared_blob<float>(td);
        biases->allocate();
        float* bias_buf = biases->buffer().as<float*>();

        for (int i = 0; i < numChannels; ++i)
        {
            bias_buf[i] = -means[0][i] * scaleFactors[0];
        }

        InferenceEngine::Builder::Layer ieLayer = InferenceEngine::Builder::ScaleShiftLayer(name);
        addConstantData("weights", weights, ieLayer);
        addConstantData("biases", biases, ieLayer);
        return Ptr<BackendNode>(new InfEngineBackendNode(ieLayer));
    }
#endif  // HAVE_DNN_IE_NN_BUILDER_2019

    std::vector<String> outNames;
    std::vector<MatShape> shapes;
    // Preprocessing parameters for each network's input.
    std::vector<double> scaleFactors;
    std::vector<Scalar> means;
    std::vector<Mat> inputsData;
    bool skip;
};

struct BlobManager
{
public:
    // Increase references counter to layer output.
    void addReference(const LayerPin& lp)
    {
        std::map<LayerPin, int>::iterator it = refCounter.find(lp);
        if (it == refCounter.end())
            refCounter[lp] = 1;
        else
            it->second += 1;
    }

    void addReferences(const std::vector<LayerPin>& pins)
    {
        for (int i = 0; i < pins.size(); i++)
        {
            addReference(pins[i]);
        }
    }

    // Returns number of references to allocated memory that used in specific
    // layer blob.
    int numReferences(const LayerPin& lp)
    {
        std::map<LayerPin, LayerPin>::iterator mapIt = reuseMap.find(lp);
        CV_Assert(mapIt != reuseMap.end());
        LayerPin memHost = mapIt->second;

        std::map<LayerPin, int>::iterator refIt = refCounter.find(memHost);
        CV_Assert(refIt != refCounter.end());
        return refIt->second;
    }

    // Reuse data allocated in <host> inside the <user> blob.
    void reuse(const LayerPin& host, const LayerPin& user)
    {
        CV_Assert(reuseMap.find(user) == reuseMap.end());
        CV_Assert(reuseMap.find(host) != reuseMap.end());
        LayerPin memHost = reuseMap[host];
        reuseMap[user] = memHost;
        if (refCounter.find(memHost) != refCounter.end())
        {
            std::map<LayerPin, int>::iterator userRefIt = refCounter.find(user);
            if (userRefIt != refCounter.end())
            {
                refCounter[memHost] += userRefIt->second;
                refCounter.erase(userRefIt);
            }
            else
                refCounter[memHost] += 1;
        }
    }

    // Decrease references counter to allocated memory inside specific blob.
    void releaseReference(const LayerPin& lp)
    {
        std::map<LayerPin, LayerPin>::iterator mapIt = reuseMap.find(lp);
        CV_Assert(mapIt != reuseMap.end());

        std::map<LayerPin, int>::iterator refIt = refCounter.find(mapIt->second);
        CV_Assert(refIt != refCounter.end());
        CV_Assert(refIt->second > 0);
        refIt->second -= 1;
    }

    void releaseReferences(const std::vector<LayerPin>& pins)
    {
        for (int i = 0; i < pins.size(); i++)
        {
            releaseReference(pins[i]);
        }
    }

    void reuseOrCreate(const MatShape& shape, const LayerPin& lp, Mat& dst, const int& dtype)
    {
        if (!DNN_DISABLE_MEMORY_OPTIMIZATIONS)
        {
            Mat bestBlob;
            LayerPin bestBlobPin;

            std::map<LayerPin, Mat>::iterator hostIt;
            std::map<LayerPin, int>::iterator refIt;

            const int targetTotal = total(shape);
            int bestBlobTotal = INT_MAX;

            for (hostIt = memHosts.begin(); hostIt != memHosts.end(); ++hostIt)
            {
                refIt = refCounter.find(hostIt->first);
                // Use only blobs that had references before because if not,
                // it might be used as output.
                if (refIt != refCounter.end() && refIt->second == 0)
                {
                    Mat& unusedBlob = hostIt->second;
                    if (unusedBlob.total() >= targetTotal &&
                        unusedBlob.total() < bestBlobTotal &&
                        unusedBlob.type() == dtype)
                    {
                        bestBlobPin = hostIt->first;
                        bestBlob = unusedBlob;
                        bestBlobTotal = unusedBlob.total();
                    }
                }
            }
            if (!bestBlob.empty())
            {
                reuse(bestBlobPin, lp);
                dst = bestBlob.reshape(1, 1).colRange(0, targetTotal).reshape(1, shape);
                return;
            }
        }

        {
            // if dst already has been allocated with total(shape) elements,
            // it won't be recreated and pointer of dst.data remains the same.
            dst.create(shape, dtype);
            addHost(lp, dst);
        }
    }

    void allocateBlobsForLayer(LayerData &ld, const LayerShapes& layerShapes,
                               std::vector<LayerPin>& pinsForInternalBlobs)
    {
        CV_TRACE_FUNCTION();

        pinsForInternalBlobs.clear();

        std::vector<Mat>& outputBlobs = ld.outputBlobs,
                &internalBlobs = ld.internals;

        const ShapesVec& outShapes = layerShapes.out,
                internalShapes = layerShapes.internal;

        outputBlobs.resize(std::max((size_t)1, outShapes.size())); //layer produce at least one output blob
        internalBlobs.resize(internalShapes.size());

        CV_Assert(ld.requiredOutputs.size() <= outShapes.size());

        // Check that layer could work in-place.
        bool inPlace = false;
        if (layerShapes.supportInPlace)
        {
            if (ld.inputBlobs.size() == 1)
            {
                // Get number of references to the input memory.
                int numRef = numReferences(ld.inputBlobsId[0]);
                // If current layer is one and only customer of this blob.
                inPlace = numRef == 1;
            }
        }

        ShapesVec shapes(outShapes);
        shapes.insert(shapes.end(), internalShapes.begin(), internalShapes.end());
        std::vector<Mat*> blobs;
        for(int i = 0; i < outputBlobs.size(); i++)
        {
            blobs.push_back(&outputBlobs[i]);
        }

        for(int i = 0; i < internalBlobs.size(); i++)
        {
            blobs.push_back(&internalBlobs[i]);
            if (total(internalShapes[i]))
            {
                pinsForInternalBlobs.push_back(LayerPin(ld.id, ld.outputBlobs.size() + i));
            }
        }

        addReferences(pinsForInternalBlobs);

        std::map<int, std::vector<int> > idxSizes;
        for(int i = 0; i < shapes.size(); i++)
        {
            idxSizes[total(shapes[i])].push_back(i);
        }

        std::map<int, std::vector<int> >::reverse_iterator it;
        for(it = idxSizes.rbegin(); it != idxSizes.rend(); it++)
        {
            for(int j = 0; j < it->second.size(); j++)
            {
                int index = it->second[j];
                if (total(shapes[index]))
                {
                    LayerPin blobPin(ld.id, index);
                    if (index < outShapes.size() && inPlace)
                    {
                        CV_Assert(ld.inputBlobs[0]->total() == total(shapes[index]));
                        ld.outputBlobs[index] = ld.inputBlobs[0]->reshape(1, shapes[index]);
                        reuse(ld.inputBlobsId[0], blobPin);
                    }
                    else
                        reuseOrCreate(shapes[index], blobPin, *blobs[index], ld.dtype);
                }
            }
        }
    }

    // Clear internal state. Calls before an every reallocation.
    void reset()
    {
        CV_TRACE_FUNCTION();

        refCounter.clear();
        reuseMap.clear();
        memHosts.clear();
    }

private:
    // Register allocated memory.
    void addHost(const LayerPin& lp, const Mat& mat)
    {
        CV_Assert(memHosts.find(lp) == memHosts.end());
        reuseMap[lp] = lp;
        memHosts[lp] = mat;
    }

    std::map<LayerPin, int> refCounter;
    // Maps pin to origin blob (for whom memory was allocated firstly).
    // For origin blobs key == value.
    std::map<LayerPin, LayerPin> reuseMap;
    std::map<LayerPin, Mat> memHosts;
};

static Ptr<BackendWrapper> wrapMat(int backendId, int targetId, cv::Mat& m)
{
    if (backendId == DNN_BACKEND_OPENCV)
    {
        if (targetId == DNN_TARGET_CPU)
            return Ptr<BackendWrapper>();
#ifdef HAVE_OPENCL
        else if (IS_DNN_OPENCL_TARGET(targetId))
            return OpenCLBackendWrapper::create(m);
#endif
        else
            CV_Error(Error::StsNotImplemented, "Unknown/unsupported target identifier");
    }
    else if (backendId == DNN_BACKEND_HALIDE)
    {
        CV_Assert(haveHalide());
#ifdef HAVE_HALIDE
        return Ptr<BackendWrapper>(new HalideBackendWrapper(targetId, m));
#endif  // HAVE_HALIDE
    }
    else if (backendId == DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019)
    {
#ifdef HAVE_DNN_IE_NN_BUILDER_2019
        return Ptr<BackendWrapper>(new InfEngineBackendWrapper(targetId, m));
#else
        CV_Error(Error::StsNotImplemented, "This OpenCV version is built without Inference Engine NN Builder API support");
#endif
    }
    else if (backendId == DNN_BACKEND_INFERENCE_ENGINE_NGRAPH)
    {
#ifdef HAVE_DNN_NGRAPH
        return Ptr<BackendWrapper>(new NgraphBackendWrapper(targetId, m));
#else
        CV_Error(Error::StsNotImplemented, "This OpenCV version is built without support of Inference Engine + nGraph");
#endif
    }
    else if (backendId == DNN_BACKEND_VKCOM)
    {
        CV_Assert(haveVulkan());
#ifdef HAVE_VULKAN
        return Ptr<BackendWrapper>(new VkComBackendWrapper(m));
#endif  // HAVE_VULKAN
    }
    else if (backendId == DNN_BACKEND_CUDA)
    {
        CV_Assert(haveCUDA());

#ifdef HAVE_CUDA
        switch (targetId)
        {
        case DNN_TARGET_CUDA:
            return CUDABackendWrapperFP32::create(m);
        case DNN_TARGET_CUDA_FP16:
            return CUDABackendWrapperFP16::create(m);
        default:
            CV_Assert(IS_DNN_CUDA_TARGET(targetId));
        }
#endif
    }
    else
        CV_Error(Error::StsNotImplemented, "Unknown backend identifier");
    return Ptr<BackendWrapper>();  // TODO Error?
}

static int g_networkId = 0;

detail::NetImplBase::NetImplBase()
    : networkId(CV_XADD(&g_networkId, 1))
    , networkDumpCounter(0)
    , dumpLevel(DNN_NETWORK_DUMP)
{
    // nothing
}

std::string detail::NetImplBase::getDumpFileNameBase()
{
    std::string dumpFileNameBase = cv::format("ocv_dnn_net_%05d_%02d", networkId, networkDumpCounter++);
    return dumpFileNameBase;
}

struct Net::Impl : public detail::NetImplBase
{
    typedef std::map<int, LayerShapes> LayersShapesMap;
    typedef std::map<int, LayerData> MapIdToLayerData;

    Impl()
    {
        //allocate fake net input layer
        netInputLayer = Ptr<DataLayer>(new DataLayer());
        LayerData &inpl = layers.insert( make_pair(0, LayerData()) ).first->second;
        inpl.id = 0;
        netInputLayer->name = inpl.name = "_input";
        inpl.type = "__NetInputLayer__";
        inpl.layerInstance = netInputLayer;
        layerNameToId.insert(std::make_pair(inpl.name, inpl.id));

        lastLayerId = 0;
        netWasAllocated = false;
        netWasQuantized = false;
        fusion = true;
        isAsync = false;
        preferableBackend = DNN_BACKEND_DEFAULT;
        preferableTarget = DNN_TARGET_CPU;
        skipInfEngineInit = false;
        hasDynamicShapes = false;
    }

    Ptr<DataLayer> netInputLayer;
    std::vector<LayerPin> blobsToKeep;
    MapIdToLayerData layers;
    std::map<String, int> layerNameToId;
    BlobManager blobManager;
    int preferableBackend;
    int preferableTarget;
    String halideConfigFile;
    bool skipInfEngineInit;
    bool hasDynamicShapes;
    // Map host data to backend specific wrapper.
    std::map<void*, Ptr<BackendWrapper> > backendWrappers;

    int lastLayerId;

    bool netWasAllocated;
    bool netWasQuantized;
    bool fusion;
    bool isAsync;
    std::vector<int64> layersTimings;
    Mat output_blob;

#ifdef HAVE_CUDA
    struct CudaInfo_t
    {
        CudaInfo_t(cuda4dnn::csl::CSLContext ctxt, cuda4dnn::csl::Stream d2h_stream_)
         : context(std::move(ctxt)), d2h_stream(std::move(d2h_stream_)) { }
        cuda4dnn::csl::CSLContext context;
        cuda4dnn::csl::Stream d2h_stream;
        cuda4dnn::csl::Workspace workspace;
    };

    std::unique_ptr<CudaInfo_t> cudaInfo;
#endif

    Ptr<BackendWrapper> wrap(Mat& host)
    {
        if (preferableBackend == DNN_BACKEND_OPENCV && preferableTarget == DNN_TARGET_CPU)
            return Ptr<BackendWrapper>();

        MatShape shape(host.dims);
        for (int i = 0; i < host.dims; ++i)
            shape[i] = host.size[i];

        void* data = host.data;
        if (backendWrappers.find(data) != backendWrappers.end())
        {
            Ptr<BackendWrapper> baseBuffer = backendWrappers[data];
            if (preferableBackend == DNN_BACKEND_OPENCV)
            {
#ifdef HAVE_OPENCL
                CV_Assert(IS_DNN_OPENCL_TARGET(preferableTarget));
                return OpenCLBackendWrapper::create(baseBuffer, host);
#else
                CV_Error(Error::StsInternal, "");
#endif
            }
            else if (preferableBackend == DNN_BACKEND_HALIDE)
            {
                CV_Assert(haveHalide());
#ifdef HAVE_HALIDE
                return Ptr<BackendWrapper>(new HalideBackendWrapper(baseBuffer, shape));
#endif
            }
            else if (preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019)
            {
                return wrapMat(preferableBackend, preferableTarget, host);
            }
            else if (preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NGRAPH)
            {
                return wrapMat(preferableBackend, preferableTarget, host);
            }
            else if (preferableBackend == DNN_BACKEND_VKCOM)
            {
  #ifdef HAVE_VULKAN
                return Ptr<BackendWrapper>(new VkComBackendWrapper(baseBuffer, host));
  #endif
            }
            else if (preferableBackend == DNN_BACKEND_CUDA)
            {
                CV_Assert(haveCUDA());
#ifdef HAVE_CUDA
                switch (preferableTarget)
                {
                case DNN_TARGET_CUDA:
                    return CUDABackendWrapperFP32::create(baseBuffer, shape);
                case DNN_TARGET_CUDA_FP16:
                    return CUDABackendWrapperFP16::create(baseBuffer, shape);
                default:
                    CV_Assert(IS_DNN_CUDA_TARGET(preferableTarget));
                }
#endif
            }
            else
                CV_Error(Error::StsNotImplemented, "Unknown backend identifier");
        }

        Ptr<BackendWrapper> wrapper = wrapMat(preferableBackend, preferableTarget, host);
        backendWrappers[data] = wrapper;
        return wrapper;
    }

#ifdef HAVE_HALIDE
    void compileHalide()
    {
        CV_TRACE_FUNCTION();

        CV_Assert(preferableBackend == DNN_BACKEND_HALIDE);

        HalideScheduler scheduler(halideConfigFile);
        std::vector< std::reference_wrapper<LayerData> > compileList; compileList.reserve(64);
        for (MapIdToLayerData::iterator it = layers.begin(); it != layers.end(); ++it)
        {
            LayerData &ld = it->second;
            Ptr<Layer> layer = ld.layerInstance;
            if (layer->supportBackend(DNN_BACKEND_HALIDE) && !ld.skip)
            {
                CV_Assert(!ld.backendNodes[DNN_BACKEND_HALIDE].empty());
                bool scheduled = scheduler.process(ld.backendNodes[DNN_BACKEND_HALIDE]);
                if (!scheduled)
                {
                    // Use automatic scheduling provided by layer.
                    layer->applyHalideScheduler(ld.backendNodes[DNN_BACKEND_HALIDE],
                                                ld.inputBlobs, ld.outputBlobs,
                                                preferableTarget);
                }
                compileList.emplace_back(ld);
            }
        }
        std::atomic<int> progress(0);
        auto fn = ([&] () -> void
        {
            for (;;)
            {
                int id = progress.fetch_add(1);
                if ((size_t)id >= compileList.size())
                    return;
                const LayerData& ld = compileList[id].get();
                Ptr<BackendNode> node = ld.backendNodes.find(DNN_BACKEND_HALIDE)->second;
                dnn::compileHalide(ld.outputBlobs, node, preferableTarget);
            }
        });
        size_t num_threads = std::min(compileList.size(), (size_t)std::thread::hardware_concurrency());
        num_threads = std::max((size_t)1u, std::min((size_t)8u, num_threads));
        std::vector<std::thread> threads(num_threads - 1);
        for (auto& t: threads) t = std::thread(fn);
        fn(); // process own tasks
        for (auto& t: threads) t.join();
    }
#endif

    void clear()
    {
        CV_TRACE_FUNCTION();

        MapIdToLayerData::iterator it;
        for (it = layers.begin(); it != layers.end(); it++)
        {
            if (it->second.id != 0) {
                it->second.inputBlobs.clear();
                it->second.outputBlobs.clear();
                it->second.internals.clear();
            }
            it->second.skip = false;
            //it->second.consumers.clear();
            Ptr<Layer> currLayer = it->second.layerInstance;

            if( currLayer.empty() )
                continue;

            currLayer->unsetAttached();
        }
        netWasAllocated = false;
        layersTimings.clear();
    }

    void setUpNet(const std::vector<LayerPin>& blobsToKeep_ = std::vector<LayerPin>())
    {
        CV_TRACE_FUNCTION();

        if (dumpLevel && networkDumpCounter == 0)
        {
            dumpNetworkToFile();
        }

        if (preferableBackend == DNN_BACKEND_DEFAULT)
            preferableBackend = (Backend)PARAM_DNN_BACKEND_DEFAULT;
#ifdef HAVE_INF_ENGINE
        if (preferableBackend == DNN_BACKEND_INFERENCE_ENGINE)
            preferableBackend = getInferenceEngineBackendTypeParam();
#endif

        CV_Assert(preferableBackend != DNN_BACKEND_OPENCV ||
                  preferableTarget == DNN_TARGET_CPU ||
                  preferableTarget == DNN_TARGET_OPENCL ||
                  preferableTarget == DNN_TARGET_OPENCL_FP16);
        CV_Assert(preferableBackend != DNN_BACKEND_HALIDE ||
                  preferableTarget == DNN_TARGET_CPU ||
                  preferableTarget == DNN_TARGET_OPENCL);
#ifdef HAVE_INF_ENGINE
        if (preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019 ||
            preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NGRAPH)
        {
            CV_Assert(
                  (preferableTarget == DNN_TARGET_CPU && (!isArmComputePlugin() || preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NGRAPH)) ||
                  preferableTarget == DNN_TARGET_OPENCL ||
                  preferableTarget == DNN_TARGET_OPENCL_FP16 ||
                  preferableTarget == DNN_TARGET_MYRIAD ||
                  preferableTarget == DNN_TARGET_HDDL ||
                  preferableTarget == DNN_TARGET_FPGA
            );
        }
#endif
        CV_Assert(preferableBackend != DNN_BACKEND_VKCOM ||
                  preferableTarget == DNN_TARGET_VULKAN);
        CV_Assert(preferableBackend != DNN_BACKEND_CUDA ||
                  IS_DNN_CUDA_TARGET(preferableTarget));
        if (!netWasAllocated || this->blobsToKeep != blobsToKeep_)
        {
            if (preferableBackend == DNN_BACKEND_OPENCV && IS_DNN_OPENCL_TARGET(preferableTarget))
#ifndef HAVE_OPENCL
            {
                CV_LOG_WARNING(NULL, "DNN: OpenCL target is not available in this OpenCV build, switching to CPU.");
                preferableTarget = DNN_TARGET_CPU;
            }
#else
            {
                if (!DNN_OPENCL_ALLOW_ALL_DEVICES)
                {
                    // Current implementation is only valid for GPU (#11494)
                    if (ocl::Device::getDefault().type() != ocl::Device::TYPE_GPU)
                    {
                        CV_LOG_WARNING(NULL, "DNN: OpenCL target is not supported with current OpenCL device (tested with GPUs only), switching to CPU.");
                        preferableTarget = DNN_TARGET_CPU;
                    }
                    else if (preferableTarget == DNN_TARGET_OPENCL_FP16 && !ocl::Device::getDefault().isIntel())
                    {
                        CV_LOG_WARNING(NULL,
                            "DNN: OpenCL target with fp16 precision is not supported "
                            "with current OpenCL device (tested with Intel GPUs only), "
                            "switching to OpenCL with fp32 precision.");
                        preferableTarget = DNN_TARGET_OPENCL;
                    }
                }
            }
#endif
            if (preferableBackend == DNN_BACKEND_VKCOM && !haveVulkan())
            {
                preferableBackend = DNN_BACKEND_OPENCV;
                preferableTarget = DNN_TARGET_CPU;
            }

            if (preferableBackend == DNN_BACKEND_CUDA && !haveCUDA())
            {
#ifdef HAVE_CUDA
                CV_LOG_WARNING(NULL, "unable to use CUDA backend; switching to CPU");
#else
                CV_LOG_WARNING(NULL, "DNN module was not built with CUDA backend; switching to CPU");
#endif
                preferableBackend = DNN_BACKEND_OPENCV;
                preferableTarget = DNN_TARGET_CPU;
            }

            clear();

            this->blobsToKeep = blobsToKeep_;

            allocateLayers(blobsToKeep_);

            MapIdToLayerData::iterator it = layers.find(0);
            CV_Assert(it != layers.end());
            it->second.skip = netInputLayer->skip;

            initBackend(blobsToKeep_);

            if (!netWasAllocated)
            {
#ifdef HAVE_HALIDE
                if (preferableBackend == DNN_BACKEND_HALIDE)
                    compileHalide();
#else
                CV_Assert(preferableBackend != DNN_BACKEND_HALIDE);
#endif
            }

            netWasAllocated = true;

            if (dumpLevel)
            {
                dumpNetworkToFile();
            }
        }
    }

    int getLayerId(const String &layerName)
    {
        std::map<String, int>::iterator it = layerNameToId.find(layerName);
        return (it != layerNameToId.end()) ? it->second : -1;
    }

    int getLayerId(int id)
    {
        MapIdToLayerData::iterator it = layers.find(id);
        return (it != layers.end()) ? id : -1;
    }

    int getLayerId(DictValue &layerDesc)
    {
        if (layerDesc.isInt())
            return getLayerId(layerDesc.get<int>());
        else if (layerDesc.isString())
            return getLayerId(layerDesc.get<String>());

        CV_Assert(layerDesc.isInt() || layerDesc.isString());
        return -1;
    }

    String getLayerName(int id)
    {
        MapIdToLayerData::iterator it = layers.find(id);
        return (it != layers.end()) ? it->second.name : "(unknown layer)";
    }

    LayerData& getLayerData(int id)
    {
        MapIdToLayerData::iterator it = layers.find(id);

        if (it == layers.end())
            CV_Error(Error::StsObjectNotFound, format("Layer with requested id=%d not found", id));

        return it->second;
    }

    LayerData& getLayerData(const String &layerName)
    {
        int id = getLayerId(layerName);

        if (id < 0)
            CV_Error(Error::StsError, "Requested layer \"" + layerName + "\" not found");

        return getLayerData(id);
    }

    LayerData& getLayerData(const DictValue &layerDesc)
    {
        CV_Assert(layerDesc.isInt() || layerDesc.isString());
        if (layerDesc.isInt())
            return getLayerData(layerDesc.get<int>());
        else /*if (layerDesc.isString())*/
            return getLayerData(layerDesc.get<String>());
    }

    static void addLayerInput(LayerData &ld, int inNum, LayerPin from)
    {
        if ((int)ld.inputBlobsId.size() <= inNum)
        {
            ld.inputBlobsId.resize(inNum + 1);
        }
        else
        {
            LayerPin storedFrom = ld.inputBlobsId[inNum];
            if (storedFrom.valid() && !storedFrom.equal(from))
                CV_Error(Error::StsError, format("Input #%d of layer \"%s\" already was connected",
                                                 inNum, ld.name.c_str()));
        }

        ld.inputBlobsId[inNum] = from;
    }

    int resolvePinOutputName(LayerData &ld, const String &outName)
    {
        if (outName.empty())
            return 0;
        return ld.getLayerInstance()->outputNameToIndex(outName);
    }

    LayerPin getPinByAlias(const String &layerName)
    {
        LayerPin pin;
        pin.lid = (layerName.empty()) ? 0 : getLayerId(layerName);

        if (pin.lid >= 0)
            pin.oid = resolvePinOutputName(getLayerData(pin.lid), layerName);

        return pin;
    }

    std::vector<LayerPin> getLayerOutPins(const String &layerName)
    {
        int lid = (layerName.empty()) ? 0 : getLayerId(layerName);

        std::vector<LayerPin> pins;

        for (int i = 0; i < layers[lid].outputBlobs.size(); i++)
        {
            pins.push_back(LayerPin(lid, i));
        }

        return pins;
    }

    void connect(int outLayerId, int outNum, int inLayerId, int inNum)
    {
        CV_Assert(outLayerId < inLayerId);
        LayerData &ldOut = getLayerData(outLayerId);
        LayerData &ldInp = getLayerData(inLayerId);

        addLayerInput(ldInp, inNum, LayerPin(outLayerId, outNum));
        ldOut.requiredOutputs.insert(outNum);
        ldOut.consumers.push_back(LayerPin(inLayerId, outNum));
    }

    void initBackend(const std::vector<LayerPin>& blobsToKeep_)
    {
        CV_TRACE_FUNCTION();
        if (preferableBackend == DNN_BACKEND_OPENCV)
        {
            CV_Assert(preferableTarget == DNN_TARGET_CPU || IS_DNN_OPENCL_TARGET(preferableTarget));
        }
        else if (preferableBackend == DNN_BACKEND_HALIDE)
            initHalideBackend();
        else if (preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019)
        {
#ifdef HAVE_DNN_IE_NN_BUILDER_2019
            initInfEngineBackend(blobsToKeep_);
#else
            CV_Assert(false && "This OpenCV version is built without Inference Engine NN Builder API support");
#endif
        }
        else if (preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NGRAPH)
        {
#ifdef HAVE_DNN_NGRAPH
            initNgraphBackend(blobsToKeep_);
#else
            CV_Error(Error::StsNotImplemented, "This OpenCV version is built without support of Inference Engine + nGraph");
#endif
        }
        else if (preferableBackend == DNN_BACKEND_VKCOM)
            initVkComBackend();
        else if (preferableBackend == DNN_BACKEND_CUDA)
            initCUDABackend(blobsToKeep_);
        else
            CV_Error(Error::StsNotImplemented, "Unknown backend identifier");
    }

    void initHalideBackend()
    {
        CV_TRACE_FUNCTION();
        CV_Assert_N(preferableBackend == DNN_BACKEND_HALIDE, haveHalide());

        // Iterator to current layer.
        MapIdToLayerData::iterator it = layers.begin();
        // Iterator to base layer for fusion. In example, in case of conv+bn+relu
        // it'll be a conv layer.
        MapIdToLayerData::iterator baseIt = layers.begin();
        for (; it != layers.end(); it++)
        {
            LayerData &ldTop = it->second;
            Ptr<Layer> layerTop = ldTop.layerInstance;
            if (!layerTop->supportBackend(preferableBackend))
            {
                // Move base iterator to layer that don't support preferable
                // backend to prevent fusion over layer of different backend.
                baseIt = it;
                continue;
            }
            // Try to do layers fusion.
            LayerData &ldBot = baseIt->second;
            Ptr<Layer> layerBot = ldBot.layerInstance;
            // 1. Check that bottom and top from the same backends.
            if (it != layers.begin() && layerBot->supportBackend(preferableBackend))
            {
                // 2. Check that current layer works in-place.
                bool inPlace = ldTop.inputBlobs.size() == 1 &&
                               ldBot.outputBlobs.size() == 1 &&
                               ldTop.inputBlobs[0]->data ==
                               ldBot.outputBlobs[0].data;
                if (inPlace)
                {
                    // 3. Try to attach node.
                    CV_Assert(!ldBot.backendNodes[preferableBackend].empty());
                    Ptr<BackendNode> fusedNode =
                        layerTop->tryAttach(ldBot.backendNodes[preferableBackend]);
                    if (!fusedNode.empty())
                    {
                        ldTop.skip = true;
                        ldBot.backendNodes[preferableBackend] = fusedNode;
                        ldBot.outputBlobsWrappers = ldTop.outputBlobsWrappers;
                        continue;
                    }
                }
            }
            // No layers fusion.
            ldTop.skip = false;
            ldTop.backendNodes[DNN_BACKEND_HALIDE] =
                layerTop->initHalide(ldTop.inputBlobsWrappers);
            baseIt = it;
        }
    }

#ifdef HAVE_DNN_IE_NN_BUILDER_2019
    // Before launching Inference Engine graph we need to specify output blobs.
    // This function requests output blobs based on inputs references of
    // layers from default backend or layers from different graphs.
    void addInfEngineNetOutputs(LayerData &ld)
    {
        CV_TRACE_FUNCTION();
        Ptr<InfEngineBackendNet> layerNet;
        if (ld.backendNodes.find(preferableBackend) != ld.backendNodes.end())
        {
            Ptr<BackendNode> node = ld.backendNodes[preferableBackend];
            if (!node.empty())
            {
                Ptr<InfEngineBackendNode> ieNode = node.dynamicCast<InfEngineBackendNode>();
                CV_Assert(!ieNode.empty()); CV_Assert(!ieNode->net.empty());
                layerNet = ieNode->net;
            }
        }
        // For an every input reference we check that it belongs to one of
        // the Inference Engine backend graphs. Request an output blob if it is.
        // Do nothing if layer's input is from the same graph.
        for (int i = 0; i < ld.inputBlobsId.size(); ++i)
        {
            LayerData &inpLd = layers[ld.inputBlobsId[i].lid];
            Ptr<BackendNode> inpNode = inpLd.backendNodes[preferableBackend];
            if (!inpNode.empty())
            {
                Ptr<InfEngineBackendNode> ieInpNode = inpNode.dynamicCast<InfEngineBackendNode>();
                CV_Assert(!ieInpNode.empty()); CV_Assert(!ieInpNode->net.empty());
                if (layerNet != ieInpNode->net)
                {
                    // layerNet is empty or nodes are from different graphs.
                    ieInpNode->net->addOutput(ieInpNode->layer.getName());
                }
            }
        }
    }

    void initInfEngineBackend(const std::vector<LayerPin>& blobsToKeep_)
    {
        CV_TRACE_FUNCTION();
        CV_Assert_N(preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019, haveInfEngine());
        MapIdToLayerData::iterator it;
        Ptr<InfEngineBackendNet> net;

        for (it = layers.begin(); it != layers.end(); ++it)
        {
            LayerData &ld = it->second;
            if (ld.id == 0)
            {
                CV_Assert((netInputLayer->outNames.empty() && ld.outputBlobsWrappers.size() == 1) ||
                          (netInputLayer->outNames.size() == ld.outputBlobsWrappers.size()));
                for (int i = 0; i < ld.outputBlobsWrappers.size(); ++i)
                {
                    InferenceEngine::DataPtr dataPtr = infEngineDataNode(ld.outputBlobsWrappers[i]);
#if defined(INF_ENGINE_RELEASE) && INF_ENGINE_VER_MAJOR_LE(2019010000)
                    dataPtr->name = netInputLayer->outNames.empty() ? ld.name : netInputLayer->outNames[i];
#else
                    dataPtr->setName(netInputLayer->outNames.empty() ? ld.name : netInputLayer->outNames[i]);
#endif
                }
            }
            else
            {
                for (int i = 0; i < ld.outputBlobsWrappers.size(); ++i)
                {
                    InferenceEngine::DataPtr dataPtr = infEngineDataNode(ld.outputBlobsWrappers[i]);
#if defined(INF_ENGINE_RELEASE) && INF_ENGINE_VER_MAJOR_LE(2019010000)
                    dataPtr->name = ld.name;
#else
                    dataPtr->setName(ld.name);
#endif
                }
            }
        }

        if (skipInfEngineInit)
        {
            Ptr<BackendNode> node = layers[lastLayerId].backendNodes[preferableBackend];
            CV_Assert(!node.empty());

            Ptr<InfEngineBackendNode> ieNode = node.dynamicCast<InfEngineBackendNode>();
            CV_Assert(!ieNode.empty());
            ieNode->net->reset();

            for (it = layers.begin(); it != layers.end(); ++it)
            {
                LayerData &ld = it->second;
                if (ld.id == 0)
                {
                    for (int i = 0; i < ld.inputBlobsWrappers.size(); ++i)
                    {
                        InferenceEngine::DataPtr dataPtr = infEngineDataNode(ld.inputBlobsWrappers[i]);
#if defined(INF_ENGINE_RELEASE) && INF_ENGINE_VER_MAJOR_LE(2019010000)
                        dataPtr->name = netInputLayer->outNames[i];
#else
                        dataPtr->setName(netInputLayer->outNames[i]);
#endif
                    }
                }
                else
                {
                    for (int i = 0; i < ld.outputBlobsWrappers.size(); ++i)
                    {
                        InferenceEngine::DataPtr dataPtr = infEngineDataNode(ld.outputBlobsWrappers[i]);
#if defined(INF_ENGINE_RELEASE) && INF_ENGINE_VER_MAJOR_LE(2019010000)
                        dataPtr->name = ld.name;
#else
                        dataPtr->setName(ld.name);
#endif
                    }
                }
                ieNode->net->addBlobs(ld.inputBlobsWrappers);
                ieNode->net->addBlobs(ld.outputBlobsWrappers);
                ld.skip = true;
            }
            layers[lastLayerId].skip = false;
            ieNode->net->init((Target)preferableTarget);
            return;
        }

        // Build Inference Engine networks from sets of layers that support this
        // backend. Split a whole model on several Inference Engine networks if
        // some of layers are not implemented.

        bool supportsCPUFallback = preferableTarget == DNN_TARGET_CPU ||
                                   BackendRegistry::checkIETarget(DNN_TARGET_CPU);

        // Set of all input and output blobs wrappers for current network.
        std::map<LayerPin, Ptr<BackendWrapper> > netBlobsWrappers;
        for (it = layers.begin(); it != layers.end(); ++it)
        {
            LayerData &ld = it->second;
            if (ld.id == 0 && ld.skip)
                continue;
            bool fused = ld.skip;

            Ptr<Layer> layer = ld.layerInstance;
            if (!fused && !layer->supportBackend(preferableBackend))
            {
                bool customizable = ld.id != 0 &&
                                    INF_ENGINE_VER_MAJOR_GE(INF_ENGINE_RELEASE_2019R2) &&
                                    supportsCPUFallback;
                // TODO: there is a bug in Myriad plugin with custom layers shape infer.
                if (preferableTarget == DNN_TARGET_MYRIAD || preferableTarget == DNN_TARGET_HDDL)
                {
                    for (int i = 0; customizable && i < ld.inputBlobs.size(); ++i)
                    {
                        customizable = ld.inputBlobs[i]->size[0] == 1;
                    }
                }

                // TODO: fix these workarounds
                if (preferableTarget == DNN_TARGET_MYRIAD ||
                    preferableTarget == DNN_TARGET_HDDL ||
                    preferableTarget == DNN_TARGET_OPENCL ||
                    preferableTarget == DNN_TARGET_OPENCL_FP16)
                    customizable &= ld.type != "Concat";

                if (preferableTarget == DNN_TARGET_OPENCL ||
                    preferableTarget == DNN_TARGET_OPENCL_FP16)
                    customizable &= ld.type != "Power";

                if (preferableTarget == DNN_TARGET_OPENCL)
                    customizable &= ld.type != "Eltwise";

                if (!customizable)
                {
                    addInfEngineNetOutputs(ld);
                    net = Ptr<InfEngineBackendNet>();
                    netBlobsWrappers.clear();  // Is not used for R5 release but we don't wrap it to #ifdef.
                    layer->preferableTarget = DNN_TARGET_CPU;
                    continue;
                }
            }
            ld.skip = true;  // Initially skip all Inference Engine supported layers.

            // Create a new network if one of inputs from different Inference Engine graph.
            for (int i = 0; i < ld.inputBlobsId.size(); ++i)
            {
                LayerData &inpLd = layers[ld.inputBlobsId[i].lid];
                Ptr<BackendNode> inpNode = inpLd.backendNodes[preferableBackend];
                if (!inpNode.empty())
                {
                    Ptr<InfEngineBackendNode> ieInpNode = inpNode.dynamicCast<InfEngineBackendNode>();
                    CV_Assert(!ieInpNode.empty()); CV_Assert(!ieInpNode->net.empty());
                    if (ieInpNode->net != net)
                    {
                        net = Ptr<InfEngineBackendNet>();
                        netBlobsWrappers.clear();  // Is not used for R5 release but we don't wrap it to #ifdef.
                        break;
                    }
                }
            }

            Ptr<BackendNode> node;
            if (!net.empty())
            {
                if (fused)
                {
                    bool inPlace = ld.inputBlobsId.size() == 1 && ld.outputBlobs.size() == 1 &&
                                   ld.inputBlobs[0]->data == ld.outputBlobs[0].data;
                    CV_Assert(inPlace);
                    node = layers[ld.inputBlobsId[0].lid].backendNodes[preferableBackend];
                    ld.inputBlobsWrappers = layers[ld.inputBlobsId[0].lid].inputBlobsWrappers;
                }
            }
            else
                net = Ptr<InfEngineBackendNet>(new InfEngineBackendNet());

            if (!fused)
            {
                if (layer->supportBackend(preferableBackend))
                    node = layer->initInfEngine(ld.inputBlobsWrappers);
                else
                {
                    node = Ptr<BackendNode>(new InfEngineBackendNode(
                        ld.layerInstance, ld.inputBlobs, ld.outputBlobs, ld.internals));
                }
            }
            else if (node.empty())
                continue;

            CV_Assert(!node.empty());
            ld.backendNodes[preferableBackend] = node;

            Ptr<InfEngineBackendNode> ieNode = node.dynamicCast<InfEngineBackendNode>();
            CV_Assert(!ieNode.empty());
            ieNode->net = net;

            for (const auto& pin : blobsToKeep_)
            {
                if (pin.lid == ld.id)
                {
                    ieNode->net->addOutput(ieNode->layer.getName());
                    break;
                }
            }

            // Convert weights in FP16 for specific targets.
            if ((preferableTarget == DNN_TARGET_OPENCL_FP16 ||
                 preferableTarget == DNN_TARGET_MYRIAD ||
                 preferableTarget == DNN_TARGET_HDDL ||
                 preferableTarget == DNN_TARGET_FPGA) && !fused)
            {
#if INF_ENGINE_VER_MAJOR_GE(INF_ENGINE_RELEASE_2019R1)
                for (const std::string& name : {"weights", "biases"})
                {
                    auto it = ieNode->layer.getParameters().find(name);
                    if (it != ieNode->layer.getParameters().end())
                    {
                        InferenceEngine::Blob::Ptr bp = it->second.as<InferenceEngine::Blob::Ptr>();
                        it->second = convertFp16(std::const_pointer_cast<InferenceEngine::Blob>(bp));
                    }
                }
#else
                auto& blobs = ieNode->layer.getConstantData();
                if (blobs.empty())
                {
                    // In case of non weightable layer we have to specify
                    // it's precision adding dummy blob.
                    auto blob = InferenceEngine::make_shared_blob<int16_t>(
                                    InferenceEngine::Precision::FP16,
                                    InferenceEngine::Layout::C, {1});
                    blob->allocate();
                    blobs[""] = blob;
                }
                else
                {
                    for (auto& it : blobs)
                        it.second = convertFp16(std::const_pointer_cast<InferenceEngine::Blob>(it.second));
                }
#endif
            }

            if (!fused)
                net->addLayer(ieNode->layer);

            net->connect(ld.inputBlobsWrappers, ld.outputBlobsWrappers, ieNode->layer.getName());
            net->addBlobs(ld.inputBlobsWrappers);
            net->addBlobs(ld.outputBlobsWrappers);
            addInfEngineNetOutputs(ld);
        }

        // Initialize all networks.
        for (MapIdToLayerData::reverse_iterator it = layers.rbegin(); it != layers.rend(); ++it)
        {
            LayerData &ld = it->second;
            if (ld.backendNodes.find(preferableBackend) == ld.backendNodes.end())
                continue;

            Ptr<BackendNode> node = ld.backendNodes[preferableBackend];
            if (node.empty())
                continue;

            Ptr<InfEngineBackendNode> ieNode = node.dynamicCast<InfEngineBackendNode>();
            if (ieNode.empty())
                continue;

            CV_Assert(!ieNode->net.empty());

            if (!ieNode->net->isInitialized())
            {
                ieNode->net->init((Target)preferableTarget);
                ld.skip = false;
            }
        }
    }
#endif  // HAVE_DNN_IE_NN_BUILDER_2019


#ifdef HAVE_DNN_NGRAPH
    void addNgraphOutputs(LayerData &ld)
    {
        CV_TRACE_FUNCTION();

        Ptr<InfEngineNgraphNet> layerNet;
        auto it = ld.backendNodes.find(preferableBackend);
        if (it != ld.backendNodes.end())
        {
            Ptr<BackendNode> node = it->second;
            if (!node.empty())
            {
                Ptr<InfEngineNgraphNode> ieNode = node.dynamicCast<InfEngineNgraphNode>();
                CV_Assert(!ieNode.empty()); CV_Assert(!ieNode->net.empty());
                layerNet = ieNode->net;
            }
        }

        for (int i = 0; i < ld.inputBlobsId.size(); ++i)
        {
            LayerData &inpLd = layers[ld.inputBlobsId[i].lid];
            Ptr<BackendNode> inpNode = inpLd.backendNodes[preferableBackend];
            if (!inpNode.empty())
            {
                Ptr<InfEngineNgraphNode> ieInpNode = inpNode.dynamicCast<InfEngineNgraphNode>();
                CV_Assert(!ieInpNode.empty()); CV_Assert(!ieInpNode->net.empty());
                if (layerNet != ieInpNode->net)
                {
                    ieInpNode->net->addOutput(ieInpNode->node->get_friendly_name());
                    ieInpNode->net->setUnconnectedNodes(ieInpNode);
                }
            }
        }
    }

    void initNgraphBackend(const std::vector<LayerPin>& blobsToKeep_)
    {
        CV_TRACE_FUNCTION();
        CV_Assert_N(preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NGRAPH, haveInfEngine());

        MapIdToLayerData::iterator it;
        Ptr<InfEngineNgraphNet> net;

        for (it = layers.begin(); it != layers.end(); ++it)
        {
            LayerData &ld = it->second;
            if (ld.id == 0)
            {
                CV_Assert((netInputLayer->outNames.empty() && ld.outputBlobsWrappers.size() == 1) ||
                          (netInputLayer->outNames.size() == ld.outputBlobsWrappers.size()));
                for (int i = 0; i < ld.outputBlobsWrappers.size(); ++i)
                {
                    InferenceEngine::DataPtr dataPtr = ngraphDataNode(ld.outputBlobsWrappers[i]);
                    std::string outputName = netInputLayer->outNames.empty() ? ld.name : netInputLayer->outNames[i];
                    outputName = ld.outputBlobsWrappers.size() > 1 ? (outputName + "." + std::to_string(i)) : outputName;
                    dataPtr->setName(outputName);
                }
            }
            else
            {
                for (int i = 0; i < ld.outputBlobsWrappers.size(); ++i)
                {
                    InferenceEngine::DataPtr dataPtr = ngraphDataNode(ld.outputBlobsWrappers[i]);
                    std::string outputName = ld.outputBlobsWrappers.size() > 1 ? (ld.name + "." + std::to_string(i)) : ld.name;
                    dataPtr->setName(outputName);
                }
            }
        }

        if (skipInfEngineInit)
        {
            Ptr<BackendNode> node = layers[lastLayerId].backendNodes[preferableBackend];
            CV_Assert(!node.empty());

            Ptr<InfEngineNgraphNode> ieNode = node.dynamicCast<InfEngineNgraphNode>();
            CV_Assert(!ieNode.empty());

            CV_Assert(ieNode->net);
            InfEngineNgraphNet& ienet = *ieNode->net;
            ienet.reset();

            for (it = layers.begin(); it != layers.end(); ++it)
            {
                LayerData &ld = it->second;
                if (ld.id == 0)
                {
                    for (int i = 0; i < ld.inputBlobsWrappers.size(); ++i)
                    {
                        InferenceEngine::DataPtr dataPtr = ngraphDataNode(ld.inputBlobsWrappers[i]);
                        dataPtr->setName(netInputLayer->outNames[i]);
                    }
                }
                else
                {
                    for (int i = 0; i < ld.outputBlobsWrappers.size(); ++i)
                    {
                        auto it = ienet.outputsDesc.find(ld.name);
                        if (it != ienet.outputsDesc.end())
                        {
                            const InferenceEngine::TensorDesc& descriptor = it->second;
                            InferenceEngine::DataPtr dataPtr = ngraphDataOutputNode(ld.outputBlobsWrappers[i], descriptor, ld.name);
                            dataPtr->setName(ld.name);
                        }
                        else
                        {
                            InferenceEngine::DataPtr dataPtr = ngraphDataNode(ld.outputBlobsWrappers[i]);
                            dataPtr->setName(ld.name);
                        }
                    }
                }
                ienet.addBlobs(ld.inputBlobsWrappers);
                ienet.addBlobs(ld.outputBlobsWrappers);
                ld.skip = true;
            }
            layers[lastLayerId].skip = false;
            ienet.init((Target)preferableTarget);
            return;
        }

        bool supportsCPUFallback = !isArmComputePlugin() && (preferableTarget == DNN_TARGET_CPU ||
                                   BackendRegistry::checkIETarget(DNN_TARGET_CPU));

        // Build Inference Engine networks from sets of layers that support this
        // backend. Split a whole model on several Inference Engine networks if
        // some of layers are not implemented.
        for (it = layers.begin(); it != layers.end(); ++it)
        {
            LayerData &ld = it->second;

            if (ld.id == 0 && ld.skip)
                continue;

            bool fused = ld.skip;
            Ptr<Layer> layer = ld.layerInstance;
            if (!fused && !layer->supportBackend(preferableBackend))
            {
                bool customizable = ld.id != 0 && supportsCPUFallback;

                // TODO: there is a bug in Myriad plugin with custom layers shape infer.
                if (preferableTarget == DNN_TARGET_MYRIAD || preferableTarget == DNN_TARGET_HDDL)
                {
                    for (int i = 0; customizable && i < ld.inputBlobs.size(); ++i)
                    {
                        customizable = ld.inputBlobs[i]->size[0] == 1;
                    }
                }

                // TODO: fix these workarounds
                if (preferableTarget == DNN_TARGET_MYRIAD ||
                    preferableTarget == DNN_TARGET_HDDL ||
                    preferableTarget == DNN_TARGET_OPENCL ||
                    preferableTarget == DNN_TARGET_OPENCL_FP16)
                    customizable &= ld.type != "Concat";

                if (preferableTarget == DNN_TARGET_OPENCL ||
                    preferableTarget == DNN_TARGET_OPENCL_FP16)
                    customizable &= ld.type != "Power";

                if (preferableTarget == DNN_TARGET_OPENCL)
                    customizable &= ld.type != "Eltwise";

                if (!customizable)
                {
                    addNgraphOutputs(ld);
                    net = Ptr<InfEngineNgraphNet>();
                    layer->preferableTarget = DNN_TARGET_CPU;

                    for (int i = 0; i < ld.inputBlobsId.size(); ++i)
                    {
                        LayerData &inpLd = layers[ld.inputBlobsId[i].lid];
                        Ptr<BackendNode> inpNode = inpLd.backendNodes[preferableBackend];
                        if (!inpNode.empty()) {
                            Ptr<InfEngineNgraphNode> ieNode = inpNode.dynamicCast<InfEngineNgraphNode>();
                            CV_Assert(!ieNode.empty());
                            ieNode->net->setUnconnectedNodes(ieNode);
                        }
                    }
                    continue;
                }
            }
            ld.skip = true;  // Initially skip all Inference Engine supported layers.

            // Create a new network if one of inputs from different Inference Engine graph.
            std::vector<Ptr<BackendNode>> inputNodes;
            for (int i = 0; i < ld.inputBlobsId.size(); ++i)
            {
                // Layer_Test_ROIPooling.Accuracy has 2 inputs inpLD = 0, 0 -> has 4 inputNodes (input, rois, input, rois)
                if (inputNodes.size() == ld.inputBlobsId.size()) {
                    break;
                }
                LayerData &inpLd = layers[ld.inputBlobsId[i].lid];
                Ptr<BackendNode> inpNode = inpLd.backendNodes[preferableBackend];
                if (!inpNode.empty())
                {
                     Ptr<InfEngineNgraphNode> ieInpNode = inpNode.dynamicCast<InfEngineNgraphNode>();
                     CV_Assert(!ieInpNode.empty()); CV_Assert(!ieInpNode->net.empty());
                     if (ieInpNode->net == net && !fused) {
                        inputNodes.push_back(inpNode);
                        continue;
                     }
                }

                if (net.empty()) {
                    net = Ptr<InfEngineNgraphNet>(new InfEngineNgraphNet(*this));
                }

                if (!fused) {
                    std::vector<std::string> inputNames;
                    std::vector<cv::Mat> inputs;

                    auto curr_pos = inpLd.consumers.begin();
                    auto compare = [&ld] (const LayerPin& lp) { return lp.lid == ld.id; };
                    auto cons = curr_pos;
                    while ((cons = std::find_if(curr_pos, inpLd.consumers.end(), compare)) !=
                            inpLd.consumers.end()) {
                        int cons_inp = cons->oid;
                        Ptr<NgraphBackendWrapper> inpWrapper = inpLd.outputBlobsWrappers[cons_inp].
                                                                     dynamicCast<NgraphBackendWrapper>();
                        CV_Assert(!inpWrapper.empty());
                        auto iter = std::find(inputNames.begin(), inputNames.end(),
                                              inpWrapper->dataPtr->getName());
                        if (iter == inputNames.end()) {
                            inputNames.push_back(inpWrapper->dataPtr->getName());
                            inputs.push_back(inpLd.outputBlobs[cons_inp]);
                        }
                        curr_pos = cons + 1;
                    }

                    auto inps = net->setInputs(inputs, inputNames);
                    for (auto& inp : inps) {
                        inputNodes.emplace_back(Ptr<BackendNode>(new InfEngineNgraphNode(inp)));
                    }
                }
            }

            Ptr<BackendNode> node;
            if (!net.empty())
            {
                if (fused)
                {
                    bool inPlace = ld.inputBlobsId.size() == 1 && ld.outputBlobs.size() == 1 &&
                                   ld.inputBlobs[0]->data == ld.outputBlobs[0].data;
                    CV_Assert(inPlace);
                    node = layers[ld.inputBlobsId[0].lid].backendNodes[preferableBackend];
                    ld.inputBlobsWrappers = layers[ld.inputBlobsId[0].lid].inputBlobsWrappers;
                }
            }
            else {
                net = Ptr<InfEngineNgraphNet>(new InfEngineNgraphNet(*this));
            }

            if (!fused)
            {
                CV_Assert(ld.inputBlobsId.size() == inputNodes.size());
                for (int i = 0; i < ld.inputBlobsId.size(); ++i)
                {
                    int lid = ld.inputBlobsId[i].lid;
                    int oid = ld.inputBlobsId[i].oid;
                    if (oid == 0 || lid == 0)
                        continue;

                    auto ieInpNode = inputNodes[i].dynamicCast<InfEngineNgraphNode>();
                    CV_Assert(oid < ieInpNode->node->get_output_size());
#if INF_ENGINE_VER_MAJOR_GT(INF_ENGINE_RELEASE_2020_4)
                    inputNodes[i] = Ptr<BackendNode>(new InfEngineNgraphNode(ieInpNode->node));
#elif INF_ENGINE_VER_MAJOR_GT(INF_ENGINE_RELEASE_2020_3)
                    inputNodes[i] = Ptr<BackendNode>(new InfEngineNgraphNode(ieInpNode->node->get_output_as_single_output_node(oid)));
#else
                    inputNodes[i] = Ptr<BackendNode>(new InfEngineNgraphNode(ieInpNode->node->get_output_as_single_output_node(oid, false)));
#endif
                }

                if (layer->supportBackend(preferableBackend))
                {
                    node = layer->initNgraph(ld.inputBlobsWrappers, inputNodes);
                    for (int i = 0; i < ld.outputBlobsWrappers.size(); ++i)
                    {
                        InferenceEngine::DataPtr dataPtr = ngraphDataNode(ld.outputBlobsWrappers[i]);
                        node.dynamicCast<InfEngineNgraphNode>()->setName(dataPtr->getName());
                    }
                }
                else
                {
                    node = Ptr<BackendNode>(new InfEngineNgraphNode(inputNodes,
                        ld.layerInstance, ld.inputBlobs, ld.outputBlobs, ld.internals));
                }
            }
            else if (node.empty())
                continue;

            ld.backendNodes[preferableBackend] = node;

            Ptr<InfEngineNgraphNode> ieNode = node.dynamicCast<InfEngineNgraphNode>();
            CV_Assert(!ieNode.empty());
            ieNode->net = net;

            if (ld.consumers.empty()) {
                // TF EAST_text_detection
                ieNode->net->setUnconnectedNodes(ieNode);
            }
            for (const auto& pin : blobsToKeep_)
            {
                if (pin.lid == ld.id)
                {
                    ieNode->net->addOutput(ieNode->node->get_friendly_name());
                    break;
                }
            }
            ieNode->net->setNodePtr(&ieNode->node);

            net->addBlobs(ld.inputBlobsWrappers);
            net->addBlobs(ld.outputBlobsWrappers);
            addNgraphOutputs(ld);
        }

        // Initialize all networks.
        for (MapIdToLayerData::reverse_iterator it = layers.rbegin(); it != layers.rend(); ++it)
        {
            LayerData &ld = it->second;
            auto iter = ld.backendNodes.find(preferableBackend);
            if (iter == ld.backendNodes.end())
                continue;

            Ptr<BackendNode>& node = iter->second;
            if (node.empty())
                continue;

            Ptr<InfEngineNgraphNode> ieNode = node.dynamicCast<InfEngineNgraphNode>();
            if (ieNode.empty())
                continue;

            CV_Assert(!ieNode->net.empty());

            if (!ieNode->net->isInitialized())
            {
                ieNode->net->setUnconnectedNodes(ieNode);
                ieNode->net->createNet((Target)preferableTarget);
                ld.skip = false;
            }
        }
    }
#endif  // HAVE_DNN_NGRAPH

    void initVkComBackend()
    {
        CV_TRACE_FUNCTION();
        CV_Assert(preferableBackend == DNN_BACKEND_VKCOM);
#ifdef HAVE_VULKAN
        if (!haveVulkan())
            return;

        MapIdToLayerData::iterator it = layers.begin();
        for (; it != layers.end(); it++)
        {
            LayerData &ld = it->second;
            Ptr<Layer> layer = ld.layerInstance;
            if (!layer->supportBackend(preferableBackend))
            {
                continue;
            }

            ld.skip = false;

            try
            {
                ld.backendNodes[DNN_BACKEND_VKCOM] =
                    layer->initVkCom(ld.inputBlobsWrappers);
            }
            catch (const cv::Exception& e)
            {
                CV_LOG_ERROR(NULL, "initVkCom failed, fallback to CPU implementation. " << e.what());
                ld.backendNodes[DNN_BACKEND_VKCOM] = Ptr<BackendNode>();
            }
        }
#endif
    }

    void initCUDABackend(const std::vector<LayerPin>& blobsToKeep_)
    {
        CV_Assert(haveCUDA());
        CV_Assert(preferableBackend == DNN_BACKEND_CUDA);

#ifdef HAVE_CUDA
        if (!cudaInfo) /* we need to check only once */
            cuda4dnn::checkVersions();

        if (cuda4dnn::getDeviceCount() <= 0)
            CV_Error(Error::StsError, "No CUDA capable device found.");

        if (cuda4dnn::getDevice() < 0)
            CV_Error(Error::StsError, "No CUDA capable device selected.");

        if (!cuda4dnn::isDeviceCompatible())
            CV_Error(Error::GpuNotSupported, "OpenCV was not built to work with the selected device. Please check CUDA_ARCH_PTX or CUDA_ARCH_BIN in your build configuration.");

        if (preferableTarget == DNN_TARGET_CUDA_FP16 && !cuda4dnn::doesDeviceSupportFP16())
        {
            CV_LOG_WARNING(NULL, "The selected CUDA device does not support FP16 target; switching to FP32 target.");
            preferableTarget = DNN_TARGET_CUDA;
        }

        if (!cudaInfo)
        {
            cuda4dnn::csl::CSLContext context;
            context.stream = cuda4dnn::csl::Stream(true);
            context.cublas_handle = cuda4dnn::csl::cublas::Handle(context.stream);
            context.cudnn_handle = cuda4dnn::csl::cudnn::Handle(context.stream);

            auto d2h_stream = cuda4dnn::csl::Stream(true); // stream for background D2H data transfers
            cudaInfo = std::unique_ptr<CudaInfo_t>(new CudaInfo_t(std::move(context), std::move(d2h_stream)));
        }

        cudaInfo->workspace = cuda4dnn::csl::Workspace(); // release workspace memory if any

        for (auto& layer : layers)
        {
            auto& ld = layer.second;
            if (ld.id == 0)
            {
                for (auto& wrapper : ld.inputBlobsWrappers)
                {
                    auto cudaWrapper = wrapper.dynamicCast<CUDABackendWrapper>();
                    cudaWrapper->setStream(cudaInfo->context.stream, cudaInfo->d2h_stream);
                }
            }

            for (auto& wrapper : ld.outputBlobsWrappers)
            {
                auto cudaWrapper = wrapper.dynamicCast<CUDABackendWrapper>();
                cudaWrapper->setStream(cudaInfo->context.stream, cudaInfo->d2h_stream);
            }
        }

        for (auto& layer : layers)
        {
            auto& ld = layer.second;
            auto& layerInstance = ld.layerInstance;

            if (!layerInstance->supportBackend(DNN_BACKEND_CUDA))
            {
                std::ostringstream os;
                os << "CUDA backend will fallback to the CPU implementation for the layer \"" << ld.name
                   << "\" of type " << ld.type << '\n';
                CV_LOG_INFO(NULL, os.str().c_str());
                continue;
            }

            /* we make a copy so that `initCUDA` doesn't modify `cudaInfo->context` */
            auto context = cudaInfo->context;
            auto node = layerInstance->initCUDA(&context, ld.inputBlobsWrappers, ld.outputBlobsWrappers);
            ld.backendNodes[DNN_BACKEND_CUDA] = node;

            auto cudaNode = node.dynamicCast<CUDABackendNode>();
            cudaInfo->workspace.require(cudaNode->get_workspace_memory_in_bytes());
        }

        if (blobsToKeep_.size() > 1)
        {
            for (const auto& pin : blobsToKeep_)
            {
                LayerData& ld = layers[pin.lid];
                ld.cudaD2HBackgroundTransfers.push_back(pin.oid);
            }
        }
#endif
    }

    void allocateLayer(int lid, const LayersShapesMap& layersShapes)
    {
        CV_TRACE_FUNCTION();

        LayerData &ld = layers[lid];

        //already allocated
        if (ld.flag)
            return;

        size_t ninputs = ld.inputBlobsId.size();
#if 0
        printf("layer %s:", ld.name.c_str());
        for (size_t i = 0; i < ninputs; i++)
        {
            int inp_lid = ld.inputBlobsId[i].lid;
            LayerData &inp_ld = layers[inp_lid];
            int inp_outputs = (int)inp_ld.outputBlobs.size();
            std::cout << " " << inp_ld.name << "(" << inp_outputs;

            for( int j = 0; j < inp_outputs; j++ )
            {
                std::cout << (j == 0 ? ": " : ", ") << inp_ld.outputBlobs[j].size;
            }
            std::cout << ")";
        }
        printf("\n");
#endif

        //determine parent layers
        for (size_t i = 0; i < ninputs; i++)
            ld.inputLayersId.insert(ld.inputBlobsId[i].lid);

        //allocate parents
        for (set<int>::iterator i = ld.inputLayersId.begin(); i != ld.inputLayersId.end(); i++)
            allocateLayer(*i, layersShapes);

        //bind inputs
        if (ld.id == 0)  // DataLayer
        {
            ninputs = netInputLayer->inputsData.size();
            ld.inputBlobsWrappers.resize(ninputs);
            for (size_t i = 0; i < ninputs; i++)
                ld.inputBlobsWrappers[i] = wrap(netInputLayer->inputsData[i]);
        }
        else
        {
            ld.inputBlobs.resize(ninputs);
            ld.inputBlobsWrappers.resize(ninputs);
            for (size_t i = 0; i < ninputs; i++)
            {
                LayerPin from = ld.inputBlobsId[i];
                CV_Assert(from.valid());
                CV_DbgAssert(layers.count(from.lid) && (int)layers[from.lid].outputBlobs.size() > from.oid);
                ld.inputBlobs[i] = &layers[from.lid].outputBlobs[from.oid];
                ld.inputBlobsWrappers[i] = layers[from.lid].outputBlobsWrappers[from.oid];
            }
        }

        LayersShapesMap::const_iterator layerShapesIt = layersShapes.find(lid);

        CV_Assert(layerShapesIt != layersShapes.end());

        if (preferableBackend == DNN_BACKEND_OPENCV && preferableTarget == DNN_TARGET_OPENCL_FP16 && ld.dtype == CV_32F)
            ld.dtype = CV_16S;

        std::vector<LayerPin> pinsForInternalBlobs;
        blobManager.allocateBlobsForLayer(ld, layerShapesIt->second, pinsForInternalBlobs);
        ld.outputBlobsWrappers.resize(ld.outputBlobs.size());
        for (int i = 0; i < ld.outputBlobs.size(); ++i)
            ld.outputBlobsWrappers[i] = wrap(ld.outputBlobs[i]);

        /* CUDA backend has its own system for internal blobs; we don't need these */
        ld.internalBlobsWrappers.resize((preferableBackend == DNN_BACKEND_CUDA) ? 0 : ld.internals.size());
        for (int i = 0; i < ld.internalBlobsWrappers.size(); ++i)
            ld.internalBlobsWrappers[i] = wrap(ld.internals[i]);

        Ptr<Layer> layerPtr = ld.getLayerInstance();
        {
            std::vector<Mat> inps(ld.inputBlobs.size());
            for (int i = 0; i < ld.inputBlobs.size(); ++i)
            {
                inps[i] = *ld.inputBlobs[i];
            }
            layerPtr->finalize(inps, ld.outputBlobs);
            layerPtr->preferableTarget = preferableTarget;
#if 0
            std::cout << "\toutputs:";
            size_t noutputs = ld.outputBlobs.size();
            for (size_t j = 0; j < noutputs; j++)
            {
                std::cout << (j == 0 ? " " : ", ") << ld.outputBlobs[j].size;
            }
            std::cout << "\n";
#endif
        }

        // After allocation of layer, we decrease counters to it's input blobs.
        blobManager.releaseReferences(ld.inputBlobsId);
        blobManager.releaseReferences(pinsForInternalBlobs);

        ld.flag = 1;
    }

#if 0
#define printf_(args) printf args
#else
#define printf_(args)
#endif

    void fuseLayers(const std::vector<LayerPin>& blobsToKeep_)
    {
        CV_TRACE_FUNCTION();

        if(!fusion || (preferableBackend != DNN_BACKEND_OPENCV &&
                        preferableBackend != DNN_BACKEND_CUDA &&
                        preferableBackend != DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019 &&
                        preferableBackend != DNN_BACKEND_INFERENCE_ENGINE_NGRAPH))
           return;

        // scan through all the layers. If there is convolution layer followed by the activation layer,
        // we try to embed this activation into the convolution and disable separate execution of the activation
        std::set<LayerPin> pinsToKeep(blobsToKeep_.begin(),
                                      blobsToKeep_.end());
        MapIdToLayerData::iterator it;
        for (it = layers.begin(); it != layers.end(); it++)
        {
            int lid = it->first;
            LayerData& ld = layers[lid];
            if( ld.skip )
            {
                printf_(("skipped %s: %s\n", ld.layerInstance->name.c_str(), ld.layerInstance->type.c_str()));
                continue;
            }
            printf_(("analyzing %s: %s\n", ld.layerInstance->name.c_str(), ld.layerInstance->type.c_str()));

            // the optimization #1. try to fuse batch norm, scaling and/or activation layers
            // with the current layer if they follow it. Normally, the are fused with the convolution layer,
            // but some of them (like activation) may be fused with fully-connected, elemwise (+) and
            // some other layers.
            Ptr<Layer>& currLayer = ld.layerInstance;
            if( ld.consumers.size() == 1 && pinsToKeep.count(LayerPin(lid, 0)) == 0 )
            {
                LayerData* nextData = &layers[ld.consumers[0].lid];
                LayerPin lpNext(ld.consumers[0].lid, 0);
                while (nextData)
                {
                    /* we use `tryFuse` member of convolution layer to fuse eltwise later
                     * it's not intended to be fused here; hence, we stop when we encounter eltwise
                     */
                    if (preferableBackend == DNN_BACKEND_CUDA && ld.type == "Convolution" && nextData->type == "Eltwise")
                        break;
                    Ptr<Layer> nextLayer = nextData->layerInstance;
                    if (currLayer->tryFuse(nextLayer))
                    {
                        printf_(("\tfused with %s\n", nextLayer->name.c_str()));
                        nextData->skip = true;
                        ld.outputBlobs = layers[lpNext.lid].outputBlobs;
                        ld.outputBlobsWrappers = layers[lpNext.lid].outputBlobsWrappers;
                        if (nextData->consumers.size() == 1)
                        {
                            int nextLayerId = nextData->consumers[0].lid;
                            nextData = &layers[nextLayerId];
                            lpNext = LayerPin(nextLayerId, 0);
                        }
                        else
                        {
                            nextData = 0;
                            break;
                        }
                    }
                    else
                        break;
                }

                if (preferableBackend != DNN_BACKEND_OPENCV && preferableBackend != DNN_BACKEND_CUDA)
                    continue;  // Go to the next layer.

                // TODO: OpenCL target support more fusion styles.
                if ( preferableBackend == DNN_BACKEND_OPENCV && IS_DNN_OPENCL_TARGET(preferableTarget) &&
                     (!cv::ocl::useOpenCL() || (ld.layerInstance->type != "Convolution" &&
                     ld.layerInstance->type != "MVN" && ld.layerInstance->type != "Pooling" &&
                     ld.layerInstance->type != "Concat")) )
                    continue;

                if (preferableBackend == DNN_BACKEND_CUDA && IS_DNN_CUDA_TARGET(preferableTarget)
                    && ld.layerInstance->type != "Convolution"
                    && ld.layerInstance->type != "Concat")
                    continue;

                while (nextData)
                {
                    // For now, OpenCL target support fusion with activation of ReLU/ChannelsPReLU/Power/Tanh
                    if (IS_DNN_OPENCL_TARGET(preferableTarget) &&
                        nextData->type != "ReLU" &&
                        nextData->type != "ChannelsPReLU" &&
                        nextData->type != "ReLU6" &&
                        nextData->type != "TanH" &&
                        nextData->type != "Power")
                        break;

                    Ptr<ActivationLayer> nextActivLayer = nextData->layerInstance.dynamicCast<ActivationLayer>();
                    if (nextActivLayer.empty())
                        break;

                    if (currLayer->setActivation(nextActivLayer))
                    {
                        printf_(("\tfused with %s\n", nextActivLayer->name.c_str()));
                        nextData->skip = true;
                        ld.outputBlobs = layers[lpNext.lid].outputBlobs;
                        ld.outputBlobsWrappers = layers[lpNext.lid].outputBlobsWrappers;
                        if (nextData->consumers.size() == 1)
                        {
                            int nextLayerId = nextData->consumers[0].lid;
                            nextData = &layers[nextLayerId];
                            lpNext = LayerPin(nextLayerId, 0);
                        }
                        else
                        {
                            nextData = 0;
                            break;
                        }
                    }
                    else
                        break;
                }

                // OpenCL: fuse convolution layer followed by eltwise + relu
                // CUDA: fuse convolution layer followed by eltwise (and optional activation)
                while (nextData &&
                    (IS_DNN_OPENCL_TARGET(preferableTarget) || IS_DNN_CUDA_TARGET(preferableTarget)) &&
                    ld.layerInstance->type == "Convolution"
                )  // semantic of 'if'
                {
                    Ptr<EltwiseLayer> nextEltwiseLayer = nextData->layerInstance.dynamicCast<EltwiseLayer>();
                    if (nextEltwiseLayer.empty())
                        break;

#ifdef HAVE_CUDA
                    // CUDA backend supports fusion with eltwise sum (without variable channels)
                    if (IS_DNN_CUDA_TARGET(preferableTarget) && !nextEltwiseLayer.empty())
                    {
                        // we create a temporary backend node for eltwise layer to obtain the eltwise configuration
                        cuda4dnn::csl::CSLContext context; // assume that initCUDA and EltwiseOp do not use the context during init
                        const auto node = nextData->layerInstance->initCUDA(&context, nextData->inputBlobsWrappers, nextData->outputBlobsWrappers);
                        auto eltwiseNode = node.dynamicCast<cuda4dnn::EltwiseOpBase>();

                        // broadcasting not supported in fused ops
                        auto required_shape = shape(nextData->outputBlobs[0]);
                        for (int i = 0; i < nextData->inputBlobs.size(); i++)
                        {
                            if (shape(*nextData->inputBlobs[i]) != required_shape)
                            {
                                eltwiseNode.reset();
                                break;
                            }
                        }

                        // CUDA backend uses EltwiseOp when all operands have the same number of channels; otherwise, ShortcutOp is used.
                        // Hence, a successful cast to EltwiseOp implies that the number of channels is same in all operand tensors.
                        if (eltwiseNode.empty() || eltwiseNode->op != cuda4dnn::EltwiseOpType::SUM || !eltwiseNode->coeffs.empty())
                            break;
                    }
#endif

                    if (IS_DNN_OPENCL_TARGET(preferableTarget) && pinsToKeep.count(lpNext) != 0)
                        break;
                    if (nextData->inputBlobsId.size() != 2)
                        break;

                    if (IS_DNN_OPENCL_TARGET(preferableTarget))
                    {
                        if (!nextData->params.has("operation") || toLowerCase(nextData->params.get<String>("operation")) == "sum")
                        {
                            if (nextData->params.has("coeff"))
                            {
                                DictValue paramCoeff = nextData->params.get("coeff");
                                int n = paramCoeff.size();
                                bool isCoeffOneOne = (n == 2);
                                for (int i = 0; isCoeffOneOne && i < n; i++)
                                {
                                    float c = paramCoeff.get<float>(i);
                                    isCoeffOneOne &= (c == 1.0f);
                                }
                                if (!isCoeffOneOne)
                                {
                                    CV_LOG_DEBUG(NULL, "DNN/OpenCL: fusion of 'Sum' without coeffs (or {1.0, 1.0}) is supported only");
                                    break;
                                }
                            }
                        }
                        else
                        {
                            CV_LOG_DEBUG(NULL, "DNN/OpenCL: fusion with eltwise operation is not supported: " << nextData->params.get<String>("operation"));
                            break;
                        }
                    }

                    {
                        LayerData *eltwiseData = nextData;

                        // Eltwise layer has two inputs. We need to determine which
                        // is a base convolution layer and which could be used as it's bias.
                        LayerData* biasLayerData = 0;
                        for (int i = 0; i < 2; ++i)
                        {
                            LayerData *downLayerData = &layers[eltwiseData->inputBlobsId[i].lid];
                            CV_Assert(downLayerData);
                            while (downLayerData->skip)
                            {
                                if (downLayerData->inputBlobsId.size() == 1)
                                    downLayerData = &layers[downLayerData->inputBlobsId[0].lid];
                                else
                                {
                                    downLayerData = 0;
                                    break;
                                }
                            }
                            if (downLayerData && ld.id == downLayerData->id)
                            {
                                biasLayerData = &layers[eltwiseData->inputBlobsId[1 - i].lid];
                                break;
                            }
                        }
                        CV_Assert(biasLayerData);
                        {
                            // fuse eltwise + activation layer
                            // bias must already be computed to fuse => bias layer must appear before convolution
                            if (biasLayerData->id < ld.id)
                            {
                                /* we can fuse activation if:
                                 * => activation layer that follows is the only consumer of eltwise output
                                 * => activation layer does not process multiple inputs
                                 * => we do not require to keep the output of eltwise
                                 */
                                Ptr<ActivationLayer> nextFusabeleActivLayer;
                                if (eltwiseData->consumers.size() == 1 && pinsToKeep.count(lpNext) == 0)
                                {
                                    nextData = &layers[eltwiseData->consumers[0].lid];
                                    lpNext = LayerPin(eltwiseData->consumers[0].lid, 0);
                                    CV_Assert(nextData);
                                    if (nextData->outputBlobs.size() == 1)
                                        nextFusabeleActivLayer = nextData->layerInstance.dynamicCast<ActivationLayer>();
                                }
                                else
                                {
                                    // OCL backend cannot fuse in this case but the CUDA backend can continue with just eltwise
                                    nextData = 0;
                                }

                                // the requirements of OCV OpenCL backend and CUDA backend are different
                                // we need to check them separately; hence, the fuse variables
                                bool fuse_eltwise = false, fuse_activation = false;

                                Ptr<PowerLayer> activ_power;
                                if (IS_DNN_OPENCL_TARGET(preferableTarget) && !nextFusabeleActivLayer.empty() &&
                                    nextData &&
                                    (!nextData->type.compare("ReLU") ||
                                     !nextData->type.compare("ChannelsPReLU") ||
                                     (!nextData->type.compare("Power") && (activ_power = nextFusabeleActivLayer.dynamicCast<PowerLayer>()) && activ_power->scale == 1.0f)
                                    ) &&
                                    currLayer->setActivation(nextFusabeleActivLayer))
                                {
                                    fuse_eltwise = true;
                                    fuse_activation = true;
                                }

                                if (IS_DNN_CUDA_TARGET(preferableTarget))
                                {
                                    /* supported fusion options:
                                     * => convolution + eltwise
                                     * => activation(convolution) + eltwise
                                     *    > convolution + activation would have been fused already; we have to fuse eltwise
                                     * => activation(convolution + eltwise)
                                     *    > fuse eltwise and then activation
                                     */
                                    auto layer = nextEltwiseLayer.staticCast<Layer>();
                                    if (currLayer->tryFuse(layer))
                                    {
                                        fuse_eltwise = true; /* eltwise was successfully fused */
                                        if (!nextFusabeleActivLayer.empty() && nextData)
                                        {
                                            if ((!nextData->type.compare("ReLU") ||
                                                 !nextData->type.compare("ReLU6") ||
                                                 !nextData->type.compare("Power") ||
                                                 !nextData->type.compare("TanH") ||
                                                 !nextData->type.compare("Sigmoid") ||
                                                 !nextData->type.compare("Swish") ||
                                                 !nextData->type.compare("Mish")) &&
                                                currLayer->setActivation(nextFusabeleActivLayer))
                                            {
                                                // activation was fused
                                                fuse_activation = true;
                                            }
                                        }
                                    }
                                }

                                CV_Assert(!fuse_activation || fuse_eltwise); /* cannot fuse activation without eltwise */
                                if(fuse_eltwise && fuse_activation)
                                {
                                    CV_Assert(nextData);
                                    CV_Assert_N(biasLayerData->outputBlobsWrappers.size() == 1, ld.inputBlobsWrappers.size() == 1);
                                    ld.inputBlobsWrappers.push_back(biasLayerData->outputBlobsWrappers[0]);
                                    printf_(("\tfused with %s\n", nextEltwiseLayer->name.c_str()));
                                    printf_(("\tfused with %s\n", nextFusabeleActivLayer->name.c_str()));
                                    eltwiseData->skip = true;
                                    nextData->skip = true;
                                    // This optimization for cases like
                                    // some_layer   conv
                                    //   |             |
                                    //   +-- eltwise --+
                                    //          |
                                    //        activ
                                    // This way all the element-wise computations
                                    // (i.e. some_layer+conv or some_layer*conv)
                                    // would be done at [conv] layer. So we need to
                                    // replace [conv]'s output blob to [eltwise]'s one
                                    // considering that [activ] is an in-place layer.
                                    // Also we need to move all the consumers' references.
                                    // To prevent memory collisions (i.e. when input of
                                    // [conv] and output of [eltwise] is the same blob)
                                    // we allocate a new blob.
                                    CV_Assert_N(ld.outputBlobs.size() == 1, ld.outputBlobsWrappers.size() == 1);
                                    ld.outputBlobs[0] = ld.outputBlobs[0].clone();
                                    ld.outputBlobsWrappers[0] = wrap(ld.outputBlobs[0]);

                                    eltwiseData->outputBlobs = ld.outputBlobs;
                                    nextData->outputBlobs = ld.outputBlobs;
                                    eltwiseData->outputBlobsWrappers = ld.outputBlobsWrappers;
                                    nextData->outputBlobsWrappers = ld.outputBlobsWrappers;

                                    // Move references of [activ] layer consumers to the newly allocated blob.
                                    for (int i = 0; i < nextData->consumers.size(); ++i)
                                    {
                                        LayerData& consumer = layers[nextData->consumers[i].lid];
                                        for (int j = 0; j < consumer.inputBlobsId.size(); ++j)
                                        {
                                            if (consumer.inputBlobsId[j].lid == lpNext.lid)
                                            {
                                                consumer.inputBlobs[j] = &ld.outputBlobs[0];
                                                consumer.inputBlobsWrappers[j] = ld.outputBlobsWrappers[0];
                                                break;
                                            }
                                        }
                                    }
                                }
                                else if (fuse_eltwise) // conv + eltwise (note: conv could have fused activations before eltwise)
                                {
                                    CV_Assert(IS_DNN_CUDA_TARGET(preferableTarget));
                                    CV_Assert_N(biasLayerData->outputBlobsWrappers.size() == 1, ld.inputBlobsWrappers.size() == 1);
                                    ld.inputBlobsWrappers.push_back(biasLayerData->outputBlobsWrappers[0]);
                                    printf_(("\tfused with %s\n", nextEltwiseLayer->name.c_str()));
                                    eltwiseData->skip = true;
                                    // This optimization is for cases like
                                    // some_layer   conv (maybe fused with activ)
                                    //   |             |
                                    //   +-- eltwise --+
                                    //
                                    // This way all the element-wise computations
                                    // (i.e. some_layer+conv or some_layer*conv)
                                    // would be done at [conv] layer. So we need to
                                    // replace [conv]'s output blob to [eltwise]'s one.
                                    // Also we need to move all the consumers' references.
                                    // To prevent memory collisions (i.e. when input of
                                    // [conv] and output of [eltwise] is the same blob)
                                    // we allocate a new blob.
                                    CV_Assert_N(ld.outputBlobs.size() == 1, ld.outputBlobsWrappers.size() == 1);
                                    ld.outputBlobs[0] = ld.outputBlobs[0].clone();
                                    ld.outputBlobsWrappers[0] = wrap(ld.outputBlobs[0]);

                                    eltwiseData->outputBlobs = ld.outputBlobs;
                                    eltwiseData->outputBlobsWrappers = ld.outputBlobsWrappers;

                                    // Move references of [eltwise] layer consumers to the newly allocated blob.
                                    for (int i = 0; i < eltwiseData->consumers.size(); ++i)
                                    {
                                        LayerData& consumer = layers[eltwiseData->consumers[i].lid];
                                        for (int j = 0; j < consumer.inputBlobsId.size(); ++j)
                                        {
                                            if (consumer.inputBlobsId[j].lid == eltwiseData->id)
                                            {
                                                consumer.inputBlobs[j] = &ld.outputBlobs[0];
                                                consumer.inputBlobsWrappers[j] = ld.outputBlobsWrappers[0];
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    break;
                }
            }

            if (preferableBackend != DNN_BACKEND_OPENCV && preferableBackend != DNN_BACKEND_CUDA)
                continue;  // Go to the next layer.

            // the optimization #2. if there is concat layer that concatenates channels
            // from the inputs together (i.e. axis == 1) then we make the inputs of
            // the concat layer to write to the concatenation output buffer
            // (and so we eliminate the concatenation layer, because the channels
            // are concatenated implicitly).
            Ptr<ConcatLayer> concatLayer = ld.layerInstance.dynamicCast<ConcatLayer>();
            if( !concatLayer.empty() && !concatLayer->padding && ld.outputBlobs.size() == 1 )
            {
                Mat& output = ld.outputBlobs[0];
                UMat umat_output;
#ifdef HAVE_OPENCL
                if (!ld.outputBlobsWrappers.empty() &&
                    (preferableBackend == DNN_BACKEND_OPENCV && IS_DNN_OPENCL_TARGET(preferableTarget)))
                {
                    size_t i, ninputs = ld.inputBlobsId.size();
                    bool conv_layer = true;
                    for( i = 0; i < ninputs; i++ )
                    {
                        LayerPin pin = ld.inputBlobsId[i];
                        LayerData* inp_i_data = &layers[pin.lid];
                        while(inp_i_data->skip &&
                              inp_i_data->inputBlobsId.size() == 1 &&
                              inp_i_data->consumers.size() == 1)
                        {
                            pin = inp_i_data->inputBlobsId[0];
                            inp_i_data = &layers[pin.lid];
                        }
                        conv_layer = conv_layer && (inp_i_data->getLayerInstance()->type == "Convolution");
                    }
                    if (!conv_layer)
                        continue;
                    std::vector<UMat> umat_outputBlobs;
                    umat_outputBlobs = OpenCLBackendWrapper::getUMatVector(ld.outputBlobsWrappers);
                    umat_output = umat_outputBlobs[0];
                }
#endif

                // TODO: in general, this optimization can always be done, but
                // many layers currently check that the input/output blobs are
                // continuous arrays. Unfortunately, this is not true when
                // the concatenation optimization is applied with batch_size > 1.
                // so, for now, we only apply this optimization in the most popular
                // case batch_size == 1.
                int axis = normalize_axis(concatLayer->axis, output.dims);
                if( output.total(0, axis) == 1 )
                {
                    size_t i, ninputs = ld.inputBlobsId.size();
                    std::vector<LayerPin> realinputs(ninputs);
                    for( i = 0; i < ninputs; i++ )
                    {
                        LayerPin pin = ld.inputBlobsId[i];
                        LayerData* inp_i_data = &layers[pin.lid];
                        while(inp_i_data->skip &&
                              inp_i_data->inputBlobsId.size() == 1 &&
                              inp_i_data->consumers.size() == 1)
                        {
                            pin = inp_i_data->inputBlobsId[0];
                            inp_i_data = &layers[pin.lid];
                        }
                        printf_(("\treal input for %s is %s\n",
                               layers[ld.inputBlobsId[i].lid].getLayerInstance()->name.c_str(),
                               inp_i_data->getLayerInstance()->name.c_str()));

                        if(inp_i_data->skip || inp_i_data->consumers.size() != 1)
                            break;
#ifdef HAVE_CUDA
                        if (preferableBackend == DNN_BACKEND_CUDA &&
                            (inp_i_data->layerInstance->supportBackend(DNN_BACKEND_CUDA) == false ||
                             (inp_i_data->layerInstance->type != "Convolution" &&
                              inp_i_data->layerInstance->type != "Pooling" &&
                              inp_i_data->layerInstance->type != "Resize"  &&
                              inp_i_data->layerInstance->type != "Flatten" &&
                              inp_i_data->layerInstance->type != "Permute" &&
                              inp_i_data->layerInstance->type != "Reorg" &&
                              inp_i_data->layerInstance->type != "Eltwise" &&
                              inp_i_data->layerInstance.dynamicCast<ActivationLayer>().empty())))
                        {
                            break;
                        }
#endif
                        realinputs[i] = pin;
                    }

                    if( i >= ninputs )
                    {
                        // Allocate new memory to prevent collisions during memory
                        // reusing (see https://github.com/opencv/opencv/pull/10456).
                        output = output.clone();
#ifdef HAVE_OPENCL
                        if (preferableBackend == DNN_BACKEND_OPENCV &&
                            IS_DNN_OPENCL_TARGET(preferableTarget))
                        {
                            std::vector<UMat> umats(1);
                            umat_output = umat_output.clone();
                            umats[0] = umat_output;
                            OpenCLBackendWrapper::update(ld.outputBlobsWrappers, umats);
                        }
#endif

#ifdef HAVE_CUDA
                        if (preferableBackend == DNN_BACKEND_CUDA)
                            ld.outputBlobsWrappers[0] = wrap(output);
#endif
                        std::vector<Range> chrange(output.dims, Range::all());
                        int ofs = 0;
                        for( i = 0; i < ninputs; i++ )
                        {
                            LayerPin pin = realinputs[i];
                            LayerData* inp_i_data = &layers[pin.lid];
                            int channels_i = ld.inputBlobs[i]->size[axis];
                            chrange[axis] = Range(ofs, ofs + channels_i);
                            printf_(("\toutput %s(%d) to channels (%d, %d)\n", inp_i_data->layerInstance->name.c_str(),
                                   pin.oid, ofs, ofs + channels_i));
                            ofs += channels_i;
                            Mat output_slice = output(chrange);
                            Mat& curr_output = inp_i_data->outputBlobs[pin.oid];
                            CV_Assert(output_slice.isContinuous() && output_slice.size == curr_output.size);
                            Mat* oldPtr = &curr_output;
                            curr_output = output_slice;
#ifdef HAVE_OPENCL
                            if (preferableBackend == DNN_BACKEND_OPENCV && IS_DNN_OPENCL_TARGET(preferableTarget))
                            {
                                std::vector<UMat> umats(inp_i_data->outputBlobsWrappers.size());
                                umats[pin.oid] = umat_output(chrange);
                                OpenCLBackendWrapper::update(inp_i_data->outputBlobsWrappers, umats);
                            }
#endif
#ifdef HAVE_CUDA
                            if (preferableBackend == DNN_BACKEND_CUDA)
                            {
                                auto cuda_wrapper = wrap(output).dynamicCast<CUDABackendWrapper>();
                                auto offset = chrange[axis].start * output_slice.total(axis + 1, output.dims);
                                auto new_shape = shape(output_slice);
                                cuda_wrapper->update(new_shape, offset);
                                inp_i_data->outputBlobsWrappers[pin.oid] = cuda_wrapper.staticCast<BackendWrapper>();
                            }
#endif
                            // Layers that refer old input Mat will refer to the
                            // new data but the same Mat object.
                            CV_Assert_N(curr_output.data == output_slice.data, oldPtr == &curr_output);
                        }

#ifdef HAVE_CUDA
                        if (preferableBackend == DNN_BACKEND_CUDA)
                        {
                            for (int i = 0; i < ld.consumers.size(); i++)
                            {
                                LayerData& consumer = layers[ld.consumers[i].lid];
                                for (int j = 0; j < consumer.inputBlobsId.size(); j++)
                                {
                                    if (consumer.inputBlobsId[j].lid == ld.id)
                                    {
                                        CV_Assert(consumer.inputBlobs[j]->data == ld.outputBlobs[0].data);
                                        consumer.inputBlobsWrappers[j] = ld.outputBlobsWrappers[0];
                                        break;
                                    }
                                }
                            }
                        }
#endif
                        ld.skip = true;
                        printf_(("\toptimized out Concat layer %s\n", concatLayer->name.c_str()));
                    }
                }
            }
        }
    }

    void allocateLayers(const std::vector<LayerPin>& blobsToKeep_)
    {
        CV_TRACE_FUNCTION();

        MapIdToLayerData::iterator it;
        for (it = layers.begin(); it != layers.end(); it++)
            it->second.flag = 0;

        CV_Assert(!layers[0].outputBlobs.empty());
        ShapesVec inputShapes;
        for(int i = 0; i < layers[0].outputBlobs.size(); i++)
        {
            Mat& inp = layers[0].outputBlobs[i];
            CV_Assert(inp.total());
            if (preferableBackend == DNN_BACKEND_OPENCV &&
                preferableTarget == DNN_TARGET_OPENCL_FP16 &&
                layers[0].dtype == CV_32F)
            {
                layers[0].outputBlobs[i].create(inp.dims, inp.size, CV_16S);
            }
            inputShapes.push_back(shape(inp));
        }
        LayersShapesMap layersShapes;
        getLayersShapes(inputShapes, layersShapes);

        blobManager.reset();
        backendWrappers.clear();

        for(auto& layer : layers)
        {
            auto& ld = layer.second;
            ld.inputBlobsWrappers.clear();
            ld.outputBlobsWrappers.clear();
            ld.internalBlobsWrappers.clear();
        }

        // Fake references to input blobs.
        for (int i = 0; i < layers[0].outputBlobs.size(); ++i)
            blobManager.addReference(LayerPin(0, i));
        for (it = layers.begin(); it != layers.end(); ++it)
        {
            const LayerData& ld = it->second;
            blobManager.addReferences(ld.inputBlobsId);
        }

        for (int i = 0; i < blobsToKeep_.size(); i++)
        {
            blobManager.addReference(blobsToKeep_[i]);
        }

        for (it = layers.begin(); it != layers.end(); it++)
        {
            int lid = it->first;
            allocateLayer(lid, layersShapes);
        }

        layersTimings.resize(lastLayerId + 1, 0);
        fuseLayers(blobsToKeep_);
    }

    void forwardLayer(LayerData &ld)
    {
        CV_TRACE_FUNCTION();

        Ptr<Layer> layer = ld.layerInstance;

        if( !ld.skip )
        {
            TickMeter tm;
            tm.start();

            std::map<int, Ptr<BackendNode> >::iterator it = ld.backendNodes.find(preferableBackend);
            if (preferableBackend == DNN_BACKEND_OPENCV || it == ld.backendNodes.end() || it->second.empty())
            {
                if (isAsync)
                    CV_Error(Error::StsNotImplemented, "Default implementation fallbacks in asynchronous mode");

                if (!layer->supportBackend(DNN_BACKEND_OPENCV))
                    CV_Error(Error::StsNotImplemented, format("Layer \"%s\" of type \"%s\" unsupported on OpenCV backend",
                                                       ld.name.c_str(), ld.type.c_str()));

#ifdef HAVE_OPENCL
                if (preferableBackend == DNN_BACKEND_OPENCV && IS_DNN_OPENCL_TARGET(preferableTarget))
                {
                    std::vector<UMat> umat_inputBlobs = OpenCLBackendWrapper::getUMatVector(ld.inputBlobsWrappers);
                    std::vector<UMat> umat_outputBlobs = OpenCLBackendWrapper::getUMatVector(ld.outputBlobsWrappers);
                    std::vector<UMat> umat_internalBlobs = OpenCLBackendWrapper::getUMatVector(ld.internalBlobsWrappers);
                    layer->forward(umat_inputBlobs,
                                   umat_outputBlobs,
                                   umat_internalBlobs);
                    if (DNN_CHECK_NAN_INF)
                    {
                        bool fail = false;
                        for (size_t i = 0; i < umat_outputBlobs.size(); ++i)
                        {
                            UMat& u = umat_outputBlobs[i];
                            Mat m;
                            if (u.depth() == CV_16S) // FP16
                                convertFp16(u, m);
                            else
                                m = u.getMat(ACCESS_READ);
                            if (!checkRange(m))
                            {
                                std::cerr << "WARNING: NaN detected in layer output: id=" << ld.id << " name=" << layer->name << std::endl;
                                std::cerr << "output id=" << i << " output shape=" << shape(m) << std::endl;
                                fail = true;
                            }
                            else if (!checkRange(m, true, NULL, -1e6, 1e6))
                            {
                                std::cerr << "WARNING: Inf detected in layer output: id=" << ld.id << " name=" << layer->name << std::endl;
                                std::cerr << "output id=" << i << " output shape=" << shape(m) << std::endl;
                                fail = true;
                            }
                        }
                        if (fail)
                        {
                            for (size_t i = 0; i < umat_inputBlobs.size(); ++i)
                            {
                                UMat& u = umat_inputBlobs[i];
                                Mat m;
                                if (u.depth() == CV_16S) // FP16
                                    convertFp16(u, m);
                                else
                                    m = u.getMat(ACCESS_READ);
                                std::cout << "INPUT " << i << " " << cv::typeToString(u.type()) << " " << shape(m) << std::endl;
                                if (DNN_CHECK_NAN_INF_DUMP) std::cout << m.reshape(1, 1) << std::endl;
                            }
                            for (size_t i = 0; i < umat_outputBlobs.size(); ++i)
                            {
                                UMat& u = umat_outputBlobs[i];
                                Mat m;
                                if (u.depth() == CV_16S) // FP16
                                    convertFp16(u, m);
                                else
                                    m = u.getMat(ACCESS_READ);
                                std::cout << "OUTPUT " << i << " " << cv::typeToString(u.type()) << " " << shape(m) << std::endl;
                                if (DNN_CHECK_NAN_INF_DUMP) std::cout << m.reshape(1, 1) << std::endl;
                            }
                            for (size_t i = 0; i < umat_internalBlobs.size(); ++i)
                            {
                                UMat& u = umat_internalBlobs[i];
                                Mat m;
                                if (u.depth() == CV_16S) // FP16
                                    convertFp16(u, m);
                                else
                                    m = u.getMat(ACCESS_READ);
                                std::cout << "INTERNAL " << i << " " << shape(m) << std::endl;
                                if (DNN_CHECK_NAN_INF_DUMP) std::cout << cv::typeToString(u.type()) << " " << m.reshape(1, 1) << std::endl;
                            }
                            if (DNN_CHECK_NAN_INF_RAISE_ERROR)
                                CV_Assert(!fail);
                        }
                    }
                    OpenCLBackendWrapper::update(ld.outputBlobsWrappers, umat_outputBlobs);
                }
                else
#endif
                {
                    for (int i = 0, n = ld.inputBlobsWrappers.size(); i < n; ++i)
                    {
                        if (!ld.inputBlobsWrappers[i].empty())
                            ld.inputBlobsWrappers[i]->copyToHost();
                    }

                    std::vector<Mat> inps(ld.inputBlobs.size());
                    for (int i = 0; i < ld.inputBlobs.size(); ++i)
                    {
                        inps[i] = *ld.inputBlobs[i];
                    }
                    layer->forward(inps, ld.outputBlobs, ld.internals);

                    if (DNN_CHECK_NAN_INF)
                    {
                        bool fail = false;
                        for (size_t i = 0; i < ld.outputBlobs.size(); ++i)
                        {
                            const Mat& m = ld.outputBlobs[i];
                            if (!checkRange(m))
                            {
                                std::cerr << "WARNING: NaN detected in layer output: id=" << ld.id << " name=" << layer->name << std::endl;
                                std::cerr << "output id=" << i << " output shape=" << shape(m) << std::endl;
                                fail = true;
                            }
                            else if (!checkRange(m, true, NULL, -1e6, 1e6))
                            {
                                std::cerr << "WARNING: Inf detected in layer output: id=" << ld.id << " name=" << layer->name << std::endl;
                                std::cerr << "output id=" << i << " output shape=" << shape(m) << std::endl;
                                fail = true;
                            }
                        }
                        if (fail)
                        {
                            for (size_t i = 0; i < ld.inputBlobs.size(); ++i)
                            {
                                const Mat* pM = ld.inputBlobs[i];
                                if (!pM)
                                {
                                    std::cout << "INPUT " << i << " is NULL" << std::endl;
                                    continue;
                                }
                                const Mat& m = *pM;
                                std::cout << "INPUT " << i << " " << cv::typeToString(m.type()) << " " << shape(m) << std::endl;
                                if (DNN_CHECK_NAN_INF_DUMP) std::cout << m.reshape(1, 1) << std::endl;
                            }
                            for (size_t i = 0; i < ld.outputBlobs.size(); ++i)
                            {
                                const Mat& m = ld.outputBlobs[i];
                                std::cout << "OUTPUT " << i << " " << cv::typeToString(m.type()) << " " << shape(m) << std::endl;
                                if (DNN_CHECK_NAN_INF_DUMP) std::cout << m.reshape(1, 1) << std::endl;
                            }
                            for (size_t i = 0; i < ld.internals.size(); ++i)
                            {
                                const Mat& m = ld.internals[i];
                                std::cout << "INTERNAL " << i << " " << cv::typeToString(m.type()) << " " << shape(m) << std::endl;
                                if (DNN_CHECK_NAN_INF_DUMP) std::cout << m.reshape(1, 1) << std::endl;
                            }
                            if (DNN_CHECK_NAN_INF_RAISE_ERROR)
                                CV_Assert(!fail);
                        }
                    }

                    for (int i = 0, n = ld.outputBlobsWrappers.size(); i < n; ++i)
                    {
                        if (!ld.outputBlobsWrappers[i].empty())
                            ld.outputBlobsWrappers[i]->setHostDirty();
                    }
                }
            }
            else
            {
                Ptr<BackendNode> node = it->second;
                CV_Assert(!node.empty());
                if (preferableBackend == DNN_BACKEND_CUDA)
                {
                    CV_Assert(haveCUDA());

#ifdef HAVE_CUDA
                    Ptr<CUDABackendNode> cudaNode = node.dynamicCast<CUDABackendNode>();
                    CV_Assert(!cudaNode.empty());

                    cudaNode->forward(ld.inputBlobsWrappers, ld.outputBlobsWrappers, cudaInfo->workspace);

                    for (auto id : ld.cudaD2HBackgroundTransfers)
                    {
                        auto wrapper = ld.outputBlobsWrappers[id].dynamicCast<CUDABackendWrapper>();
                        wrapper->copyToHostInBackground();
                    }
#endif
                }
                else if (preferableBackend == DNN_BACKEND_HALIDE)
                {
                    forwardHalide(ld.outputBlobsWrappers, node);
                }
                else if (preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019)
                {
                    forwardInfEngine(ld.outputBlobsWrappers, node, isAsync);
                }
                else if (preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NGRAPH)
                {
                    forwardNgraph(ld.outputBlobsWrappers, node, isAsync);
                }
                else if (preferableBackend == DNN_BACKEND_VKCOM)
                {
                    try
                    {
                        forwardVkCom(ld.outputBlobsWrappers, node);
                    }
                    catch (const cv::Exception& e)
                    {
                        CV_LOG_ERROR(NULL, "forwardVkCom failed, fallback to CPU implementation. " << e.what());
                        it->second = Ptr<BackendNode>();
                        forwardLayer(ld);
                    }
                }
                else
                {
                    CV_Error(Error::StsNotImplemented, "Unknown backend identifier");
                }
            }

            tm.stop();
            int64 t = tm.getTimeTicks();
            layersTimings[ld.id] = (t > 0) ? t : t + 1;  // zero for skipped layers only
        }
        else
        {
            layersTimings[ld.id] = 0;
        }

        ld.flag = 1;
    }

    void forwardToLayer(LayerData &ld, bool clearFlags = true)
    {
        CV_TRACE_FUNCTION();

        if (clearFlags)
        {
            MapIdToLayerData::iterator it;
            for (it = layers.begin(); it != layers.end(); it++)
                it->second.flag = 0;
        }

        //already was forwarded
        if (ld.flag)
            return;

        //forward parents
        MapIdToLayerData::iterator it;
        for (it = layers.begin(); it != layers.end() && (it->second.id < ld.id); ++it)
        {
            LayerData &ld = it->second;
            if (ld.flag)
                continue;
            forwardLayer(ld);
        }

        //forward itself
        forwardLayer(ld);

#ifdef HAVE_CUDA
        if (preferableBackend == DNN_BACKEND_CUDA)
            cudaInfo->context.stream.synchronize();
#endif
    }

    void getQuantizationParams(const Mat& src, std::vector<float>& scales, std::vector<int>& zeropoints)
    {
        const int qmin = -128; // INT8_MIN
        const int qmax = 127;  // INT8_MAX

        double rmin, rmax, sc, zp;
        cv::minMaxIdx(src, &rmin, &rmax);

        // 0 must be present in the range [rmin, rmax]
        rmin = std::min(rmin, 0.0);
        rmax = std::max(rmax, 0.0);

        sc = (rmax == rmin) ? 1.0 : (rmax - rmin)/(qmax - qmin);
        zp = qmin - (rmin/sc);

        scales.push_back((float)sc);
        zeropoints.push_back((int)std::round(zp));
    }

    void getLayerShapesRecursively(int id, LayersShapesMap& inOutShapes)
    {
        std::vector<LayerPin>& inputLayerIds = layers[id].inputBlobsId;

        if (id == 0 && inOutShapes[id].in[0].empty())
        {
            if (!layers[0].outputBlobs.empty())
            {
                ShapesVec shapes;
                for (int i = 0; i < layers[0].outputBlobs.size(); i++)
                {
                    Mat& inp = layers[0].outputBlobs[i];
                    CV_Assert(inp.total());
                    shapes.push_back(shape(inp));
                }
                inOutShapes[0].in = shapes;
            }
            else
            {
                const std::vector<MatShape>& inputShapes = netInputLayer->shapes;
                bool none = true;
                for (size_t i = 0; i < inputShapes.size(); i++)
                {
                    if (!inputShapes[i].empty())
                    {
                        none = false;
                        break;
                    }
                }
                if (none)
                {
                    inOutShapes[0].out.clear();
                    return;
                }
                else
                {
                    inOutShapes[0].in = inputShapes;
                }
            }
        }

        if (inOutShapes[id].in.empty())
        {
            for(int i = 0; i < inputLayerIds.size(); i++)
            {
                int layerId = inputLayerIds[i].lid;
                LayersShapesMap::iterator it =
                        inOutShapes.find(layerId);
                if(it == inOutShapes.end() ||
                        it->second.out.empty())
                {
                    getLayerShapesRecursively(layerId, inOutShapes);
                }
                const MatShape& shape = inOutShapes[layerId].out[inputLayerIds[i].oid];
                inOutShapes[id].in.push_back(shape);
            }
        }
        const ShapesVec& is = inOutShapes[id].in;
        ShapesVec& os = inOutShapes[id].out;
        ShapesVec& ints = inOutShapes[id].internal;
        int requiredOutputs = layers[id].requiredOutputs.size();
        Ptr<Layer> l = layers[id].getLayerInstance();
        CV_Assert(l);
        bool layerSupportInPlace = false;
        try
        {
            layerSupportInPlace = l->getMemoryShapes(is, requiredOutputs, os, ints);
        }
        catch (const cv::Exception& e)
        {
            CV_LOG_ERROR(NULL, "OPENCV/DNN: [" << l->type << "]:(" << l->name << "): getMemoryShapes() throws exception." <<
                    " inputs=" << is.size() <<
                    " outputs=" << os.size() << "/" << requiredOutputs <<
                    " blobs=" << l->blobs.size());
            for (size_t i = 0; i < is.size(); ++i)
            {
                CV_LOG_ERROR(NULL, "    input[" << i << "] = " << toString(is[i]));
            }
            for (size_t i = 0; i < os.size(); ++i)
            {
                CV_LOG_ERROR(NULL, "    output[" << i << "] = " << toString(os[i]));
            }
            for (size_t i = 0; i < l->blobs.size(); ++i)
            {
                CV_LOG_ERROR(NULL, "    blobs[" << i << "] = " << typeToString(l->blobs[i].type()) << " " << toString(shape(l->blobs[i])));
            }
            CV_LOG_ERROR(NULL, "Exception message: " << e.what());
            throw;
        }
        inOutShapes[id].supportInPlace = layerSupportInPlace;

        for (int i = 0; i < ints.size(); i++)
            CV_Assert(total(ints[i]) > 0);

        for (int i = 0; i < os.size(); i++)
            CV_Assert(total(os[i]) > 0);
    }

    void getLayersShapes(const ShapesVec& netInputShapes,
                         LayersShapesMap& inOutShapes)
    {
        inOutShapes.clear();

        inOutShapes[0].in = netInputShapes; //insert shape for first input layer
        for (MapIdToLayerData::iterator it = layers.begin();
             it != layers.end(); it++)
        {
            getLayerShapesRecursively(it->first, inOutShapes);
        }
    }

    void getLayerShapes(const ShapesVec& netInputShapes,
                        const int layerId,
                        LayerShapes& shapes)
    {
        LayersShapesMap inOutShapes;
        inOutShapes[0].in = netInputShapes; //insert shape for first input layer
        getLayerShapesRecursively(layerId, inOutShapes);
        shapes = inOutShapes[layerId];
    }

    void updateLayersShapes()
    {
        CV_Assert(!layers[0].outputBlobs.empty());
        ShapesVec inputShapes;
        for(int i = 0; i < layers[0].outputBlobs.size(); i++)
        {
            Mat& inp = layers[0].outputBlobs[i];
            CV_Assert(inp.total());
            if (preferableBackend == DNN_BACKEND_OPENCV &&
                preferableTarget == DNN_TARGET_OPENCL_FP16 &&
                layers[0].dtype == CV_32F)
            {
                layers[0].outputBlobs[i].create(inp.dims, inp.size, CV_16S);
            }
            inputShapes.push_back(shape(inp));
        }
        LayersShapesMap layersShapes;
        layersShapes[0].in = inputShapes;
        for (MapIdToLayerData::iterator it = layers.begin();
             it != layers.end(); it++)
        {
            int layerId = it->first;
            std::vector<LayerPin>& inputLayerIds = it->second.inputBlobsId;
            if (layersShapes[layerId].in.empty())
            {
                for(int i = 0; i < inputLayerIds.size(); i++)
                {
                    int inputLayerId = inputLayerIds[i].lid;
                    LayersShapesMap::iterator inputIt = layersShapes.find(inputLayerId);
                    if(inputIt == layersShapes.end() || inputIt->second.out.empty())
                    {
                        getLayerShapesRecursively(inputLayerId, layersShapes);
                    }
                    const MatShape& shape = layersShapes[inputLayerId].out[inputLayerIds[i].oid];
                    layersShapes[layerId].in.push_back(shape);
                }
                it->second.getLayerInstance()->updateMemoryShapes(layersShapes[layerId].in);
            }
        }
    }

    LayerPin getLatestLayerPin(const std::vector<LayerPin>& pins)
    {
        return *std::max_element(pins.begin(), pins.end());
    }

    Mat getBlob(const LayerPin& pin)
    {
        CV_TRACE_FUNCTION();

        if (!pin.valid())
            CV_Error(Error::StsObjectNotFound, "Requested blob not found");

        LayerData &ld = layers[pin.lid];
        if ((size_t)pin.oid >= ld.outputBlobs.size())
        {
            CV_Error(Error::StsOutOfRange, format("Layer \"%s\" produce only %zu outputs, "
                                           "the #%d was requested", ld.name.c_str(),
                                           ld.outputBlobs.size(), pin.oid));
        }
        if (preferableTarget != DNN_TARGET_CPU)
        {
            CV_Assert(!ld.outputBlobsWrappers.empty() && !ld.outputBlobsWrappers[pin.oid].empty());
            // Transfer data to CPU if it's require.
            ld.outputBlobsWrappers[pin.oid]->copyToHost();
        }

        if (ld.outputBlobs[pin.oid].depth() == CV_16S)
        {
            convertFp16(ld.outputBlobs[pin.oid], output_blob);
            return output_blob;
        }
        else
            return ld.outputBlobs[pin.oid];
    }

    Mat getBlob(String outputName)
    {
        return getBlob(getPinByAlias(outputName));
    }

#ifdef CV_CXX11
    AsyncArray getBlobAsync(const LayerPin& pin)
    {
        CV_TRACE_FUNCTION();
#ifdef HAVE_INF_ENGINE
        if (!pin.valid())
            CV_Error(Error::StsObjectNotFound, "Requested blob not found");

        LayerData &ld = layers[pin.lid];
        if ((size_t)pin.oid >= ld.outputBlobs.size())
        {
            CV_Error(Error::StsOutOfRange, format("Layer \"%s\" produce only %d outputs, "
                                           "the #%d was requested", ld.name.c_str(),
                                           (int)ld.outputBlobs.size(), (int)pin.oid));
        }
        if (preferableTarget != DNN_TARGET_CPU)
        {
            CV_Assert(!ld.outputBlobsWrappers.empty() && !ld.outputBlobsWrappers[pin.oid].empty());
            // Transfer data to CPU if it's require.
            ld.outputBlobsWrappers[pin.oid]->copyToHost();
        }
        CV_Assert(preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019 || preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NGRAPH);

        if (preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019) {
#ifdef HAVE_DNN_IE_NN_BUILDER_2019
            Ptr<InfEngineBackendWrapper> wrapper = ld.outputBlobsWrappers[pin.oid].dynamicCast<InfEngineBackendWrapper>();
            return std::move(wrapper->futureMat);
#else
            CV_Error(Error::StsNotImplemented, "This OpenCV version is built without Inference Engine NN Builder API support");
#endif
        }
        else if (preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NGRAPH)
        {
#ifdef HAVE_DNN_NGRAPH
            Ptr<NgraphBackendWrapper> wrapper = ld.outputBlobsWrappers[pin.oid].dynamicCast<NgraphBackendWrapper>();
            return std::move(wrapper->futureMat);
#else
            CV_Error(Error::StsNotImplemented, "This OpenCV version is built without support of Inference Engine + nGraph");
#endif
        }
#endif  // HAVE_INF_ENGINE
        CV_Error(Error::StsNotImplemented, "DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019 backend is required");
    }

    AsyncArray getBlobAsync(String outputName)
    {
        return getBlobAsync(getPinByAlias(outputName));
    }
#endif  // CV_CXX11

#ifdef HAVE_INF_ENGINE
    static
    Net createNetworkFromModelOptimizer(InferenceEngine::CNNNetwork& ieNet);
#endif

    string dump();

    void dumpNetworkToFile()
    {
#ifndef OPENCV_DNN_DISABLE_NETWORK_AUTO_DUMP
        string dumpFileNameBase = getDumpFileNameBase();
        string dumpFileName = dumpFileNameBase + ".dot";
        try
        {
            string dumpStr = dump();
            std::ofstream out(dumpFileName.c_str(), std::ios::out | std::ios::binary);
            out << dumpStr;
        }
        catch (const std::exception& e)
        {
            std::ofstream out((dumpFileName + ".error").c_str(), std::ios::out);
            out << "Exception: " << e.what() << std::endl;
        }
        catch (...)
        {
            std::ofstream out((dumpFileName + ".error").c_str(), std::ios::out);
            out << "Can't dump: unknown exception" << std::endl;
        }
#endif
    }
};

Net::Net() : impl(new Net::Impl)
{
}

#ifdef HAVE_INF_ENGINE
/*static*/
Net Net::Impl::createNetworkFromModelOptimizer(InferenceEngine::CNNNetwork& ieNet)
{
    CV_TRACE_FUNCTION();

    CV_TRACE_REGION("register_inputs");

    std::vector<String> inputsNames;
    std::vector<MatShape> inp_shapes;
    for (auto& it : ieNet.getInputsInfo())
    {
        inputsNames.push_back(it.first);
        std::vector<size_t> dims = it.second->getTensorDesc().getDims();
        inp_shapes.push_back(std::vector<int>(dims.begin(), dims.end()));
    }

    Net cvNet;
    cvNet.setInputsNames(inputsNames);

    // set empty input to determine input shapes
    for (int inp_id = 0; inp_id < inputsNames.size(); ++inp_id)
    {
        cvNet.setInputShape(inputsNames[inp_id], inp_shapes[inp_id]);
    }

    CV_TRACE_REGION_NEXT("backendNode");

    Ptr<BackendNode> backendNode;
#ifdef HAVE_DNN_NGRAPH
    if (DNN_BACKEND_INFERENCE_ENGINE_NGRAPH == getInferenceEngineBackendTypeParam())
    {
        auto fake_node = std::make_shared<ngraph::op::Parameter>(ngraph::element::f32, ngraph::Shape{});
        Ptr<InfEngineNgraphNode> backendNodeNGraph(new InfEngineNgraphNode(fake_node));
        backendNodeNGraph->net = Ptr<InfEngineNgraphNet>(new InfEngineNgraphNet(*(cvNet.impl), ieNet));
        backendNode = backendNodeNGraph;
    }
    else
#endif
    {
#ifdef HAVE_DNN_IE_NN_BUILDER_2019
        Ptr<InfEngineBackendNode> backendNodeNN(new InfEngineBackendNode(InferenceEngine::Builder::Layer("")));
        backendNodeNN->net = Ptr<InfEngineBackendNet>(new InfEngineBackendNet(ieNet));
        backendNode = backendNodeNN;
#else
        CV_Error(Error::StsNotImplemented, "This OpenCV version is built without Inference Engine NN Builder API support");
#endif
    }

    CV_TRACE_REGION_NEXT("register_outputs");

#ifdef HAVE_DNN_NGRAPH
    auto ngraphFunction = ieNet.getFunction();
#if INF_ENGINE_VER_MAJOR_LT(INF_ENGINE_RELEASE_2020_2)
    std::list< std::shared_ptr<ngraph::Node> > ngraphOperations;
#else
    std::vector< std::shared_ptr<ngraph::Node> > ngraphOperations;
#endif
    if (ngraphFunction)
    {
        ngraphOperations = ngraphFunction->get_ops();
    }
#endif

    for (auto& it : ieNet.getOutputsInfo())
    {
        CV_TRACE_REGION("output");
        const auto& outputName = it.first;

        LayerParams lp;
        int lid = cvNet.addLayer(it.first, "", lp);

        LayerData& ld = cvNet.impl->layers[lid];

#ifdef HAVE_DNN_NGRAPH
        if (DNN_BACKEND_INFERENCE_ENGINE_NGRAPH == getInferenceEngineBackendTypeParam())
        {
            Ptr<Layer> cvLayer(new NgraphBackendLayer(ieNet));
            cvLayer->name = outputName;
            cvLayer->type = "_unknown_";

            auto process_layer = [&](const std::string& name) -> bool
            {
                if (ngraphFunction)
                {
                    CV_TRACE_REGION("ngraph_function");
                    for (const auto& op : ngraphOperations)
                    {
                        CV_Assert(op);
                        if (op->get_friendly_name() == name)
                        {
                            const std::string typeName = op->get_type_info().name;
                            cvLayer->type = typeName;
                            return true;
                        }
                    }
                    return false;
                }
                else
                {
#if INF_ENGINE_VER_MAJOR_GT(INF_ENGINE_RELEASE_2020_4)
                    CV_Error(Error::StsNotImplemented, "This OpenCV version is built with Inference Engine which has dropped IR v7 support");
#else
                    CV_TRACE_REGION("legacy_cnn_layer");
                    try
                    {
                        InferenceEngine::CNNLayerPtr ieLayer = ieNet.getLayerByName(name.c_str());
                        CV_Assert(ieLayer);

                        cvLayer->type = ieLayer->type;
                        return true;
                    }
                    catch (const std::exception& e)
                    {
                        CV_UNUSED(e);
                        CV_LOG_DEBUG(NULL, "IE layer extraction failure: '" << name << "' - " << e.what());
                        return false;
                    }
#endif

                }
            };

            bool found = process_layer(outputName);
            if (!found)
            {
                auto pos = outputName.rfind('.');  // cut port number: ".0"
                if (pos != std::string::npos)
                {
                    std::string layerName = outputName.substr(0, pos);
                    found = process_layer(layerName);
                }
            }
            if (!found)
                CV_LOG_WARNING(NULL, "DNN/IE: Can't determine output layer type: '" << outputName << "'");

            ld.layerInstance = cvLayer;
            ld.backendNodes[DNN_BACKEND_INFERENCE_ENGINE_NGRAPH] = backendNode;
        }
        else
#endif
        {
#ifdef HAVE_DNN_IE_NN_BUILDER_2019
            Ptr<Layer> cvLayer(new InfEngineBackendLayer(ieNet));

            InferenceEngine::CNNLayerPtr ieLayer;
            try
            {
                ieLayer = ieNet.getLayerByName(outputName.c_str());
            }
            catch (...)
            {
                auto pos = outputName.rfind('.');  // cut port number: ".0"
                if (pos != std::string::npos)
                {
                    std::string layerName = outputName.substr(0, pos);
                    ieLayer = ieNet.getLayerByName(layerName.c_str());
                }
            }
            CV_Assert(ieLayer);

            cvLayer->name = outputName;
            cvLayer->type = ieLayer->type;
            ld.layerInstance = cvLayer;

            ld.backendNodes[DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019] = backendNode;
#else
            CV_Error(Error::StsNotImplemented, "This OpenCV version is built without Inference Engine NN Builder API support");
#endif
        }

        for (int i = 0; i < inputsNames.size(); ++i)
            cvNet.connect(0, i, lid, i);
    }

    CV_TRACE_REGION_NEXT("finalize");

    cvNet.setPreferableBackend(getInferenceEngineBackendTypeParam());

    cvNet.impl->skipInfEngineInit = true;
    return cvNet;
}
#endif  // HAVE_INF_ENGINE

Net Net::readFromModelOptimizer(const String& xml, const String& bin)
{
    CV_TRACE_FUNCTION();
#ifndef HAVE_INF_ENGINE
    CV_UNUSED(xml); CV_UNUSED(bin);
    CV_Error(Error::StsError, "Build OpenCV with Inference Engine to enable loading models from Model Optimizer.");
#else
#if INF_ENGINE_VER_MAJOR_LE(INF_ENGINE_RELEASE_2019R3)
    InferenceEngine::CNNNetReader reader;
    reader.ReadNetwork(xml);
    reader.ReadWeights(bin);

    InferenceEngine::CNNNetwork ieNet = reader.getNetwork();
#else
    InferenceEngine::Core& ie = getCore("");
    InferenceEngine::CNNNetwork ieNet = ie.ReadNetwork(xml, bin);
#endif

    return Impl::createNetworkFromModelOptimizer(ieNet);
#endif  // HAVE_INF_ENGINE
}

Net Net::readFromModelOptimizer(const std::vector<uchar>& bufferModelConfig, const std::vector<uchar>& bufferWeights)
{
    CV_TRACE_FUNCTION();
    CV_Assert(!bufferModelConfig.empty());
    CV_Assert(!bufferWeights.empty());
    return readFromModelOptimizer(bufferModelConfig.data(), bufferModelConfig.size(),
                                           bufferWeights.data(), bufferWeights.size());
}

Net Net::readFromModelOptimizer(
        const uchar* bufferModelConfigPtr, size_t bufferModelConfigSize,
        const uchar* bufferWeightsPtr, size_t bufferWeightsSize
)
{
    CV_TRACE_FUNCTION();
#ifndef HAVE_INF_ENGINE
    CV_UNUSED(bufferModelConfigPtr); CV_UNUSED(bufferWeightsPtr);
    CV_UNUSED(bufferModelConfigSize); CV_UNUSED(bufferModelConfigSize);
    CV_Error(Error::StsError, "Build OpenCV with Inference Engine to enable loading models from Model Optimizer.");
#else

#if INF_ENGINE_VER_MAJOR_LE(INF_ENGINE_RELEASE_2019R3)
    InferenceEngine::CNNNetReader reader;

    try
    {
        reader.ReadNetwork(bufferModelConfigPtr, bufferModelConfigSize);

        InferenceEngine::TensorDesc tensorDesc(InferenceEngine::Precision::U8, { bufferWeightsSize }, InferenceEngine::Layout::C);
        InferenceEngine::TBlob<uint8_t>::Ptr weightsBlobPtr(new InferenceEngine::TBlob<uint8_t>(tensorDesc));
        weightsBlobPtr->allocate();
        std::memcpy(weightsBlobPtr->buffer(), (uchar*)bufferWeightsPtr, bufferWeightsSize);
        reader.SetWeights(weightsBlobPtr);
    }
    catch (const std::exception& e)
    {
        CV_Error(Error::StsError, std::string("DNN: IE failed to load model: ") + e.what());
    }

    InferenceEngine::CNNNetwork ieNet = reader.getNetwork();
#else
    InferenceEngine::Core& ie = getCore("");

    std::string model; model.assign((char*)bufferModelConfigPtr, bufferModelConfigSize);

    InferenceEngine::CNNNetwork ieNet;
    try
    {
        InferenceEngine::TensorDesc tensorDesc(InferenceEngine::Precision::U8, { bufferWeightsSize }, InferenceEngine::Layout::C);
        InferenceEngine::Blob::CPtr weights_blob = InferenceEngine::make_shared_blob<uint8_t>(tensorDesc, (uint8_t*)bufferWeightsPtr, bufferWeightsSize);

        ieNet = ie.ReadNetwork(model, weights_blob);
    }
    catch (const std::exception& e)
    {
        CV_Error(Error::StsError, std::string("DNN: IE failed to load model: ") + e.what());
    }
#endif

    return Impl::createNetworkFromModelOptimizer(ieNet);
#endif  // HAVE_INF_ENGINE
}


Net::~Net()
{
}

int Net::addLayer(const String &name, const String &type, const int &dtype, LayerParams &params)
{
    CV_TRACE_FUNCTION();

    int id = impl->getLayerId(name);
    if (id >= 0)
    {
        if (!DNN_DIAGNOSTICS_RUN || type != "NotImplemented")
        {
            CV_Error(Error::StsBadArg, "Layer \"" + name + "\" already into net");
            return -1;
        }
        else
        {
            LayerData& ld = impl->layers.find(id)->second;
            ld.type = type;
            ld.params = params;
            return -1;
        }
    }

    id = ++impl->lastLayerId;
    impl->layerNameToId.insert(std::make_pair(name, id));
    impl->layers.insert(std::make_pair(id, LayerData(id, name, type, dtype, params)));
    if (params.get<bool>("has_dynamic_shapes", false))
        impl->hasDynamicShapes = true;

    if (dtype == CV_8S)
        impl->netWasQuantized = true;

    return id;
}

int Net::addLayer(const String &name, const String &type, LayerParams &params)
{
    CV_TRACE_FUNCTION();
    return addLayer(name, type, CV_32F, params);
}

int Net::addLayerToPrev(const String &name, const String &type, const int &dtype, LayerParams &params)
{
    CV_TRACE_FUNCTION();

    int prvLid = impl->lastLayerId;
    int newLid = this->addLayer(name, type, dtype, params);
    this->connect(prvLid, 0, newLid, 0);
    return newLid;
}

int Net::addLayerToPrev(const String &name, const String &type, LayerParams &params)
{
    CV_TRACE_FUNCTION();
    return addLayerToPrev(name, type, CV_32F, params);
}

void Net::connect(int outLayerId, int outNum, int inpLayerId, int inpNum)
{
    CV_TRACE_FUNCTION();

    impl->connect(outLayerId, outNum, inpLayerId, inpNum);
}

void Net::connect(String _outPin, String _inPin)
{
    CV_TRACE_FUNCTION();

    LayerPin outPin = impl->getPinByAlias(_outPin);
    LayerPin inpPin = impl->getPinByAlias(_inPin);

    CV_Assert(outPin.valid() && inpPin.valid());

    impl->connect(outPin.lid, outPin.oid, inpPin.lid, inpPin.oid);
}

Mat Net::forward(const String& outputName)
{
    CV_TRACE_FUNCTION();
    CV_Assert(!empty());

    String layerName = outputName;

    if (layerName.empty())
    {
        std::vector<String> layerNames = getLayerNames();
        CV_Assert(!layerNames.empty());
        layerName = layerNames.back();
    }

    std::vector<LayerPin> pins(1, impl->getPinByAlias(layerName));
    impl->setUpNet(pins);
    impl->forwardToLayer(impl->getLayerData(layerName));

    return impl->getBlob(layerName);
}

AsyncArray Net::forwardAsync(const String& outputName)
{
    CV_TRACE_FUNCTION();
    CV_Assert(!empty());

#ifdef CV_CXX11
    String layerName = outputName;

    if (layerName.empty())
    {
        std::vector<String> layerNames = getLayerNames();
        CV_Assert(!layerNames.empty());
        layerName = layerNames.back();
    }

    std::vector<LayerPin> pins(1, impl->getPinByAlias(layerName));
    impl->setUpNet(pins);

    if (!(impl->preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019 || impl->preferableBackend == DNN_BACKEND_INFERENCE_ENGINE_NGRAPH))
        CV_Error(Error::StsNotImplemented, "DNN: Asynchronous forward is supported for Inference Engine backends only");

    impl->isAsync = true;
    impl->forwardToLayer(impl->getLayerData(layerName));
    impl->isAsync = false;

    return impl->getBlobAsync(layerName);
#else
    CV_Error(Error::StsNotImplemented, "DNN: Asynchronous forward requires build with enabled C++11");
#endif  // CV_CXX11
}

void Net::forward(OutputArrayOfArrays outputBlobs, const String& outputName)
{
    CV_TRACE_FUNCTION();
    CV_Assert(!empty());

    String layerName = outputName;

    if (layerName.empty())
    {
        std::vector<String> layerNames = getLayerNames();
        CV_Assert(!layerNames.empty());
        layerName = layerNames.back();
    }

    std::vector<LayerPin> pins(1, impl->getPinByAlias(layerName));
    impl->setUpNet(pins);
    impl->forwardToLayer(impl->getLayerData(layerName));

    LayerPin pin = impl->getPinByAlias(layerName);
    LayerData &ld = impl->layers[pin.lid];

    if (outputBlobs.isUMat())
    {
        impl->getBlob(layerName).copyTo(outputBlobs);
    }
    else if (outputBlobs.isMat())
    {
        outputBlobs.assign(impl->getBlob(layerName));
    }
    else if (outputBlobs.isMatVector())
    {
        if (impl->preferableTarget != DNN_TARGET_CPU)
        {
            for (int i = 0; i < ld.outputBlobsWrappers.size(); ++i)
            {
                CV_Assert(!ld.outputBlobsWrappers[i].empty());
                ld.outputBlobsWrappers[i]->copyToHost();
            }
        }
        if (ld.outputBlobs[0].depth() == CV_16S)
        {
            std::vector<Mat> & outputvec = *(std::vector<Mat> *)outputBlobs.getObj();
            outputvec.resize(ld.outputBlobs.size());
            for (int i = 0; i < outputvec.size(); i++)
                convertFp16(ld.outputBlobs[i], outputvec[i]);
        }
        else
        {
            // Output depth can be CV_32F or CV_8S
            std::vector<Mat> & outputvec = *(std::vector<Mat> *)outputBlobs.getObj();
            outputvec = ld.outputBlobs;
        }
    }
    else if (outputBlobs.isUMatVector())
    {
        std::vector<UMat> & outputvec = *(std::vector<UMat> *)outputBlobs.getObj();

#ifdef HAVE_OPENCL
        if (impl->preferableBackend == DNN_BACKEND_OPENCV &&
            IS_DNN_OPENCL_TARGET(impl->preferableTarget))
        {
            if (impl->preferableTarget == DNN_TARGET_OPENCL)
                outputvec = OpenCLBackendWrapper::getUMatVector(ld.outputBlobsWrappers);
            else if (impl->preferableTarget == DNN_TARGET_OPENCL_FP16)
            {
                std::vector<UMat> out_vec = OpenCLBackendWrapper::getUMatVector(ld.outputBlobsWrappers);
                outputvec.resize(out_vec.size());
                for (int i = 0; i < out_vec.size(); i++)
                    convertFp16(out_vec[i], outputvec[i]);
            }
        }
        else
#endif
        {
            outputvec.resize(ld.outputBlobs.size());
            for (int i = 0; i < outputvec.size(); ++i)
                ld.outputBlobs[i].copyTo(outputvec[i]);
        }
    }
}

void Net::forward(OutputArrayOfArrays outputBlobs,
                  const std::vector<String>& outBlobNames)
{
    CV_TRACE_FUNCTION();

    std::vector<LayerPin> pins;
    for (int i = 0; i < outBlobNames.size(); i++)
    {
        pins.push_back(impl->getPinByAlias(outBlobNames[i]));
    }

    impl->setUpNet(pins);

    LayerPin out = impl->getLatestLayerPin(pins);

    impl->forwardToLayer(impl->getLayerData(out.lid));

    std::vector<Mat> matvec;
    for (int i = 0; i < pins.size(); i++)
    {
        matvec.push_back(impl->getBlob(pins[i]));
    }

    outputBlobs.create((int)matvec.size(), 1, CV_32F/*FIXIT*/, -1);  // allocate vector
    outputBlobs.assign(matvec);
}

void Net::forward(std::vector<std::vector<Mat> >& outputBlobs,
                     const std::vector<String>& outBlobNames)
{
    CV_TRACE_FUNCTION();

    std::vector<LayerPin> pins;
    for (int i = 0; i < outBlobNames.size(); i++)
    {
        pins.push_back(impl->getPinByAlias(outBlobNames[i]));
    }

    impl->setUpNet(pins);

    LayerPin out = impl->getLatestLayerPin(pins);

    impl->forwardToLayer(impl->getLayerData(out.lid));

    outputBlobs.resize(outBlobNames.size());
    for (int i = 0; i < outBlobNames.size(); i++)
    {
        std::vector<LayerPin> lp = impl->getLayerOutPins(outBlobNames[i]);
        outputBlobs[i].resize(lp.size());
        for (int j = 0; j < lp.size(); j++)
        {
            outputBlobs[i][j] = impl->getBlob(lp[j]);
        }
    }
}

Net Net::quantize(InputArrayOfArrays calibData, int inputsDtype, int outputsDtype)
{
    CV_TRACE_FUNCTION();

    // Net can be quantized only once.
    if (impl->netWasQuantized)
        CV_Error(Error::StsBadArg, "Cannot quantize a quantized net");

    CV_CheckType(inputsDtype, inputsDtype == CV_32F || inputsDtype == CV_8S, "Input depth should be CV_32F or CV_8S");
    CV_CheckType(outputsDtype, outputsDtype == CV_32F || outputsDtype == CV_8S, "Output depth should be CV_32F or CV_8S");

    bool originalFusion = impl->fusion;
    int prefBackend = impl->preferableBackend;
    int prefTarget = impl->preferableTarget;

    // Disable fusions and use CPU backend to quantize net
    setPreferableBackend(DNN_BACKEND_OPENCV);
    setPreferableTarget(DNN_TARGET_CPU);
    enableFusion(false);

    if (calibData.isMat())
    {
        setInput(calibData.getMat());
    }
    else if (calibData.isMatVector())
    {
        std::vector<Mat> calibDataVec;
        calibData.getMatVector(calibDataVec);

        std::vector<String> inpNames = impl->netInputLayer->outNames;
        CV_CheckEQ(calibDataVec.size(), inpNames.size(), "Calibration data size should be equal to number of inputs");
        for (int i = 0; i < calibDataVec.size(); i++)
            setInput(calibDataVec[i], inpNames[i]);
    }

    std::vector<String> outNames = getUnconnectedOutLayersNames();
    std::vector<LayerPin> pins;
    for (int i = 0; i < outNames.size(); i++)
        pins.push_back(impl->getPinByAlias(outNames[i]));
    impl->setUpNet(pins);

    // Compute scales and zeropoints for all the layers
    std::vector<std::vector<float> > scales;
    std::vector<std::vector<int> > zeropoints;
    for (Impl::MapIdToLayerData::iterator it = impl->layers.begin(); it != impl->layers.end(); it++)
    {
        LayerData& ld = it->second;
        if (!ld.skip)
        {
            Ptr<Layer> layer = ld.layerInstance;
            std::vector<Mat> inps(ld.inputBlobs.size());
            for (int i = 0; i < ld.inputBlobs.size(); ++i)
                inps[i] = *ld.inputBlobs[i];
            layer->forward(inps, ld.outputBlobs, ld.internals);
        }

        std::vector<float> sc;
        std::vector<int> zp;
        if (ld.type == "TanH")
        {
            sc.push_back(1.f/128);
            zp.push_back(0);
        }
        else if (ld.type == "Sigmoid" || ld.type == "Softmax" || ld.type == "SoftMax")
        {
            if (ld.params.get<bool>("log_softmax", false))
            {
                sc.push_back(16.f/256);
                zp.push_back(127);
            }
            else
            {
                sc.push_back(1.f/256);
                zp.push_back(-128);
            }
        }
        else if (ld.type == "Split" || ld.type == "Slice" || ld.type == "Crop")
        {
            std::vector<float> inp_sc; std::vector<int> inp_zp;
            impl->getQuantizationParams(*ld.inputBlobs[0], inp_sc, inp_zp);
            sc.assign(ld.outputBlobs.size(), inp_sc[0]);
            zp.assign(ld.outputBlobs.size(), inp_zp[0]);
        }
        else
        {
            for (int i = 0; i < ld.outputBlobs.size(); i++)
                impl->getQuantizationParams(ld.outputBlobs[i], sc, zp);
        }
        scales.push_back(sc);
        zeropoints.push_back(zp);
    }

    // For some layers, the input and output scales/zeropoints must be equal so that rescaling of inputs
    // is not needed during quantized inference. We start from the last layer and modify the layer's input scales/zeropoints
    // TODO : Need a different approach. Current solution fails when 2 such layers have the same input layer
    for (Impl::MapIdToLayerData::reverse_iterator it = impl->layers.rbegin(); it != impl->layers.rend(); ++it)
    {
        LayerData& ld = it->second;
        // Layers with multiple outputs. Number of outputs is equal to number of inputs
        if (ld.type == "Blank" || ld.type == "Dropout" || ld.type == "Identity" || ld.type == "Silence" ||
            ld.type == "Flatten" || ld.type == "Padding" || ld.type == "Permute" || ld.type == "Reshape" ||
            ld.type == "ReLU6" || ld.type == "Reorg" || ld.type == "ShuffleChannel" || ld.type == "Resize" ||
           (ld.type == "ReLU" && !ld.params.get<float>("negative_slope", 0.f)) /* ReLU with negative slope 0 */)
        {
            for (int i = 0; i < ld.outputBlobs.size(); i++)
            {
                LayerPin &pin = ld.inputBlobsId[i];
                scales[pin.lid][pin.oid] = scales[ld.id][i];
                zeropoints[pin.lid][pin.oid] = zeropoints[ld.id][i];
            }
        }
        // Layers with multiple inputs and single output.
        else if ((ld.type == "Pooling" && toLowerCase(ld.params.get<String>("pool", "max")) == "max") /* Max Pooling */ ||
                 (ld.type == "Eltwise" && toLowerCase(ld.params.get<String>("operation", "sum")) == "max") /* Elementwise max */ ||
                  ld.type == "Concat")
        {
            for (int i = 0; i < ld.inputBlobsId.size(); i++)
            {
                LayerPin &pin = ld.inputBlobsId[i];
                scales[pin.lid][pin.oid] = scales[ld.id][0];
                zeropoints[pin.lid][pin.oid] = zeropoints[ld.id][0];
            }
        }
    }

    // Create a new Net and add quantized layers to it.
    Net dstNet;
    dstNet.impl->netWasQuantized = true;
    dstNet.setInputsNames(impl->netInputLayer->outNames);
    dstNet.setPreferableBackend(prefBackend);
    dstNet.setPreferableTarget(prefTarget);
    dstNet.enableFusion(originalFusion);

    for (Impl::MapIdToLayerData::iterator it = impl->layers.begin(); it != impl->layers.end(); it++)
    {
        LayerData ld = it->second;
        if (ld.id == 0)
        {
            LayerData &quantInpLd = dstNet.impl->layers[0];
            quantInpLd.dtype = inputsDtype;
            quantInpLd.params.set("scales", DictValue::arrayReal(scales[0].data(), scales[0].size()));
            quantInpLd.params.set("zeropoints", DictValue::arrayInt(zeropoints[0].data(), zeropoints[0].size()));
            continue;
        }

        std::vector<LayerPin> inpPins = ld.inputBlobsId;
        // Fill input and output scales/zeropoints for the layer
        std::vector<std::vector<float> > inp_out_sc(2);
        std::vector<std::vector<int> > inp_out_zp(2);
        for (int i = 0; i < inpPins.size(); i++)
        {
            LayerPin &pin = inpPins[i];
            inp_out_sc[0].push_back(scales[pin.lid][pin.oid]);
            inp_out_zp[0].push_back(zeropoints[pin.lid][pin.oid]);
        }
        inp_out_sc[1] = scales[ld.id];
        inp_out_zp[1] = zeropoints[ld.id];

        // Quantize layer
        Ptr<Layer> layer = ld.layerInstance;
        if (layer->tryQuantize(inp_out_sc, inp_out_zp, ld.params))
        {
            ld.type += "Int8";
            ld.dtype = CV_8S;
        }
        ld.params.set("scales", DictValue::arrayReal(inp_out_sc[1].data(), inp_out_sc[1].size()));
        ld.params.set("zeropoints", DictValue::arrayInt(inp_out_zp[1].data(), inp_out_zp[1].size()));

        // Check and add quantize/dequantize node before layer
        for (int i = 0; i < inpPins.size(); i++)
        {
            LayerPin &pin = inpPins[i];
            LayerData &inpLd = dstNet.impl->getLayerData(impl->getLayerName(pin.lid));
            pin.lid = inpLd.id;
            if (inpLd.dtype != ld.dtype)
            {
                String layerName = (inpLd.dtype == CV_32F && ld.dtype == CV_8S) ? cv::format("quantize/%s/%d", inpLd.name.c_str(), pin.oid)
                                                                                : cv::format("dequantize/%s/%d", inpLd.name.c_str(), pin.oid);
                // Check if quantize/dequantize node for the input layer already exists
                if (dstNet.impl->getLayerId(layerName) >= 0)
                {
                    pin.lid = dstNet.impl->getLayerId(layerName);
                    pin.oid = 0;
                }
                else
                {
                    LayerParams lp;
                    lp.set("scales", inp_out_sc[0][i]);
                    lp.set("zeropoints", inp_out_zp[0][i]);
                    lp.name = layerName;
                    lp.type = (inpLd.dtype == CV_32F && ld.dtype == CV_8S) ? "Quantize" : "Dequantize";
                    int newLid = dstNet.addLayer(lp.name, lp.type, ld.dtype, lp);
                    dstNet.connect(pin.lid, pin.oid, newLid, 0);
                    pin.lid = newLid; pin.oid = 0;
                }
            }
        }

        // Add quantized layer to Net and connect to its inputs.
        int newLid = dstNet.addLayer(ld.name, ld.type, ld.dtype, ld.params);
        for( int i = 0; i < inpPins.size(); i++ )
            dstNet.connect(inpPins[i].lid, inpPins[i].oid, newLid, i);

        // If the layer is a output layer, add quantize/dequantize node after it based on output's data type.
        if (ld.requiredOutputs.size() == 0 && ld.dtype != outputsDtype)
        {
            LayerParams lp;
            lp.set("scales", inp_out_sc[1][0]);
            lp.set("zeropoints", inp_out_zp[1][0]);
            lp.name = ((ld.dtype == CV_32F && outputsDtype == CV_8S) ? "quantize/" : "dequantize/") + ld.name;
            lp.type = (ld.dtype == CV_32F && outputsDtype == CV_8S) ? "Quantize" : "Dequantize";
            dstNet.addLayerToPrev(lp.name, lp.type, outputsDtype, lp);
        }
    }
    // Restore FP32 Net's backend, target and fusion
    setPreferableBackend(prefBackend);
    setPreferableTarget(prefTarget);
    enableFusion(originalFusion);
    return dstNet;
}

void Net::getInputDetails(std::vector<float>& scales, std::vector<int>& zeropoints) const
{
    if (!impl->netWasQuantized)
        CV_Error(Error::StsBadFunc, "Net isn't quantized");

    LayerParams &lp = impl->layers[0].params;
    DictValue sc = lp.get("scales");
    DictValue zp = lp.get("zeropoints");

    for (int i = 0; i < sc.size(); i++)
    {
        scales.push_back(sc.get<float>(i));
        zeropoints.push_back(zp.get<int>(i));
    }
}

void Net::getOutputDetails(std::vector<float>& scales, std::vector<int>& zeropoints) const
{
    if (!impl->netWasQuantized)
        CV_Error(Error::StsBadFunc, "Net isn't quantized");

    std::vector<int> outLayerIds = getUnconnectedOutLayers();
    for (auto &lid : outLayerIds)
    {
        LayerParams &lp = impl->layers[lid].params;
        DictValue sc = lp.get("scales");
        DictValue zp = lp.get("zeropoints");

        for (int i = 0; i < sc.size(); i++)
        {
            scales.push_back(sc.get<float>(i));
            zeropoints.push_back(zp.get<int>(i));
        }
    }
}

void Net::setPreferableBackend(int backendId)
{
    CV_TRACE_FUNCTION();
    CV_TRACE_ARG(backendId);

    if (backendId == DNN_BACKEND_DEFAULT)
        backendId = (Backend)PARAM_DNN_BACKEND_DEFAULT;

    if (impl->netWasQuantized && backendId != DNN_BACKEND_OPENCV)
    {
        CV_LOG_WARNING(NULL, "DNN: Only default backend supports quantized networks");
        backendId = DNN_BACKEND_OPENCV;
    }

#ifdef HAVE_INF_ENGINE
    if (backendId == DNN_BACKEND_INFERENCE_ENGINE)
        backendId = getInferenceEngineBackendTypeParam();
#endif

    if( impl->preferableBackend != backendId )
    {
        impl->preferableBackend = backendId;
        impl->clear();
    }
}

void Net::setPreferableTarget(int targetId)
{
    CV_TRACE_FUNCTION();
    CV_TRACE_ARG(targetId);

    if (impl->netWasQuantized && targetId != DNN_TARGET_CPU &&
        targetId != DNN_TARGET_OPENCL && targetId != DNN_TARGET_OPENCL_FP16)
    {
        CV_LOG_WARNING(NULL, "DNN: Only CPU and OpenCL/OpenCL FP16 target is supported by quantized networks");
        targetId = DNN_TARGET_CPU;
    }

    if( impl->preferableTarget != targetId )
    {
        impl->preferableTarget = targetId;
        if (IS_DNN_OPENCL_TARGET(targetId))
        {
#ifndef HAVE_OPENCL
#ifdef HAVE_INF_ENGINE
            if (impl->preferableBackend == DNN_BACKEND_OPENCV)
#else
            if (impl->preferableBackend == DNN_BACKEND_DEFAULT ||
                impl->preferableBackend == DNN_BACKEND_OPENCV)
#endif  // HAVE_INF_ENGINE
                impl->preferableTarget = DNN_TARGET_CPU;
#else
            bool fp16 = ocl::Device::getDefault().isExtensionSupported("cl_khr_fp16");
            if (!fp16 && targetId == DNN_TARGET_OPENCL_FP16)
                impl->preferableTarget = DNN_TARGET_OPENCL;
#endif
        }
        impl->clear();
    }
}

void Net::setInputsNames(const std::vector<String> &inputBlobNames)
{
    CV_TRACE_FUNCTION();

    impl->netInputLayer->setNames(inputBlobNames);
}

void Net::setInputShape(const String &inputName, const MatShape& shape)
{
    CV_TRACE_FUNCTION();

    impl->netInputLayer->setInputShape(inputName, shape);
}

void Net::setInput(InputArray blob, const String& name, double scalefactor, const Scalar& mean)
{
    CV_TRACE_FUNCTION();
    CV_TRACE_ARG_VALUE(name, "name", name.c_str());

    LayerPin pin;
    pin.lid = 0;
    pin.oid = impl->resolvePinOutputName(impl->getLayerData(pin.lid), name);

    if (!pin.valid())
        CV_Error(Error::StsObjectNotFound, "Requested blob \"" + name + "\" not found");

    Mat blob_ = blob.getMat();  // can't use InputArray directly due MatExpr stuff
    MatShape blobShape = shape(blob_);

    if (pin.lid == 0)
    {
        CV_Assert(!impl->netInputLayer.empty());
        const DataLayer& netInputLayer = *impl->netInputLayer.get();
        if (!netInputLayer.shapes.empty())
        {
            CV_CheckLT(pin.oid, (int)netInputLayer.shapes.size(), "");
            const MatShape& inputShapeLimitation = netInputLayer.shapes[pin.oid];
            if (!inputShapeLimitation.empty())
            {
                CV_CheckEQ(inputShapeLimitation.size(), blobShape.size(), "");
#if 0  // TODO: DNNTestNetwork.MobileNet_SSD_Caffe_Different_Width_Height/0
                const size_t dims = inputShapeLimitation.size();
                for (size_t dim = 0; dim < dims; dim++)
                {
                    if (dims >= 3 && dim == 0 && inputShapeLimitation[0] == 1)
                        continue;  // don't limit batch
                    CV_CheckEQ(inputShapeLimitation[dim], blobShape[dim], "");
                }
#endif
            }
        }
    }

    LayerData &ld = impl->layers[pin.lid];
    const int numInputs = std::max(pin.oid+1, (int)ld.requiredOutputs.size());
    ld.outputBlobs.resize(numInputs);
    ld.outputBlobsWrappers.resize(numInputs);
    impl->netInputLayer->inputsData.resize(numInputs);
    impl->netInputLayer->scaleFactors.resize(numInputs);
    impl->netInputLayer->means.resize(numInputs);

    MatShape prevShape = shape(impl->netInputLayer->inputsData[pin.oid]);
    bool oldShape = prevShape == blobShape;

    blob_.copyTo(impl->netInputLayer->inputsData[pin.oid]);
    if (!oldShape) {
        ld.outputBlobs[pin.oid] = impl->netInputLayer->inputsData[pin.oid];
        if (impl->hasDynamicShapes)
        {
            impl->updateLayersShapes();
        }
    }

    if (!ld.outputBlobsWrappers[pin.oid].empty())
    {
        ld.outputBlobsWrappers[pin.oid]->setHostDirty();
    }
    impl->netInputLayer->scaleFactors[pin.oid] = scalefactor;
    impl->netInputLayer->means[pin.oid] = mean;
    impl->netWasAllocated = impl->netWasAllocated && oldShape;
}

Mat Net::getParam(LayerId layer, int numParam)
{
    LayerData &ld = impl->getLayerData(layer);
    std::vector<Mat> &layerBlobs = ld.getLayerInstance()->blobs;
    CV_Assert(numParam < (int)layerBlobs.size());
    return layerBlobs[numParam];
}

void Net::setParam(LayerId layer, int numParam, const Mat &blob)
{
    LayerData &ld = impl->getLayerData(layer);

    std::vector<Mat> &layerBlobs = ld.getLayerInstance()->blobs;
    CV_Assert(numParam < (int)layerBlobs.size());
    //we don't make strong checks, use this function carefully
    layerBlobs[numParam] = blob;
}

int Net::getLayerId(const String &layer)
{
    return impl->getLayerId(layer);
}

static
string dumpLayerParameterSize(const string& name, const LayerParams& lp)
{
    std::ostringstream out(name, std::ios::ate);
    DictValue param = lp.get(name);
    switch (param.size())
    {
        case 1: out << " : "; break;
        case 2: out << " (HxW): "; break;
        case 3: out << " (DxHxW): "; break;
        default:
            CV_LOG_INFO(NULL, format("DNN/dumpLayerParameterSize(): Unsupported '%s' size = %d", name.c_str(), param.size()));
            out << ": ";
    }
    for (size_t i = 0; i < param.size(); i++)
    {
        if (i > 0)
            out << " x ";
        out << param.get<int>(i);
    }
    return out.str();
}

String Net::dump()
{
    CV_Assert(!empty());

    bool hasInput = !impl->netInputLayer->inputsData.empty();

    if (hasInput)
    {
        if (!impl->netWasAllocated)
            impl->setUpNet();
    }

    return impl->dump();
}

string Net::Impl::dump()
{
    bool hasInput = !netInputLayer->inputsData.empty();

    std::ostringstream out;
    const std::map<int, LayerData>& map = layers;

    Backend prefBackend = (Backend)preferableBackend;
    std::vector<std::vector<int> > skippedLayers;
    std::vector<int> skipId;
    std::vector<int> allLayers(map.size(), -1);
    int idPrev = -1;
    Ptr<BackendNode> prevNode;
    for (std::map<int, LayerData>::const_reverse_iterator rit = map.rbegin(); rit != map.rend(); ++rit)
    {
        std::map<int, Ptr<BackendNode> >::const_iterator itBackend = rit->second.backendNodes.find(prefBackend);
        if (prefBackend == DNN_BACKEND_OPENCV || itBackend == rit->second.backendNodes.end() ||
            itBackend->second.empty())
        {
                if (rit->second.skip)
                    skipId.push_back(rit->first);
                else if (!skipId.empty())
                {
                    if (prefBackend == DNN_BACKEND_OPENCV || prevNode.empty())
                        skipId.push_back(rit->first);
                    else if (idPrev != -1)
                        skipId.push_back(idPrev);

                    std::sort(skipId.begin(), skipId.end());
                    for (int i = 0; i < skipId.size(); i++) {
                        allLayers[skipId[i]] = skippedLayers.size();
                    }
                    skippedLayers.push_back(skipId);
                    skipId.clear();
                }
        }
        else
        {
            if (itBackend->second == prevNode)
                skipId.push_back(idPrev);
            else if (!skipId.empty())
            {
                skipId.push_back(idPrev);
                std::sort(skipId.begin(), skipId.end());
                for (int i = 0; i < skipId.size(); i++) {
                    allLayers[skipId[i]] = skippedLayers.size();
                }
                skippedLayers.push_back(skipId);
                skipId.clear();
            }
            idPrev = rit->first;
            prevNode = itBackend->second;
        }
    }
    std::vector<string> colors = {"#ffffb3", "#fccde5", "#8dd3c7", "#bebada", "#80b1d3", "#fdb462", "#ff4848", "#b35151", "#b266ff"};
    string backend;
    switch (prefBackend)
    {
        case DNN_BACKEND_DEFAULT: backend = "DEFAULT/"; break;
        case DNN_BACKEND_HALIDE: backend = "HALIDE/"; break;
        case DNN_BACKEND_INFERENCE_ENGINE: // fallthru
        case DNN_BACKEND_INFERENCE_ENGINE_NN_BUILDER_2019: backend = "DLIE/"; break;
        case DNN_BACKEND_INFERENCE_ENGINE_NGRAPH: backend = "NGRAPH/"; break;
        case DNN_BACKEND_OPENCV: backend = "OCV/"; break;
        case DNN_BACKEND_VKCOM: backend = "VULKAN/"; break;
        case DNN_BACKEND_CUDA: backend = "CUDA/"; break;
        // don't use default:
    }
    out << "digraph G {\n";
    // Add nodes
    for (std::map<int, LayerData>::const_iterator it = map.begin(); it != map.end(); ++it)
    {
        const LayerData& ld = it->second;
        string name = ld.params.name;
        std::vector<int> clusterIds(1, it->first);
        if (allLayers[it->first] == -1 && !name.empty())
        {
            out << "\t\"" << name << "\" [label=\"";
        }
        else if (name.empty() || it->first != skippedLayers[allLayers[it->first]][0])
        {
            continue;
        }
        else // first node in cluster : it->first == skippedLayers[allLayers[it->first]][0]
        {
            int cluster = allLayers[it->first];
            out << "\t\"" << "cluster_" << cluster << "\" [label=\"{";
            clusterIds = skippedLayers[allLayers[it->first]]; // vertices in current cluster
        }
        for (int i = 0; i < clusterIds.size(); i++)
        {
            CV_DbgAssert(map.find(clusterIds[i]) != map.end());
            const LayerParams& lp = map.find(clusterIds[i])->second.params;
            if (!lp.name.empty()) {
                if (i > 0) {
                    out << " | ";
                }
                out << lp.name << "\\n" << lp.type << "\\n";  // align center
                if (lp.has("kernel_size"))
                {
                    string kernel = dumpLayerParameterSize("kernel_size", lp);
                    out << kernel;
                    out << "\\l";  // align left
                } else if (lp.has("kernel_h") && lp.has("kernel_w")) {
                    DictValue h = lp.get("kernel_h");
                    DictValue w = lp.get("kernel_w");
                    out << "kernel (HxW): " << h << " x " << w;
                    out << "\\l";  // align left
                }
                if (lp.has("stride")) {
                    string stride = dumpLayerParameterSize("stride", lp);
                    out << stride;
                    out << "\\l";  // align left
                } else if (lp.has("stride_h") && lp.has("stride_w")) {
                    DictValue h = lp.get("stride_h");
                    DictValue w = lp.get("stride_w");
                    out << "stride (HxW): " << h << " x " << w;
                    out << "\\l";  // align left
                }
                if (lp.has("dilation")) {
                    string dilation = dumpLayerParameterSize("dilation", lp);
                    out << dilation;
                    out << "\\l";  // align left
                } else if (lp.has("dilation_h") && lp.has("dilation_w")) {
                    DictValue h = lp.get("dilation_h");
                    DictValue w = lp.get("dilation_w");
                    out << "dilation (HxW): " << h << " x " << w;
                    out << "\\l";  // align left
                }
                if (lp.has("pad")) {
                    DictValue pad = lp.get("pad");
                    out << "pad ";
                    switch (pad.size())
                    {
                        case 1: out << ": " << pad; break;
                        case 2:
                            out << "(HxW): (" << pad.get<int>(0) << " x " << pad.get<int>(1) << ")";
                            break;
                        case 4:
                            out << "(HxW): (" << pad.get<int>(0) << ", " << pad.get<int>(2)
                                << ") x (" << pad.get<int>(1) << ", " << pad.get<int>(3) << ")";
                            break;
                        case 6:
                            out << "(DxHxW): (" << pad.get<int>(0) << ", " << pad.get<int>(3)
                                << ") x (" << pad.get<int>(1) << ", " << pad.get<int>(4)
                                << ") x (" << pad.get<int>(2) << ", " << pad.get<int>(5) << ")";
                            break;
                        default: CV_Error(Error::StsNotImplemented,  format("Unsupported pad size = %d", pad.size()));
                    }
                    out << "\\l";  // align left
                } else if (lp.has("pad_l") && lp.has("pad_t") && lp.has("pad_r") && lp.has("pad_b")) {
                    DictValue l = lp.get("pad_l");
                    DictValue t = lp.get("pad_t");
                    DictValue r = lp.get("pad_r");
                    DictValue b = lp.get("pad_b");
                    out << "pad (HxW): (" << t << ", " << b << ") x (" << l << ", " << r << ")";
                    out << "\\l";  // align left
                }
                else if (lp.has("pooled_w") || lp.has("pooled_h")) {
                    DictValue h = lp.get("pooled_h");
                    DictValue w = lp.get("pooled_w");
                    out << "pad pooled (HxW): " << h << " x " << w;
                    out << "\\l";  // align left
                }
                if (lp.has("pool")) {
                    out << "pool: " << lp.get("pool");
                    out << "\\l";  // align left
                }
                if (lp.has("global_pooling")) {
                    out << "global_pooling: " << lp.get("global_pooling");
                    out << "\\l";  // align left
                }
                if (lp.has("group")) {
                    out << "group: " << lp.get("group");
                    out << "\\l";  // align left
                }
            }
        }
        if (!ld.outputBlobs.empty())
        {
            out << "output: " << ld.outputBlobs[0].size;
            out << "\\l";  // align left
        }

        Ptr<BackendNode> layerBackend;
        std::map<int, Ptr<BackendNode> >::const_iterator ibn = ld.backendNodes.find(prefBackend);
        if (ibn != ld.backendNodes.end())
            layerBackend = ibn->second;
        out << (!layerBackend.empty() ? backend : "OCV/");
        int colorId = 0;
        const Target target = ld.layerInstance.empty()
                         ? DNN_TARGET_CPU
                                 : (Target)(ld.layerInstance->preferableTarget);  // TODO fix preferableTarget type
        switch (target)
        {
            case DNN_TARGET_CPU: out << "CPU"; colorId = layerBackend.empty() ? 0 : 5; break;
            case DNN_TARGET_OPENCL: out << "OCL"; colorId = 1; break;
            case DNN_TARGET_OPENCL_FP16: out << "OCL_FP16"; colorId = 2; break;
            case DNN_TARGET_MYRIAD: out << "MYRIAD"; colorId = 3; break;
            case DNN_TARGET_HDDL: out << "HDDL"; colorId = 8; break;
            case DNN_TARGET_VULKAN: out << "VULKAN"; colorId = 7; break;
            case DNN_TARGET_FPGA: out << "FPGA"; colorId = 4; break;
            case DNN_TARGET_CUDA: out << "CUDA"; colorId = 5; break;
            case DNN_TARGET_CUDA_FP16: out << "CUDA_FP16"; colorId = 6; break;
            // don't use default:
        }
        CV_Assert(colorId < colors.size());
        out << "\\n";  // align center
        out << ((clusterIds.size() == 1)? "\" " : " }\" ");
        out << "fillcolor=\"" << colors[colorId] << "\" ";
        out << "style=filled ";
        out << "shape=" << ((clusterIds.size() == 1)? "box" : "record") << "]\n";
    }
    out << '\n';
    // Add edges
    int inputsSize = hasInput ? netInputLayer->outNames.size() : 0;
    for (std::map<int, LayerData>::const_iterator it = map.begin(); it != map.end(); ++it)
    {
        const LayerData& ld = it->second;
        if (allLayers[it->first] == -1)  // node
        {
            for (int i = 0; i < ld.consumers.size(); i++)
            {
                int outId = ld.consumers[i].lid;
                if (it == map.begin() && inputsSize > 1)
                    out << "\t\"" << ld.name << "_" << i << "\"" << " -> ";
                else
                    out << "\t\"" << ld.name << "\"" << " -> ";
                if (allLayers[outId] == -1)  // node
                {
                    CV_DbgAssert(map.find(outId) != map.end());
                    out << "\"" << map.find(outId)->second.name << "\"\n";
                }
                else  // cluster
                {
                    out << "\"" << "cluster_" << allLayers[outId] << "\"\n";
                }
            }
        }
        else if (it->first == skippedLayers[allLayers[it->first]].back())  // edges from last layer in cluster
        {
            for (int i = 0; i < ld.consumers.size(); i++)
            {
                int outId = ld.consumers[i].lid;
                if (allLayers[outId] == -1) // node
                {
                    CV_DbgAssert(map.find(outId) != map.end());
                    out << "\t\"" << "cluster_" << allLayers[it->first] << "\"" << " -> ";
                    out << "\"" << map.find(outId)->second.name << "\"\n";
                }
                else if (allLayers[outId] != allLayers[it->first]) { // another cluster
                    out << "\t\"" << "cluster_" << allLayers[it->first] << "\"" << " -> ";
                    out << "\"" << "cluster_" << allLayers[outId] << "\"\n";
                }
            }
        }
    }
    out << "}\n";
    return out.str();
}

void Net::dumpToFile(const String& path) {
    std::ofstream file(path.c_str());
    file << dump();
    file.close();
}

Ptr<Layer> Net::getLayer(LayerId layerId)
{
    LayerData &ld = impl->getLayerData(layerId);
    return ld.getLayerInstance();
}

std::vector<Ptr<Layer> > Net::getLayerInputs(LayerId layerId)
{
    LayerData &ld = impl->getLayerData(layerId);

    std::vector<Ptr<Layer> > inputLayers;
    inputLayers.reserve(ld.inputBlobsId.size());
    for (int i = 0; i < ld.inputBlobsId.size(); ++i) {
        inputLayers.push_back(getLayer(ld.inputBlobsId[i].lid));
    }
    return inputLayers;
}

std::vector<String> Net::getLayerNames() const
{
    CV_TRACE_FUNCTION();

    std::vector<String> res;
    res.reserve(impl->layers.size());

    Impl::MapIdToLayerData::iterator it;
    for (it = impl->layers.begin(); it != impl->layers.end(); it++)
    {
        if (it->second.id) //skip Data layer
            res.push_back(it->second.name);
    }

    return res;
}

bool Net::empty() const
{
    return impl->layers.size() <= 1; //first layer is default Data layer
}

std::vector<int> Net::getUnconnectedOutLayers() const
{
    std::vector<int> layersIds;

    Impl::MapIdToLayerData::iterator it;
    for (it = impl->layers.begin(); it != impl->layers.end(); it++)
    {
        int lid = it->first;
        LayerData &ld = it->second;

        if (ld.requiredOutputs.size() == 0)
            layersIds.push_back(lid);
    }

    return layersIds;
}

std::vector<String> Net::getUnconnectedOutLayersNames() const
{
    std::vector<int> ids = getUnconnectedOutLayers();
    const size_t n = ids.size();
    std::vector<String> names(n);
    for (size_t i = 0; i < n; ++i)
    {
        names[i] = impl->layers[ids[i]].name;
    }
    return names;
}

void Net::getLayersShapes(const ShapesVec& netInputShapes,
                          std::vector<int>& layersIds,
                          std::vector<ShapesVec>& inLayersShapes,
                          std::vector<ShapesVec>& outLayersShapes) const
{
    layersIds.clear();
    inLayersShapes.clear();
    outLayersShapes.clear();

    Impl::LayersShapesMap inOutShapes;
    impl->getLayersShapes(netInputShapes, inOutShapes);

    for(Impl::LayersShapesMap::const_iterator it = inOutShapes.begin();
        it != inOutShapes.end(); it++)
    {
        layersIds.push_back(it->first);
        inLayersShapes.push_back(it->second.in);
        outLayersShapes.push_back(it->second.out);
    }
}

void Net::getLayersShapes(const MatShape& netInputShape,
                          std::vector<int>& layerIds,
                          std::vector<ShapesVec>& inLayersShapes,
                          std::vector<ShapesVec>& outLayersShapes) const
{
    getLayersShapes(ShapesVec(1, netInputShape),
                    layerIds, inLayersShapes, outLayersShapes);
}

void Net::getLayerShapes(const MatShape& netInputShape,
                         const int layerId,
                         ShapesVec& inLayerShapes,
                         ShapesVec& outLayerShapes) const
{
    getLayerShapes(ShapesVec(1, netInputShape),
                   layerId, inLayerShapes, outLayerShapes);

}

void Net::getLayerShapes(const ShapesVec& netInputShapes,
                    const int layerId,
                    ShapesVec& inLayerShapes,
                    ShapesVec& outLayerShapes) const
{
    LayerShapes shapes;
    impl->getLayerShapes(netInputShapes, layerId, shapes);
    inLayerShapes = shapes.in;
    outLayerShapes = shapes.out;
}

int64 Net::getFLOPS(const std::vector<MatShape>& netInputShapes) const
{
    CV_TRACE_FUNCTION();

    int64 flops = 0;
    std::vector<int> ids;
    std::vector<std::vector<MatShape> > inShapes, outShapes;
    getLayersShapes(netInputShapes, ids, inShapes, outShapes);
    CV_Assert(inShapes.size() == outShapes.size());
    CV_Assert(inShapes.size() == ids.size());

    for(int i = 0; i < ids.size(); i++)
    {
        flops += impl->layers[ids[i]].getLayerInstance()->getFLOPS(inShapes[i],
                                                                   outShapes[i]);
    }

    return flops;
}

int64 Net::getFLOPS(const MatShape& netInputShape) const
{
    return getFLOPS(std::vector<MatShape>(1, netInputShape));
}

int64 Net::getFLOPS(const int layerId,
              const std::vector<MatShape>& netInputShapes) const
{
    Impl::MapIdToLayerData::iterator layer = impl->layers.find(layerId);
    CV_Assert(layer != impl->layers.end());

    LayerShapes shapes;
    impl->getLayerShapes(netInputShapes, layerId, shapes);

    return layer->second.getLayerInstance()->getFLOPS(shapes.in, shapes.out);
}

int64 Net::getFLOPS(const int layerId,
              const MatShape& netInputShape) const
{
    return getFLOPS(layerId, std::vector<MatShape>(1, netInputShape));
}

void Net::getLayerTypes(std::vector<String>& layersTypes) const
{
    layersTypes.clear();

    std::map<String, int> layers;
    for (Impl::MapIdToLayerData::iterator it = impl->layers.begin();
         it != impl->layers.end(); it++)
    {
        if (layers.find(it->second.type) == layers.end())
            layers[it->second.type] = 0;
        layers[it->second.type]++;
    }

    for (std::map<String, int>::iterator it = layers.begin();
         it != layers.end(); it++)
    {
        layersTypes.push_back(it->first);
    }
}

int Net::getLayersCount(const String& layerType) const
{
    int count = 0;
    for (Impl::MapIdToLayerData::iterator it = impl->layers.begin();
         it != impl->layers.end(); it++)
    {
        if (it->second.type == layerType)
            count++;
    }
    return count;
}

void Net::getMemoryConsumption(const int layerId,
                               const std::vector<MatShape>& netInputShapes,
                               size_t& weights, size_t& blobs) const
{
    CV_TRACE_FUNCTION();

    Impl::MapIdToLayerData::iterator layer = impl->layers.find(layerId);
    CV_Assert(layer != impl->layers.end());

    weights = blobs = 0;

    for(int i = 0; i < layer->second.params.blobs.size(); i++)
    {
        const Mat& weightsBlob = layer->second.params.blobs[i];
        weights += weightsBlob.total()*weightsBlob.elemSize();
    }

    ShapesVec inLayerShapes, outLayerShapes;
    getLayerShapes(netInputShapes, layerId, inLayerShapes, outLayerShapes);
    size_t elemSize = (impl->netWasQuantized) ? sizeof(char) : sizeof(float);
    for(int i = 0; i < outLayerShapes.size(); i++)
    {
        blobs += total(outLayerShapes[i]) * elemSize;
    }
}

void Net::getMemoryConsumption(const std::vector<MatShape>& netInputShapes,
                               size_t& weights, size_t& blobs) const
{
    CV_TRACE_FUNCTION();

    std::vector<int> layerIds;
    std::vector<size_t> w, b;
    getMemoryConsumption(netInputShapes, layerIds, w, b);

    weights = blobs = 0;
    for(int i = 0; i < layerIds.size(); i++)
    {
        weights += w[i];
        blobs += b[i];
    }
}

void Net::getMemoryConsumption(const int layerId,
                               const MatShape& netInputShape,
                               size_t& weights, size_t& blobs) const
{
    getMemoryConsumption(layerId, std::vector<MatShape>(1, netInputShape),
                         weights, blobs);
}

void Net::getMemoryConsumption(const MatShape& netInputShape,
                               size_t& weights, size_t& blobs) const
{
    getMemoryConsumption(std::vector<MatShape>(1, netInputShape),
                         weights, blobs);
}

void Net::getMemoryConsumption(const std::vector<MatShape>& netInputShapes,
                                  std::vector<int>& layerIds, std::vector<size_t>& weights,
                                  std::vector<size_t>& blobs) const
{
    CV_TRACE_FUNCTION();

    layerIds.clear();
    weights.clear();
    blobs.clear();

    std::vector<std::vector<MatShape> > inLayerShapes, outLayerShapes;

    getLayersShapes(netInputShapes, layerIds, inLayerShapes, outLayerShapes);
    size_t elemSize = (impl->netWasQuantized) ? sizeof(char) : sizeof(float);
    for(int i = 0; i < layerIds.size(); i++)
    {
        int w = 0, b = 0;
        Impl::MapIdToLayerData::iterator layer = impl->layers.find(layerIds[i]);
        CV_Assert(layer != impl->layers.end());

        for(int j = 0; j < layer->second.params.blobs.size(); j++)
        {
            const Mat& weightsBlob = layer->second.params.blobs[j];
            w += weightsBlob.total()*weightsBlob.elemSize();
        }

        for(int j = 0; j < outLayerShapes[i].size(); j++)
        {
            b += total(outLayerShapes[i][j]) * elemSize;
        }

        weights.push_back(w);
        blobs.push_back(b);
    }
}

void Net::getMemoryConsumption(const MatShape& netInputShape, std::vector<int>& layerIds,
                               std::vector<size_t>& weights, std::vector<size_t>& blobs) const
{
    getMemoryConsumption(std::vector<MatShape>(1, netInputShape), layerIds,
                         weights, blobs);
}

void Net::enableFusion(bool fusion)
{
    if( impl->fusion != fusion )
    {
        impl->fusion = fusion;
        impl->clear();
    }
}

void Net::setHalideScheduler(const String& scheduler)
{
    CV_TRACE_FUNCTION();
    CV_TRACE_ARG_VALUE(scheduler, "scheduler", scheduler.c_str());

    impl->halideConfigFile = scheduler;
}

int64 Net::getPerfProfile(std::vector<double>& timings)
{
    timings = std::vector<double>(impl->layersTimings.begin() + 1, impl->layersTimings.end());
    int64 total = (int64)std::accumulate(timings.begin(), timings.end(), 0.0);
    return total;
}

//////////////////////////////////////////////////////////////////////////

Layer::Layer() { preferableTarget = DNN_TARGET_CPU; }

Layer::Layer(const LayerParams &params)
    : blobs(params.blobs), name(params.name), type(params.type)
{
    preferableTarget = DNN_TARGET_CPU;
}

void Layer::setParamsFrom(const LayerParams &params)
{
    blobs = params.blobs;
    name = params.name;
    type = params.type;
}

int Layer::inputNameToIndex(String)
{
    return -1;
}

int Layer::outputNameToIndex(const String&)
{
    return 0;
}

bool Layer::supportBackend(int backendId)
{
    return backendId == DNN_BACKEND_OPENCV;
}

Ptr<BackendNode> Layer::initCUDA(
    void*,
    const std::vector<Ptr<BackendWrapper>>&,
    const std::vector<Ptr<BackendWrapper>>&)
{
    CV_Error(Error::StsNotImplemented, "CUDA pipeline of " + type +
                                       " layers is not defined.");
    return Ptr<BackendNode>();
}

Ptr<BackendNode> Layer::initVkCom(const std::vector<Ptr<BackendWrapper> > &)
{
    CV_Error(Error::StsNotImplemented, "VkCom pipeline of " + type +
                                       " layers is not defined.");
    return Ptr<BackendNode>();
}

Ptr<BackendNode> Layer::initHalide(const std::vector<Ptr<BackendWrapper> > &)
{
    CV_Error(Error::StsNotImplemented, "Halide pipeline of " + type +
                                       " layers is not defined.");
    return Ptr<BackendNode>();
}

Ptr<BackendNode> Layer::initInfEngine(const std::vector<Ptr<BackendWrapper> > &)
{
    CV_Error(Error::StsNotImplemented, "Inference Engine pipeline of " + type +
                                       " layers is not defined.");
    return Ptr<BackendNode>();
}

Ptr<BackendNode> Layer::initNgraph(const std::vector<Ptr<BackendWrapper> > & inputs, const std::vector<Ptr<BackendNode> >& nodes)
{
    CV_Error(Error::StsNotImplemented, "Inference Engine pipeline of " + type +
                                       " layers is not defined.");
    return Ptr<BackendNode>();
}

void Layer::applyHalideScheduler(Ptr<BackendNode>& node, const std::vector<Mat*> &inputs,
                                 const std::vector<Mat> &outputs, int targetId) const
{
#ifdef  HAVE_HALIDE
    CV_TRACE_FUNCTION();

    Halide::Var x("x"), y("y"), c("c"), n("n"), co("co"), ci("ci"),
                xo("xo"), xi("xi"), yo("yo"), yi("yi"), tile("tile");
    Halide::Func& top = node.dynamicCast<HalideBackendNode>()->funcs.back();

    int outW, outH, outC, outN;
    getCanonicalSize(outputs[0].size, &outW, &outH, &outC, &outN);

    if (targetId == DNN_TARGET_CPU)
    {
        if (outW == 1 && outH == 1)
        {
            if (outC + outN == 1)
                return;

            if (outC > 8)
              top.split(c, co, ci, 8)
                 .fuse(x, y, tile).fuse(co, tile, tile).fuse(n, tile, tile)
                 .parallel(tile)
                 .vectorize(ci, 8);
            else
              top.fuse(x, y, tile).fuse(c, tile, tile).fuse(n, tile, tile)
                 .parallel(tile);
        }
        else
        {
            if (outH > 2)
            {
                top.reorder(x, c, y)
                   .split(y, yo, yi, 2)
                   .fuse(yo, n, tile)
                   .parallel(tile)
                   .unroll(yi)
                   .vectorize(x, outW >= 16 ? 16 : outW);
            }
        }
    }
    else if (targetId == DNN_TARGET_OPENCL)
    {
        if (outW == 1 && outH == 1)
        {
            int c_split = outC > 8 ? (outC > 16 ? 8 : 4) : outC;
            top.split(c, co, ci, c_split)
               .fuse(x, y, tile).fuse(co, tile, tile).fuse(n, tile, tile)
               .gpu_blocks(tile)
               .gpu_threads(ci);
        }
        else
        {
            int x_split = outW > 8 ? (outW >= 32 ? 16 : 8) : outW;
            int y_split = outH > 8 ? (outH >= 32 ? 16 : 8) : outH;
            // Supported vectorization widths: 2, 3, 4, 8, 16
            int c_split = outC > 8 ? (outC > 16 ? 8 : 4) : std::min(4, outC);
            top.split(x, xo, xi, x_split).split(y, yo, yi, y_split)
               .split(c, co, ci, c_split)
               .gpu_blocks(xo, yo, co)
               .gpu_threads(xi, yi)
               .reorder(xi, yi, ci, xo, yo, co)
               .vectorize(ci);
        }
    }
    else
        CV_Error(Error::StsNotImplemented, "Unknown target identifier");
#endif  // HAVE_HALIDE
}

Ptr<BackendNode> Layer::tryAttach(const Ptr<BackendNode>& node)
{
    return Ptr<BackendNode>();
}

bool Layer::setActivation(const Ptr<ActivationLayer>&) { return false; }
bool Layer::tryFuse(Ptr<Layer>&) { return false; }
void Layer::getScaleShift(Mat& scale, Mat& shift) const
{
    scale = Mat();
    shift = Mat();
}

void Layer::getScaleZeropoint(float& scale, int& zeropoint) const
{
    scale = 1.f;
    zeropoint = 0;
}

void Layer::unsetAttached()
{
    setActivation(Ptr<ActivationLayer>());
}

template <typename T>
static void vecToPVec(const std::vector<T> &v, std::vector<T*> &pv)
{
    pv.resize(v.size());
    for (size_t i = 0; i < v.size(); i++)
        pv[i] = const_cast<T*>(&v[i]);
}

void Layer::finalize(const std::vector<Mat> &inputs, std::vector<Mat> &outputs)
{
    CV_TRACE_FUNCTION();
    this->finalize((InputArrayOfArrays)inputs, (OutputArrayOfArrays)outputs);
}

void Layer::finalize(const std::vector<Mat*> &input, std::vector<Mat> &output)
{
    CV_UNUSED(input);CV_UNUSED(output);
}

void Layer::finalize(InputArrayOfArrays inputs_arr, OutputArrayOfArrays outputs_arr)
{
    CV_TRACE_FUNCTION();
    std::vector<Mat> inputs, outputs;
    inputs_arr.getMatVector(inputs);
    outputs_arr.getMatVector(outputs);

    std::vector<Mat*> inputsp;
    vecToPVec(inputs, inputsp);
    this->finalize(inputsp, outputs);
}

std::vector<Mat> Layer::finalize(const std::vector<Mat> &inputs)
{
    CV_TRACE_FUNCTION();

    std::vector<Mat> outputs;
    this->finalize(inputs, outputs);
    return outputs;
}

void Layer::forward(std::vector<Mat*> &input, std::vector<Mat> &output, std::vector<Mat> &internals)
{
    // We kept this method for compatibility. DNN calls it now only to support users' implementations.
}

void Layer::forward(InputArrayOfArrays inputs_arr, OutputArrayOfArrays outputs_arr, OutputArrayOfArrays internals_arr)
{
    CV_TRACE_FUNCTION();
    CV_TRACE_ARG_VALUE(name, "name", name.c_str());

    Layer::forward_fallback(inputs_arr, outputs_arr, internals_arr);
}

void Layer::forward_fallback(InputArrayOfArrays inputs_arr, OutputArrayOfArrays outputs_arr, OutputArrayOfArrays internals_arr)
{
    CV_TRACE_FUNCTION();
    CV_TRACE_ARG_VALUE(name, "name", name.c_str());

    if (preferableTarget == DNN_TARGET_OPENCL_FP16 && inputs_arr.depth() == CV_16S)
    {
        std::vector<UMat> inputs;
        std::vector<UMat> outputs;
        std::vector<UMat> internals;

        std::vector<UMat> orig_inputs;
        std::vector<UMat> orig_outputs;
        std::vector<UMat> orig_internals;

        inputs_arr.getUMatVector(orig_inputs);
        outputs_arr.getUMatVector(orig_outputs);
        internals_arr.getUMatVector(orig_internals);

        inputs.resize(orig_inputs.size());
        for (size_t i = 0; i < orig_inputs.size(); i++)
            convertFp16(orig_inputs[i], inputs[i]);

        outputs.resize(orig_outputs.size());
        for (size_t i = 0; i < orig_outputs.size(); i++)
            outputs[i].create(shape(orig_outputs[i]), CV_32F);

        internals.resize(orig_internals.size());
        for (size_t i = 0; i < orig_internals.size(); i++)
            internals[i].create(shape(orig_internals[i]), CV_32F);

        forward(inputs, outputs, internals);

        for (size_t i = 0; i < outputs.size(); i++)
            convertFp16(outputs[i], orig_outputs[i]);

        // sync results back
        outputs_arr.assign(orig_outputs);
        internals_arr.assign(orig_internals);
        return;
    }
    std::vector<Mat> inpvec;
    std::vector<Mat> outputs;
    std::vector<Mat> internals;

    inputs_arr.getMatVector(inpvec);
    outputs_arr.getMatVector(outputs);
    internals_arr.getMatVector(internals);

    std::vector<Mat*> inputs(inpvec.size());
    for (int i = 0; i < inpvec.size(); i++)
        inputs[i] = &inpvec[i];

    this->forward(inputs, outputs, internals);

    // sync results back
    outputs_arr.assign(outputs);
    internals_arr.assign(internals);
}

void Layer::run(const std::vector<Mat> &inputs, std::vector<Mat> &outputs, std::vector<Mat> &internals)
{
    CV_TRACE_FUNCTION();

    this->finalize(inputs, outputs);
    this->forward(inputs, outputs, internals);
}

bool Layer::tryQuantize(const std::vector<std::vector<float> > &scales,
                        const std::vector<std::vector<int> > &zeropoints, LayerParams& params)
{
    return false;
}

Layer::~Layer() {}

bool Layer::getMemoryShapes(const std::vector<MatShape> &inputs,
                            const int requiredOutputs,
                            std::vector<MatShape> &outputs,
                            std::vector<MatShape> &internals) const
{
    CV_Assert(inputs.size());
    outputs.assign(std::max(requiredOutputs, (int)inputs.size()), inputs[0]);
    return false;
}

bool Layer::updateMemoryShapes(const std::vector<MatShape> &inputs)
{
    return true;
}
//////////////////////////////////////////////////////////////////////////

Mutex& getLayerFactoryMutex()
{
    static Mutex* volatile instance = NULL;
    if (instance == NULL)
    {
        cv::AutoLock lock(getInitializationMutex());
        if (instance == NULL)
            instance = new Mutex();
    }
    return *instance;
}

static LayerFactory_Impl& getLayerFactoryImpl_()
{
    static LayerFactory_Impl impl;
    return impl;
}

LayerFactory_Impl& getLayerFactoryImpl()
{
    static LayerFactory_Impl* volatile instance = NULL;
    if (instance == NULL)
    {
        cv::AutoLock lock(getLayerFactoryMutex());
        if (instance == NULL)
        {
            instance = &getLayerFactoryImpl_();
            initializeLayerFactory();
        }
    }
    return *instance;
}

void LayerFactory::registerLayer(const String &type, Constructor constructor)
{
    CV_TRACE_FUNCTION();
    CV_TRACE_ARG_VALUE(type, "type", type.c_str());

    cv::AutoLock lock(getLayerFactoryMutex());
    LayerFactory_Impl::iterator it = getLayerFactoryImpl().find(type);

    if (it != getLayerFactoryImpl().end())
    {
        if (it->second.back() == constructor)
            CV_Error(cv::Error::StsBadArg, "Layer \"" + type + "\" already was registered");
        it->second.push_back(constructor);
    }
    getLayerFactoryImpl().insert(std::make_pair(type, std::vector<Constructor>(1, constructor)));
}

void LayerFactory::unregisterLayer(const String &type)
{
    CV_TRACE_FUNCTION();
    CV_TRACE_ARG_VALUE(type, "type", type.c_str());

    cv::AutoLock lock(getLayerFactoryMutex());

    LayerFactory_Impl::iterator it = getLayerFactoryImpl().find(type);
    if (it != getLayerFactoryImpl().end())
    {
        if (it->second.size() > 1)
            it->second.pop_back();
        else
            getLayerFactoryImpl().erase(it);
    }
}

Ptr<Layer> LayerFactory::createLayerInstance(const String &type, LayerParams& params)
{
    CV_TRACE_FUNCTION();
    CV_TRACE_ARG_VALUE(type, "type", type.c_str());

    cv::AutoLock lock(getLayerFactoryMutex());
    LayerFactory_Impl::const_iterator it = getLayerFactoryImpl().find(type);

    if (it != getLayerFactoryImpl().end())
    {
        CV_Assert(!it->second.empty());
        return it->second.back()(params);
    }
    else
    {
        return Ptr<Layer>(); //NULL
    }
}

BackendNode::BackendNode(int backendId) : backendId(backendId) {}

BackendNode::~BackendNode() {};

BackendWrapper::BackendWrapper(int backendId, int targetId)
    : backendId(backendId), targetId(targetId) {}

BackendWrapper::BackendWrapper(int targetId, const cv::Mat& m)
{
    CV_Error(Error::StsNotImplemented,
             "Constructor of backend wrapper must be implemented");
}

BackendWrapper::BackendWrapper(const Ptr<BackendWrapper>& base, const MatShape& shape)
{
    CV_Error(Error::StsNotImplemented,
             "Constructor of backend wrapper must be implemented");
}

BackendWrapper::~BackendWrapper() {}

Net readNet(const String& _model, const String& _config, const String& _framework)
{
    String framework = toLowerCase(_framework);
    String model = _model;
    String config = _config;
    const std::string modelExt = model.substr(model.rfind('.') + 1);
    const std::string configExt = config.substr(config.rfind('.') + 1);
    if (framework == "caffe" || modelExt == "caffemodel" || configExt == "caffemodel" ||
                                modelExt == "prototxt" || configExt == "prototxt")
    {
        if (modelExt == "prototxt" || configExt == "caffemodel")
            std::swap(model, config);
        return readNetFromCaffe(config, model);
    }
    if (framework == "tensorflow" || modelExt == "pb" || configExt == "pb" ||
                                     modelExt == "pbtxt" || configExt == "pbtxt")
    {
        if (modelExt == "pbtxt" || configExt == "pb")
            std::swap(model, config);
        return readNetFromTensorflow(model, config);
    }
    if (framework == "torch" || modelExt == "t7" || modelExt == "net" ||
                                configExt == "t7" || configExt == "net")
    {
        return readNetFromTorch(model.empty() ? config : model);
    }
    if (framework == "darknet" || modelExt == "weights" || configExt == "weights" ||
                                  modelExt == "cfg" || configExt == "cfg")
    {
        if (modelExt == "cfg" || configExt == "weights")
            std::swap(model, config);
        return readNetFromDarknet(config, model);
    }
    if (framework == "dldt" || modelExt == "bin" || configExt == "bin" ||
                               modelExt == "xml" || configExt == "xml")
    {
        if (modelExt == "xml" || configExt == "bin")
            std::swap(model, config);
        return readNetFromModelOptimizer(config, model);
    }
    if (framework == "onnx" || modelExt == "onnx")
    {
        return readNetFromONNX(model);
    }
    CV_Error(Error::StsError, "Cannot determine an origin framework of files: " +
                                      model + (config.empty() ? "" : ", " + config));
}

Net readNet(const String& _framework, const std::vector<uchar>& bufferModel,
            const std::vector<uchar>& bufferConfig)
{
    String framework = toLowerCase(_framework);
    if (framework == "caffe")
        return readNetFromCaffe(bufferConfig, bufferModel);
    else if (framework == "tensorflow")
        return readNetFromTensorflow(bufferModel, bufferConfig);
    else if (framework == "darknet")
        return readNetFromDarknet(bufferConfig, bufferModel);
    else if (framework == "torch")
        CV_Error(Error::StsNotImplemented, "Reading Torch models from buffers");
    else if (framework == "dldt")
        return readNetFromModelOptimizer(bufferConfig, bufferModel);
    CV_Error(Error::StsError, "Cannot determine an origin framework with a name " + framework);
}

Net readNetFromModelOptimizer(const String &xml, const String &bin)
{
    return Net::readFromModelOptimizer(xml, bin);
}

Net readNetFromModelOptimizer(const std::vector<uchar>& bufferCfg, const std::vector<uchar>& bufferModel)
{
    return Net::readFromModelOptimizer(bufferCfg, bufferModel);
}

Net readNetFromModelOptimizer(
        const uchar* bufferModelConfigPtr, size_t bufferModelConfigSize,
        const uchar* bufferWeightsPtr, size_t bufferWeightsSize
)
{
    return Net::readFromModelOptimizer(
        bufferModelConfigPtr, bufferModelConfigSize,
        bufferWeightsPtr, bufferWeightsSize
    );
}

CV__DNN_INLINE_NS_END
}} // namespace
