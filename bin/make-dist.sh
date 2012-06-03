#!/bin/bash

HERE=$(dirname $(readlink -f "$0"))

cd ${HERE}/../install
zip -9 ${HERE}/../amigadx36.bin.zip *
cd ${HERE}/../

ARCHIVE=amigadx36.src.zip
git archive --format=zip HEAD >${ARCHIVE}

