from os import chdir
from os import getcwd
from pathlib import Path
from tempfile import NamedTemporaryFile
from tempfile import TemporaryDirectory
from typing import Dict
from typing import List
from unittest.mock import Mock
from unittest.mock import patch

from pytest import mark
from pytest import raises

from libkeyringctl import util
from libkeyringctl.types import Fingerprint
from libkeyringctl.types import Trust


def test_cwd() -> None:
    with TemporaryDirectory() as tmp_dir_name:
        tmp_dir = Path(tmp_dir_name)
        test_dir = tmp_dir / "test"
        test_dir.mkdir()
        chdir(tmp_dir)
        with util.cwd(new_dir=test_dir):
            assert getcwd() == str(test_dir)
        assert getcwd() == str(tmp_dir)


@mark.parametrize(
    "input_list, output_list",
    [
        ([Path("/foo"), Path("/bar/foo"), Path("/foo/20")], [Path("/foo/20"), Path("/foo"), Path("/bar/foo")]),
    ],
)
def test_natural_sort_path(input_list: List[Path], output_list: List[Path]) -> None:
    assert util.natural_sort_path(_list=input_list) == output_list


@mark.parametrize(
    "raise_on_cmd, exit_on_error",
    [
        (False, True),
        (False, False),
        (True, True),
        (True, False),
    ],
)
@patch("libkeyringctl.util.exit")
@patch("libkeyringctl.util.check_output")
def test_system(check_output_mock: Mock, exit_mock: Mock, raise_on_cmd: bool, exit_on_error: bool) -> None:
    if raise_on_cmd:
        check_output_mock.side_effect = util.CalledProcessError(returncode=1, cmd="foo", output=b"output")

        with raises(util.CalledProcessError):
            util.system(["foo"], exit_on_error=exit_on_error)
            if exit_on_error:
                exit_mock.assert_called_once_with(1)

    else:
        check_output_mock.return_value = b"output"
        assert util.system(["foo"], exit_on_error=exit_on_error) == "output"


def test_absolute_path() -> None:
    with TemporaryDirectory() as tmp_dir_name:
        tmp_dir = Path(tmp_dir_name)
        test_dir = tmp_dir / "test"
        test_dir.mkdir()
        chdir(tmp_dir)
        assert util.absolute_path(path="test") == test_dir


def test_transform_fd_to_tmpfile() -> None:
    with TemporaryDirectory() as tmp_dir_name:
        tmp_dir = Path(tmp_dir_name)
        with NamedTemporaryFile(dir=tmp_dir) as tmp_file:
            tmp_file_fd = tmp_file.fileno()
            util.transform_fd_to_tmpfile(
                working_dir=tmp_dir,
                sources=[Path("/foo"), Path(f"/proc/self/fd/{tmp_file_fd}")],
            )


def test_get_cert_paths() -> None:
    with TemporaryDirectory() as tmp_dir_name:
        tmp_dir = Path(tmp_dir_name)

        cert_dir1 = tmp_dir / "cert1"
        cert_dir1.mkdir()
        cert1 = cert_dir1 / "cert1.asc"
        cert1.touch()
        cert_dir2 = tmp_dir / "cert2"
        cert_dir2.mkdir()
        cert2 = cert_dir2 / "cert2.asc"
        cert2.touch()

        assert util.get_cert_paths(paths=[tmp_dir]) == {cert_dir1, cert_dir2}


def test_get_parent_cert_paths() -> None:
    with TemporaryDirectory() as tmp_dir_name:
        tmp_dir = Path(tmp_dir_name)

        keyring_dir = tmp_dir / "keyring"
        group_dir = keyring_dir / "parent"
        user_dir = group_dir / "parent"
        cert_dir1 = user_dir / "cert1"
        cert_dir1.mkdir(parents=True)
        cert1 = cert_dir1 / "cert1.asc"
        cert1.touch()
        cert_dir2 = cert_dir1 / "cert2"
        cert_dir2.mkdir(parents=True)
        cert2 = cert_dir2 / "cert2.asc"
        cert2.touch()

        assert util.get_parent_cert_paths(paths=[cert1, cert2]) == {cert_dir1}


@mark.parametrize(
    "fingerprints, fingerprint, result",
    [
        (
            [Fingerprint("foo"), Fingerprint("bar")],
            Fingerprint("foo"),
            True,
        ),
        (
            [Fingerprint("foo"), Fingerprint("bar")],
            Fingerprint("baz"),
            False,
        ),
    ],
)
def test_contains_fingerprint(fingerprints: List[Fingerprint], fingerprint: Fingerprint, result: bool) -> None:
    assert util.contains_fingerprint(fingerprints=fingerprints, fingerprint=fingerprint) is result


@mark.parametrize(
    "fingerprints, fingerprint, result",
    [
        ([Fingerprint("blahfoo"), Fingerprint("blahbar")], Fingerprint("foo"), Fingerprint("blahfoo")),
        ([Fingerprint("blahfoo"), Fingerprint("blahbar")], Fingerprint("blahfoo"), Fingerprint("blahfoo")),
        (
            [Fingerprint("bazfoo"), Fingerprint("bazbar")],
            Fingerprint("baz"),
            None,
        ),
    ],
)
def test_get_fingerprint_from_partial(fingerprints: List[Fingerprint], fingerprint: Fingerprint, result: bool) -> None:
    assert util.get_fingerprint_from_partial(fingerprints=fingerprints, fingerprint=fingerprint) is result


@mark.parametrize(
    "trusts, trust, result",
    [
        (
            {Fingerprint("foo"): Trust.full, Fingerprint("bar"): Trust.marginal},
            Trust.full,
            [Fingerprint("foo")],
        ),
        (
            {Fingerprint("foo"): Trust.full, Fingerprint("bar"): Trust.full},
            Trust.full,
            [Fingerprint("foo"), Fingerprint("bar")],
        ),
        (
            {Fingerprint("foo"): Trust.full, Fingerprint("bar"): Trust.marginal},
            Trust.unknown,
            [],
        ),
        (
            {},
            Trust.unknown,
            [],
        ),
    ],
)
def test_filter_fingerprints_by_trust(
    trusts: Dict[Fingerprint, Trust], trust: Trust, result: List[Fingerprint]
) -> None:
    assert util.filter_fingerprints_by_trust(trusts=trusts, trust=trust) == result


@mark.parametrize(
    "_str, result",
    [
        ("foobar", "foobar"),
        ("", ""),
        ("bbàáâãğț aa", "bbaaaagt_aa"),
        ("<>", ""),
        ("!#$%^&*()_☃", "___________"),
        ("_-.+@", "_-.+@"),
    ],
)
def test_simplify_ascii(_str: str, result: str) -> None:
    assert util.simplify_ascii(_str=_str) == result
