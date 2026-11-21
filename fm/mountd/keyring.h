// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "backend.h"
#include "remoteurl.h"

// Server passwords in the user's keyring through the Secret Service. Blocking, for the
// threads that connect; a locked keyring is passed over rather than prompted for.
namespace Keyring {

// The user, domain and password saved for url's protocol, server and port, if any. A user
// named in url (or credentials) narrows the search to that account.
bool find(const RemoteUrl &url, Credentials *credentials);
void store(const RemoteUrl &url, const Credentials &credentials);

} // namespace Keyring
