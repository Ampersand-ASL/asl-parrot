#!/bin/bash
# Downloads the Piper voice and espeak-ng data into the given etc directory
# (default: this tree's etc/). debian/install packages them from there.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEST="${1:-$ROOT/etc}"
URL=https://ampersand-asl.s3.us-west-1.amazonaws.com/releases/libpiper-etc.tar.gz

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
wget -q "$URL" -O "$TMP/libpiper-etc.tar.gz"
tar -C "$TMP" -xf "$TMP/libpiper-etc.tar.gz"
cp "$TMP/libpiper-etc/en_US-amy-low.onnx" "$DEST/"
cp "$TMP/libpiper-etc/en_US-amy-low.onnx.json" "$DEST/"
rm -rf "$DEST/espeak-ng-data"
cp -r "$TMP/libpiper-etc/espeak-ng-data" "$DEST/"
