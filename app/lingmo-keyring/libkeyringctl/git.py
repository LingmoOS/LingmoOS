# SPDX-License-Identifier: GPL-3.0-or-later

from pathlib import Path
from typing import List
from typing import Optional
from typing import Tuple

from .util import system


def git_changed_files(
    git_path: Optional[Path] = None, base: Optional[str] = None, paths: Optional[List[Path]] = None
) -> Tuple[List[Path], List[Path], List[Path]]:
    """Returns lists of created, deleted and modified files based on diff stats related to a base commit
    and optional paths.

    Parameters
    ----------
    git_path: Path to the git repository, current directory by default
    base: Optional base rev or current index by default
    paths: Optional list of paths to take into account, unfiltered by default

    Returns
    -------
    Lists of created, deleted and modified paths
    """
    cmd = ["git"]
    if git_path:
        cmd += ["-C", str(git_path)]
    cmd += ["--no-pager", "diff", "--color=never", "--summary", "--numstat"]
    if base:
        cmd += [base]
    if paths:
        cmd += ["--"]
        cmd += [str(path) for path in paths]

    result: str = system(cmd)

    created: List[Path] = []
    deleted: List[Path] = []
    modified: List[Path] = []

    for line in result.splitlines():
        line = line.strip()
        if line.startswith("create"):
            created.append(Path(line.split(maxsplit=3)[3]))
            continue
        if line.startswith("delete"):
            deleted.append(Path(line.split(maxsplit=3)[3]))
            continue
        modified.append(Path(line.split(maxsplit=2)[2]))

    modified = [path for path in modified if path not in created and path not in deleted]

    return created, deleted, modified
