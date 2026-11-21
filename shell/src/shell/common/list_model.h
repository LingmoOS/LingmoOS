// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include "base/signal.h"

namespace Lingmo::Shell {

// What a list model tells its views, row by row, so a view keeps the items of
// rows that stay and can animate the ones that come, go or move. Each signal
// follows the change it reports: the model already holds the new rows.
struct ListModelSignals {
    // first, count
    base::Signal<int, int> rowsInserted;
    // first, count: the rows that were there
    base::Signal<int, int> rowsRemoved;
    // from, to: the row's index before and after the move
    base::Signal<int, int> rowMoved;
    // first, last: rows whose values changed in place
    base::Signal<int, int> dataChanged;
    // Everything may have changed.
    base::Signal<> modelReset;
};

} // namespace Lingmo::Shell

