#!/bin/bash

opkg update
opkg install subversion --force-overwrite

opkg install \
    task-native-sdk \
    cmake \
    git-core \
    curl libcurl4 curl-dev \
    libfreetype6 libfreetype-dev \
    lcms lcms-dev \
    libfontconfig1 libfontconfig-dev \
    libjpeg8 libjpeg-dev \
    libz1 libz-static libz-dev \
    libpng12-0 libpng12-dev \

opkg install libpng3 libpng libpng-dev --force-overwrite
opkg install python python-dev # installs A LOT of crap, unfortunately

# needed to grab AtomicParsley from bitbucket
wget http://mercurial.selenic.com/release/mercurial-1.7.5.tar.gz
tar xf mercurial-1.7.5.tar.gz
cd mercurial-1.7.5
python setup.py install

make deps # fontconfig will fail, but it's okay
make mediatags
