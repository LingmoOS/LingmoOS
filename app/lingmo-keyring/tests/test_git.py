from pathlib import Path
from typing import List
from typing import Optional
from unittest.mock import Mock
from unittest.mock import patch

from pytest import mark

from libkeyringctl import git


@mark.parametrize(
    "git_path, base, paths",
    [
        (None, None, None),
        (Path("git_path"), None, None),
        (Path("git_path"), "base", None),
        (Path("git_path"), "base", [Path("foo"), Path("bar")]),
    ],
)
@patch("libkeyringctl.git.system")
def test_git_changed_files(
    system_mock: Mock,
    git_path: Optional[Path],
    base: Optional[str],
    paths: Optional[List[Path]],
) -> None:
    system_mock.return_value = "create some thing foo\n" "delete some thing bar\n" "some thing baz\n"

    assert git.git_changed_files(git_path=git_path, base=base, paths=paths) == (
        [Path("foo")],
        [Path("bar")],
        [Path("baz")],
    )

    name, args, kwargs = system_mock.mock_calls[0]
    if git_path:
        assert "-C" in args[0]
        assert str(git_path) in args[0]
    if base:
        assert base in args[0]
    if paths:
        assert "--" in args[0]
        for path in paths:
            assert str(path) in args[0]
