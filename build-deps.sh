#!/bin/bash

CURDIR=`pwd`

cd deps

#
# TagLib
#
function build_taglib {
  # Note taglib_config.h
  svn co -r 1221631 svn://anonsvn.kde.org/home/kde/trunk/kdesupport/taglib taglib
  svn up -r 1221631 
  mkdir -p taglib/build
  cd taglib/build
  cmake ..
  make
  sudo make install
  cd -
}

#
# AtomicParsley
#
function build_atomicparsley_hg {
  # The current AP has cleaned up a few things we depended on
  # TODO make the switch
  
  hg clone https://bitbucket.org/wez/atomicparsley
  cd atomicparsley
  ./autogen.sh
  ./configure
  make
  sudo make install
}
function build_atomicparsley_svn {
  # SVN version is broken in later versions
  svn co -r 91 https://atomicparsley.svn.sourceforge.net/svnroot/atomicparsley/trunk/atomicparsley atomicparsley
  cd atomicparsley
  rm -rf ./*
  svn up -r 91
  patch -p1 < ../../atomicparsley.patch
  ./build
  # uses autoconf in later versions
  #autoconf && autoheader
  #./configure
  #make
  cd -
}
function build_atomicparsley_zip {
  if [ ! -f "AtomicParsley-source-0.9.0.zip" ]
  then
    wget http://downloads.sourceforge.net/project/atomicparsley/atomicparsley/AtomicParsley%20v0.9.0/AtomicParsley-source-0.9.0.zip
  fi
  if [ -d "AtomicParsley-source-0.9.0" ]
  then
    rm -rf AtomicParsley-source-0.9.0
  fi
  rm -rf __MACOSX
  unzip AtomicParsley-source-0.9.0.zip
  rm -rf __MACOSX
  cd AtomicParsley-source-0.9.0
  patch -p1 < ../../atomicparsley.patch
  ./build
  cd -
}

#build_taglib
build_atomicparsley_svn
