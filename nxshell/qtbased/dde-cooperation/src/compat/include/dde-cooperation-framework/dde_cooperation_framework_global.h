// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBAL_DDE_COOPERATION_FRAMEWORK_H
#define GLOBAL_DDE_COOPERATION_FRAMEWORK_H

#define DPF_NAMESPACE dpf

#define DPF_BEGIN_NAMESPACE namespace DPF_NAMESPACE {
#define DPF_END_NAMESPACE }
#define DPF_USE_NAMESPACE using namespace DPF_NAMESPACE;

#define DPF_STR(s) #s
#define DPF_MACRO_TO_STR(s) DPF_STR(s)

#ifdef EXPORT_LIB
#define DPF_EXPORT Q_DECL_EXPORT
#else
#define DPF_EXPORT Q_DECL_IMPORT
#endif

#endif   // GLOBAL_DDE_COOPERATION_FRAMEWORK_H
