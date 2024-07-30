/**
 * Copyright © 2014 Red Hat, Inc
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <QMetaType>

// This file contains helpers, constants and other minor nice things from flatpak internals.

/* See flatpak-metadata(5) */

#define FLATPAK_METADATA_GROUP_APPLICATION "Application"
#define FLATPAK_METADATA_GROUP_RUNTIME "Runtime"
#define FLATPAK_METADATA_KEY_COMMAND "command"
#define FLATPAK_METADATA_KEY_NAME "name"
#define FLATPAK_METADATA_KEY_REQUIRED_FLATPAK "required-flatpak"
#define FLATPAK_METADATA_KEY_RUNTIME "runtime"
#define FLATPAK_METADATA_KEY_SDK "sdk"
#define FLATPAK_METADATA_KEY_TAGS "tags"

#define FLATPAK_METADATA_GROUP_CONTEXT "Context"
#define FLATPAK_METADATA_KEY_SHARED "shared"
#define FLATPAK_METADATA_KEY_SOCKETS "sockets"
#define FLATPAK_METADATA_KEY_FILESYSTEMS "filesystems"
#define FLATPAK_METADATA_KEY_PERSISTENT "persistent"
#define FLATPAK_METADATA_KEY_DEVICES "devices"
#define FLATPAK_METADATA_KEY_FEATURES "features"
#define FLATPAK_METADATA_KEY_UNSET_ENVIRONMENT "unset-environment"

#define FLATPAK_METADATA_GROUP_INSTANCE "Instance"
#define FLATPAK_METADATA_KEY_INSTANCE_PATH "instance-path"
#define FLATPAK_METADATA_KEY_INSTANCE_ID "instance-id"
#define FLATPAK_METADATA_KEY_ORIGINAL_APP_PATH "original-app-path"
#define FLATPAK_METADATA_KEY_APP_PATH "app-path"
#define FLATPAK_METADATA_KEY_APP_COMMIT "app-commit"
#define FLATPAK_METADATA_KEY_APP_EXTENSIONS "app-extensions"
#define FLATPAK_METADATA_KEY_ARCH "arch"
#define FLATPAK_METADATA_KEY_BRANCH "branch"
#define FLATPAK_METADATA_KEY_FLATPAK_VERSION "flatpak-version"
#define FLATPAK_METADATA_KEY_ORIGINAL_RUNTIME_PATH "original-runtime-path"
#define FLATPAK_METADATA_KEY_RUNTIME_PATH "runtime-path"
#define FLATPAK_METADATA_KEY_RUNTIME_COMMIT "runtime-commit"
#define FLATPAK_METADATA_KEY_RUNTIME_EXTENSIONS "runtime-extensions"
#define FLATPAK_METADATA_KEY_SESSION_BUS_PROXY "session-bus-proxy"
#define FLATPAK_METADATA_KEY_SYSTEM_BUS_PROXY "system-bus-proxy"
#define FLATPAK_METADATA_KEY_EXTRA_ARGS "extra-args"
#define FLATPAK_METADATA_KEY_SANDBOX "sandbox"
#define FLATPAK_METADATA_KEY_BUILD "build"
#define FLATPAK_METADATA_KEY_DEVEL "devel"

#define FLATPAK_METADATA_GROUP_SESSION_BUS_POLICY "Session Bus Policy"
#define FLATPAK_METADATA_GROUP_SYSTEM_BUS_POLICY "System Bus Policy"
#define FLATPAK_METADATA_GROUP_PREFIX_POLICY "Policy "
#define FLATPAK_METADATA_GROUP_ENVIRONMENT "Environment"

#define FLATPAK_METADATA_GROUP_PREFIX_EXTENSION "Extension "
#define FLATPAK_METADATA_KEY_ADD_LD_PATH "add-ld-path"
#define FLATPAK_METADATA_KEY_AUTODELETE "autodelete"
#define FLATPAK_METADATA_KEY_DIRECTORY "directory"
#define FLATPAK_METADATA_KEY_DOWNLOAD_IF "download-if"
#define FLATPAK_METADATA_KEY_ENABLE_IF "enable-if"
#define FLATPAK_METADATA_KEY_AUTOPRUNE_UNLESS "autoprune-unless"
#define FLATPAK_METADATA_KEY_MERGE_DIRS "merge-dirs"
#define FLATPAK_METADATA_KEY_NO_AUTODOWNLOAD "no-autodownload"
#define FLATPAK_METADATA_KEY_SUBDIRECTORIES "subdirectories"
#define FLATPAK_METADATA_KEY_SUBDIRECTORY_SUFFIX "subdirectory-suffix"
#define FLATPAK_METADATA_KEY_LOCALE_SUBSET "locale-subset"
#define FLATPAK_METADATA_KEY_VERSION "version"
#define FLATPAK_METADATA_KEY_VERSIONS "versions"

#define FLATPAK_METADATA_KEY_COLLECTION_ID "collection-id"

#define FLATPAK_METADATA_GROUP_EXTRA_DATA "Extra Data"
#define FLATPAK_METADATA_KEY_EXTRA_DATA_CHECKSUM "checksum"
#define FLATPAK_METADATA_KEY_EXTRA_DATA_INSTALLED_SIZE "installed-size"
#define FLATPAK_METADATA_KEY_EXTRA_DATA_NAME "name"
#define FLATPAK_METADATA_KEY_EXTRA_DATA_SIZE "size"
#define FLATPAK_METADATA_KEY_EXTRA_DATA_URI "uri"
#define FLATPAK_METADATA_KEY_NO_RUNTIME "NoRuntime"

#define FLATPAK_METADATA_GROUP_EXTENSION_OF "ExtensionOf"
#define FLATPAK_METADATA_KEY_PRIORITY "priority"
#define FLATPAK_METADATA_KEY_REF "ref"
#define FLATPAK_METADATA_KEY_TAG "tag"

/* clang-format off */

typedef enum {
    FLATPAK_POLICY_NONE,
    FLATPAK_POLICY_SEE,
    FLATPAK_POLICY_TALK,
    FLATPAK_POLICY_OWN
} FlatpakPolicy;

Q_DECLARE_METATYPE(FlatpakPolicy)

typedef enum {
    FLATPAK_CONTEXT_SHARED_NETWORK   = 1 << 0,
    FLATPAK_CONTEXT_SHARED_IPC       = 1 << 1,
} FlatpakContextShares;

typedef enum {
    FLATPAK_CONTEXT_SOCKET_X11         = 1 << 0,
    FLATPAK_CONTEXT_SOCKET_WAYLAND     = 1 << 1,
    FLATPAK_CONTEXT_SOCKET_PULSEAUDIO  = 1 << 2,
    FLATPAK_CONTEXT_SOCKET_SESSION_BUS = 1 << 3,
    FLATPAK_CONTEXT_SOCKET_SYSTEM_BUS  = 1 << 4,
    FLATPAK_CONTEXT_SOCKET_FALLBACK_X11 = 1 << 5, /* For backwards compat, also set SOCKET_X11 */
    FLATPAK_CONTEXT_SOCKET_SSH_AUTH    = 1 << 6,
    FLATPAK_CONTEXT_SOCKET_PCSC        = 1 << 7,
    FLATPAK_CONTEXT_SOCKET_CUPS        = 1 << 8,
    FLATPAK_CONTEXT_SOCKET_GPG_AGENT   = 1 << 9,
} FlatpakContextSockets;

typedef enum {
    FLATPAK_CONTEXT_DEVICE_DRI         = 1 << 0,
    FLATPAK_CONTEXT_DEVICE_ALL         = 1 << 1,
    FLATPAK_CONTEXT_DEVICE_KVM         = 1 << 2,
    FLATPAK_CONTEXT_DEVICE_SHM         = 1 << 3,
} FlatpakContextDevices;

typedef enum {
    FLATPAK_CONTEXT_FEATURE_DEVEL        = 1 << 0,
    FLATPAK_CONTEXT_FEATURE_MULTIARCH    = 1 << 1,
    FLATPAK_CONTEXT_FEATURE_BLUETOOTH    = 1 << 2,
    FLATPAK_CONTEXT_FEATURE_CANBUS       = 1 << 3,
    FLATPAK_CONTEXT_FEATURE_PER_APP_DEV_SHM = 1 << 4,
} FlatpakContextFeatures;

/* clang-format on */

namespace FlatpakStrings
{

extern const char *flatpak_policy[];
extern const char *flatpak_context_sockets[];
extern const char *flatpak_context_devices[];
extern const char *flatpak_context_features[];
extern const char *flatpak_context_shares[];
extern const char *flatpak_context_special_filesystems[];

}
