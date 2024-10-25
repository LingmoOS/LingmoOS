from pathlib import Path
from typing import List
from unittest.mock import Mock
from unittest.mock import patch

from pytest import mark
from pytest import raises

from libkeyringctl.trust import certificate_trust
from libkeyringctl.trust import certificate_trust_from_paths
from libkeyringctl.trust import filter_by_trust
from libkeyringctl.trust import format_trust_label
from libkeyringctl.trust import trust_color
from libkeyringctl.trust import trust_icon
from libkeyringctl.types import Color
from libkeyringctl.types import Fingerprint
from libkeyringctl.types import Trust
from libkeyringctl.types import TrustFilter
from libkeyringctl.types import Uid
from libkeyringctl.types import Username

from .conftest import create_certificate
from .conftest import create_key_revocation
from .conftest import create_signature_revocation
from .conftest import create_uid_certification
from .conftest import test_all_fingerprints
from .conftest import test_keyring_certificates
from .conftest import test_main_fingerprints


@mark.parametrize(
    "sources",
    [
        ([Path("foobar")]),
        ([Path("foobar"), Path("quxdoo")]),
    ],
)
@patch("libkeyringctl.trust.certificate_trust")
def test_certificate_trust_from_paths(
    certificate_trust_mock: Mock,
    sources: List[Path],
    working_dir: Path,
) -> None:
    certificate_trust_mock.return_value = Trust.full
    for source in sources:
        source.mkdir(parents=True, exist_ok=True)
        cert = source / "foo.asc"
        cert.touch()

    trusts = certificate_trust_from_paths(
        sources=sources, main_keys=test_main_fingerprints, all_fingerprints=test_all_fingerprints
    )
    for i, source in enumerate(sources):
        name, args, kwargs = certificate_trust_mock.mock_calls[i]
        assert kwargs["certificate"] == source
        assert kwargs["main_keys"] == test_main_fingerprints
        assert kwargs["all_fingerprints"] == test_all_fingerprints
        fingerprint = Fingerprint(source.name)
        assert Trust.full == trusts[fingerprint]
    assert len(trusts) == len(sources)


@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")], keyring_type="main")
def test_certificate_trust_main_key_has_full_trust(working_dir: Path, keyring_dir: Path) -> None:
    trust = certificate_trust(
        test_keyring_certificates[Username("foobar")][0],
        test_main_fingerprints,
        test_all_fingerprints,
    )
    assert Trust.full == trust


@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")], keyring_type="main")
@create_key_revocation(username=Username("foobar"), keyring_type="main")
def test_certificate_trust_main_key_revoked(working_dir: Path, keyring_dir: Path) -> None:
    trust = certificate_trust(
        test_keyring_certificates[Username("foobar")][0],
        test_main_fingerprints,
        test_all_fingerprints,
    )
    assert Trust.revoked == trust


@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")], keyring_type="main")
@create_key_revocation(username=Username("foobar"), keyring_type="main")
def test_certificate_trust_main_key_revoked_unknown_fingerprint_lookup(working_dir: Path, keyring_dir: Path) -> None:
    fingerprint = Fingerprint(test_keyring_certificates[Username("foobar")][0].name)
    revocation = list((keyring_dir / "main" / "foobar" / fingerprint / "revocation").iterdir())[0]
    revocation.rename(revocation.parent / "12341234.asc")
    with raises(Exception):
        certificate_trust(
            test_keyring_certificates[Username("foobar")][0],
            test_main_fingerprints,
            {Fingerprint("12341234")},
        )


@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")], keyring_type="main")
@create_key_revocation(username=Username("foobar"), keyring_type="main")
def test_certificate_trust_main_key_revoked_unknown_self_revocation(working_dir: Path, keyring_dir: Path) -> None:
    fingerprint = Fingerprint(test_keyring_certificates[Username("foobar")][0].name)
    revocation = list((keyring_dir / "main" / "foobar" / fingerprint / "revocation").iterdir())[0]
    revocation.rename(revocation.parent / "12341234.asc")
    with raises(Exception):
        certificate_trust(
            test_keyring_certificates[Username("foobar")][0],
            test_main_fingerprints,
            set(),
        )


@create_certificate(username=Username("main"), uids=[Uid("main <foo@bar.xyz>")])
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
def test_certificate_trust_no_signature_is_unknown(working_dir: Path, keyring_dir: Path) -> None:
    trust = certificate_trust(
        test_keyring_certificates[Username("foobar")][0],
        test_main_fingerprints,
        test_all_fingerprints,
    )
    assert Trust.unknown == trust


@create_certificate(username=Username("main"), uids=[Uid("main <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
@create_uid_certification(issuer=Username("main"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
def test_certificate_trust_one_signature_is_marginal(working_dir: Path, keyring_dir: Path) -> None:
    trust = certificate_trust(
        test_keyring_certificates[Username("foobar")][0],
        test_main_fingerprints,
        test_all_fingerprints,
    )
    assert Trust.marginal == trust


@create_certificate(username=Username("main"), uids=[Uid("main <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("not_main"), uids=[Uid("main <foo@bar.xyz>")])
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
@create_uid_certification(issuer=Username("not_main"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
def test_certificate_trust_one_none_main_signature_gives_no_trust(working_dir: Path, keyring_dir: Path) -> None:
    trust = certificate_trust(
        test_keyring_certificates[Username("foobar")][0],
        test_main_fingerprints,
        test_all_fingerprints,
    )
    assert Trust.unknown == trust


@create_certificate(username=Username("main1"), uids=[Uid("main1 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("main2"), uids=[Uid("main2 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("main3"), uids=[Uid("main3 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
@create_uid_certification(issuer=Username("main1"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_uid_certification(issuer=Username("main2"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_uid_certification(issuer=Username("main3"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
def test_certificate_trust_three_main_signature_gives_full_trust(working_dir: Path, keyring_dir: Path) -> None:
    trust = certificate_trust(
        test_keyring_certificates[Username("foobar")][0],
        test_main_fingerprints,
        test_all_fingerprints,
    )
    assert Trust.full == trust


@create_certificate(username=Username("main1"), uids=[Uid("main1 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("main2"), uids=[Uid("main2 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("main3"), uids=[Uid("main3 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
@create_uid_certification(issuer=Username("main1"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_uid_certification(issuer=Username("main2"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_uid_certification(issuer=Username("main3"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_key_revocation(username=Username("main3"), keyring_type="main")
def test_certificate_trust_three_main_signature_one_revoked(working_dir: Path, keyring_dir: Path) -> None:
    trust = certificate_trust(
        test_keyring_certificates[Username("foobar")][0],
        test_main_fingerprints,
        test_all_fingerprints,
    )
    assert Trust.marginal == trust


@create_certificate(username=Username("main"), uids=[Uid("main <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
@create_key_revocation(username=Username("foobar"))
def test_certificate_trust_revoked_key(working_dir: Path, keyring_dir: Path) -> None:
    trust = certificate_trust(
        test_keyring_certificates[Username("foobar")][0],
        test_main_fingerprints,
        test_all_fingerprints,
    )
    assert Trust.revoked == trust


@create_certificate(username=Username("main"), uids=[Uid("main <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
@create_uid_certification(issuer=Username("main"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_signature_revocation(issuer=Username("main"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
def test_certificate_trust_one_signature_revoked(working_dir: Path, keyring_dir: Path) -> None:
    trust = certificate_trust(
        test_keyring_certificates[Username("foobar")][0],
        test_main_fingerprints,
        test_all_fingerprints,
    )
    assert Trust.revoked == trust


@create_certificate(username=Username("main1"), uids=[Uid("main1 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("main2"), uids=[Uid("main2 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("main3"), uids=[Uid("main3 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
@create_uid_certification(issuer=Username("main1"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_uid_certification(issuer=Username("main2"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_uid_certification(issuer=Username("main3"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_signature_revocation(issuer=Username("main3"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
def test_certificate_trust_revoked_if_below_full(working_dir: Path, keyring_dir: Path) -> None:
    trust = certificate_trust(
        test_keyring_certificates[Username("foobar")][0],
        test_main_fingerprints,
        test_all_fingerprints,
    )
    assert Trust.revoked == trust


@create_certificate(username=Username("main1"), uids=[Uid("main1 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("main2"), uids=[Uid("main2 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("main3"), uids=[Uid("main3 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("main4"), uids=[Uid("main4 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
@create_uid_certification(issuer=Username("main1"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_uid_certification(issuer=Username("main2"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_uid_certification(issuer=Username("main3"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_uid_certification(issuer=Username("main4"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_signature_revocation(issuer=Username("main4"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
def test_certificate_trust_full_remains_if_enough_sigs_present(working_dir: Path, keyring_dir: Path) -> None:
    trust = certificate_trust(
        test_keyring_certificates[Username("foobar")][0],
        test_main_fingerprints,
        test_all_fingerprints,
    )
    assert Trust.full == trust


@create_certificate(username=Username("main1"), uids=[Uid("main1 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("main2"), uids=[Uid("main2 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("main3"), uids=[Uid("main3 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>"), Uid("old <old@old.old>")])
@create_uid_certification(issuer=Username("main1"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_uid_certification(issuer=Username("main2"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_signature_revocation(issuer=Username("foobar"), certified=Username("foobar"), uid=Uid("old <old@old.old>"))
def test_certificate_trust_not_revoked_if_only_one_uid_is_self_revoked(working_dir: Path, keyring_dir: Path) -> None:
    trust = certificate_trust(
        test_keyring_certificates[Username("foobar")][0],
        test_main_fingerprints,
        test_all_fingerprints,
    )
    assert Trust.marginal == trust


@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>"), Uid("old <old@old.old>")])
@create_signature_revocation(issuer=Username("foobar"), certified=Username("foobar"), uid=Uid("old <old@old.old>"))
def test_certificate_trust_unknown_if_only_contains_self_revoked(working_dir: Path, keyring_dir: Path) -> None:
    trust = certificate_trust(
        test_keyring_certificates[Username("foobar")][0],
        test_main_fingerprints,
        test_all_fingerprints,
    )
    assert Trust.unknown == trust


@create_certificate(username=Username("main1"), uids=[Uid("main1 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>"), Uid("old <old@old.old>")])
@create_uid_certification(issuer=Username("main1"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
def test_certificate_trust_missing_signature_fingerprint_lookup(working_dir: Path, keyring_dir: Path) -> None:
    with raises(Exception):
        certificate_trust(
            test_keyring_certificates[Username("foobar")][0],
            test_main_fingerprints,
            set(),
        )


@create_certificate(username=Username("foobar"), uids=[Uid("old <old@old.old>")])
@create_signature_revocation(issuer=Username("foobar"), certified=Username("foobar"), uid=Uid("old <old@old.old>"))
def test_certificate_trust_missing_revocation_fingerprint_lookup(working_dir: Path, keyring_dir: Path) -> None:
    with raises(Exception):
        certificate_trust(
            test_keyring_certificates[Username("foobar")][0],
            test_main_fingerprints,
            set(),
        )


@create_certificate(username=Username("main1"), uids=[Uid("main1 <foo@bar.xyz>")], keyring_type="main")
@create_certificate(username=Username("foobar"), uids=[Uid("foobar <foo@bar.xyz>")])
@create_certificate(username=Username("packager"), uids=[Uid("packager <packager@bar.xyz>")])
@create_uid_certification(issuer=Username("main1"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_uid_certification(issuer=Username("packager"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
@create_signature_revocation(issuer=Username("packager"), certified=Username("foobar"), uid=Uid("foobar <foo@bar.xyz>"))
def test_certificate_trust_ignore_3rd_party_revocation(working_dir: Path, keyring_dir: Path) -> None:
    trust = certificate_trust(
        test_keyring_certificates[Username("foobar")][0],
        test_main_fingerprints,
        test_all_fingerprints,
    )
    assert Trust.marginal == trust


@mark.parametrize(
    "trust, result",
    [
        (Trust.revoked, Color.RED),
        (Trust.full, Color.GREEN),
        (Trust.marginal, Color.YELLOW),
        (Trust.unknown, Color.YELLOW),
    ],
)
def test_trust_color(trust: Trust, result: Color) -> None:
    assert trust_color(trust) == result


@mark.parametrize(
    "trust, result",
    [
        (Trust.revoked, "✗"),
        (Trust.full, "✓"),
        (Trust.marginal, "~"),
        (Trust.unknown, "~"),
        (None, "?"),
    ],
)
def test_trust_icon(trust: Trust, result: str) -> None:
    assert trust_icon(trust) == result


@mark.parametrize(
    "trust",
    [
        Trust.revoked,
        Trust.full,
        Trust.marginal,
        Trust.unknown,
    ],
)
@patch("libkeyringctl.trust.trust_icon")
@patch("libkeyringctl.trust.trust_color")
def test_format_trust_label(trust_color_mock: Mock, trust_icon_mock: Mock, trust: Trust) -> None:
    trust_icon_mock.return_value = "ICON"
    trust_color_mock.return_value = Color.GREEN
    assert f"{Color.GREEN.value}ICON {trust.name}{Color.RST.value}" == format_trust_label(trust)


@mark.parametrize(
    "trust, trust_filter, result",
    [
        (Trust.revoked, TrustFilter.unknown, False),
        (Trust.full, TrustFilter.unknown, False),
        (Trust.marginal, TrustFilter.unknown, False),
        (Trust.unknown, TrustFilter.unknown, True),
        (Trust.revoked, TrustFilter.marginal, False),
        (Trust.full, TrustFilter.marginal, False),
        (Trust.marginal, TrustFilter.marginal, True),
        (Trust.unknown, TrustFilter.marginal, False),
        (Trust.revoked, TrustFilter.full, False),
        (Trust.full, TrustFilter.full, True),
        (Trust.marginal, TrustFilter.full, False),
        (Trust.unknown, TrustFilter.full, False),
        (Trust.revoked, TrustFilter.revoked, True),
        (Trust.full, TrustFilter.revoked, False),
        (Trust.marginal, TrustFilter.revoked, False),
        (Trust.unknown, TrustFilter.revoked, False),
        (Trust.revoked, TrustFilter.unrevoked, False),
        (Trust.full, TrustFilter.unrevoked, True),
        (Trust.marginal, TrustFilter.unrevoked, True),
        (Trust.unknown, TrustFilter.unrevoked, True),
        (Trust.revoked, TrustFilter.all, True),
        (Trust.full, TrustFilter.all, True),
        (Trust.marginal, TrustFilter.all, True),
        (Trust.unknown, TrustFilter.all, True),
    ],
)
def test_filter_by_trust(trust: Trust, trust_filter: TrustFilter, result: bool) -> None:
    assert filter_by_trust(trust=trust, trust_filter=trust_filter) == result
