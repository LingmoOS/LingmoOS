# This file was automatically generated by SWIG (https://www.swig.org).
# Version 4.1.1
#
# Do not make changes to this file unless you know what you are doing - modify
# the SWIG interface file instead.

from sys import version_info as _swig_python_version_info
# Import the low-level C/C++ module
if __package__ or "." in __name__:
    from . import _audit
else:
    import _audit

try:
    import builtins as __builtin__
except ImportError:
    import __builtin__

def _swig_repr(self):
    try:
        strthis = "proxy of " + self.this.__repr__()
    except __builtin__.Exception:
        strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)


def _swig_setattr_nondynamic_instance_variable(set):
    def set_instance_attr(self, name, value):
        if name == "this":
            set(self, name, value)
        elif name == "thisown":
            self.this.own(value)
        elif hasattr(self, name) and isinstance(getattr(type(self), name), property):
            set(self, name, value)
        else:
            raise AttributeError("You cannot add instance attributes to %s" % self)
    return set_instance_attr


def _swig_setattr_nondynamic_class_variable(set):
    def set_class_attr(cls, name, value):
        if hasattr(cls, name) and not isinstance(getattr(cls, name), property):
            set(cls, name, value)
        else:
            raise AttributeError("You cannot add class attributes to %s" % cls)
    return set_class_attr


def _swig_add_metaclass(metaclass):
    """Class decorator for adding a metaclass to a SWIG wrapped class - a slimmed down version of six.add_metaclass"""
    def wrapper(cls):
        return metaclass(cls.__name__, cls.__bases__, cls.__dict__.copy())
    return wrapper


class _SwigNonDynamicMeta(type):
    """Meta class to enforce nondynamic attributes (no new attributes) for a class"""
    __setattr__ = _swig_setattr_nondynamic_class_variable(type.__setattr__)


AUDIT_GET = _audit.AUDIT_GET
AUDIT_SET = _audit.AUDIT_SET
AUDIT_LIST = _audit.AUDIT_LIST
AUDIT_ADD = _audit.AUDIT_ADD
AUDIT_DEL = _audit.AUDIT_DEL
AUDIT_USER = _audit.AUDIT_USER
AUDIT_LOGIN = _audit.AUDIT_LOGIN
AUDIT_WATCH_INS = _audit.AUDIT_WATCH_INS
AUDIT_WATCH_REM = _audit.AUDIT_WATCH_REM
AUDIT_WATCH_LIST = _audit.AUDIT_WATCH_LIST
AUDIT_SIGNAL_INFO = _audit.AUDIT_SIGNAL_INFO
AUDIT_ADD_RULE = _audit.AUDIT_ADD_RULE
AUDIT_DEL_RULE = _audit.AUDIT_DEL_RULE
AUDIT_LIST_RULES = _audit.AUDIT_LIST_RULES
AUDIT_TRIM = _audit.AUDIT_TRIM
AUDIT_MAKE_EQUIV = _audit.AUDIT_MAKE_EQUIV
AUDIT_TTY_GET = _audit.AUDIT_TTY_GET
AUDIT_TTY_SET = _audit.AUDIT_TTY_SET
AUDIT_SET_FEATURE = _audit.AUDIT_SET_FEATURE
AUDIT_GET_FEATURE = _audit.AUDIT_GET_FEATURE
AUDIT_FIRST_USER_MSG = _audit.AUDIT_FIRST_USER_MSG
AUDIT_USER_AVC = _audit.AUDIT_USER_AVC
AUDIT_USER_TTY = _audit.AUDIT_USER_TTY
AUDIT_LAST_USER_MSG = _audit.AUDIT_LAST_USER_MSG
AUDIT_FIRST_USER_MSG2 = _audit.AUDIT_FIRST_USER_MSG2
AUDIT_LAST_USER_MSG2 = _audit.AUDIT_LAST_USER_MSG2
AUDIT_DAEMON_START = _audit.AUDIT_DAEMON_START
AUDIT_DAEMON_END = _audit.AUDIT_DAEMON_END
AUDIT_DAEMON_ABORT = _audit.AUDIT_DAEMON_ABORT
AUDIT_DAEMON_CONFIG = _audit.AUDIT_DAEMON_CONFIG
AUDIT_SYSCALL = _audit.AUDIT_SYSCALL
AUDIT_PATH = _audit.AUDIT_PATH
AUDIT_IPC = _audit.AUDIT_IPC
AUDIT_SOCKETCALL = _audit.AUDIT_SOCKETCALL
AUDIT_CONFIG_CHANGE = _audit.AUDIT_CONFIG_CHANGE
AUDIT_SOCKADDR = _audit.AUDIT_SOCKADDR
AUDIT_CWD = _audit.AUDIT_CWD
AUDIT_EXECVE = _audit.AUDIT_EXECVE
AUDIT_IPC_SET_PERM = _audit.AUDIT_IPC_SET_PERM
AUDIT_MQ_OPEN = _audit.AUDIT_MQ_OPEN
AUDIT_MQ_SENDRECV = _audit.AUDIT_MQ_SENDRECV
AUDIT_MQ_NOTIFY = _audit.AUDIT_MQ_NOTIFY
AUDIT_MQ_GETSETATTR = _audit.AUDIT_MQ_GETSETATTR
AUDIT_KERNEL_OTHER = _audit.AUDIT_KERNEL_OTHER
AUDIT_FD_PAIR = _audit.AUDIT_FD_PAIR
AUDIT_OBJ_PID = _audit.AUDIT_OBJ_PID
AUDIT_TTY = _audit.AUDIT_TTY
AUDIT_EOE = _audit.AUDIT_EOE
AUDIT_BPRM_FCAPS = _audit.AUDIT_BPRM_FCAPS
AUDIT_CAPSET = _audit.AUDIT_CAPSET
AUDIT_MMAP = _audit.AUDIT_MMAP
AUDIT_NETFILTER_PKT = _audit.AUDIT_NETFILTER_PKT
AUDIT_NETFILTER_CFG = _audit.AUDIT_NETFILTER_CFG
AUDIT_SECCOMP = _audit.AUDIT_SECCOMP
AUDIT_PROCTITLE = _audit.AUDIT_PROCTITLE
AUDIT_FEATURE_CHANGE = _audit.AUDIT_FEATURE_CHANGE
AUDIT_REPLACE = _audit.AUDIT_REPLACE
AUDIT_KERN_MODULE = _audit.AUDIT_KERN_MODULE
AUDIT_FANOTIFY = _audit.AUDIT_FANOTIFY
AUDIT_TIME_INJOFFSET = _audit.AUDIT_TIME_INJOFFSET
AUDIT_TIME_ADJNTPVAL = _audit.AUDIT_TIME_ADJNTPVAL
AUDIT_BPF = _audit.AUDIT_BPF
AUDIT_EVENT_LISTENER = _audit.AUDIT_EVENT_LISTENER
AUDIT_URINGOP = _audit.AUDIT_URINGOP
AUDIT_OPENAT2 = _audit.AUDIT_OPENAT2
AUDIT_DM_CTRL = _audit.AUDIT_DM_CTRL
AUDIT_DM_EVENT = _audit.AUDIT_DM_EVENT
AUDIT_AVC = _audit.AUDIT_AVC
AUDIT_SELINUX_ERR = _audit.AUDIT_SELINUX_ERR
AUDIT_AVC_PATH = _audit.AUDIT_AVC_PATH
AUDIT_MAC_POLICY_LOAD = _audit.AUDIT_MAC_POLICY_LOAD
AUDIT_MAC_STATUS = _audit.AUDIT_MAC_STATUS
AUDIT_MAC_CONFIG_CHANGE = _audit.AUDIT_MAC_CONFIG_CHANGE
AUDIT_MAC_UNLBL_ALLOW = _audit.AUDIT_MAC_UNLBL_ALLOW
AUDIT_MAC_CIPSOV4_ADD = _audit.AUDIT_MAC_CIPSOV4_ADD
AUDIT_MAC_CIPSOV4_DEL = _audit.AUDIT_MAC_CIPSOV4_DEL
AUDIT_MAC_MAP_ADD = _audit.AUDIT_MAC_MAP_ADD
AUDIT_MAC_MAP_DEL = _audit.AUDIT_MAC_MAP_DEL
AUDIT_MAC_IPSEC_ADDSA = _audit.AUDIT_MAC_IPSEC_ADDSA
AUDIT_MAC_IPSEC_DELSA = _audit.AUDIT_MAC_IPSEC_DELSA
AUDIT_MAC_IPSEC_ADDSPD = _audit.AUDIT_MAC_IPSEC_ADDSPD
AUDIT_MAC_IPSEC_DELSPD = _audit.AUDIT_MAC_IPSEC_DELSPD
AUDIT_MAC_IPSEC_EVENT = _audit.AUDIT_MAC_IPSEC_EVENT
AUDIT_MAC_UNLBL_STCADD = _audit.AUDIT_MAC_UNLBL_STCADD
AUDIT_MAC_UNLBL_STCDEL = _audit.AUDIT_MAC_UNLBL_STCDEL
AUDIT_MAC_CALIPSO_ADD = _audit.AUDIT_MAC_CALIPSO_ADD
AUDIT_MAC_CALIPSO_DEL = _audit.AUDIT_MAC_CALIPSO_DEL
AUDIT_FIRST_KERN_ANOM_MSG = _audit.AUDIT_FIRST_KERN_ANOM_MSG
AUDIT_LAST_KERN_ANOM_MSG = _audit.AUDIT_LAST_KERN_ANOM_MSG
AUDIT_ANOM_PROMISCUOUS = _audit.AUDIT_ANOM_PROMISCUOUS
AUDIT_ANOM_ABEND = _audit.AUDIT_ANOM_ABEND
AUDIT_ANOM_LINK = _audit.AUDIT_ANOM_LINK
AUDIT_ANOM_CREAT = _audit.AUDIT_ANOM_CREAT
AUDIT_INTEGRITY_DATA = _audit.AUDIT_INTEGRITY_DATA
AUDIT_INTEGRITY_METADATA = _audit.AUDIT_INTEGRITY_METADATA
AUDIT_INTEGRITY_STATUS = _audit.AUDIT_INTEGRITY_STATUS
AUDIT_INTEGRITY_HASH = _audit.AUDIT_INTEGRITY_HASH
AUDIT_INTEGRITY_PCR = _audit.AUDIT_INTEGRITY_PCR
AUDIT_INTEGRITY_RULE = _audit.AUDIT_INTEGRITY_RULE
AUDIT_INTEGRITY_EVM_XATTR = _audit.AUDIT_INTEGRITY_EVM_XATTR
AUDIT_INTEGRITY_POLICY_RULE = _audit.AUDIT_INTEGRITY_POLICY_RULE
AUDIT_KERNEL = _audit.AUDIT_KERNEL
AUDIT_FILTER_USER = _audit.AUDIT_FILTER_USER
AUDIT_FILTER_TASK = _audit.AUDIT_FILTER_TASK
AUDIT_FILTER_ENTRY = _audit.AUDIT_FILTER_ENTRY
AUDIT_FILTER_WATCH = _audit.AUDIT_FILTER_WATCH
AUDIT_FILTER_EXIT = _audit.AUDIT_FILTER_EXIT
AUDIT_FILTER_EXCLUDE = _audit.AUDIT_FILTER_EXCLUDE
AUDIT_FILTER_TYPE = _audit.AUDIT_FILTER_TYPE
AUDIT_FILTER_FS = _audit.AUDIT_FILTER_FS
AUDIT_FILTER_URING_EXIT = _audit.AUDIT_FILTER_URING_EXIT
AUDIT_NR_FILTERS = _audit.AUDIT_NR_FILTERS
AUDIT_FILTER_PREPEND = _audit.AUDIT_FILTER_PREPEND
AUDIT_NEVER = _audit.AUDIT_NEVER
AUDIT_POSSIBLE = _audit.AUDIT_POSSIBLE
AUDIT_ALWAYS = _audit.AUDIT_ALWAYS
AUDIT_MAX_FIELDS = _audit.AUDIT_MAX_FIELDS
AUDIT_MAX_KEY_LEN = _audit.AUDIT_MAX_KEY_LEN
AUDIT_BITMASK_SIZE = _audit.AUDIT_BITMASK_SIZE
AUDIT_SYSCALL_CLASSES = _audit.AUDIT_SYSCALL_CLASSES
AUDIT_CLASS_DIR_WRITE = _audit.AUDIT_CLASS_DIR_WRITE
AUDIT_CLASS_DIR_WRITE_32 = _audit.AUDIT_CLASS_DIR_WRITE_32
AUDIT_CLASS_CHATTR = _audit.AUDIT_CLASS_CHATTR
AUDIT_CLASS_CHATTR_32 = _audit.AUDIT_CLASS_CHATTR_32
AUDIT_CLASS_READ = _audit.AUDIT_CLASS_READ
AUDIT_CLASS_READ_32 = _audit.AUDIT_CLASS_READ_32
AUDIT_CLASS_WRITE = _audit.AUDIT_CLASS_WRITE
AUDIT_CLASS_WRITE_32 = _audit.AUDIT_CLASS_WRITE_32
AUDIT_CLASS_SIGNAL = _audit.AUDIT_CLASS_SIGNAL
AUDIT_CLASS_SIGNAL_32 = _audit.AUDIT_CLASS_SIGNAL_32
AUDIT_UNUSED_BITS = _audit.AUDIT_UNUSED_BITS
AUDIT_COMPARE_UID_TO_OBJ_UID = _audit.AUDIT_COMPARE_UID_TO_OBJ_UID
AUDIT_COMPARE_GID_TO_OBJ_GID = _audit.AUDIT_COMPARE_GID_TO_OBJ_GID
AUDIT_COMPARE_EUID_TO_OBJ_UID = _audit.AUDIT_COMPARE_EUID_TO_OBJ_UID
AUDIT_COMPARE_EGID_TO_OBJ_GID = _audit.AUDIT_COMPARE_EGID_TO_OBJ_GID
AUDIT_COMPARE_AUID_TO_OBJ_UID = _audit.AUDIT_COMPARE_AUID_TO_OBJ_UID
AUDIT_COMPARE_SUID_TO_OBJ_UID = _audit.AUDIT_COMPARE_SUID_TO_OBJ_UID
AUDIT_COMPARE_SGID_TO_OBJ_GID = _audit.AUDIT_COMPARE_SGID_TO_OBJ_GID
AUDIT_COMPARE_FSUID_TO_OBJ_UID = _audit.AUDIT_COMPARE_FSUID_TO_OBJ_UID
AUDIT_COMPARE_FSGID_TO_OBJ_GID = _audit.AUDIT_COMPARE_FSGID_TO_OBJ_GID
AUDIT_COMPARE_UID_TO_AUID = _audit.AUDIT_COMPARE_UID_TO_AUID
AUDIT_COMPARE_UID_TO_EUID = _audit.AUDIT_COMPARE_UID_TO_EUID
AUDIT_COMPARE_UID_TO_FSUID = _audit.AUDIT_COMPARE_UID_TO_FSUID
AUDIT_COMPARE_UID_TO_SUID = _audit.AUDIT_COMPARE_UID_TO_SUID
AUDIT_COMPARE_AUID_TO_FSUID = _audit.AUDIT_COMPARE_AUID_TO_FSUID
AUDIT_COMPARE_AUID_TO_SUID = _audit.AUDIT_COMPARE_AUID_TO_SUID
AUDIT_COMPARE_AUID_TO_EUID = _audit.AUDIT_COMPARE_AUID_TO_EUID
AUDIT_COMPARE_EUID_TO_SUID = _audit.AUDIT_COMPARE_EUID_TO_SUID
AUDIT_COMPARE_EUID_TO_FSUID = _audit.AUDIT_COMPARE_EUID_TO_FSUID
AUDIT_COMPARE_SUID_TO_FSUID = _audit.AUDIT_COMPARE_SUID_TO_FSUID
AUDIT_COMPARE_GID_TO_EGID = _audit.AUDIT_COMPARE_GID_TO_EGID
AUDIT_COMPARE_GID_TO_FSGID = _audit.AUDIT_COMPARE_GID_TO_FSGID
AUDIT_COMPARE_GID_TO_SGID = _audit.AUDIT_COMPARE_GID_TO_SGID
AUDIT_COMPARE_EGID_TO_FSGID = _audit.AUDIT_COMPARE_EGID_TO_FSGID
AUDIT_COMPARE_EGID_TO_SGID = _audit.AUDIT_COMPARE_EGID_TO_SGID
AUDIT_COMPARE_SGID_TO_FSGID = _audit.AUDIT_COMPARE_SGID_TO_FSGID
AUDIT_MAX_FIELD_COMPARE = _audit.AUDIT_MAX_FIELD_COMPARE
AUDIT_PID = _audit.AUDIT_PID
AUDIT_UID = _audit.AUDIT_UID
AUDIT_EUID = _audit.AUDIT_EUID
AUDIT_SUID = _audit.AUDIT_SUID
AUDIT_FSUID = _audit.AUDIT_FSUID
AUDIT_GID = _audit.AUDIT_GID
AUDIT_EGID = _audit.AUDIT_EGID
AUDIT_SGID = _audit.AUDIT_SGID
AUDIT_FSGID = _audit.AUDIT_FSGID
AUDIT_LOGINUID = _audit.AUDIT_LOGINUID
AUDIT_PERS = _audit.AUDIT_PERS
AUDIT_ARCH = _audit.AUDIT_ARCH
AUDIT_MSGTYPE = _audit.AUDIT_MSGTYPE
AUDIT_SUBJ_USER = _audit.AUDIT_SUBJ_USER
AUDIT_SUBJ_ROLE = _audit.AUDIT_SUBJ_ROLE
AUDIT_SUBJ_TYPE = _audit.AUDIT_SUBJ_TYPE
AUDIT_SUBJ_SEN = _audit.AUDIT_SUBJ_SEN
AUDIT_SUBJ_CLR = _audit.AUDIT_SUBJ_CLR
AUDIT_PPID = _audit.AUDIT_PPID
AUDIT_OBJ_USER = _audit.AUDIT_OBJ_USER
AUDIT_OBJ_ROLE = _audit.AUDIT_OBJ_ROLE
AUDIT_OBJ_TYPE = _audit.AUDIT_OBJ_TYPE
AUDIT_OBJ_LEV_LOW = _audit.AUDIT_OBJ_LEV_LOW
AUDIT_OBJ_LEV_HIGH = _audit.AUDIT_OBJ_LEV_HIGH
AUDIT_LOGINUID_SET = _audit.AUDIT_LOGINUID_SET
AUDIT_SESSIONID = _audit.AUDIT_SESSIONID
AUDIT_FSTYPE = _audit.AUDIT_FSTYPE
AUDIT_DEVMAJOR = _audit.AUDIT_DEVMAJOR
AUDIT_DEVMINOR = _audit.AUDIT_DEVMINOR
AUDIT_INODE = _audit.AUDIT_INODE
AUDIT_EXIT = _audit.AUDIT_EXIT
AUDIT_SUCCESS = _audit.AUDIT_SUCCESS
AUDIT_WATCH = _audit.AUDIT_WATCH
AUDIT_PERM = _audit.AUDIT_PERM
AUDIT_DIR = _audit.AUDIT_DIR
AUDIT_FILETYPE = _audit.AUDIT_FILETYPE
AUDIT_OBJ_UID = _audit.AUDIT_OBJ_UID
AUDIT_OBJ_GID = _audit.AUDIT_OBJ_GID
AUDIT_FIELD_COMPARE = _audit.AUDIT_FIELD_COMPARE
AUDIT_EXE = _audit.AUDIT_EXE
AUDIT_SADDR_FAM = _audit.AUDIT_SADDR_FAM
AUDIT_ARG0 = _audit.AUDIT_ARG0
AUDIT_ARG1 = _audit.AUDIT_ARG1
AUDIT_ARG2 = _audit.AUDIT_ARG2
AUDIT_ARG3 = _audit.AUDIT_ARG3
AUDIT_FILTERKEY = _audit.AUDIT_FILTERKEY
AUDIT_NEGATE = _audit.AUDIT_NEGATE
AUDIT_BIT_MASK = _audit.AUDIT_BIT_MASK
AUDIT_LESS_THAN = _audit.AUDIT_LESS_THAN
AUDIT_GREATER_THAN = _audit.AUDIT_GREATER_THAN
AUDIT_NOT_EQUAL = _audit.AUDIT_NOT_EQUAL
AUDIT_EQUAL = _audit.AUDIT_EQUAL
AUDIT_BIT_TEST = _audit.AUDIT_BIT_TEST
AUDIT_LESS_THAN_OR_EQUAL = _audit.AUDIT_LESS_THAN_OR_EQUAL
AUDIT_GREATER_THAN_OR_EQUAL = _audit.AUDIT_GREATER_THAN_OR_EQUAL
AUDIT_OPERATORS = _audit.AUDIT_OPERATORS
Audit_equal = _audit.Audit_equal
Audit_not_equal = _audit.Audit_not_equal
Audit_bitmask = _audit.Audit_bitmask
Audit_bittest = _audit.Audit_bittest
Audit_lt = _audit.Audit_lt
Audit_gt = _audit.Audit_gt
Audit_le = _audit.Audit_le
Audit_ge = _audit.Audit_ge
Audit_bad = _audit.Audit_bad
AUDIT_STATUS_ENABLED = _audit.AUDIT_STATUS_ENABLED
AUDIT_STATUS_FAILURE = _audit.AUDIT_STATUS_FAILURE
AUDIT_STATUS_PID = _audit.AUDIT_STATUS_PID
AUDIT_STATUS_RATE_LIMIT = _audit.AUDIT_STATUS_RATE_LIMIT
AUDIT_STATUS_BACKLOG_LIMIT = _audit.AUDIT_STATUS_BACKLOG_LIMIT
AUDIT_STATUS_BACKLOG_WAIT_TIME = _audit.AUDIT_STATUS_BACKLOG_WAIT_TIME
AUDIT_STATUS_LOST = _audit.AUDIT_STATUS_LOST
AUDIT_STATUS_BACKLOG_WAIT_TIME_ACTUAL = _audit.AUDIT_STATUS_BACKLOG_WAIT_TIME_ACTUAL
AUDIT_FEATURE_BITMAP_BACKLOG_LIMIT = _audit.AUDIT_FEATURE_BITMAP_BACKLOG_LIMIT
AUDIT_FEATURE_BITMAP_BACKLOG_WAIT_TIME = _audit.AUDIT_FEATURE_BITMAP_BACKLOG_WAIT_TIME
AUDIT_FEATURE_BITMAP_EXECUTABLE_PATH = _audit.AUDIT_FEATURE_BITMAP_EXECUTABLE_PATH
AUDIT_FEATURE_BITMAP_EXCLUDE_EXTEND = _audit.AUDIT_FEATURE_BITMAP_EXCLUDE_EXTEND
AUDIT_FEATURE_BITMAP_SESSIONID_FILTER = _audit.AUDIT_FEATURE_BITMAP_SESSIONID_FILTER
AUDIT_FEATURE_BITMAP_LOST_RESET = _audit.AUDIT_FEATURE_BITMAP_LOST_RESET
AUDIT_FEATURE_BITMAP_FILTER_FS = _audit.AUDIT_FEATURE_BITMAP_FILTER_FS
AUDIT_FEATURE_BITMAP_ALL = _audit.AUDIT_FEATURE_BITMAP_ALL
AUDIT_VERSION_LATEST = _audit.AUDIT_VERSION_LATEST
AUDIT_VERSION_BACKLOG_LIMIT = _audit.AUDIT_VERSION_BACKLOG_LIMIT
AUDIT_VERSION_BACKLOG_WAIT_TIME = _audit.AUDIT_VERSION_BACKLOG_WAIT_TIME
AUDIT_FAIL_SILENT = _audit.AUDIT_FAIL_SILENT
AUDIT_FAIL_PRINTK = _audit.AUDIT_FAIL_PRINTK
AUDIT_FAIL_PANIC = _audit.AUDIT_FAIL_PANIC
__AUDIT_ARCH_CONVENTION_MASK = _audit.__AUDIT_ARCH_CONVENTION_MASK
__AUDIT_ARCH_CONVENTION_MIPS64_N32 = _audit.__AUDIT_ARCH_CONVENTION_MIPS64_N32
__AUDIT_ARCH_64BIT = _audit.__AUDIT_ARCH_64BIT
__AUDIT_ARCH_LE = _audit.__AUDIT_ARCH_LE
AUDIT_PERM_EXEC = _audit.AUDIT_PERM_EXEC
AUDIT_PERM_WRITE = _audit.AUDIT_PERM_WRITE
AUDIT_PERM_READ = _audit.AUDIT_PERM_READ
AUDIT_PERM_ATTR = _audit.AUDIT_PERM_ATTR
AUDIT_MESSAGE_TEXT_MAX = _audit.AUDIT_MESSAGE_TEXT_MAX
AUDIT_NLGRP_NONE = _audit.AUDIT_NLGRP_NONE
AUDIT_NLGRP_READLOG = _audit.AUDIT_NLGRP_READLOG
__AUDIT_NLGRP_MAX = _audit.__AUDIT_NLGRP_MAX
class audit_status(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc="The membership flag")
    __repr__ = _swig_repr
    mask = property(_audit.audit_status_mask_get, _audit.audit_status_mask_set)
    enabled = property(_audit.audit_status_enabled_get, _audit.audit_status_enabled_set)
    failure = property(_audit.audit_status_failure_get, _audit.audit_status_failure_set)
    pid = property(_audit.audit_status_pid_get, _audit.audit_status_pid_set)
    rate_limit = property(_audit.audit_status_rate_limit_get, _audit.audit_status_rate_limit_set)
    backlog_limit = property(_audit.audit_status_backlog_limit_get, _audit.audit_status_backlog_limit_set)
    lost = property(_audit.audit_status_lost_get, _audit.audit_status_lost_set)
    backlog = property(_audit.audit_status_backlog_get, _audit.audit_status_backlog_set)
    version = property(_audit.audit_status_version_get, _audit.audit_status_version_set)
    feature_bitmap = property(_audit.audit_status_feature_bitmap_get, _audit.audit_status_feature_bitmap_set)
    backlog_wait_time = property(_audit.audit_status_backlog_wait_time_get, _audit.audit_status_backlog_wait_time_set)
    backlog_wait_time_actual = property(_audit.audit_status_backlog_wait_time_actual_get, _audit.audit_status_backlog_wait_time_actual_set)

    def __init__(self):
        _audit.audit_status_swiginit(self, _audit.new_audit_status())
    __swig_destroy__ = _audit.delete_audit_status

# Register audit_status in _audit:
_audit.audit_status_swigregister(audit_status)
class audit_features(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc="The membership flag")
    __repr__ = _swig_repr
    vers = property(_audit.audit_features_vers_get, _audit.audit_features_vers_set)
    mask = property(_audit.audit_features_mask_get, _audit.audit_features_mask_set)
    features = property(_audit.audit_features_features_get, _audit.audit_features_features_set)
    lock = property(_audit.audit_features_lock_get, _audit.audit_features_lock_set)

    def __init__(self):
        _audit.audit_features_swiginit(self, _audit.new_audit_features())
    __swig_destroy__ = _audit.delete_audit_features

# Register audit_features in _audit:
_audit.audit_features_swigregister(audit_features)
AUDIT_FEATURE_VERSION = _audit.AUDIT_FEATURE_VERSION

AUDIT_FEATURE_ONLY_UNSET_LOGINUID = _audit.AUDIT_FEATURE_ONLY_UNSET_LOGINUID
AUDIT_FEATURE_LOGINUID_IMMUTABLE = _audit.AUDIT_FEATURE_LOGINUID_IMMUTABLE
AUDIT_LAST_FEATURE = _audit.AUDIT_LAST_FEATURE
class audit_tty_status(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc="The membership flag")
    __repr__ = _swig_repr
    enabled = property(_audit.audit_tty_status_enabled_get, _audit.audit_tty_status_enabled_set)
    log_passwd = property(_audit.audit_tty_status_log_passwd_get, _audit.audit_tty_status_log_passwd_set)

    def __init__(self):
        _audit.audit_tty_status_swiginit(self, _audit.new_audit_tty_status())
    __swig_destroy__ = _audit.delete_audit_tty_status

# Register audit_tty_status in _audit:
_audit.audit_tty_status_swigregister(audit_tty_status)
class audit_rule_data(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc="The membership flag")
    __repr__ = _swig_repr
    flags = property(_audit.audit_rule_data_flags_get, _audit.audit_rule_data_flags_set)
    action = property(_audit.audit_rule_data_action_get, _audit.audit_rule_data_action_set)
    field_count = property(_audit.audit_rule_data_field_count_get, _audit.audit_rule_data_field_count_set)
    mask = property(_audit.audit_rule_data_mask_get, _audit.audit_rule_data_mask_set)
    fields = property(_audit.audit_rule_data_fields_get, _audit.audit_rule_data_fields_set)
    values = property(_audit.audit_rule_data_values_get, _audit.audit_rule_data_values_set)
    fieldflags = property(_audit.audit_rule_data_fieldflags_get, _audit.audit_rule_data_fieldflags_set)
    buflen = property(_audit.audit_rule_data_buflen_get, _audit.audit_rule_data_buflen_set)

    def __init__(self):
        _audit.audit_rule_data_swiginit(self, _audit.new_audit_rule_data())
    __swig_destroy__ = _audit.delete_audit_rule_data

# Register audit_rule_data in _audit:
_audit.audit_rule_data_swigregister(audit_rule_data)
AUDIT_USER_AUTH = _audit.AUDIT_USER_AUTH
AUDIT_USER_ACCT = _audit.AUDIT_USER_ACCT
AUDIT_USER_MGMT = _audit.AUDIT_USER_MGMT
AUDIT_CRED_ACQ = _audit.AUDIT_CRED_ACQ
AUDIT_CRED_DISP = _audit.AUDIT_CRED_DISP
AUDIT_USER_START = _audit.AUDIT_USER_START
AUDIT_USER_END = _audit.AUDIT_USER_END
AUDIT_USER_CHAUTHTOK = _audit.AUDIT_USER_CHAUTHTOK
AUDIT_USER_ERR = _audit.AUDIT_USER_ERR
AUDIT_CRED_REFR = _audit.AUDIT_CRED_REFR
AUDIT_USYS_CONFIG = _audit.AUDIT_USYS_CONFIG
AUDIT_USER_LOGIN = _audit.AUDIT_USER_LOGIN
AUDIT_USER_LOGOUT = _audit.AUDIT_USER_LOGOUT
AUDIT_ADD_USER = _audit.AUDIT_ADD_USER
AUDIT_DEL_USER = _audit.AUDIT_DEL_USER
AUDIT_ADD_GROUP = _audit.AUDIT_ADD_GROUP
AUDIT_DEL_GROUP = _audit.AUDIT_DEL_GROUP
AUDIT_DAC_CHECK = _audit.AUDIT_DAC_CHECK
AUDIT_CHGRP_ID = _audit.AUDIT_CHGRP_ID
AUDIT_TEST = _audit.AUDIT_TEST
AUDIT_TRUSTED_APP = _audit.AUDIT_TRUSTED_APP
AUDIT_USER_SELINUX_ERR = _audit.AUDIT_USER_SELINUX_ERR
AUDIT_USER_CMD = _audit.AUDIT_USER_CMD
AUDIT_CHUSER_ID = _audit.AUDIT_CHUSER_ID
AUDIT_GRP_AUTH = _audit.AUDIT_GRP_AUTH
AUDIT_SYSTEM_BOOT = _audit.AUDIT_SYSTEM_BOOT
AUDIT_SYSTEM_SHUTDOWN = _audit.AUDIT_SYSTEM_SHUTDOWN
AUDIT_SYSTEM_RUNLEVEL = _audit.AUDIT_SYSTEM_RUNLEVEL
AUDIT_SERVICE_START = _audit.AUDIT_SERVICE_START
AUDIT_SERVICE_STOP = _audit.AUDIT_SERVICE_STOP
AUDIT_GRP_MGMT = _audit.AUDIT_GRP_MGMT
AUDIT_GRP_CHAUTHTOK = _audit.AUDIT_GRP_CHAUTHTOK
AUDIT_MAC_CHECK = _audit.AUDIT_MAC_CHECK
AUDIT_ACCT_LOCK = _audit.AUDIT_ACCT_LOCK
AUDIT_ACCT_UNLOCK = _audit.AUDIT_ACCT_UNLOCK
AUDIT_USER_DEVICE = _audit.AUDIT_USER_DEVICE
AUDIT_SOFTWARE_UPDATE = _audit.AUDIT_SOFTWARE_UPDATE
AUDIT_FIRST_DAEMON = _audit.AUDIT_FIRST_DAEMON
AUDIT_LAST_DAEMON = _audit.AUDIT_LAST_DAEMON
AUDIT_DAEMON_RECONFIG = _audit.AUDIT_DAEMON_RECONFIG
AUDIT_DAEMON_ROTATE = _audit.AUDIT_DAEMON_ROTATE
AUDIT_DAEMON_RESUME = _audit.AUDIT_DAEMON_RESUME
AUDIT_DAEMON_ACCEPT = _audit.AUDIT_DAEMON_ACCEPT
AUDIT_DAEMON_CLOSE = _audit.AUDIT_DAEMON_CLOSE
AUDIT_DAEMON_ERR = _audit.AUDIT_DAEMON_ERR
AUDIT_FIRST_EVENT = _audit.AUDIT_FIRST_EVENT
AUDIT_LAST_EVENT = _audit.AUDIT_LAST_EVENT
AUDIT_FIRST_SELINUX = _audit.AUDIT_FIRST_SELINUX
AUDIT_LAST_SELINUX = _audit.AUDIT_LAST_SELINUX
AUDIT_FIRST_APPARMOR = _audit.AUDIT_FIRST_APPARMOR
AUDIT_LAST_APPARMOR = _audit.AUDIT_LAST_APPARMOR
AUDIT_AA = _audit.AUDIT_AA
AUDIT_APPARMOR_AUDIT = _audit.AUDIT_APPARMOR_AUDIT
AUDIT_APPARMOR_ALLOWED = _audit.AUDIT_APPARMOR_ALLOWED
AUDIT_APPARMOR_DENIED = _audit.AUDIT_APPARMOR_DENIED
AUDIT_APPARMOR_HINT = _audit.AUDIT_APPARMOR_HINT
AUDIT_APPARMOR_STATUS = _audit.AUDIT_APPARMOR_STATUS
AUDIT_APPARMOR_ERROR = _audit.AUDIT_APPARMOR_ERROR
AUDIT_APPARMOR_KILL = _audit.AUDIT_APPARMOR_KILL
AUDIT_FIRST_KERN_CRYPTO_MSG = _audit.AUDIT_FIRST_KERN_CRYPTO_MSG
AUDIT_LAST_KERN_CRYPTO_MSG = _audit.AUDIT_LAST_KERN_CRYPTO_MSG
AUDIT_INTEGRITY_FIRST_MSG = _audit.AUDIT_INTEGRITY_FIRST_MSG
AUDIT_INTEGRITY_LAST_MSG = _audit.AUDIT_INTEGRITY_LAST_MSG
AUDIT_FIRST_ANOM_MSG = _audit.AUDIT_FIRST_ANOM_MSG
AUDIT_LAST_ANOM_MSG = _audit.AUDIT_LAST_ANOM_MSG
AUDIT_ANOM_LOGIN_FAILURES = _audit.AUDIT_ANOM_LOGIN_FAILURES
AUDIT_ANOM_LOGIN_TIME = _audit.AUDIT_ANOM_LOGIN_TIME
AUDIT_ANOM_LOGIN_SESSIONS = _audit.AUDIT_ANOM_LOGIN_SESSIONS
AUDIT_ANOM_LOGIN_ACCT = _audit.AUDIT_ANOM_LOGIN_ACCT
AUDIT_ANOM_LOGIN_LOCATION = _audit.AUDIT_ANOM_LOGIN_LOCATION
AUDIT_ANOM_MAX_DAC = _audit.AUDIT_ANOM_MAX_DAC
AUDIT_ANOM_MAX_MAC = _audit.AUDIT_ANOM_MAX_MAC
AUDIT_ANOM_AMTU_FAIL = _audit.AUDIT_ANOM_AMTU_FAIL
AUDIT_ANOM_RBAC_FAIL = _audit.AUDIT_ANOM_RBAC_FAIL
AUDIT_ANOM_RBAC_INTEGRITY_FAIL = _audit.AUDIT_ANOM_RBAC_INTEGRITY_FAIL
AUDIT_ANOM_CRYPTO_FAIL = _audit.AUDIT_ANOM_CRYPTO_FAIL
AUDIT_ANOM_ACCESS_FS = _audit.AUDIT_ANOM_ACCESS_FS
AUDIT_ANOM_EXEC = _audit.AUDIT_ANOM_EXEC
AUDIT_ANOM_MK_EXEC = _audit.AUDIT_ANOM_MK_EXEC
AUDIT_ANOM_ADD_ACCT = _audit.AUDIT_ANOM_ADD_ACCT
AUDIT_ANOM_DEL_ACCT = _audit.AUDIT_ANOM_DEL_ACCT
AUDIT_ANOM_MOD_ACCT = _audit.AUDIT_ANOM_MOD_ACCT
AUDIT_ANOM_ROOT_TRANS = _audit.AUDIT_ANOM_ROOT_TRANS
AUDIT_ANOM_LOGIN_SERVICE = _audit.AUDIT_ANOM_LOGIN_SERVICE
AUDIT_ANOM_LOGIN_ROOT = _audit.AUDIT_ANOM_LOGIN_ROOT
AUDIT_ANOM_ORIGIN_FAILURES = _audit.AUDIT_ANOM_ORIGIN_FAILURES
AUDIT_ANOM_SESSION = _audit.AUDIT_ANOM_SESSION
AUDIT_FIRST_ANOM_RESP = _audit.AUDIT_FIRST_ANOM_RESP
AUDIT_LAST_ANOM_RESP = _audit.AUDIT_LAST_ANOM_RESP
AUDIT_RESP_ANOMALY = _audit.AUDIT_RESP_ANOMALY
AUDIT_RESP_ALERT = _audit.AUDIT_RESP_ALERT
AUDIT_RESP_KILL_PROC = _audit.AUDIT_RESP_KILL_PROC
AUDIT_RESP_TERM_ACCESS = _audit.AUDIT_RESP_TERM_ACCESS
AUDIT_RESP_ACCT_REMOTE = _audit.AUDIT_RESP_ACCT_REMOTE
AUDIT_RESP_ACCT_LOCK_TIMED = _audit.AUDIT_RESP_ACCT_LOCK_TIMED
AUDIT_RESP_ACCT_UNLOCK_TIMED = _audit.AUDIT_RESP_ACCT_UNLOCK_TIMED
AUDIT_RESP_ACCT_LOCK = _audit.AUDIT_RESP_ACCT_LOCK
AUDIT_RESP_TERM_LOCK = _audit.AUDIT_RESP_TERM_LOCK
AUDIT_RESP_SEBOOL = _audit.AUDIT_RESP_SEBOOL
AUDIT_RESP_EXEC = _audit.AUDIT_RESP_EXEC
AUDIT_RESP_SINGLE = _audit.AUDIT_RESP_SINGLE
AUDIT_RESP_HALT = _audit.AUDIT_RESP_HALT
AUDIT_RESP_ORIGIN_BLOCK = _audit.AUDIT_RESP_ORIGIN_BLOCK
AUDIT_RESP_ORIGIN_BLOCK_TIMED = _audit.AUDIT_RESP_ORIGIN_BLOCK_TIMED
AUDIT_RESP_ORIGIN_UNBLOCK_TIMED = _audit.AUDIT_RESP_ORIGIN_UNBLOCK_TIMED
AUDIT_FIRST_USER_LSPP_MSG = _audit.AUDIT_FIRST_USER_LSPP_MSG
AUDIT_LAST_USER_LSPP_MSG = _audit.AUDIT_LAST_USER_LSPP_MSG
AUDIT_USER_ROLE_CHANGE = _audit.AUDIT_USER_ROLE_CHANGE
AUDIT_ROLE_ASSIGN = _audit.AUDIT_ROLE_ASSIGN
AUDIT_ROLE_REMOVE = _audit.AUDIT_ROLE_REMOVE
AUDIT_LABEL_OVERRIDE = _audit.AUDIT_LABEL_OVERRIDE
AUDIT_LABEL_LEVEL_CHANGE = _audit.AUDIT_LABEL_LEVEL_CHANGE
AUDIT_USER_LABELED_EXPORT = _audit.AUDIT_USER_LABELED_EXPORT
AUDIT_USER_UNLABELED_EXPORT = _audit.AUDIT_USER_UNLABELED_EXPORT
AUDIT_DEV_ALLOC = _audit.AUDIT_DEV_ALLOC
AUDIT_DEV_DEALLOC = _audit.AUDIT_DEV_DEALLOC
AUDIT_FS_RELABEL = _audit.AUDIT_FS_RELABEL
AUDIT_USER_MAC_POLICY_LOAD = _audit.AUDIT_USER_MAC_POLICY_LOAD
AUDIT_ROLE_MODIFY = _audit.AUDIT_ROLE_MODIFY
AUDIT_USER_MAC_CONFIG_CHANGE = _audit.AUDIT_USER_MAC_CONFIG_CHANGE
AUDIT_USER_MAC_STATUS = _audit.AUDIT_USER_MAC_STATUS
AUDIT_FIRST_CRYPTO_MSG = _audit.AUDIT_FIRST_CRYPTO_MSG
AUDIT_CRYPTO_TEST_USER = _audit.AUDIT_CRYPTO_TEST_USER
AUDIT_CRYPTO_PARAM_CHANGE_USER = _audit.AUDIT_CRYPTO_PARAM_CHANGE_USER
AUDIT_CRYPTO_LOGIN = _audit.AUDIT_CRYPTO_LOGIN
AUDIT_CRYPTO_LOGOUT = _audit.AUDIT_CRYPTO_LOGOUT
AUDIT_CRYPTO_KEY_USER = _audit.AUDIT_CRYPTO_KEY_USER
AUDIT_CRYPTO_FAILURE_USER = _audit.AUDIT_CRYPTO_FAILURE_USER
AUDIT_CRYPTO_REPLAY_USER = _audit.AUDIT_CRYPTO_REPLAY_USER
AUDIT_CRYPTO_SESSION = _audit.AUDIT_CRYPTO_SESSION
AUDIT_CRYPTO_IKE_SA = _audit.AUDIT_CRYPTO_IKE_SA
AUDIT_CRYPTO_IPSEC_SA = _audit.AUDIT_CRYPTO_IPSEC_SA
AUDIT_LAST_CRYPTO_MSG = _audit.AUDIT_LAST_CRYPTO_MSG
AUDIT_FIRST_VIRT_MSG = _audit.AUDIT_FIRST_VIRT_MSG
AUDIT_VIRT_CONTROL = _audit.AUDIT_VIRT_CONTROL
AUDIT_VIRT_RESOURCE = _audit.AUDIT_VIRT_RESOURCE
AUDIT_VIRT_MACHINE_ID = _audit.AUDIT_VIRT_MACHINE_ID
AUDIT_VIRT_INTEGRITY_CHECK = _audit.AUDIT_VIRT_INTEGRITY_CHECK
AUDIT_VIRT_CREATE = _audit.AUDIT_VIRT_CREATE
AUDIT_VIRT_DESTROY = _audit.AUDIT_VIRT_DESTROY
AUDIT_VIRT_MIGRATE_IN = _audit.AUDIT_VIRT_MIGRATE_IN
AUDIT_VIRT_MIGRATE_OUT = _audit.AUDIT_VIRT_MIGRATE_OUT
AUDIT_LAST_VIRT_MSG = _audit.AUDIT_LAST_VIRT_MSG

def audit_open():
    return _audit.audit_open()

def audit_close(fd):
    return _audit.audit_close(fd)

def audit_value_needs_encoding(str, size):
    return _audit.audit_value_needs_encoding(str, size)

def audit_encode_value(final, buf, size):
    return _audit.audit_encode_value(final, buf, size)

def audit_encode_nv_string(name, value, vlen):
    return _audit.audit_encode_nv_string(name, value, vlen)

def audit_log_user_message(audit_fd, type, message, hostname, addr, tty, result):
    return _audit.audit_log_user_message(audit_fd, type, message, hostname, addr, tty, result)

def audit_log_user_comm_message(audit_fd, type, message, comm, hostname, addr, tty, result):
    return _audit.audit_log_user_comm_message(audit_fd, type, message, comm, hostname, addr, tty, result)

def audit_log_acct_message(audit_fd, type, pgname, op, name, id, host, addr, tty, result):
    return _audit.audit_log_acct_message(audit_fd, type, pgname, op, name, id, host, addr, tty, result)

def audit_log_user_avc_message(audit_fd, type, message, hostname, addr, tty, auid):
    return _audit.audit_log_user_avc_message(audit_fd, type, message, hostname, addr, tty, auid)

def audit_log_semanage_message(audit_fd, type, pgname, op, name, id, new_seuser, new_role, new_range, old_seuser, old_role, old_range, host, addr, tty, result):
    return _audit.audit_log_semanage_message(audit_fd, type, pgname, op, name, id, new_seuser, new_role, new_range, old_seuser, old_role, old_range, host, addr, tty, result)

def audit_log_user_command(audit_fd, type, command, tty, result):
    return _audit.audit_log_user_command(audit_fd, type, command, tty, result)

def audit_elf_to_machine(elf):
    return _audit.audit_elf_to_machine(elf)

def audit_machine_to_name(machine):
    return _audit.audit_machine_to_name(machine)

def audit_syscall_to_name(sc, machine):
    return _audit.audit_syscall_to_name(sc, machine)

def audit_detect_machine():
    return _audit.audit_detect_machine()

def audit_msg_type_to_name(msg_type):
    return _audit.audit_msg_type_to_name(msg_type)

