#!/bin/bash
# Builds /tmp/asl-parrot-<version>.tar.gz from this source tree.
# The version is the VERSION constant in src/main.cpp (see
# parrot-version.sh). The top entry of debian/changelog must carry the
# same upstream version, e.g. "asl-parrot (<version>-1)".
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VERSION=$("$ROOT/scripts/parrot-version.sh")

# Make sure the Debian changelog agrees with main.cpp
"$ROOT/scripts/check-changelog-version.sh"

OUT=/tmp/asl-parrot-$VERSION
rm -rf "$OUT" "$OUT.tar.gz"
cp -r "$ROOT" "$OUT"
# Clean out some things we don't want in the tarball
rm -rf "$OUT/build"
find "$OUT" -name .git -prune -exec rm -rf {} +
rm -f "$OUT"/ed25519/*.dll
# Pull down some files related to Piper
"$ROOT/scripts/fetch-piper-etc.sh" "$OUT/etc"
# Make the tar
tar -C /tmp -czf "$OUT.tar.gz" "asl-parrot-$VERSION"
echo "$OUT.tar.gz"
