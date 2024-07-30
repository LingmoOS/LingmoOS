from collections import defaultdict
from functools import wraps
from pathlib import Path
from random import choice
from random import randint
from shutil import copytree
from string import ascii_letters
from string import digits
from string import hexdigits
from string import punctuation
from subprocess import PIPE
from subprocess import Popen
from tempfile import NamedTemporaryFile
from tempfile import TemporaryDirectory
from typing import Any
from typing import Callable
from typing import Dict
from typing import Generator
from typing import List
from typing import Optional
from typing import Set

from pytest import fixture

from libkeyringctl.keyring import convert_certificate
from libkeyringctl.keyring import export
from libkeyringctl.keyring import get_fingerprints_from_keyring_files
from libkeyringctl.sequoia import certify
from libkeyringctl.sequoia import key_extract_certificate
from libkeyringctl.sequoia import key_generate
from libkeyringctl.sequoia import keyring_merge
from libkeyringctl.sequoia import packet_join
from libkeyringctl.types import Fingerprint
from libkeyringctl.types import Uid
from libkeyringctl.types import Username
from libkeyringctl.util import cwd
from libkeyringctl.util import simplify_uid
from libkeyringctl.util import system

test_keys: Dict[Username, List[Path]] = defaultdict(list)
test_key_revocation: Dict[Username, List[Path]] = defaultdict(list)
test_certificates: Dict[Username, List[Path]] = defaultdict(list)
test_certificate_uids: Dict[Username, List[List[Uid]]] = defaultdict(list)
test_keyring_certificates: Dict[Username, List[Path]] = defaultdict(list)
test_main_fingerprints: Set[Fingerprint] = set()
test_all_fingerprints: Set[Fingerprint] = set()


@fixture(autouse=True)
def reset_storage() -> None:
    test_keys.clear()
    test_key_revocation.clear()
    test_certificates.clear()
    test_certificate_uids.clear()
    test_keyring_certificates.clear()
    test_main_fingerprints.clear()
    test_all_fingerprints.clear()


def create_certificate(
    username: Username,
    uids: List[Uid],
    keyring_type: str = "packager",
    func: Optional[Callable[..., Any]] = None,
) -> Callable[..., Any]:
    def decorator(decorated_func: Callable[..., None]) -> Callable[..., Any]:
        @wraps(decorated_func)
        def wrapper(working_dir: Path, *args: Any, **kwargs: Any) -> None:
            key_directory = working_dir / "secret" / f"{username}"
            key_directory.mkdir(parents=True, exist_ok=True)

            key_file: Path = key_directory / f"{username}.asc"
            key_generate(uids=uids, outfile=key_file)
            test_keys[username].append(key_file)

            certificate_directory = working_dir / "certificate" / f"{username}"
            certificate_directory.mkdir(parents=True, exist_ok=True)

            keyring_root: Path = working_dir / "keyring"
            keyring_root.mkdir(parents=True, exist_ok=True)
            certificate_file: Path = certificate_directory / f"{username}.asc"

            key_extract_certificate(key=key_file, output=certificate_file)
            test_certificates[username].append(certificate_file)
            test_certificate_uids[username].append(uids)

            key_revocation_packet = key_file.parent / f"{key_file.name}.rev"
            key_revocation_joined = key_file.parent / f"{key_file.name}.joined.rev"
            key_revocation_cert = key_file.parent / f"{key_file.name}.cert.rev"
            packet_join(packets=[certificate_file, key_revocation_packet], output=key_revocation_joined)
            keyring_merge(certificates=[key_revocation_joined], output=key_revocation_cert)
            test_key_revocation[username].append(key_revocation_cert)

            target_dir = keyring_root / keyring_type

            for fingerprint in get_fingerprints_from_keyring_files(
                working_dir=working_dir, source=[certificate_file]
            ).keys():
                test_all_fingerprints.add(fingerprint)

            decomposed_path: Path = convert_certificate(
                working_dir=working_dir,
                certificate=certificate_file,
                keyring_dir=keyring_root / keyring_type,
                fingerprint_filter=test_all_fingerprints,
            )
            user_dir = decomposed_path.parent
            (target_dir / user_dir.name).mkdir(parents=True, exist_ok=True)
            copytree(src=user_dir, dst=(target_dir / user_dir.name), dirs_exist_ok=True)
            test_keyring_certificates[username].append(target_dir / user_dir.name / decomposed_path.name)

            certificate_fingerprint: Fingerprint = Fingerprint(decomposed_path.name)
            if "main" == keyring_type:
                test_main_fingerprints.add(certificate_fingerprint)
            test_all_fingerprints.add(certificate_fingerprint)

            decorated_func(working_dir=working_dir, *args, **kwargs)

        return wrapper

    if not func:
        return decorator
    return decorator(func)


def create_uid_certification(
    issuer: Username, certified: Username, uid: Uid, func: Optional[Callable[[Any], None]] = None
) -> Callable[..., Any]:
    def decorator(decorated_func: Callable[..., None]) -> Callable[..., Any]:
        @wraps(decorated_func)
        def wrapper(working_dir: Path, *args: Any, **kwargs: Any) -> None:
            key: Path = test_keys[issuer][0]
            certificate: Path = test_certificates[certified][0]
            fingerprint: Fingerprint = Fingerprint(test_keyring_certificates[certified][0].name)
            issuer_fingerprint: Fingerprint = Fingerprint(test_keyring_certificates[issuer][0].name)
            simplified_uid = simplify_uid(Uid(uid))

            output: Path = (
                working_dir
                / "keyring"
                / "packager"
                / certified
                / fingerprint
                / "uid"
                / simplified_uid
                / "certification"
                / f"{issuer_fingerprint}.asc"
            )
            output.parent.mkdir(parents=True, exist_ok=True)

            certify(key, certificate, uid, output)

            decorated_func(working_dir=working_dir, *args, **kwargs)

        return wrapper

    if not func:
        return decorator
    return decorator(func)


def create_key_revocation(
    username: Username,
    keyring_type: str = "packager",
    func: Optional[Callable[..., Any]] = None,
) -> Callable[..., Any]:
    def decorator(decorated_func: Callable[..., None]) -> Callable[..., Any]:
        @wraps(decorated_func)
        def wrapper(working_dir: Path, *args: Any, **kwargs: Any) -> None:
            revocation = test_key_revocation[username][0]

            keyring_root: Path = working_dir / "keyring"
            keyring_root.mkdir(parents=True, exist_ok=True)
            target_dir = keyring_root / keyring_type

            decomposed_path: Path = convert_certificate(
                working_dir=working_dir,
                certificate=revocation,
                keyring_dir=keyring_root / keyring_type,
                fingerprint_filter=test_all_fingerprints,
            )
            user_dir = decomposed_path.parent
            (target_dir / user_dir.name).mkdir(parents=True, exist_ok=True)
            copytree(src=user_dir, dst=(target_dir / user_dir.name), dirs_exist_ok=True)

            decorated_func(working_dir=working_dir, *args, **kwargs)

        return wrapper

    if not func:
        return decorator
    return decorator(func)


def create_signature_revocation(
    issuer: Username, certified: Username, uid: Uid, func: Optional[Callable[[Any], None]] = None
) -> Callable[..., Any]:
    def decorator(decorated_func: Callable[..., None]) -> Callable[..., Any]:
        @wraps(decorated_func)
        def wrapper(working_dir: Path, *args: Any, **kwargs: Any) -> None:
            issuer_key: Path = test_keys[issuer][0]
            keyring_root: Path = working_dir / "keyring"

            keyring_certificate: Path = test_keyring_certificates[certified][0]
            certified_fingerprint = keyring_certificate.name

            with NamedTemporaryFile(dir=str(working_dir), prefix=f"{certified}", suffix=".asc") as certificate:
                certificate_path: Path = Path(certificate.name)
                export(
                    working_dir=working_dir,
                    keyring_root=keyring_root,
                    sources=[keyring_certificate],
                    output=certificate_path,
                )

                with TemporaryDirectory(prefix="gnupg") as gnupg_home:
                    env = {"GNUPGHOME": gnupg_home}

                    print(
                        system(
                            [
                                "gpg",
                                "--no-auto-check-trustdb",
                                "--import",
                                f"{str(issuer_key)}",
                                f"{str(certificate_path)}",
                            ],
                            env=env,
                        )
                    )

                    uid_confirmations = ""
                    for cert_uid in test_certificate_uids[certified][0]:
                        if uid == cert_uid:
                            uid_confirmations += "y\n"
                        else:
                            uid_confirmations += "n\n"

                    commands = Popen(["echo", "-e", f"{uid_confirmations}y\n0\ny\n\ny\ny\nsave\n"], stdout=PIPE)
                    system(
                        [
                            "gpg",
                            "--no-auto-check-trustdb",
                            "--command-fd",
                            "0",
                            "--expert",
                            "--yes",
                            "--batch",
                            "--edit-key",
                            f"{certified_fingerprint}",
                            "revsig",
                            "save",
                        ],
                        _stdin=commands.stdout,
                        env=env,
                    )

                    revoked_certificate = system(["gpg", "--armor", "--export", f"{certified_fingerprint}"], env=env)
                    certificate.truncate(0)
                    certificate.seek(0)
                    certificate.write(revoked_certificate.encode())
                    certificate.flush()

                    target_dir = keyring_root / "packager"
                    decomposed_path: Path = convert_certificate(
                        working_dir=working_dir,
                        certificate=certificate_path,
                        keyring_dir=target_dir,
                        fingerprint_filter=test_all_fingerprints,
                    )
                    user_dir = decomposed_path.parent
                    (target_dir / user_dir.name).mkdir(parents=True, exist_ok=True)
                    copytree(src=user_dir, dst=(target_dir / user_dir.name), dirs_exist_ok=True)

            decorated_func(working_dir=working_dir, *args, **kwargs)

        return wrapper

    if not func:
        return decorator
    return decorator(func)


@fixture(scope="function")
def working_dir() -> Generator[Path, None, None]:
    with TemporaryDirectory(prefix="arch-keyringctl-test-") as tempdir:
        path: Path = Path(tempdir)
        with cwd(path):
            yield path


@fixture(scope="function")
def keyring_dir(working_dir: Path) -> Generator[Path, None, None]:
    yield working_dir / "keyring"


def random_string(length: int, chars: str) -> str:
    return "".join(choice(chars) for x in range(length))


@fixture(scope="function", params=[16, 40], ids=["short_id", "long_id"])
def valid_fingerprint(request: Any) -> Generator[str, None, None]:
    yield random_string(length=request.param, chars="ABCDEF" + digits)


@fixture(scope="function", params=[16, 40], ids=["short_id", "long_id"])
def valid_subkey_fingerprint(request: Any) -> Generator[str, None, None]:
    yield random_string(length=request.param, chars="ABCDEF" + digits)


@fixture(
    scope="function",
    params=[
        (
            16,
            ascii_letters + hexdigits + punctuation,
        ),
        (
            40,
            ascii_letters + hexdigits + punctuation,
        ),
        (
            randint(0, 15),
            "ABCDEF" + digits,
        ),
        (
            randint(17, 39),
            "ABCDEF" + digits,
        ),
        (
            randint(41, 100),
            "ABCDEF" + digits,
        ),
    ],
    ids=[
        "short_id_wrong_chars",
        "long_id_wrong_chars",
        "right_chars_shorter_than_short_id",
        "right_chars_shorter_than_long_id_longer_than_short_id",
        "right_chars_longer_than_long_id",
    ],
)
def invalid_fingerprint(request: Any) -> Generator[str, None, None]:
    yield random_string(length=request.param[0], chars=request.param[1])
