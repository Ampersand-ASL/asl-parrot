#!/bin/bash
# Run this on the source tree before making a .tar
VERSION=$ASL_PARROT_VERSION
rm -rf /tmp/asl-parrot-$VERSION
cp -r ../asl-parrot /tmp
mv /tmp/asl-parrot /tmp/asl-parrot-$VERSION
# Clean out some things we don't want in the tarball
rm /tmp/asl-parrot-$VERSION/ed25519/*.dll
# Make the tar
cd /tmp
tar -czf /tmp/asl-parrot-$VERSION.tar.gz asl-parrot-$VERSION



