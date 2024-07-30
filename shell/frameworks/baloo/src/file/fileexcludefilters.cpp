/*
    This file is part of the KDE Project
    SPDX-FileCopyrightText: 2008-2010 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fileexcludefilters.h"

namespace
{
const char* const s_defaultFileExcludeFilters[] = {
    // tmp files
    "*~",
    "*.part",

    // temporary build files
    "*.o",
    "*.la",
    "*.lo",
    "*.loT",
    "*.moc",
    "moc_*.cpp",
    "qrc_*.cpp",
    "ui_*.h",
    "cmake_install.cmake",
    "CMakeCache.txt",
    "CTestTestfile.cmake",
    "libtool",
    "config.status",
    "confdefs.h",
    "autom4te",
    "conftest",
    "confstat",
    "Makefile.am",
    "*.gcode", // CNC machine/3D printer toolpath files
    ".ninja_deps",
    ".ninja_log",
    "build.ninja",

    // misc
    "*.csproj",
    "*.m4",
    "*.rej",
    "*.gmo",
    "*.pc",
    "*.omf",
    "*.aux",
    "*.tmp",
    "*.po",
    "*.vm*",
    "*.nvram",
    "*.rcore",
    "*.swp",
    "*.swap",
    "lzo",
    "litmain.sh",
    "*.orig",
    ".histfile.*",
    ".xsession-errors*",
    "*.map",
    "*.so",
    "*.a",
    "*.db",
    "*.qrc",
    "*.ini",
    "*.init",
    "*.img",    // typical extension for raw disk images
    "*.vdi",    // Virtualbox disk images
    "*.vbox*",  // Virtualbox VM files
    "vbox.log", // Virtualbox log files
    "*.qcow2",  // QEMU QCOW2 disk images
    "*.vmdk",   // VMware disk images
    "*.vhd",    // Hyper-V disk images
    "*.vhdx",   // Hyper-V disk images
    "*.sql",     // SQL database dumps
    "*.sql.gz",  // Compressed SQL database dumps
    "*.ytdl",    // youtube-dl temp files
    "*.tfstate*", // Terraform state files

    // Bytecode files
    "*.class", // Java
    "*.pyc",   // Python
    "*.pyo",   // More Python
    "*.elc",   // Emacs Lisp
    "*.qmlc",  // QML
    "*.jsc",   // Javascript

    // files known in bioinformatics containing huge amount of unindexable data
    "*.fastq",
    "*.fq",
    "*.gb",
    "*.fasta",
    "*.fna",
    "*.gbff",
    "*.faa",
    "*.fna",
    // end of list
    nullptr
};

const int s_defaultFileExcludeFiltersVersion = 9;

const char* const s_defaultFolderExcludeFilters[] = {
    "po",

    // VCS
    "CVS",
    ".svn",
    ".git",
    "_darcs",
    ".bzr",
    ".hg",

    // development
    "CMakeFiles",
    "CMakeTmp",
    "CMakeTmpQmake",
    ".moc",
    ".obj",
    ".pch",
    ".uic",
    ".npm",
    ".yarn",
    ".yarn-cache",
    "__pycache__",
    "node_modules",
    "node_packages",
    "nbproject",
    ".terraform",
    ".venv",
    "venv",

    //misc
    "core-dumps",
    "lost+found",

    // end of list
    nullptr
};

const int s_defaultFolderExcludeFiltersVersion = 4;

const char* const s_sourceCodeMimeTypes[] = {
    "text/css",
    "text/x-c++src",
    "text/x-c++hdr",
    "text/x-csrc",
    "text/x-chdr", // c header files
    "text/x-python",
    "text/x-assembly",
    "text/x-java",
    "text/x-objsrc",
    "text/x-ruby",
    "text/x-scheme",
    "text/x-pascal",
    "text/x-fortran",
    "text/x-erlang",
    "text/x-cmake",
    "text/x-lua",
    "text/x-yacc",
    "text/x-sed",
    "text/x-haskell",
    "text/x-copying", // COPYING files
    "text/x-readme", // README files
    "text/x-qml",
    "text/asp",
    "text/jsx",
    "text/csx",
    "text/vnd.trolltech.linguist",
    "application/x-awk",
    "application/x-cgi",
    "application/x-csh",
    "application/x-ipynb+json",
    "application/x-java",
    "application/x-javascript",
    "application/x-perl",
    "application/x-php",
    "application/x-python",
    "application/x-sh",
    "application/xml",
    "application/javascript",
    "application/json",
    "application/geo+json",
    "application/json-patch+json",
    "application/ld+json",
    "application/x-ipynb+json", // Jupyter notebooks

    // Not really source code, but inherited from text/plain
    "application/pgp-encrypted", // pgp encrypted, with or without ASCII Armor

    // end of list
    nullptr
};
const int s_sourceCodeMimeTypesVersion = 3;
}

QStringList Baloo::defaultExcludeFilterList()
{
    QStringList l;
    for (int i = 0; s_defaultFileExcludeFilters[i]; ++i) {
        l << QLatin1String(s_defaultFileExcludeFilters[i]);
    }
    for (int i = 0; s_defaultFolderExcludeFilters[i]; ++i) {
        l << QLatin1String(s_defaultFolderExcludeFilters[i]);
    }
    return l;
}

int Baloo::defaultExcludeFilterListVersion()
{
    return qMax(s_defaultFileExcludeFiltersVersion, s_defaultFolderExcludeFiltersVersion);
}

QStringList Baloo::sourceCodeMimeTypes()
{
    QStringList l;
    for (int i = 0; s_sourceCodeMimeTypes[i]; ++i) {
        l << QLatin1String(s_sourceCodeMimeTypes[i]);
    }

    return l;
}

QStringList Baloo::defaultExcludeMimetypes()
{
    return sourceCodeMimeTypes();
}

int Baloo::defaultExcludeMimetypesVersion()
{
    // The +1 is the image, video and audio mimetypes
    return s_sourceCodeMimeTypesVersion + 1;
}
