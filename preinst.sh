#! /bin/bash
git submodule update --init
./scripts/pkgdep.sh
./run.sh -c
