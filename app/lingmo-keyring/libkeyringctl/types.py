# SPDX-License-Identifier: GPL-3.0-or-later

from enum import Enum
from enum import auto
from typing import NewType

Fingerprint = NewType("Fingerprint", str)
Uid = NewType("Uid", str)
Username = NewType("Username", str)
PacketKind = NewType("PacketKind", str)


class Trust(Enum):
    unknown = auto()
    revoked = auto()
    marginal = auto()
    full = auto()


class TrustFilter(Enum):
    unknown = "unknown"
    revoked = "revoked"
    marginal = "marginal"
    full = "full"
    unrevoked = "unrevoked"
    all = "all"


TRUST_MAX_LENGTH: int = max([len(e.name) for e in Trust])


class Color(Enum):
    RED = "\033[31m"
    GREEN = "\033[32m"
    YELLOW = "\033[33m"
    RST = "\033[0m"
    BOLD = "\033[1m"
    UNDERLINE = "\033[4m"
