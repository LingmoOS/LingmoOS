// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
//
// Copyright (C) 2021 Intel Corporation

#ifndef GAPI_STREAMING_ONEVPL_ENGINE_ENGINE_SESSION_HPP
#define GAPI_STREAMING_ONEVPL_ENGINE_ENGINE_SESSION_HPP

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "opencv2/gapi/own/exports.hpp" // GAPI_EXPORTS

#ifdef HAVE_ONEVPL
#include <vpl/mfxvideo.h>

namespace cv {
namespace gapi {
namespace wip {
namespace onevpl {

// GAPI_EXPORTS for tests
struct GAPI_EXPORTS DecoderParams {
    mfxBitstream stream;
    mfxVideoParam param;
};

struct GAPI_EXPORTS EngineSession {
    mfxSession session;
    mfxBitstream stream;
    mfxSyncPoint sync;
    mfxStatus last_status;

    EngineSession(mfxSession sess, mfxBitstream&& str);
    std::string error_code_to_str() const;
    virtual ~EngineSession();
};
} // namespace onevpl
} // namespace wip
} // namespace gapi
} // namespace cv

#endif // HAVE_ONEVPL
#endif // GAPI_STREAMING_ONEVPL_ENGINE_ENGINE_SESSION_HPP
