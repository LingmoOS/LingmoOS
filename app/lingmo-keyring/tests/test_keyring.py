from collections import defaultdict
from contextlib import nullcontext as does_not_raise
from copy import deepcopy
from datetime import datetime
from pathlib import Path
from random import choice
from string import digits
from typing import ContextManager
from typing import Dict
from typing import List
from typing import Optional
from typing import Set
from unittest.mock import Mock
from unittest.mock import patch

from pytest import mark
from pytest import raises

from libkeyringctl import keyring
from libkeyringctl.keyring import PACKET_FILENAME_DATETIME_FORMAT
from libkeyringctl.types import Fingerprint
from libkeyringctl.types import TrustFilter
from libkeyringctl.types import Uid
from libkeyringctl.types import Username

from .conftest import create_certificate
from .conftest import create_key_revocation
from .conftest import create_signature_revocation
from .conftest import create_uid_certification
from .conftest import test_all_fingerprints
from .conftest import test_certificates
from .conftest import test_keyring_certificates
from .conftest import test_keys
from .conftest import test_main_fingerprints


def test_is_pgp_fingerprint(
    valid_fingerprint: str,
    invalid_fingerprint: str,
) -> None:
    assert keyring.is_pgp_fingerprint(string=valid_fingerprint) is True
    assert keyring.is_pgp_fingerprint(string=invalid_fingerprint) is False


@mark.parametrize(
    "create_paths, create_paths_in_keyring_dir",
    [
        (True, False),
        (True, True),
        (False, True),
        (False, False),
    ],
)
def test_transform_username_to_keyring_path(
    create_paths: bool,
    create_paths_in_keyring_dir: bool,
    working_dir: Path,
    keyring_dir: Path,
) -> None:
    paths = [Path("test")]
    input_paths = deepcopy(paths)

    for index, path in enumerate(paths):
        path_in_working_dir = working_dir / path
        if create_paths:
            path_in_working_dir.mkdir()

        if create_paths_in_keyring_dir:
            (keyring_dir / path).mkdir(parents=True)

        paths[index] = path_in_working_dir

    modified_paths = deepcopy(paths)

    keyring.transform_username_to_keyring_path(keyring_dir=keyring_dir, paths=paths)

    for index, path in enumerate(paths):
        if create_paths or (not create_paths and not create_paths_in_keyring_dir):
            assert path == modified_paths[index]
        if not create_paths and create_paths_in_keyring_dir:
            assert path == keyring_dir / input_paths[index]


@mark.parametrize(
    "fingerprint_path, create_paths, create_paths_in_keyring_dir",
    [
        (True, True, False),
        (True, True, True),
        (True, False, True),
        (True, False, False),
        (False, True, False),
        (False, True, True),
        (False, False, True),
        (False, False, False),
    ],
)
def test_transform_fingerprint_to_keyring_path(
    fingerprint_path: bool,
    create_paths: bool,
    create_paths_in_keyring_dir: bool,
    working_dir: Path,
    keyring_dir: Path,
    valid_fingerprint: str,
) -> None:
    paths = [Path(valid_fingerprint) if fingerprint_path else Path("test")]
    input_paths = deepcopy(paths)

    keyring_subdir = keyring_dir / "type" / "username"

    for index, path in enumerate(paths):
        path_in_working_dir = working_dir / path
        if create_paths:
            path_in_working_dir.mkdir()

        if create_paths_in_keyring_dir:
            (keyring_subdir / path).mkdir(parents=True)

        paths[index] = path_in_working_dir

    modified_paths = deepcopy(paths)

    keyring.transform_fingerprint_to_keyring_path(keyring_root=keyring_dir, paths=paths)

    for index, path in enumerate(paths):
        if create_paths or (not fingerprint_path and not create_paths):
            assert path == modified_paths[index]
        if not create_paths and fingerprint_path and create_paths_in_keyring_dir:
            assert path == keyring_subdir / input_paths[index]


@mark.parametrize(
    "valid_current_packet_fingerprint, packet_type, issuer, expectation",
    [
        (True, "KeyRevocation", "self", does_not_raise()),
        (True, "DirectKey", "self", does_not_raise()),
        (True, "GenericCertification", "self", does_not_raise()),
        (True, "KeyRevocation", None, does_not_raise()),
        (True, "CertificationRevocation", None, does_not_raise()),
        (True, "CertificationRevocation", "self", does_not_raise()),
        (True, "DirectKey", None, does_not_raise()),
        (True, "GenericCertification", None, does_not_raise()),
        (True, "KeyRevocation", "foo", raises(Exception)),
        (True, "DirectKey", "foo", does_not_raise()),
        (True, "GenericCertification", "foo", does_not_raise()),
        (True, "foo", "foo", does_not_raise()),
        (True, "foo", "self", raises(Exception)),
        (False, "KeyRevocation", True, raises(Exception)),
        (False, "DirectKey", True, raises(Exception)),
        (False, "GenericCertification", True, raises(Exception)),
        (False, "CertificationRevocation", True, raises(Exception)),
    ],
)
@patch("libkeyringctl.keyring.get_fingerprint_from_partial")
@patch("libkeyringctl.keyring.packet_dump_field")
def test_convert_pubkey_signature_packet(
    packet_dump_field_mock: Mock,
    get_fingerprint_from_partial_mock: Mock,
    valid_current_packet_fingerprint: bool,
    packet_type: str,
    issuer: Optional[str],
    expectation: ContextManager[str],
    working_dir: Path,
    valid_fingerprint: Fingerprint,
) -> None:
    packet = working_dir / "packet"
    key_revocations: Dict[Fingerprint, Path] = {}
    direct_revocations: Dict[Fingerprint, List[Path]] = defaultdict(list)
    direct_sigs: Dict[Fingerprint, List[Path]] = defaultdict(list)
    current_packet_fingerprint = None

    if valid_current_packet_fingerprint:
        current_packet_fingerprint = valid_fingerprint

    packet_dump_field_mock.return_value = packet_type
    if issuer == "self":
        get_fingerprint_from_partial_mock.return_value = valid_fingerprint
    else:
        get_fingerprint_from_partial_mock.return_value = None if issuer is None else Fingerprint(issuer)

    with expectation:
        keyring.convert_pubkey_signature_packet(
            packet=packet,
            certificate_fingerprint=valid_fingerprint,
            fingerprint_filter=None,
            current_packet_fingerprint=current_packet_fingerprint,
            key_revocations=key_revocations,
            direct_revocations=direct_revocations,
            direct_sigs=direct_sigs,
        )

        if issuer is None or current_packet_fingerprint is None:
            assert not direct_revocations and not direct_sigs and not key_revocations
        else:
            if packet_type == "KeyRevocation":
                assert key_revocations[valid_fingerprint] == packet
            elif packet_type in ["CertificationRevocation"]:
                if issuer != "self":
                    assert not direct_revocations
                else:
                    assert direct_revocations[valid_fingerprint if issuer == "self" else Fingerprint(issuer)] == [
                        packet
                    ]
            elif packet_type in ["DirectKey", "GenericCertification"]:
                if issuer != "self":
                    assert not direct_sigs
                else:
                    assert direct_sigs[valid_fingerprint if issuer == "self" else Fingerprint(issuer)] == [packet]


@mark.parametrize(
    "valid_current_packet_uid, packet_type, provide_issuer, issuer_in_filter, expectation",
    [
        (True, "CertificationRevocation", "self", True, does_not_raise()),
        (True, "CertificationRevocation", "self", False, does_not_raise()),
        (True, "SomeCertification", "self", True, does_not_raise()),
        (True, "SomeCertification", "self", False, does_not_raise()),
        (True, "CertificationRevocation", None, True, does_not_raise()),
        (True, "CertificationRevocation", None, False, does_not_raise()),
        (True, "SomeCertification", None, True, does_not_raise()),
        (True, "SomeCertification", None, False, does_not_raise()),
        (False, "CertificationRevocation", "self", True, raises(Exception)),
        (False, "CertificationRevocation", "self", False, raises(Exception)),
        (False, "SomeCertification", "self", True, raises(Exception)),
        (False, "SomeCertification", "self", False, raises(Exception)),
        (True, "foo", "self", True, raises(Exception)),
        (True, "foo", "self", False, raises(Exception)),
    ],
)
@patch("libkeyringctl.keyring.get_fingerprint_from_partial")
@patch("libkeyringctl.keyring.packet_dump_field")
def test_convert_uid_signature_packet(
    packet_dump_field_mock: Mock,
    get_fingerprint_from_partial_mock: Mock,
    valid_current_packet_uid: bool,
    packet_type: str,
    provide_issuer: Optional[str],
    issuer_in_filter: bool,
    expectation: ContextManager[str],
    working_dir: Path,
    valid_fingerprint: Fingerprint,
) -> None:
    packet = working_dir / "packet"
    certifications: Dict[Uid, Dict[Fingerprint, List[Path]]] = defaultdict(lambda: defaultdict(list))
    revocations: Dict[Uid, Dict[Fingerprint, List[Path]]] = defaultdict(lambda: defaultdict(list))
    current_packet_uid = None
    issuer = None
    fingerprint_filter: Set[Fingerprint] = {Fingerprint("foo")}

    if valid_current_packet_uid:
        current_packet_uid = Uid("Foobar McFooface <foo@barmcfoofa.ce>")

    packet_dump_field_mock.return_value = packet_type
    if provide_issuer == "self":
        issuer = valid_fingerprint
    else:
        if provide_issuer is not None:
            issuer = Fingerprint(provide_issuer)

    get_fingerprint_from_partial_mock.return_value = issuer

    if issuer_in_filter and issuer is not None:
        fingerprint_filter.add(issuer)

    with expectation:
        keyring.convert_uid_signature_packet(
            packet=packet,
            current_packet_uid=current_packet_uid,
            fingerprint_filter=fingerprint_filter,
            certifications=certifications,
            revocations=revocations,
        )

        if not valid_current_packet_uid or issuer is None:
            assert not certifications and not revocations
        else:
            if packet_type == "CertificationRevocation" and valid_current_packet_uid and issuer_in_filter:
                assert revocations[current_packet_uid][issuer] == [packet]  # type: ignore
            elif packet_type.endswith("Certification") and issuer_in_filter:
                assert certifications[current_packet_uid][issuer] == [packet]  # type: ignore
            elif packet_type.endswith("Certification") and not issuer_in_filter:
                assert not certifications


@mark.parametrize(
    "valid_current_packet_fingerprint, packet_type, issuer, expectation",
    [
        (True, "SubkeyBinding", "self", does_not_raise()),
        (True, "SubkeyRevocation", "self", does_not_raise()),
        (True, "SubkeyBinding", None, does_not_raise()),
        (True, "SubkeyRevocation", None, does_not_raise()),
        (True, "SubkeyBinding", "foo", raises(Exception)),
        (True, "SubkeyRevocation", "foo", raises(Exception)),
        (False, "SubkeyBinding", "self", raises(Exception)),
        (False, "SubkeyRevocation", "self", raises(Exception)),
        (True, "foo", "self", raises(Exception)),
    ],
)
@patch("libkeyringctl.keyring.get_fingerprint_from_partial")
@patch("libkeyringctl.keyring.packet_dump_field")
def test_convert_subkey_signature_packet(
    packet_dump_field_mock: Mock,
    get_fingerprint_from_partial_mock: Mock,
    valid_current_packet_fingerprint: bool,
    packet_type: str,
    issuer: Optional[str],
    expectation: ContextManager[str],
    working_dir: Path,
    valid_fingerprint: Fingerprint,
) -> None:
    packet = working_dir / "packet"
    subkey_bindings: Dict[Fingerprint, List[Path]] = defaultdict(list)
    subkey_revocations: Dict[Fingerprint, List[Path]] = defaultdict(list)
    current_packet_fingerprint = None

    if valid_current_packet_fingerprint:
        current_packet_fingerprint = valid_fingerprint

    packet_dump_field_mock.return_value = packet_type
    if issuer == "self":
        get_fingerprint_from_partial_mock.return_value = valid_fingerprint
    else:
        get_fingerprint_from_partial_mock.return_value = None if issuer is None else Fingerprint(issuer)

    with expectation:
        keyring.convert_subkey_signature_packet(
            packet=packet,
            certificate_fingerprint=valid_fingerprint,
            current_packet_fingerprint=current_packet_fingerprint,
            fingerprint_filter=None,
            subkey_bindings=subkey_bindings,
            subkey_revocations=subkey_revocations,
        )

        if issuer is None or not valid_current_packet_fingerprint:
            assert not subkey_bindings and not subkey_revocations
        else:
            if packet_type == "SubkeyBinding" and issuer == "self":
                assert subkey_bindings[valid_fingerprint] == [packet]
            elif packet_type == "SubkeyRevocation" and issuer == "self":
                assert subkey_revocations[valid_fingerprint] == [packet]


@mark.parametrize(
    "valid_certificate_fingerprint, current_packet_mode, expectation",
    [
        (True, "pubkey", does_not_raise()),
        (True, "uid", does_not_raise()),
        (True, "subkey", does_not_raise()),
        (True, "uattr", does_not_raise()),
        (False, "pubkey", raises(Exception)),
        (False, "uid", raises(Exception)),
        (False, "subkey", raises(Exception)),
        (False, "uattr", raises(Exception)),
        (True, "foo", raises(Exception)),
    ],
)
@patch("libkeyringctl.keyring.convert_pubkey_signature_packet")
@patch("libkeyringctl.keyring.convert_uid_signature_packet")
@patch("libkeyringctl.keyring.convert_subkey_signature_packet")
def test_convert_signature_packet(
    convert_subkey_signature_packet_mock: Mock,
    convert_uid_signature_packet_mock: Mock,
    convert_pubkey_signature_packet_mock: Mock,
    valid_certificate_fingerprint: bool,
    current_packet_mode: str,
    expectation: ContextManager[str],
    valid_fingerprint: Fingerprint,
) -> None:
    certificate_fingerprint = None
    key_revocations: Dict[Fingerprint, Path] = {}
    direct_revocations: Dict[Fingerprint, List[Path]] = defaultdict(list)
    direct_sigs: Dict[Fingerprint, List[Path]] = defaultdict(list)
    certifications: Dict[Uid, Dict[Fingerprint, List[Path]]] = defaultdict(lambda: defaultdict(list))
    revocations: Dict[Uid, Dict[Fingerprint, List[Path]]] = defaultdict(lambda: defaultdict(list))
    subkey_bindings: Dict[Fingerprint, List[Path]] = defaultdict(list)
    subkey_revocations: Dict[Fingerprint, List[Path]] = defaultdict(list)

    if valid_certificate_fingerprint:
        certificate_fingerprint = valid_fingerprint

    with expectation:
        keyring.convert_signature_packet(
            packet=Path("foo"),
            current_packet_mode=current_packet_mode,
            certificate_fingerprint=certificate_fingerprint,
            fingerprint_filter=None,
            key_revocations=key_revocations,
            current_packet_fingerprint=None,
            current_packet_uid=None,
            direct_revocations=direct_revocations,
            direct_sigs=direct_sigs,
            certifications=certifications,
            revocations=revocations,
            subkey_bindings=subkey_bindings,
            subkey_revocations=subkey_revocations,
        )

        if current_packet_mode == "pubkey":
            convert_pubkey_signature_packet_mock.assert_called_once()
        elif current_packet_mode == "uid":
            convert_uid_signature_packet_mock.assert_called_once()
        elif current_packet_mode == "subkey":
            convert_subkey_signature_packet_mock.assert_called_once()


@mark.parametrize(
    "packet, packet_split, packet_dump_field, name_override, expectation",
    [
        (
            Path("foo.asc"),
            [
                Path(keyring.PacketType.PUBLIC_KEY.value),
                Path(keyring.PacketType.SIGNATURE.value),
                Path(keyring.PacketType.USER_ID.value),
                Path(keyring.PacketType.USER_ATTRIBUTE.value),
                Path(keyring.PacketType.PUBLIC_SUBKEY.value),
                Path(keyring.PacketType.SIGNATURE.value),
            ],
            [
                "".join(choice("ABCDEF" + digits) for _ in range(40)),
                "foo <foo@bar.com>",
                "".join(choice("ABCDEF" + digits) for _ in range(40)),
            ],
            "bar",
            does_not_raise(),
        ),
        (
            Path("foo.asc"),
            [
                Path(keyring.PacketType.PUBLIC_KEY.value),
                Path(keyring.PacketType.SIGNATURE.value),
                Path(keyring.PacketType.USER_ID.value),
                Path(keyring.PacketType.USER_ID.value),
            ],
            [
                "".join(choice("ABCDEF" + digits) for _ in range(40)),
                "foo <foo@bar.com>",
                "foo <foo@bar.com>",
            ],
            "bar",
            raises(Exception),
        ),
        (
            Path("foo.asc"),
            [
                Path(keyring.PacketType.SECRET_KEY.value),
            ],
            [],
            None,
            raises(Exception),
        ),
        (
            Path("foo.asc"),
            [
                Path("foo"),
            ],
            [],
            None,
            raises(Exception),
        ),
        (
            Path("foo.asc"),
            [
                Path(keyring.PacketType.PUBLIC_KEY.value),
            ],
            [
                None,
            ],
            "bar",
            raises(Exception),
        ),
    ],
)
@patch("libkeyringctl.keyring.persist_key_material")
@patch("libkeyringctl.keyring.packet_split")
@patch("libkeyringctl.keyring.convert_signature_packet")
@patch("libkeyringctl.keyring.packet_dump_field")
@patch("libkeyringctl.keyring.derive_username_from_fingerprint")
def test_convert_certificate(
    derive_username_from_fingerprint_mock: Mock,
    packet_dump_field_mock: Mock,
    convert_signature_packet_mock: Mock,
    packet_split_mock: Mock,
    persist_key_material_mock: Mock,
    packet: Path,
    packet_split: List[Path],
    packet_dump_field: List[str],
    name_override: Optional[Username],
    expectation: ContextManager[str],
    working_dir: Path,
    keyring_dir: Path,
) -> None:
    packet_split_mock.return_value = packet_split
    packet_dump_field_mock.side_effect = packet_dump_field

    with expectation:
        keyring.convert_certificate(
            working_dir=working_dir,
            certificate=packet,
            keyring_dir=keyring_dir,
            name_override=name_override,
            fingerprint_filter=None,
        )


@patch("libkeyringctl.keyring.latest_certification")
@patch("libkeyringctl.keyring.packet_join")
def test_persist_subkey_revocations(
    packet_join_mock: Mock,
    latest_certification_mock: Mock,
    working_dir: Path,
    keyring_dir: Path,
    valid_fingerprint: Fingerprint,
) -> None:
    revocation_packet = working_dir / "latest_revocation.asc"
    latest_certification_mock.return_value = revocation_packet
    subkey_revocations: Dict[Fingerprint, List[Path]] = {
        valid_fingerprint: [revocation_packet, working_dir / "earlier_revocation.asc"]
    }
    keyring.persist_subkey_revocations(
        key_dir=keyring_dir,
        subkey_revocations=subkey_revocations,
        issuer=valid_fingerprint,
    )
    packet_join_mock.assert_called_once_with(
        packets=[revocation_packet],
        output=keyring_dir / "subkey" / valid_fingerprint / "revocation" / f"{valid_fingerprint}.asc",
        force=True,
    )


@patch("libkeyringctl.keyring.packet_signature_creation_time")
@patch("libkeyringctl.keyring.packet_join")
def test_persist_directkey_revocations(
    packet_join_mock: Mock,
    packet_signature_creation_time_mock: Mock,
    working_dir: Path,
    keyring_dir: Path,
    valid_fingerprint: Fingerprint,
) -> None:
    revocation_packet = working_dir / "latest_revocation.asc"
    directkey_revocations: Dict[Fingerprint, List[Path]] = {valid_fingerprint: [revocation_packet]}

    dt = datetime(2000, 1, 12, 11, 22, 33)
    packet_signature_creation_time_mock.return_value = dt
    keyring.persist_direct_key_revocations(
        key_dir=keyring_dir,
        direct_key_revocations=directkey_revocations,
    )
    packet_join_mock.assert_called_once_with(
        packets=[revocation_packet],
        output=keyring_dir
        / "directkey"
        / "revocation"
        / valid_fingerprint
        / f"{dt.strftime(PACKET_FILENAME_DATETIME_FORMAT)}.asc",
        force=True,
    )


@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
def test_convert(working_dir: Path, keyring_dir: Path) -> None:
    keyring.convert(
        working_dir=working_dir,
        keyring_root=keyring_dir,
        sources=test_certificates[Username("foobar")],
        target_dir=keyring_dir / "packager",
    )

    with raises(Exception):
        keyring.convert(
            working_dir=working_dir,
            keyring_root=keyring_dir,
            sources=test_keys[Username("foobar")],
            target_dir=keyring_dir / "packager",
        )


@create_certificate(username=Username("main"), uids=[Uid("main <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
@create_uid_certification(issuer=Username("main"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_signature_revocation(issuer=Username("main"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
def test_clean_keyring(working_dir: Path, keyring_dir: Path) -> None:
    # first pass clean up certification
    keyring.clean_keyring(keyring=keyring_dir)
    # second pass skipping clean up because lack of certification
    keyring.clean_keyring(keyring=keyring_dir)


@create_certificate(username=Username("main"), uids=[Uid("main <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("other_main"), uids=[Uid("other main <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
def test_export_ownertrust(working_dir: Path, keyring_dir: Path) -> None:
    output = working_dir / "build"

    keyring.export_ownertrust(
        certs=[keyring_dir / "main"],
        keyring_root=keyring_dir,
        output=output,
    )

    with open(file=output, mode="r") as output_file:
        for line in output_file.readlines():
            assert line.split(":")[0] in test_main_fingerprints


@create_certificate(username=Username("main"), uids=[Uid("main <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
@create_key_revocation(username=Username("foobar"))
def test_export_revoked(working_dir: Path, keyring_dir: Path) -> None:
    output = working_dir / "build"

    keyring.export_revoked(
        certs=[keyring_dir / "packager"],
        keyring_root=keyring_dir,
        main_keys=test_main_fingerprints,
        output=output,
    )

    revoked_fingerprints = test_all_fingerprints - test_main_fingerprints
    with open(file=output, mode="r") as output_file:
        for line in output_file.readlines():
            assert line.strip() in revoked_fingerprints


@mark.parametrize("path_exists", [(True), (False)])
@create_certificate(username=Username("main"), uids=[Uid("main <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
@create_uid_certification(issuer=Username("main"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_key_revocation(username=Username("foobar"))
def test_get_packets_from_path(working_dir: Path, keyring_dir: Path, path_exists: bool) -> None:
    if not path_exists:
        assert keyring.get_packets_from_path(path=working_dir / "nope") == []
    else:
        for username, paths in test_keyring_certificates.items():
            for path in paths:
                keyring.get_packets_from_path(path=path)


@mark.parametrize("path_exists", [(True), (False)])
@patch("libkeyringctl.keyring.get_packets_from_path")
def test_get_packets_from_listing(get_packets_from_path_mock: Mock, working_dir: Path, path_exists: bool) -> None:
    path = working_dir / "path"
    if not path_exists:
        assert keyring.get_packets_from_listing(path=path) == []
    else:
        get_packets_from_path_mock.return_value = []
        sub_path = path / "sub"
        sub_path.mkdir(parents=True)
        assert keyring.get_packets_from_listing(path=path) == []
        get_packets_from_path_mock.assert_called_once_with(sub_path)


@create_certificate(username=Username("main"), uids=[Uid("main <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
@create_uid_certification(issuer=Username("main"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_key_revocation(username=Username("foobar"))
def test_export(working_dir: Path, keyring_dir: Path) -> None:
    output_file = working_dir / "output"

    empty_dir = working_dir / "empty"
    empty_dir.mkdir()
    assert not keyring.export(working_dir=working_dir, keyring_root=empty_dir, sources=None, output=output_file)
    assert not output_file.exists()

    keyring.export(working_dir=working_dir, keyring_root=keyring_dir, sources=None, output=output_file)
    assert output_file.exists()


@create_certificate(username=Username("main"), uids=[Uid("main <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
@create_uid_certification(issuer=Username("main"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_key_revocation(username=Username("foobar"))
def test_build(working_dir: Path, keyring_dir: Path) -> None:
    output_dir = working_dir / "output"

    with raises(FileNotFoundError):
        empty_dir = working_dir / "empty"
        empty_dir.mkdir()
        keyring.build(working_dir=working_dir, keyring_root=empty_dir, target_dir=output_dir)

    keyring.build(working_dir=working_dir, keyring_root=keyring_dir, target_dir=output_dir)
    assert (
        (output_dir / "archlinux.gpg").exists()
        and (output_dir / "archlinux-trusted").exists()
        and (output_dir / "archlinux-revoked").exists()
    )


@mark.parametrize(
    "create_dir, duplicate_fingerprints, expectation",
    [
        (True, False, does_not_raise()),
        (True, True, raises(Exception)),
        (False, False, does_not_raise()),
        (False, True, does_not_raise()),
    ],
)
def test_derive_username_from_fingerprint(
    create_dir: bool,
    duplicate_fingerprints: bool,
    expectation: ContextManager[str],
    keyring_dir: Path,
    valid_fingerprint: str,
) -> None:
    username = "username"
    other_username = "other_user"

    typed_keyring_dir = keyring_dir / "type"

    if create_dir:
        (typed_keyring_dir / username / valid_fingerprint).mkdir(parents=True)
        if duplicate_fingerprints:
            (typed_keyring_dir / other_username / valid_fingerprint).mkdir(parents=True)

    with expectation:
        returned_username = keyring.derive_username_from_fingerprint(
            keyring_dir=typed_keyring_dir,
            certificate_fingerprint=Fingerprint(valid_fingerprint),
        )
        if create_dir and not duplicate_fingerprints:
            assert returned_username == username
        else:
            assert returned_username is None


@create_certificate(username=Username("main"), uids=[Uid("main <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
def test_list_keyring(working_dir: Path, keyring_dir: Path) -> None:
    packager_fingerprints = test_all_fingerprints - test_main_fingerprints

    with patch("builtins.print") as print_mock:
        keyring.list_keyring(keyring_root=keyring_dir, sources=None, main_keys=False)
        print_args = [mock_call[1][0] for mock_call in print_mock.mock_calls]
        for fingerprint in packager_fingerprints:
            assert any([fingerprint in print_arg for print_arg in print_args])

    with patch("builtins.print") as print_mock:
        keyring.list_keyring(keyring_root=keyring_dir, sources=None, main_keys=True)
        print_args = [mock_call[1][0] for mock_call in print_mock.mock_calls]
        for fingerprint in test_main_fingerprints:
            assert any([fingerprint in print_arg for print_arg in print_args])

    for name, paths in test_keyring_certificates.items():
        if all(["main" in str(path) for path in paths]):
            for path in paths:
                with patch("builtins.print") as print_mock:
                    keyring.list_keyring(keyring_root=keyring_dir, sources=[path], main_keys=True)
                    print_args = [mock_call[1][0] for mock_call in print_mock.mock_calls]
                    assert name in print_args[0] and path.stem in print_args[0]
        elif all(["packager" in str(path) for path in paths]):
            for path in paths:
                with patch("builtins.print") as print_mock:
                    keyring.list_keyring(keyring_root=keyring_dir, sources=[path], main_keys=False)
                    print_args = [mock_call[1][0] for mock_call in print_mock.mock_calls]
                    assert name in print_args[0] and path.stem in print_args[0]
        with patch("builtins.print") as print_mock:
            keyring.list_keyring(
                keyring_root=keyring_dir, sources=paths, main_keys=False, trust_filter=TrustFilter.revoked
            )
            print_args = [mock_call[1][0] for mock_call in print_mock.mock_calls]
            assert not print_args


@create_certificate(username=Username("main"), uids=[Uid("main <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
def test_inspect_keyring(working_dir: Path, keyring_dir: Path) -> None:
    inspect_string = keyring.inspect_keyring(working_dir=working_dir, keyring_root=keyring_dir, sources=None)
    for fingerprint in test_all_fingerprints:
        assert fingerprint in inspect_string

    for name, paths in test_keyring_certificates.items():
        if all(["main" in str(path) for path in paths]):
            for path in paths:
                inspect_string = keyring.inspect_keyring(
                    working_dir=working_dir,
                    keyring_root=keyring_dir,
                    sources=[path],
                )
                assert path.stem in inspect_string
        elif all(["packager" in str(path) for path in paths]):
            for path in paths:
                inspect_string = keyring.inspect_keyring(
                    working_dir=working_dir,
                    keyring_root=keyring_dir,
                    sources=[path],
                )
                assert path.stem in inspect_string


def test_get_fingerprints_from_paths(keyring_dir: Path, valid_fingerprint: str, valid_subkey_fingerprint: str) -> None:
    fingerprint_dir = keyring_dir / "type" / "username" / valid_fingerprint
    fingerprint_dir.mkdir(parents=True)
    (fingerprint_dir / (fingerprint_dir.name + ".asc")).touch()

    fingerprint_subkey_dir = fingerprint_dir / "subkey" / valid_subkey_fingerprint
    fingerprint_subkey_dir.mkdir(parents=True)
    fingerprint_subkey_asc = fingerprint_subkey_dir / (fingerprint_subkey_dir.name + ".asc")
    fingerprint_subkey_asc.touch()

    assert keyring.get_fingerprints_from_paths(sources=[fingerprint_subkey_dir]) == {
        Fingerprint(valid_subkey_fingerprint)
    }
    assert keyring.get_fingerprints_from_paths(sources=[fingerprint_dir]) == {Fingerprint(valid_fingerprint)}
