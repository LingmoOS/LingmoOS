from logging import debug
from pathlib import Path
from subprocess import PIPE
from subprocess import Popen
from tempfile import NamedTemporaryFile
from typing import List
from typing import Optional
from typing import Set

from libkeyringctl.keyring import export
from libkeyringctl.keyring import get_fingerprints_from_paths
from libkeyringctl.keyring import is_pgp_fingerprint
from libkeyringctl.keyring import transform_fingerprint_to_keyring_path
from libkeyringctl.keyring import transform_username_to_keyring_path
from libkeyringctl.sequoia import packet_dump_field
from libkeyringctl.sequoia import packet_kinds
from libkeyringctl.types import Fingerprint
from libkeyringctl.types import Uid
from libkeyringctl.util import get_cert_paths
from libkeyringctl.util import get_fingerprint_from_partial
from libkeyringctl.util import simplify_uid
from libkeyringctl.util import system


def verify(  # noqa: ignore=C901
    working_dir: Path,
    keyring_root: Path,
    sources: Optional[List[Path]],
    lint_hokey: bool = True,
    lint_sq_keyring: bool = True,
) -> None:
    """Verify certificates against modern expectations using `sq keyring lint` and hokey

    Parameters
    ----------
    working_dir: A directory to use for temporary files
    keyring_root: The keyring root directory to look up username shorthand sources
    sources: A list of username, fingerprint or directories from which to read PGP packet information
        (defaults to `keyring_root`)
    lint_hokey: Whether to run hokey lint
    lint_sq_keyring: Whether to run sq keyring lint
    """

    if not sources:
        sources = [keyring_root]

    # transform shorthand paths to actual keyring paths
    transform_username_to_keyring_path(keyring_dir=keyring_root / "packager", paths=sources)
    transform_fingerprint_to_keyring_path(keyring_root=keyring_root, paths=sources)

    cert_paths: Set[Path] = get_cert_paths(sources)
    all_fingerprints = get_fingerprints_from_paths([keyring_root])

    for certificate in sorted(cert_paths):
        print(f"Verify {certificate.name} owned by {certificate.parent.name}")

        verify_integrity(certificate=certificate, all_fingerprints=all_fingerprints)

        with NamedTemporaryFile(
            dir=working_dir, prefix=f"{certificate.parent.name}-{certificate.name}", suffix=".asc"
        ) as keyring:
            keyring_path = Path(keyring.name)
            export(
                working_dir=working_dir,
                keyring_root=keyring_root,
                sources=[certificate],
                output=keyring_path,
            )

            if lint_hokey:
                keyring_fd = Popen(("sq", "toolbox", "dearmor", f"{str(keyring_path)}"), stdout=PIPE)
                print(system(["hokey", "lint"], _stdin=keyring_fd.stdout), end="")
            if lint_sq_keyring:
                print(system(["sq", "cert", "lint", f"{str(keyring_path)}"]), end="")


def verify_integrity(certificate: Path, all_fingerprints: Set[Fingerprint]) -> None:  # noqa: ignore=C901
    if not is_pgp_fingerprint(certificate.name):
        raise Exception(f"Unexpected certificate name for certificate {certificate.name}: {str(certificate)}")

    pubkey = certificate / f"{certificate.name}.asc"
    if not pubkey.is_file():
        raise Exception(f"Missing certificate pubkey {certificate.name}: {str(pubkey)}")

    if not list(certificate.glob("uid/*/*.asc")):
        raise Exception(f"Missing at least one UID for {certificate.name}")

    # check packet files
    for path in certificate.iterdir():
        if path.is_file():
            if path.name != f"{certificate.name}.asc":
                raise Exception(f"Unexpected file in certificate {certificate.name}: {str(path)}")
            assert_packet_kind(path=path, expected="Public-Key")
            assert_filename_matches_packet_fingerprint(path=path, check=certificate.name)
            debug(f"OK: {path}")
        elif path.is_dir():
            if "revocation" == path.name:
                verify_integrity_key_revocations(path=path)
            elif "directkey" == path.name:
                for directkey in path.iterdir():
                    assert_is_dir(path=directkey)
                    if "certification" == directkey.name:
                        verify_integrity_direct_key_certifications(path=directkey)
                    elif "revocation" == directkey.name:
                        verify_integrity_direct_key_revocations(path=directkey)
                    else:
                        raise_unexpected_file(path=directkey)
            elif "uid" == path.name:
                for uid in path.iterdir():
                    assert_is_dir(path=uid)
                    uid_packet = uid / f"{uid.name}.asc"
                    assert_is_file(path=uid_packet)

                    uid_binding_sig = uid / "certification" / f"{certificate.name}.asc"
                    uid_revocation_sig = uid / "revocation" / f"{certificate.name}.asc"
                    if not uid_binding_sig.is_file() and not uid_revocation_sig:
                        raise Exception(f"Missing uid binding/revocation sig for {certificate.name}: {str(uid)}")

                    for uid_path in uid.iterdir():
                        if uid_path.is_file():
                            if uid_path.name != f"{uid.name}.asc":
                                raise Exception(f"Unexpected file in certificate {certificate.name}: {str(uid_path)}")

                            assert_packet_kind(path=uid_path, expected="User")

                            uid_value = simplify_uid(Uid(packet_dump_field(packet=uid_path, query="Value")))
                            if uid_value != uid.name:
                                raise Exception(f"Unexpected uid in file {str(uid_path)}: {uid_value}")
                        elif not uid_path.is_dir():
                            raise Exception(f"Unexpected file type in certificate {certificate.name}: {str(uid_path)}")
                        elif "certification" == uid_path.name:
                            for sig in uid_path.iterdir():
                                assert_is_file(path=sig)
                                assert_is_pgp_fingerprint(path=sig, _str=sig.stem)
                                assert_has_suffix(path=sig, suffix=".asc")

                                assert_packet_kind(path=sig, expected="Signature")
                                assert_signature_type_certification(path=sig)

                                issuer = get_fingerprint_from_partial(
                                    fingerprints=all_fingerprints,
                                    fingerprint=Fingerprint(
                                        packet_dump_field(packet=sig, query="Hashed area|Unhashed area.Issuer")
                                    ),
                                )
                                if issuer != sig.stem:
                                    raise Exception(f"Unexpected issuer in file {str(sig)}: {issuer}")
                                debug(f"OK: {sig}")
                        elif "revocation" == uid_path.name:
                            for sig in uid_path.iterdir():
                                assert_is_file(path=sig)
                                assert_is_pgp_fingerprint(path=sig, _str=sig.stem)
                                assert_has_suffix(path=sig, suffix=".asc")

                                assert_packet_kind(path=sig, expected="Signature")
                                assert_signature_type(path=sig, expected="CertificationRevocation")

                                issuer = get_fingerprint_from_partial(
                                    fingerprints=all_fingerprints,
                                    fingerprint=Fingerprint(
                                        packet_dump_field(packet=sig, query="Hashed area|Unhashed area.Issuer")
                                    ),
                                )
                                if issuer != sig.stem:
                                    raise Exception(f"Unexpected issuer in file {str(sig)}: {issuer}")

                                certification = uid_path.parent / "certification" / sig.name
                                if certification.exists():
                                    raise Exception(f"Certification exists for revocation {str(sig)}: {certification}")

                                debug(f"OK: {sig}")
                        else:
                            raise Exception(f"Unexpected directory in certificate {certificate.name}: {str(uid_path)}")
                        debug(f"OK: {uid_path}")
                    debug(f"OK: {uid}")
            elif "subkey" == path.name:
                for subkey in path.iterdir():
                    assert_is_dir(path=subkey)
                    assert_is_pgp_fingerprint(path=subkey, _str=subkey.name)

                    subkey_packet = subkey / f"{subkey.name}.asc"
                    assert_is_file(path=subkey_packet)

                    subkey_binding_sig = subkey / "certification" / f"{certificate.name}.asc"
                    subkey_revocation_sig = subkey / "revocation" / f"{certificate.name}.asc"
                    if not subkey_binding_sig.is_file() and not subkey_revocation_sig:
                        raise Exception(f"Missing subkey binding/revocation sig for {certificate.name}: {str(subkey)}")

                    for subkey_path in subkey.iterdir():
                        if subkey_path.is_file():
                            if subkey_path.name != f"{subkey.name}.asc":
                                raise Exception(
                                    f"Unexpected file in certificate {certificate.name}: {str(subkey_path)}"
                                )

                            assert_packet_kind(path=subkey_path, expected="Public-Subkey")
                            assert_filename_matches_packet_fingerprint(path=subkey_path, check=subkey_path.stem)
                        elif not subkey_path.is_dir():
                            raise Exception(
                                f"Unexpected file type in certificate {certificate.name}: {str(subkey_path)}"
                            )
                        elif "certification" == subkey_path.name:
                            for sig in subkey_path.iterdir():
                                assert_is_file(path=sig)
                                assert_is_pgp_fingerprint(path=sig, _str=sig.stem)
                                assert_has_suffix(path=sig, suffix=".asc")

                                assert_packet_kind(path=sig, expected="Signature")
                                assert_signature_type(path=sig, expected="SubkeyBinding")

                                assert_filename_matches_packet_issuer_fingerprint(path=sig, check=certificate.name)
                        elif "revocation" == subkey_path.name:
                            for sig in subkey_path.iterdir():
                                assert_is_file(path=sig)
                                assert_is_pgp_fingerprint(path=sig, _str=sig.stem)
                                assert_has_suffix(path=sig, suffix=".asc")

                                assert_packet_kind(path=sig, expected="Signature")
                                assert_signature_type(path=sig, expected="SubkeyRevocation")

                                assert_filename_matches_packet_issuer_fingerprint(path=sig, check=certificate.name)
                        else:
                            raise Exception(
                                f"Unexpected directory in certificate {certificate.name}: {str(subkey_path)}"
                            )
                        debug(f"OK: {subkey_path}")
            else:
                raise Exception(f"Unexpected directory in certificate {certificate.name}: {str(path)}")
        else:
            raise Exception(f"Unexpected file type in certificate {certificate.name}: {str(path)}")


def assert_packet_kind(path: Path, expected: str) -> None:
    kinds = packet_kinds(packet=path)
    if not kinds or len(kinds) != 1:
        raise Exception(f"Unexpected amount of packets in file {str(path)}: {kinds}")
    kind = kinds[0]
    if kind != expected:
        raise Exception(f"Unexpected packet in file {str(path)} kind: {kind} expected: {expected}")


def assert_signature_type(path: Path, expected: str) -> None:
    sig_type = packet_dump_field(packet=path, query="Type")
    if sig_type != expected:
        raise Exception(f"Unexpected packet type in file {str(path)} type: {sig_type} expected: {expected}")


def assert_signature_type_certification(path: Path) -> None:
    sig_type = packet_dump_field(packet=path, query="Type")
    if sig_type not in ["GenericCertification", "PersonaCertification", "CasualCertification", "PositiveCertification"]:
        raise Exception(f"Unexpected packet certification type in file {str(path)} type: {sig_type}")


def assert_is_pgp_fingerprint(path: Path, _str: str) -> None:
    if not is_pgp_fingerprint(_str):
        raise Exception(f"Unexpected file name, not a pgp fingerprint: {str(path)}")


def assert_filename_matches_packet_issuer_fingerprint(path: Path, check: str) -> None:
    fingerprint = packet_dump_field(packet=path, query="Unhashed area|Hashed area.Issuer Fingerprint")
    if not fingerprint == check:
        raise Exception(f"Unexpected packet fingerprint in file {str(path)}: {fingerprint}")


def assert_filename_matches_packet_fingerprint(path: Path, check: str) -> None:
    fingerprint = packet_dump_field(packet=path, query="Fingerprint")
    if not fingerprint == check:
        raise Exception(f"Unexpected packet fingerprint in file {str(path)}: {fingerprint}")


def assert_has_suffix(path: Path, suffix: str) -> None:
    if path.suffix != suffix:
        raise Exception(f"Unexpected file suffix in {str(path)} expected: {suffix}")


def assert_is_file(path: Path) -> None:
    if not path.is_file():
        raise Exception(f"Unexpected type, should be file: {str(path)}")


def assert_is_dir(path: Path) -> None:
    if not path.is_dir():
        raise Exception(f"Unexpected type, should be directory: {str(path)}")


def raise_unexpected_file(path: Path) -> None:
    raise Exception(f"Unexpected file in directory: {str(path)}")


def verify_integrity_key_revocations(path: Path) -> None:
    assert_is_dir(path=path)
    for sig in path.iterdir():
        assert_is_file(path=sig)
        assert_is_pgp_fingerprint(path=sig, _str=sig.stem)
        assert_has_suffix(path=sig, suffix=".asc")

        assert_packet_kind(path=sig, expected="Signature")
        assert_signature_type(path=sig, expected="KeyRevocation")

        assert_filename_matches_packet_issuer_fingerprint(path=sig, check=sig.stem)

        debug(f"OK: {sig}")


def verify_integrity_direct_key_certifications(path: Path) -> None:
    for issuer_dir in path.iterdir():
        assert_is_dir(path=issuer_dir)
        assert_is_pgp_fingerprint(path=issuer_dir, _str=issuer_dir.name)
        for certification in issuer_dir.iterdir():
            verify_integrity_direct_key_certification(path=certification)


def verify_integrity_direct_key_revocations(path: Path) -> None:
    for issuer_dir in path.iterdir():
        assert_is_dir(path=issuer_dir)
        assert_is_pgp_fingerprint(path=issuer_dir, _str=issuer_dir.name)
        for certification in issuer_dir.iterdir():
            verify_integrity_direct_key_revocation(path=certification)


def verify_integrity_direct_key_certification(path: Path) -> None:
    assert_is_file(path=path)
    assert_has_suffix(path=path, suffix=".asc")

    assert_packet_kind(path=path, expected="Signature")
    assert_signature_type(path=path, expected="DirectKey")

    assert_filename_matches_packet_issuer_fingerprint(path=path, check=path.parent.name)

    debug(f"OK: {path}")


def verify_integrity_direct_key_revocation(path: Path) -> None:
    assert_is_file(path=path)
    assert_has_suffix(path=path, suffix=".asc")

    assert_packet_kind(path=path, expected="Signature")
    assert_signature_type(path=path, expected="CertificationRevocation")

    assert_filename_matches_packet_issuer_fingerprint(path=path, check=path.parent.name)

    debug(f"OK: {path}")
