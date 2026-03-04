#!/bin/bash
# Run this on the source tree before making a .tar
VERSION=$ASL_PARROT_VERSION
rm -rf /tmp/asl-parrot-$VERSION
cp -r ../asl-parrot /tmp
mv /tmp/asl-parrot /tmp/asl-parrot-$VERSION
# Clean out some things we don't want in the tarball
rm /tmp/asl-parrot-$VERSION/ed25519/*.dll
# Pull down some files related to Piper
cd /tmp
wget https://ampersand-asl.s3.us-west-1.amazonaws.com/releases/libpiper-etc.tar.gz -O libpiper-etc.tar.gz
tar xvf libpiper-etc.tar.gz
cp libpiper-etc/en_US-amy-low.onnx /tmp/asl-parrot-$VERSION/etc
cp libpiper-etc/en_US-amy-low.onnx.json /tmp/asl-parrot-$VERSION/etc
cp -r libpiper-etc/espeak-ng-data /tmp/asl-parrot-$VERSION/etc
# Make the tar
cd /tmp
tar -czf /tmp/asl-parrot-$VERSION.tar.gz asl-parrot-$VERSION



