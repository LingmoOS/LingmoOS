#pragma once

#include <string>

namespace dm {

// A VT in KD_GRAPHICS that comes to the front keeps the previous master's
// frame instead of showing the console.
bool setGraphicsMode(int vt);

// Undoes setGraphicsMode when nobody took the VT: in front, KD_GRAPHICS with
// VT_AUTO makes the kernel ignore every VT switch (change_console).
void restoreTextMode(int vt);

// logind has taken the VT for a session's controller (VT_PROCESS).
bool isControlled(int vt);

int activeVt();

// logind's ActivateSession: the VT switch plus the DRM and input handover.
bool activateSession(const std::string &sessionId);

} // namespace dm
