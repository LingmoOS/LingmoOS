# KService

## Introduction

KService allows to query information about installed applications and their associated file types.

The two relevant classes are:
- KService: This models an installed application and allows to query various properties about it
- KApplicationTrader: This is used to list available applications as well as applications associated with a given file type

## KSycoca

KSycoca caches the information about available applications and file associations for faster access.

## Usage

If you are using CMake, you need to have

    find_package(KF6Service)

(or similar) in your CMakeLists.txt file, and you need to link to KF6::Service.

