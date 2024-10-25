from contextlib import nullcontext as does_not_raise
from datetime import datetime
from datetime import timedelta
from datetime import timezone
from pathlib import Path
from tempfile import TemporaryDirectory
from typing import ContextManager
from typing import Dict
from typing import Optional
from unittest.mock import Mock
from unittest.mock import patch

from pytest import mark
from pytest import raises

from libkeyringctl import sequoia
from libkeyringctl.types import Fingerprint
from libkeyringctl.types import PacketKind
from libkeyringctl.types import Uid
from libkeyringctl.types import Username


@mark.parametrize(
    "create_subdir, preserve_filename",
    [
        (False, True),
        (False, False),
        (True, True),
        (True, False),
    ],
)
@patch("libkeyringctl.sequoia.system")
@patch("libkeyringctl.sequoia.mkdtemp")
def test_keyring_split(mkdtemp_mock: Mock, system_mock: Mock, create_subdir: bool, preserve_filename: bool) -> None:
    with TemporaryDirectory() as tmp_dir_name:
        tmp_dir = Path(tmp_dir_name)

        keyring_tmp_dir = tmp_dir / "keyring"
        keyring_tmp_dir.mkdir()
        mkdtemp_mock.return_value = keyring_tmp_dir.absolute()

        if create_subdir:
            keyring_sub_dir = keyring_tmp_dir / "foo"
            keyring_sub_dir.mkdir()

        returned = sequoia.keyring_split(
            working_dir=tmp_dir,
            keyring=Path("foo"),
            preserve_filename=preserve_filename,
        )

        if create_subdir:
            assert returned == [keyring_sub_dir]
        else:
            assert returned == []


@mark.parametrize(
    "output",
    [
        None,
        Path("output"),
    ],
)
@patch("libkeyringctl.sequoia.system")
def test_keyring_merge(system_mock: Mock, output: Optional[Path]) -> None:
    certificates = [Path("foo"), Path("bar")]
    system_mock.return_value = "return"

    assert sequoia.keyring_merge(certificates=certificates, output=output) == "return"

    name, args, kwargs = system_mock.mock_calls[0]
    for cert in certificates:
        assert str(cert) in args[0]
    if output:
        assert "--output" in args[0] and str(output) in args[0]


@patch("libkeyringctl.sequoia.system")
@patch("libkeyringctl.sequoia.mkdtemp")
def test_packet_split(mkdtemp_mock: Mock, system_mock: Mock) -> None:
    certificate = Path("certificate")
    with TemporaryDirectory() as tmp_dir_name:
        tmp_dir = Path(tmp_dir_name)

        keyring_tmp_dir = tmp_dir / "keyring"
        keyring_tmp_dir.mkdir()
        mkdtemp_mock.return_value = keyring_tmp_dir.absolute()
        keyring_sub_dir = keyring_tmp_dir / "foo"
        keyring_sub_dir.mkdir()

        assert sequoia.packet_split(working_dir=tmp_dir, certificate=certificate) == [keyring_sub_dir]
        name, args, kwargs = system_mock.mock_calls[0]
        assert str(certificate) == args[0][-1]


@mark.parametrize("output, force", [(None, True), (None, False), (Path("output"), True), (Path("output"), False)])
@patch("libkeyringctl.sequoia.system")
def test_packet_join(system_mock: Mock, output: Optional[Path], force: bool) -> None:
    packets = [Path("packet1"), Path("packet2")]
    system_return = "return"
    system_mock.return_value = system_return

    assert sequoia.packet_join(packets, output=output, force=force) == system_return

    name, args, kwargs = system_mock.mock_calls[0]
    for packet in packets:
        assert str(packet) in args[0]
    if force:
        assert "--force" == args[0][1]
    if output:
        assert "--output" in args[0] and str(output) in args[0]


@mark.parametrize(
    "certifications_in_result, certifications, fingerprints",
    [
        ("something: 0123456789123456789012345678901234567890\n", True, None),
        ("something: 0123456789123456789012345678901234567890\n", False, None),
        (
            "something: 0123456789123456789012345678901234567890\n",
            True,
            {Fingerprint("0123456789123456789012345678901234567890"): Username("foo")},
        ),
        (
            "something: 0123456789123456789012345678901234567890\n",
            False,
            {Fingerprint("0123456789123456789012345678901234567890"): Username("foo")},
        ),
        (
            "something: 5678901234567890\n",
            True,
            {Fingerprint("0123456789123456789012345678901234567890"): Username("foo")},
        ),
        (
            "something: 5678901234567890\n",
            False,
            {Fingerprint("0123456789123456789012345678901234567890"): Username("foo")},
        ),
    ],
)
@patch("libkeyringctl.sequoia.system")
def test_inspect(
    system_mock: Mock,
    certifications_in_result: str,
    certifications: bool,
    fingerprints: Optional[Dict[Fingerprint, Username]],
) -> None:
    packet = Path("packet")
    result_header = "result\n"

    if certifications:
        system_mock.return_value = result_header + "\n" + certifications_in_result
    else:
        system_mock.return_value = result_header

    returned = sequoia.inspect(packet=packet, certifications=certifications, fingerprints=fingerprints)

    if fingerprints and certifications:
        for fingerprint, username in fingerprints.items():
            assert f"{fingerprint[24:]} {username}" in returned
    assert result_header in returned


@patch("libkeyringctl.sequoia.system")
def test_packet_dump(system_mock: Mock) -> None:
    system_mock.return_value = "return"
    assert sequoia.packet_dump(packet=Path("packet")) == "return"
    system_mock.assert_called_once_with(["sq", "toolbox", "packet", "dump", "packet"])


@mark.parametrize(
    "packet_dump_return, query, result, expectation",
    [
        (
            """
Signature Packet
    Version: 4
    Type: SubkeyBinding
    Hash algo: SHA512
""",
            "Type",
            "SubkeyBinding",
            does_not_raise(),
        ),
        (
            """
Signature Packet
    Version: 4
    Type: SubkeyBinding
    Hash algo: SHA512
    Hashed area:
      Signature creation time: 2022-12-31 15:53:59 UTC
      Issuer: BBBBBB
    Unhashed area:
      Issuer: 42424242
""",
            "Unhashed area.Issuer",
            "42424242",
            does_not_raise(),
        ),
        (
            """
Signature Packet
    Version: 4
    Type: SubkeyBinding
    Hash algo: SHA512
    Hashed area:
      Signature creation time: 2022-12-31 15:53:59 UTC
    Unhashed area:
      Issuer: 42424242
""",
            "Hashed area|Unhashed area.Issuer",
            "42424242",
            does_not_raise(),
        ),
        (
            """
Signature Packet
    Version: 4
    Type: SubkeyBinding
    Hash algo: SHA1
    Hashed area:
      Signature creation time: 2022-12-31
""",
            "*.Signature creation time",
            "2022-12-31",
            does_not_raise(),
        ),
        (
            """
Signature Packet
    a:
      b:
        x: foo
    b:
      b:
        c: bar
""",
            "*.b.c",
            "bar",
            does_not_raise(),
        ),
        (
            """
Signature Packet
    a:
      b:
        x:
          y:
            z: foo
    b:
      b:
        x:
          y:
            z: foo
          w:
            w: foo
        k:
          i:
            c: bar
""",
            "*.b.*.*.c",
            "bar",
            does_not_raise(),
        ),
        (
            """
Signature Packet
    a:
      c:
        b: foo
    a:
      b: bar
""",
            "a.b",
            "bar",
            does_not_raise(),
        ),
        (
            """
Signature Packet
    Version: 4
    Type: SubkeyBinding
    Hash algo: SHA512
    Hashed area:
      Signature creation time: 2022-12-31 15:53:59 UTC
    Unhashed area:
      Issuer: 42424242
    Issuer: BBBBBBBB
""",
            "Hashed area.Issuer",
            None,
            raises(Exception),
        ),
    ],
)
@patch("libkeyringctl.sequoia.packet_dump")
def test_packet_dump_field(
    packet_dump_mock: Mock,
    packet_dump_return: str,
    query: str,
    result: str,
    expectation: ContextManager[str],
) -> None:
    packet_dump_mock.return_value = packet_dump_return

    with expectation:
        assert sequoia.packet_dump_field(packet=Path("packet"), query=query) == result


@patch("libkeyringctl.sequoia.packet_dump_field")
def test_packet_signature_creation_time(packet_dump_field_mock: Mock) -> None:
    creation_time = "2021-10-31 00:48:09 UTC"
    packet_dump_field_mock.return_value = creation_time
    assert sequoia.packet_signature_creation_time(packet=Path("packet")) == datetime.strptime(
        creation_time, "%Y-%m-%d %H:%M:%S %Z"
    )


@patch("libkeyringctl.sequoia.packet_dump")
def test_packet_kinds(packet_dump_mock: Mock) -> None:
    lines = [
        "Type1 something",
        " foo",
        "Type2",
        "WARNING",
        "Type3  other",
        " bar",
    ]
    path = Path("foo")
    packet_dump_mock.return_value = "\n".join(lines)

    assert sequoia.packet_kinds(packet=path) == [PacketKind("Type1"), PacketKind("Type2"), PacketKind("Type3")]


@patch("libkeyringctl.sequoia.packet_signature_creation_time")
def test_latest_certification(packet_signature_creation_time_mock: Mock) -> None:
    now = datetime.now(tz=timezone.utc)
    later = now + timedelta(days=1)
    early_cert = Path("cert1")
    later_cert = Path("cert2")

    packet_signature_creation_time_mock.side_effect = [now, later]
    assert sequoia.latest_certification(certifications=[early_cert, later_cert]) == later_cert

    packet_signature_creation_time_mock.side_effect = [later, now]
    assert sequoia.latest_certification(certifications=[later_cert, early_cert]) == later_cert


@mark.parametrize("output", [(None), (Path("output"))])
@patch("libkeyringctl.sequoia.system")
def test_key_extract_certificate(system_mock: Mock, output: Optional[Path]) -> None:
    system_mock.return_value = "return"
    assert sequoia.key_extract_certificate(key=Path("key"), output=output) == "return"
    name, args, kwargs = system_mock.mock_calls[0]
    if output:
        assert str(output) == args[0][-1]


@mark.parametrize("output", [(None), (Path("output"))])
@patch("libkeyringctl.sequoia.system")
def test_certify(system_mock: Mock, output: Optional[Path]) -> None:
    system_mock.return_value = "return"
    assert sequoia.certify(key=Path("key"), certificate=Path("cert"), uid=Uid("uid"), output=output) == "return"
    name, args, kwargs = system_mock.mock_calls[0]
    if output:
        assert str(output) == args[0][-1]
