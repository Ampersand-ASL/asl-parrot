#!/bin/bash
set -e
set -x

if [ -f /etc/os-release ] ; then
  OS_CODENAME=$(grep "^VERSION_CODENAME=" /etc/os-release | sed 's/VERSION_CODENAME=\(.*\)/\1/g')
else
  OS_CODENAME=unknown
fi
echo "OS_CODENAME: ${OS_CODENAME}"

REPO_DIR=${REPO_DIR:-asl-parrot}
OUT_DIR=${OUT_DIR:-/build/_debs}

# The source tree (and its submodules) is bind-mounted and may be owned by
# another UID.
git config --global --add safe.directory '*'

export EMAIL="AllStarLink <autobuild@allstarlink.org>"
cd "/build/${REPO_DIR}"
make deb
cd /build
mkdir -p "${OUT_DIR}"
mv /build/*.deb "${OUT_DIR}/"
