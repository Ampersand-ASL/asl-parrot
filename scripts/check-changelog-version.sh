#!/bin/bash
# Fails unless the top entry of debian/changelog has the same upstream
# version as VERSION in src/main.cpp (see parrot-version.sh).
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VERSION=$("$ROOT/scripts/parrot-version.sh")
DEB_VERSION=$(dpkg-parsechangelog -l "$ROOT/debian/changelog" -S Version)
DEB_UPSTREAM=${DEB_VERSION%-*}
if [ "$DEB_UPSTREAM" != "$VERSION" ]; then
    echo "error: src/main.cpp VERSION is $VERSION but debian/changelog" >&2
    echo "       top entry is $DEB_VERSION. Add a changelog entry, e.g.:" >&2
    echo "       dch -c $ROOT/debian/changelog -v $VERSION-1" >&2
    exit 1
fi
