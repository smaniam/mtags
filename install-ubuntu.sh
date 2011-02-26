#!/bin/bash
sudo apt-get update
sudo apt-get install \
  build-essential \
  subversion \
  mercurial \
  git-core \
  cmake \
  libfreetype6 \
  libfreetype6-dev \
  libtool

make deps
make mediatags
