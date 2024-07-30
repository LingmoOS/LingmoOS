# KAuth

Execute actions as privileged user

## Introduction

KAuth provides a convenient, system-integrated way to offload actions that need
to be performed as a privileged user (root, for example) to small (hopefully
secure) helper utilities.

## Usage

If you are using CMake, you need to have

    find_package(KF6Auth NO_MODULE)

(or find KF6 with the Auth component) in your CMakeLists.txt file. You need
to link to KF6::AuthCore.

Executing privileged actions typically involves having a minimal helper utility
that does the actual work, and calling that utility with escalated privileges if
the user has permission to do so (often requiring the user to enter appropriate
credentials, like entering a password).

Therefore, use of the KAuth library is in two parts.  In the main part of your
code, you use KAuth::Action (and specifically KAuth::Action::execute()) when you
need to do something privileged, like write to a file normally only writable by
root.

The code that actually performs that action, such as writing to a file, needs to
be placed in the slot of a helper QObject class, which should use the methods of
KAuth::HelperSupport and be compiled into an executable.  You will also want to
use the `kauth_install_helper_files` and `kauth_install_actions` macros in your
CMakeLists.txt.

See <https://develop.kde.org/docs/features/kauth/> for a
detailed tutorial on how to use KAuth.

