// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
//
// Copyright (C) 2021 Intel Corporation

#ifndef GAPI_STREAMING_ONEVPL_ONEVPL_UTILS_HPP
#define GAPI_STREAMING_ONEVPL_ONEVPL_UTILS_HPP

#ifdef HAVE_ONEVPL
#if (MFX_VERSION >= 2000)
#include <vpl/mfxdispatcher.h>
#endif // MFX_VERSION

#include <vpl/mfx.h>
#include <vpl/mfxvideo.h>

#include <map>
#include <string>

#include <opencv2/gapi/streaming/onevpl/cfg_params.hpp>


namespace cv {
namespace gapi {
namespace wip {
namespace onevpl {

inline std::string mfxstatus_to_string(mfxStatus) {
    return "UNKNOWN";
}

} // namespace onevpl
} // namespace wip
} // namespace gapi
} // namespace cv
#endif // HAVE_ONEVPL
#endif // GAPI_STREAMING_ONEVPL_ONEVPL_UTILS_HPP
