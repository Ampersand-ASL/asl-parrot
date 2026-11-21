#!/bin/bash
#
# Build asl-parrot .deb packages locally in the builder image.
# Builds only: nothing is uploaded or published.
#
#   builder/localbuild.sh [-o trixie] [-a amd64|arm64] [-e podman|docker]
#
# Packages are written to ../_debs (next to this repository).
#
# The architecture defaults to the host's (amd64 or arm64). Building the
# other one runs under qemu emulation (sudo apt install qemu-user-static),
# which is slow.

set -euo pipefail

OS=trixie
HOST_ARCH=$(dpkg --print-architecture 2>/dev/null || uname -m)
case $HOST_ARCH in
	x86_64) HOST_ARCH=amd64 ;;
	aarch64) HOST_ARCH=arm64 ;;
esac
ARCH=$HOST_ARCH
ENGINE=${CONTAINER_ENGINE:-podman}

while [[ $# -gt 0 ]]; do
	case $1 in
		-o) OS="$2"; shift 2 ;;
		-a) ARCH="$2"; shift 2 ;;
		-e) ENGINE="$2"; shift 2 ;;
		*) echo "Usage: $0 [-o os] [-a arch] [-e podman|docker]" >&2; exit 1 ;;
	esac
done

case $ARCH in
	amd64) IMG_ARCH=amd64; QEMU=x86_64 ;;
	arm64) IMG_ARCH=arm64v8; QEMU=aarch64 ;;
	*) echo "Unsupported architecture: $ARCH" >&2; exit 1 ;;
esac

if [ "$HOST_ARCH" != "$ARCH" ] && \
	! ls /proc/sys/fs/binfmt_misc/ 2>/dev/null | grep -q "qemu-$QEMU"; then
	echo "Building $ARCH on $HOST_ARCH needs qemu emulation, which is not registered." >&2
	echo "Install it with: sudo apt install qemu-user-static" >&2
	echo "(On WSL, if it is still missing, run: sudo systemctl restart systemd-binfmt)" >&2
	exit 1
fi

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(dirname "$DIR")"
REPO_DIR="$(basename "$REPO")"
OUT="$(dirname "$REPO")/_debs"
TAG="asl-parrot-builder.${OS}.${IMG_ARCH}"

# The build compiles sources from the submodules.
git -C "$REPO" submodule update --init --recursive

mkdir -p "$OUT"

"$ENGINE" build -f "$DIR/Dockerfile" -t "$TAG" \
	--platform "linux/$ARCH" \
	--build-arg ARCH="$IMG_ARCH" \
	--build-arg OS="$OS" \
	"$DIR"

# Rootless podman maps container root to the invoking user, so files written
# to the mounted tree stay owned by you.
"$ENGINE" run --rm \
	--platform "linux/$ARCH" \
	-e REPO_DIR="$REPO_DIR" -e OUT_DIR=/out \
	-v "$REPO:/build/$REPO_DIR" \
	-v "$OUT:/out" \
	"$TAG"

echo "Packages in $OUT:"
ls -l "$OUT"
