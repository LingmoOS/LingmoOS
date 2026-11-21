// Copyright 2017 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.sommelier file.
//
// Upstream: chromiumos/platform2/vm_tools/sommelier/sommelier.cc (selection handlers)
// Revision: aa8a57908bdcff297e229679dc3240b1b108f57e

#pragma once

#include <map>
#include <string>

#include "clipboard/selection.h"

namespace Lingmo::Clipboard {

// A source whose payloads the compositor holds, written out without blocking.
class StoredSource : public DataSource
{
public:
    std::map<std::string, std::string> payloads;

    void send(const std::string &mime, int fd) override;
};

} // namespace Lingmo::Clipboard
