# SPDX-License-Identifier: GPL-3.0-or-later

from collections import deque
from datetime import datetime
from functools import reduce
from pathlib import Path
from platform import python_version_tuple
from re import sub
from tempfile import mkdtemp
from typing import Dict

# NOTE: remove after python 3.8.x is no longer supported upstream
if int(python_version_tuple()[1]) < 9:  # pragma: no cover
    from typing import Iterable
else:
    from collections.abc import Iterable
from typing import List
from typing import Optional

from .types import Fingerprint
from .types import PacketKind
from .types import Uid
from .types import Username
from .util import cwd
from .util import natural_sort_path
from .util import system


def keyring_split(working_dir: Path, keyring: Path, preserve_filename: bool = False) -> Iterable[Path]:
    """Split a file containing a PGP keyring into separate certificate files

    The original keyring filename is preserved if the split only yields a single certificate.
    If preserve_filename is True, all keyrings are placed into separate directories while preserving
    the filename.

    The file is split using sq.

    Parameters
    ----------
    working_dir: The path of the working directory below which to create the output files
    keyring: The path of a file containing a PGP keyring
    preserve_filename: If True, all keyrings are placed into separate directories while preserving the filename

    Returns
    -------
    An iterable over the naturally sorted list of certificate files derived from a keyring
    """

    keyring_dir = Path(mkdtemp(dir=working_dir, prefix="keyring-")).absolute()

    with cwd(keyring_dir):
        system(["sq", "toolbox", "keyring", "split", str(keyring)])

    keyrings: List[Path] = list(natural_sort_path(keyring_dir.iterdir()))

    if 1 == len(keyrings) or preserve_filename:
        for index, key in enumerate(keyrings):
            keyring_sub_dir = Path(mkdtemp(dir=keyring_dir, prefix=f"{keyring.name}-")).absolute()
            keyrings[index] = key.rename(keyring_sub_dir / keyring.name)

    return keyrings


def keyring_merge(certificates: List[Path], output: Optional[Path] = None, force: bool = False) -> str:
    """Merge multiple certificates into a keyring

    Parameters
    ----------
    certificates: List of paths to certificates to merge into a keyring
    output: Path to a file which the keyring is written, return the result instead if None
    force: Whether to force overwriting existing files (defaults to False)

    Returns
    -------
    The result if no output file has been used
    """

    cmd = ["sq", "toolbox", "keyring", "merge"]
    if force:
        cmd.insert(1, "--force")
    if output:
        cmd += ["--output", str(output)]
    cmd += [str(cert) for cert in sorted(certificates)]

    return system(cmd)


def packet_split(working_dir: Path, certificate: Path) -> Iterable[Path]:
    """Split a file containing a PGP certificate into separate packet files

    The files are split using sq

    Parameters
    ----------
    working_dir: The path of the working directory below which to create the output files
    certificate: The absolute path of a file containing one PGP certificate

    Returns
    -------
    An iterable over the naturally sorted list of packet files derived from certificate
    """

    packet_dir = Path(mkdtemp(dir=working_dir, prefix="packet-")).absolute()

    with cwd(packet_dir):
        system(["sq", "toolbox", "packet", "split", "--prefix", "''", str(certificate)])
    return natural_sort_path(packet_dir.iterdir())


def packet_join(packets: List[Path], output: Optional[Path] = None, force: bool = False) -> str:
    """Join PGP packet data in files to a single output file

    Parameters
    ----------
    packets: A list of paths to files that contain PGP packet data
    output: Path to a file to which all PGP packet data is written, return the result instead if None
    force: Whether to force overwriting existing files (defaults to False)

    Returns
    -------
    The result if no output file has been used
    """

    cmd = ["sq", "toolbox", "packet", "join"]
    if force:
        cmd.insert(1, "--force")
    packets_str = list(map(lambda path: str(path), packets))
    cmd.extend(packets_str)
    cmd.extend(["--output", str(output)])
    return system(cmd)


def inspect(
    packet: Path, certifications: bool = True, fingerprints: Optional[Dict[Fingerprint, Username]] = None
) -> str:
    """Inspect PGP packet data and return the result

    Parameters
    ----------
    packet: Path to a file that contain PGP data
    certifications: Whether to print third-party certifications
    fingerprints: Optional dict of fingerprints to usernames to enrich the output with

    Returns
    -------
    The result of the inspection
    """

    cmd = ["sq", "inspect"]
    if certifications:
        cmd.append("--certifications")
    cmd.append(str(packet))
    result: str = system(cmd)

    if fingerprints:
        for fingerprint, username in fingerprints.items():
            result = sub(f"{fingerprint}", f"{fingerprint} {username}", result)
            result = sub(f" {fingerprint[24:]}", f" {fingerprint[24:]} {username}", result)

    return result


def packet_dump(packet: Path) -> str:
    """Dump a PGP packet to string

    The `sq packet dump` command is used to retrieve a dump of information from a PGP packet

    Parameters
    ----------
    packet: The path to the PGP packet to retrieve the value from

    Returns
    -------
    The contents of the packet dump
    """

    return system(["sq", "toolbox", "packet", "dump", str(packet)])


def packet_dump_field(packet: Path, query: str) -> str:
    """Retrieve the value of a field from a PGP packet

    Field queries are possible with the following notation during tree traversal:
    - Use '.' to separate the parent section
    - Use '*' as a wildcard for the current section
    - Use '|' inside the current level as a logical OR

    Example:
    - Version
    - Hashed area|Unhashed area.Issuer
    - *.Issuer

    Parameters
    ----------
    packet: The path to the PGP packet to retrieve the value from
    query: The name of the field as a query notation

    Raises
    ------
    Exception: If the field is not found in the PGP packet

    Returns
    -------
    The value of the field found in packet
    """

    dump = packet_dump(packet)

    queries = deque(query.split("."))
    path = [queries.popleft()]
    depth = 0

    # remove leading 4 space indention
    lines = list(filter(lambda line: line.startswith("    "), dump.splitlines()))
    lines = [sub(r"^ {4}", "", line, count=1) for line in lines]
    # filter empty lines
    lines = list(filter(lambda line: line.strip(), lines))

    for line in lines:
        # determine current line depth by counting whitespace pairs
        depth_line = int((len(line) - len(line.lstrip(" "))) / 2)
        line = line.lstrip(" ")

        # skip nodes that are deeper as our currently matched path
        if depth < depth_line:
            continue

        # unwind the current query path until reaching previous match depth
        while depth > depth_line:
            queries.appendleft(path.pop())
            depth -= 1
        matcher = path[-1].split("|")

        # check if current field matches the query expression
        field = line.split(sep=":", maxsplit=1)[0]
        if field not in matcher and "*" not in matcher:
            continue

        # next depth is one level deeper as the current line
        depth = depth_line + 1

        # check if matcher is not the leaf of the query expression
        if queries:
            path.append(queries.popleft())
            continue

        # return final match
        return line.split(sep=": ", maxsplit=1)[1] if ": " in line else line

    raise Exception(f"Packet '{packet}' did not match the query '{query}'")


def packet_signature_creation_time(packet: Path) -> datetime:
    """Retrieve the signature creation time field as datetime

    Parameters
    ----------
    packet: The path to the PGP packet to retrieve the value from

    Returns
    -------
    The signature creation time as datetime
    """
    field = packet_dump_field(packet, "Hashed area.Signature creation time")
    field = " ".join(field.split(" ", 3)[0:3])
    return datetime.strptime(field, "%Y-%m-%d %H:%M:%S %Z")


def packet_kinds(packet: Path) -> List[PacketKind]:
    """Retrieve the PGP packet types of a packet path

    Parameters
    ----------
    packet: The path to the PGP packet to retrieve the kind of

    Returns
    -------
    The kind of PGP packet
    """

    dump = packet_dump(packet)
    lines = [line for line in dump.splitlines()]
    lines = list(
        filter(lambda line: not line.startswith(" ") and not line.startswith("WARNING") and line.strip(), lines)
    )
    return [PacketKind(line.split()[0]) for line in lines]


def latest_certification(certifications: Iterable[Path]) -> Path:
    """Returns the latest certification based on the signature creation time from a list of packets.

    Parameters
    ----------
    certifications: List of certification from which to choose the latest from

    Returns
    -------
    The latest certification from a list of packets
    """
    return reduce(
        lambda a, b: a if packet_signature_creation_time(a) > packet_signature_creation_time(b) else b,
        certifications,
    )


def key_generate(uids: List[Uid], outfile: Path) -> str:
    """Generate a PGP key with specific uids

    Parameters
    ----------
    uids: List of uids that the key should have
    outfile: Path to the file to which the key should be written to

    Returns
    -------
    The result of the key generate call
    """

    cmd = ["sq", "key", "generate"]
    for uid in uids:
        cmd.extend(["--userid", str(uid)])
    cmd.extend(["--output", str(outfile)])
    return system(cmd)


def key_extract_certificate(key: Path, output: Optional[Path]) -> str:
    """Extracts the non secret part from a key into a certificate

    Parameters
    ----------
    key: Path to a file that contain secret key material
    output: Path to the file to which the key should be written to, stdout if None

    Returns
    -------
    The result of the extract in case output is None
    """

    cmd = ["sq", "toolbox", "extract-cert", str(key)]
    if output:
        cmd.extend(["--output", str(output)])
    return system(cmd)


def certify(key: Path, certificate: Path, uid: Uid, output: Optional[Path]) -> str:
    """Inspect PGP packet data and return the result

    Parameters
    ----------
    key: Path to a file that contain secret key material
    certificate: Path to a certificate file whose uid should be certified
    uid: Uid contain in the certificate that should be certified
    output: Path to the file to which the key should be written to, stdout if None

    Returns
    -------
    The result of the certification in case output is None
    """

    cmd = ["sq", "pki", "certify", str(key), str(certificate), uid]
    if output:
        cmd.extend(["--output", str(output)])
    return system(cmd)
