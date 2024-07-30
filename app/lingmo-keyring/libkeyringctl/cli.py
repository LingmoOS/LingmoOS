# SPDX-License-Identifier: GPL-3.0-or-later

from argparse import ArgumentParser
from logging import DEBUG
from logging import basicConfig
from logging import debug
from pathlib import Path
from tempfile import TemporaryDirectory
from tempfile import mkdtemp

from .ci import ci
from .keyring import Username
from .keyring import build
from .keyring import convert
from .keyring import export
from .keyring import inspect_keyring
from .keyring import list_keyring
from .types import TrustFilter
from .util import absolute_path
from .util import cwd
from .verify import verify

parser = ArgumentParser()
parser.add_argument(
    "-v", "--verbose", action="store_true", help="Causes to print debugging messages about the progress"
)
parser.add_argument("--wait", action="store_true", help="Block before cleaning up the temp directory")
parser.add_argument(
    "-f",
    "--force",
    action="store_true",
    default=False,
    help="force the execution of subcommands (e.g. overwriting of files)",
)
subcommands = parser.add_subparsers(dest="subcommand")

convert_parser = subcommands.add_parser(
    "convert",
    help="convert one or multiple PGP public keys to a decomposed directory structure",
)
convert_parser.add_argument("source", type=absolute_path, nargs="+", help="Files or directorie to convert")
convert_parser.add_argument("--target", type=absolute_path, help="Target directory instead of a random tmpdir")
convert_parser.add_argument(
    "--name",
    type=Username,
    default=None,
    help="override the username to use (only useful when using a single file as source)",
)

import_parser = subcommands.add_parser(
    "import",
    help="import one or several PGP keys to the keyring directory structure",
)
import_parser.add_argument("source", type=absolute_path, nargs="+", help="Files or directories to import")
import_parser.add_argument(
    "--name",
    type=Username,
    default=None,
    help="override the username to use (only useful when using a single file as source)",
)
import_parser.add_argument("--main", action="store_true", help="Import a main signing key into the keyring")

export_parser = subcommands.add_parser(
    "export",
    help="export a directory structure of PGP packet data to a combined file",
)
export_parser.add_argument("-o", "--output", type=absolute_path, help="file to write PGP packet data to")
export_parser.add_argument(
    "source",
    nargs="*",
    help="username, fingerprint or directories containing certificates",
    type=absolute_path,
)

build_parser = subcommands.add_parser(
    "build",
    help="build keyring PGP artifacts alongside ownertrust and revoked status files",
)

list_parser = subcommands.add_parser(
    "list",
    help="list the certificates in the keyring",
)
list_parser.add_argument("--main", action="store_true", help="List main signing keys instead of packager keys")
list_parser.add_argument(
    "--trust",
    choices=[e.value for e in TrustFilter],
    default=TrustFilter.all.value,
    help="Filter the list based on trust",
)
list_parser.add_argument(
    "source",
    nargs="*",
    help="username, fingerprint or directories containing certificates",
    type=absolute_path,
)

inspect_parser = subcommands.add_parser(
    "inspect",
    help="inspect certificates in the keyring and pretty print the data",
)
inspect_parser.add_argument(
    "source",
    nargs="*",
    help="username, fingerprint or directories containing certificates",
    type=absolute_path,
)

verify_parser = subcommands.add_parser(
    "verify",
    help="verify certificates against modern expectations",
)
verify_parser.add_argument(
    "source",
    nargs="*",
    help="username, fingerprint or directories containing certificates",
    type=absolute_path,
)
verify_parser.add_argument("--no-lint-hokey", dest="lint_hokey", action="store_false", help="Do not run hokey lint")
verify_parser.add_argument(
    "--no-lint-sq-keyring", dest="lint_sq_keyring", action="store_false", help="Do not run sq keyring lint"
)
verify_parser.set_defaults(lint_hokey=True, lint_sq_keyring=True)

check_parser = subcommands.add_parser(
    "check",
    help="Run keyring integrity and consistency checks",
)

ci_parser = subcommands.add_parser(
    "ci",
    help="ci command to verify certain aspects and expectations in pipelines",
)


def main() -> None:  # noqa: ignore=C901
    args = parser.parse_args()

    if args.verbose:
        basicConfig(level=DEBUG)

    # temporary working directory that gets auto cleaned
    with TemporaryDirectory(prefix="arch-keyringctl-") as tempdir:
        project_root = Path(".").absolute()
        keyring_root = Path("keyring").absolute()
        working_dir = Path(tempdir)
        debug(f"Working directory: {working_dir}")
        with cwd(working_dir):
            if "convert" == args.subcommand:
                target_dir = args.target or Path(mkdtemp(prefix="arch-keyringctl-")).absolute()
                print(
                    convert(
                        working_dir=working_dir,
                        keyring_root=keyring_root,
                        sources=args.source,
                        target_dir=target_dir,
                        name_override=args.name,
                    )
                )
            elif "import" == args.subcommand:
                target_dir = "main" if args.main else "packager"
                print(
                    convert(
                        working_dir=working_dir,
                        keyring_root=keyring_root,
                        sources=args.source,
                        target_dir=keyring_root / target_dir,
                        name_override=args.name,
                    )
                )
            elif "export" == args.subcommand:
                result = export(
                    working_dir=working_dir,
                    keyring_root=keyring_root,
                    sources=args.source,
                    output=args.output,
                )
                if result:
                    print(
                        result,
                        end="",
                    )
            elif "build" == args.subcommand:
                build(
                    working_dir=working_dir,
                    keyring_root=keyring_root,
                    target_dir=keyring_root.parent / "build",
                )
            elif "list" == args.subcommand:
                trust_filter = TrustFilter[args.trust]
                list_keyring(
                    keyring_root=keyring_root,
                    sources=args.source,
                    main_keys=args.main,
                    trust_filter=trust_filter,
                )
            elif "inspect" == args.subcommand:
                print(
                    inspect_keyring(
                        working_dir=working_dir,
                        keyring_root=keyring_root,
                        sources=args.source,
                    ),
                    end="",
                )
            elif "verify" == args.subcommand:
                verify(
                    working_dir=working_dir,
                    keyring_root=keyring_root,
                    sources=args.source,
                    lint_hokey=args.lint_hokey,
                    lint_sq_keyring=args.lint_sq_keyring,
                )
            elif "ci" == args.subcommand:
                ci(working_dir=working_dir, keyring_root=keyring_root, project_root=project_root)
            elif "check" == args.subcommand:
                verify(
                    working_dir=working_dir,
                    keyring_root=keyring_root,
                    sources=[keyring_root],
                    lint_hokey=False,
                    lint_sq_keyring=False,
                )
            else:
                parser.print_help()

            if args.wait:
                print("Press [ENTER] to continue")
                input()
