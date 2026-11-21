#!/bin/bash
# Prints the asl-parrot version, taken from the VERSION constant in
# src/main.cpp. That constant is the single source of truth for the
# version: it is logged at startup, returned by /network-test, and used
# as the upstream version of the Debian package.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VERSION=$(sed -n 's/^static const char\* VERSION = "\([^"]*\)";.*/\1/p' \
    "$ROOT/src/main.cpp")
if [ -z "$VERSION" ]; then
    echo "error: could not find VERSION in $ROOT/src/main.cpp" >&2
    exit 1
fi
echo "$VERSION"
