# SPDX-License-Identifier: GPL-3.0-or-later
from contextlib import contextmanager
from hashlib import sha256
from os import chdir
from os import environ
from os import getcwd
from pathlib import Path
from platform import python_version_tuple
from re import escape
from re import split
from re import sub
from string import ascii_letters
from string import digits
from subprocess import STDOUT
from subprocess import CalledProcessError
from subprocess import check_output
from sys import exit
from sys import stderr
from tempfile import mkstemp
from traceback import print_stack
from typing import IO
from typing import AnyStr
from typing import Dict

# NOTE: remove after python 3.8.x is no longer supported upstream
if int(python_version_tuple()[1]) < 9:  # pragma: no cover
    from typing import Iterable
    from typing import Iterator
else:
    from collections.abc import Iterable
    from collections.abc import Iterator
from typing import List
from typing import Optional
from typing import Set
from typing import Union

from libkeyringctl.types import Fingerprint
from libkeyringctl.types import Trust
from libkeyringctl.types import Uid


@contextmanager
def cwd(new_dir: Path) -> Iterator[None]:
    """Change to a new current working directory in a context and go back to the previous dir after the context is done

    Parameters
    ----------
    new_dir: A path to change to
    """

    previous_dir = getcwd()
    chdir(new_dir)
    try:
        yield
    finally:
        chdir(previous_dir)


def natural_sort_path(_list: Iterable[Path]) -> Iterable[Path]:
    """Sort an Iterable of Paths naturally

    Parameters
    ----------
    _list: An iterable containing paths to be sorted

    Return
    ------
    An Iterable of paths that are naturally sorted
    """

    def convert_text_chunk(text: str) -> Union[int, str]:
        """Convert input text to int or str

        Parameters
        ----------
        text: An input string

        Returns
        -------
        Either an integer if text is a digit, else text in lower-case representation
        """

        return int(text) if text.isdigit() else text.lower()

    def alphanum_key(key: Path) -> List[Union[int, str]]:
        """Retrieve an alphanumeric key from a Path, that can be used in sorted()

        Parameters
        ----------
        key: A path for which to create a key

        Returns
        -------
        A list of either int or str objects that may serve as 'key' argument for sorted()
        """

        return [convert_text_chunk(c) for c in split("([0-9]+)", str(key.name))]

    return sorted(_list, key=alphanum_key)


def system(
    cmd: List[str],
    _stdin: Optional[IO[AnyStr]] = None,
    exit_on_error: bool = False,
    env: Optional[Dict[str, str]] = None,
) -> str:
    """Execute a command using check_output

    Parameters
    ----------
    cmd: A list of strings to be fed to check_output
    _stdin: input fd used for the spawned process
    exit_on_error: Whether to exit the script when encountering an error (defaults to False)
    env: Optional environment vars for the shell invocation

    Raises
    ------
    CalledProcessError: If not exit_on_error and `check_output()` encounters an error

    Returns
    -------
    The output of cmd
    """
    if not env:
        env = {"HOME": environ["HOME"], "PATH": environ["PATH"], "LANG": "en_US.UTF-8"}

    try:
        return check_output(cmd, stderr=STDOUT, stdin=_stdin, env=env).decode()
    except CalledProcessError as e:
        stderr.buffer.write(e.stdout)
        print_stack()
        if exit_on_error:
            exit(e.returncode)
        raise e


def absolute_path(path: str) -> Path:
    """Return the absolute path of a given str

    Parameters
    ----------
    path: A string representing a path

    Returns
    -------
    The absolute path representation of path
    """

    return Path(path).absolute()


def transform_fd_to_tmpfile(working_dir: Path, sources: List[Path]) -> None:
    """Transforms an input list of paths from any file descriptor of the current process to a tempfile in working_dir.

    Using this function on fd inputs allow to pass the content to another process while hidepid is active and /proc
    not visible for the other process.

    Parameters
    ----------
    working_dir: A directory to use for temporary files
    sources: Paths that should be iterated and all fd's transformed to tmpfiles
    """
    for index, source in enumerate(sources):
        source_str = str(source)
        if source_str.startswith("/proc/self/fd/") or source_str.startswith("/dev/fd/"):
            file = mkstemp(dir=working_dir, prefix=f"{source.name}", suffix=".fd")[1]
            with open(file, mode="wb") as f:
                f.write(source.read_bytes())
                f.flush()
            sources[index] = Path(file)


def get_cert_paths(paths: Iterable[Path]) -> Set[Path]:
    """Walks a list of paths and resolves all discovered certificate paths

    Parameters
    ----------
    paths: A list of paths to walk and resolve to certificate paths.

    Returns
    -------
    A set of paths to certificates
    """

    # depth first search certificate paths
    cert_paths: Set[Path] = set()
    visit: List[Path] = list(paths)
    while visit:
        path = visit.pop()
        # this level contains a certificate, abort depth search
        if list(path.glob("*.asc")):
            cert_paths.add(path)
            continue
        visit.extend([path for path in path.iterdir() if path.is_dir()])
    return cert_paths


def get_parent_cert_paths(paths: Iterable[Path]) -> Set[Path]:
    """Walks a list of paths upwards and resolves all discovered parent certificate paths

    Parameters
    ----------
    paths: A list of paths to walk and resolve to certificate paths.

    Returns
    -------
    A set of paths to certificates
    """

    # depth first search certificate paths
    cert_paths: Set[Path] = set()
    visit: List[Path] = list(paths)
    while visit:
        node = visit.pop().parent
        # this level contains a certificate, abort depth search
        if "keyring" == node.parent.parent.parent.name:
            cert_paths.add(node)
            continue
        visit.append(node)
    return cert_paths


def contains_fingerprint(fingerprints: Iterable[Fingerprint], fingerprint: Fingerprint) -> bool:
    """Returns weather an iterable structure of fingerprints contains a specific fingerprint

    Parameters
    ----------
    fingerprints: Iteratable structure of fingerprints that should be searched
    fingerprint: Fingerprint to search for

    Returns
    -------
    Weather an iterable structure of fingerprints contains a specific fingerprint
    """

    return any(filter(lambda e: str(e).endswith(fingerprint), fingerprints))


def get_fingerprint_from_partial(
    fingerprints: Iterable[Fingerprint], fingerprint: Fingerprint
) -> Optional[Fingerprint]:
    """Returns the full fingerprint looked up from a partial fingerprint like a key-id

    Parameters
    ----------
    fingerprints: Iteratable structure of fingerprints that should be searched
    fingerprint: Partial fingerprint to search for

    Returns
    -------
    The full fingerprint or None
    """

    for fingerprint in filter(lambda e: str(e).endswith(fingerprint), fingerprints):
        return fingerprint
    return None


def filter_fingerprints_by_trust(trusts: Dict[Fingerprint, Trust], trust: Trust) -> List[Fingerprint]:
    """Filters a dict of Fingerprint to Trust by a passed Trust parameter and returns the matching fingerprints.

    Parameters
    ----------
    trusts: Dict of Fingerprint to Trust that should be filtered based on the trust parameter
    trust: Trust that should be used to filter the trusts dict

    Returns
    -------
    The matching fingerprints of the dict filtered by trust
    """

    return list(
        map(
            lambda item: item[0],
            filter(lambda item: trust == item[1], trusts.items()),
        )
    )


simple_printable: str = ascii_letters + digits + "_-.+@"
ascii_mapping: Dict[str, str] = {
    "àáâãäæąăǎа": "a",
    "ćçĉċč": "c",
    "ďđ": "d",
    "éèêëęēĕėěɇ": "e",
    "ĝğġģ": "g",
    "ĥħȟ": "h",
    "ìíîïĩīĭįıĳ": "i",
    "ĵɉ": "j",
    "ķ": "k",
    "ł": "l",
    "ńņň": "n",
    "òóôõöøŏőðȍǿ": "o",
    "śș": "s",
    "ß": "ss",
    "ț": "t",
    "úûüȗűȕù": "u",
    "ýÿ": "y",
    "źż": "z",
}
ascii_mapping_lookup: Dict[str, str] = {}
for key, value in ascii_mapping.items():
    for c in key:
        ascii_mapping_lookup[c] = value
        ascii_mapping_lookup[c.upper()] = value.upper()


def simplify_ascii(_str: str) -> str:
    """Simplify a string to contain more filesystem and printable friendly characters

    Parameters
    ----------
    _str: A string to simplify (e.g. 'Foobar McFooface <foobar@foo.face>')

    Returns
    -------
    The simplified representation of _str
    """
    _str = _str.strip("<")
    _str = _str.strip(">")
    _str = "".join([ascii_mapping_lookup.get(char) or char for char in _str])
    _str = sub("[^" + escape(simple_printable) + "]", "_", _str)
    return _str


def simplify_uid(uid: Uid, hash_postfix: bool = True) -> str:
    """Simplify a uid to contain more filesystem and printable friendly characters with an optional
    collision resistant hash postfix.

    Parameters
    ----------
    uid: Uid to simplify (e.g. 'Foobar McFooface <foobar@foo.face>')
    hash_postfix: Whether to add a hash of the uid as postfix

    Returns
    -------
    Simplified str representation of uid
    """
    _hash = "" if not hash_postfix else f"_{sha256(uid.encode()).hexdigest()[:8]}"
    return f"{simplify_ascii(_str=uid)}{_hash}"
