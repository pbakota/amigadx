#!/bin/bash

HERE=$(dirname $(readlink -f "$0"))

cd ${HERE}/../install
zip -9 ${HERE}/../amigadx36_x86_64.bin.zip *
cd ${HERE}/../

ARCHIVE=amigadx.src.tgz
git archive --format=tar.gz --prefix=amigadx HEAD >${ARCHIVE}

