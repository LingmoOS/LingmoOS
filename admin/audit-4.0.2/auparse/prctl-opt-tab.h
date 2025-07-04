/* prctl-opt-tab.h --
 * Copyright 2013-16,2018,2020-23 Red Hat Inc.
 * All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:
 *      Steve Grubb <sgrubb@redhat.com>
 * Location: include/uapi/linux/prctl.h
 */

_S(1,  "PR_SET_PDEATHSIG")
_S(2,  "PR_GET_PDEATHSIG")
_S(3,  "PR_GET_DUMPABLE")
_S(4,  "PR_SET_DUMPABLE")
_S(5,  "PR_GET_UNALIGN")
_S(6,  "PR_SET_UNALIGN")
_S(7,  "PR_GET_KEEPCAPS")
_S(8,  "PR_SET_KEEPCAPS")
_S(9,  "PR_GET_FPEMU")
_S(10, "PR_SET_FPEMU")
_S(11, "PR_GET_FPEXC")
_S(12, "PR_SET_FPEXC")
_S(13, "PR_GET_TIMING")
_S(14, "PR_SET_TIMING")
_S(15, "PR_SET_NAME")
_S(16, "PR_GET_NAME")
_S(19, "PR_GET_ENDIAN")
_S(20, "PR_SET_ENDIAN")
_S(21, "PR_GET_SECCOMP")
_S(22, "PR_SET_SECCOMP")
_S(23, "PR_CAPBSET_READ")
_S(24, "PR_CAPBSET_DROP")
_S(25, "PR_GET_TSC")
_S(26, "PR_SET_TSC")
_S(27, "PR_GET_SECUREBITS")
_S(28, "PR_SET_SECUREBITS")
_S(29, "PR_SET_TIMERSLACK")
_S(30, "PR_GET_TIMERSLACK")
_S(31, "PR_TASK_PERF_EVENTS_DISABLE")
_S(32, "PR_TASK_PERF_EVENTS_ENABLE")
_S(33, "PR_MCE_KILL")
_S(34, "PR_MCE_KILL_GET")
_S(35, "PR_SET_MM")
_S(36, "PR_SET_CHILD_SUBREAPER")
_S(37, "PR_GET_CHILD_SUBREAPER")
_S(38, "PR_SET_NO_NEW_PRIVS")
_S(39, "PR_GET_NO_NEW_PRIVS")
_S(40, "PR_GET_TID_ADDRESS")
_S(41, "PR_SET_THP_DISABLE")
_S(42, "PR_GET_THP_DISABLE")
_S(43, "PR_MPX_ENABLE_MANAGEMENT")
_S(44, "PR_MPX_DISABLE_MANAGEMENT")
_S(45, "PR_SET_FP_MODE")
_S(46, "PR_GET_FP_MODE")
_S(47, "PR_CAP_AMBIENT")
_S(50, "PR_SVE_SET_VL")
_S(51, "PR_SVE_GET_VL")
_S(52, "PR_GET_SPECULATION_CTRL")
_S(53, "PR_SET_SPECULATION_CTRL")
_S(54, "PR_PAC_RESET_KEYS")
_S(55, "PR_SET_TAGGED_ADDR_CTRL")
_S(56, "PR_GET_TAGGED_ADDR_CTRL")
_S(57, "PR_SET_IO_FLUSHER")
_S(58, "PR_GET_IO_FLUSHER")
_S(59, "PR_SET_SYSCALL_USER_DISPATCH")
_S(60, "PR_PAC_SET_ENABLED_KEYS")
_S(61, "PR_PAC_GET_ENABLED_KEYS")
_S(62, "PR_SCHED_CORE")
_S(63, "PR_SME_SET_VL")
_S(64, "PR_SME_GET_VL")
_S(65, "PR_SET_MDWE")
_S(66, "PR_GET_MDWE")
_S(67, "PR_SET_MEMORY_MERGE")
_S(68, "PR_GET_MEMORY_MERGE")
_S(69, "PR_RISCV_V_SET_CONTROL")
_S(70, "PR_RISCV_V_GET_CONTROL")

