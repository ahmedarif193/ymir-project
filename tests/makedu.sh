#!/bin/bash

rm -rf tarball
mkdir -p tarball
mksquashfs staging_dir/target-x86_64_musl/root-x86/ ./tarball/rootfs.squashfs
cp metadata.json tarball/
cd tarball; tar -czvf ../tarball.tar.gz *;
cd -;
rm -rf tarball
