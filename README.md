MediaTags
====

[Click here to lend your support to: mediatags - meta-data tags as JSON and make a donation at www.pledgie.com !][1]

[![Click here to lend your support to: mediatags - meta-data tags as JSON and make a donation at www.pledgie.com !][2]][1]

[1]: http://www.pledgie.com/campaigns/14039
[2]: http://www.pledgie.com/campaigns/14039.png?skin_name=chrome

Author: smaniam@ymail.com

Maintainer: coolaj86

Date: 2011-01-27

Please report issues to the [coolaj86's branch](https://github.com/coolaj86/mtags).

Instalation
====

    make deps
    make mediatags

dependencies
---

The following dependencies will be built (because the ones your package manager provides may or may not be suitable)

  * libjson
  * libb64
  * mhash
  * AtomicParsley
  * taglib
  * exiv2
  * fontconfig
  * poppler

You may need to install build system tools as follows

    sudo apt-get install \
      build-essential \
      subversion \
      mercurial \
      git-core \
      cmake \
      libfreetype6 \
      libfreetype6-dev \
      libtool

Usage
====

Version 0.34

Warning: This is a work in progress things can change rapidly

  0. Current support is for m4atags, id3tagsi, imgtags and pdftags
  0. m4atags is dependent on AtomicParsley and mhash (both are bundled)
  0. id3tags is dependent on TagLib and libjson (both are bundled)
  0. imgtags is dependent on libexiv2 and libjson (both are bundled)
  0. pdftags is dependent on libpoppler - This is not bundled in this release
  0. Building m4atags, id3tags and imgtags for the very first time:
    * Type: make libs
    * Type: make mediatags
  0. Executables are statically linked with the libs and have no execution dependencies
  0. m4atags binary is located in the m4a directory
  0. id3tags binary is located in the id3 directory
  0. imgtags binary is located in the img directory
  0. pdftags binary is located in the pdf directory
  0. Design philosophy:
    * No modifications to the existing Libraries (they are allowed to evolve)
    * Self contained build - No other external dependencies
  0. m4atags Options supported:
    * `m4atags --literal [ --with-md5sum ] [ --with-sha1sum ] [ --extract-art | --extract-art-to=<path> ] <m4afile>`
    * `m4atags --verbose <m4afile>`
    * `m4atags [ --with-md5sum ] [ --with-sha1sum ] <m4afile>`
    * `m4atags [ --extract-art | --extract-art-to=<path> ] <m4afile>`
    * `m4atags --help`
  0. id3tags Options supported:
    * `id3tags --literal [ --with-md5sum ] [ --with-sha1sum ] [ --extract-art | --extract-art-to=<path> ] <id3file>`
    * `id3tags --verbose <id3file>`
    * `id3tags [ --with-md5sum ] [ --with-sha1sum ] <id3file>`
    * `id3tags [ --extract-art | --extract-art-to=<path> ] <id3file>`
    * `id3tags --help`
  0. imgtags Options supported:
    * `imgtags --literal=[e|i|x] [ --with-md5sum ] [ --with-sha1sum ] <imgfile>`
    * `imgtags --verbose <imgfile>`
    * `imgtags [ --with-md5sum ] [ --with-sha1sum ] <imgfile>`
    * `imgtags --help`
  0. pdftags Options supported:
    * `pdftags --literal [ --with-md5sum ] [ --with-sha1sum ] <pdffile>`
    * `pdftags --verbose <pdffile>`
    * `pdftags [ --with-md5sum | --with-sha1sum ] <pdffile>`
    * `pdftags --help`
  0. This release uses the Original AtomicParsley 0.9 and not the bitbucket fork
  0. libexiv2 requires expat (libexpat-dev) for XMP support
  0. imgtags supports verbose and checksum mode for JPG files only
  0. Known Issues:
    * Compiling atomicparsley generates a lot of Warning messages (Ignore)
    * This was tested with a limited number of files (fetched from internet), 
    any help in obtaining some non-contraband stuff would be more than welcome
  0. Dependencies:
    * Compiling pdftags requires libpoppler (libpoppler-dev package)

Project Goals
====

  * [MediaTags Project Specification](http://coolaj86.info/articles/mediatags.html)
  * [m4atags Specification](http://coolaj86.info/articles/example-of-verbose-output-from-mediatags.html)
