MediaTags
====

Author: smaniam@ymail.com

Date: 2010-11-20

Please report issues to the [coolaj86's branch](https://github.com/coolaj86/mtags).

Version 0.3
====

Warning: This is a work in progress things can change rapidly

  0. Current support is for m4atags and id3tags(limited) only
  0. m4atags is dependent on AtomicParsley and mhash (both are bundled)
  0. id3tags is dependent on TagLib and libjson (both are bundled)
  0. Building m4atags and id3tags for the very first time:
    * Type: make libs
    * Type: make mediatags
  0. Executables are statically linked with the libs and have no execution dependencies
  0. m4atags binary is located in the m4a directory
  0. id3tags binary is located in the id3 directory
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
    * `id3tags --literal <id3file>`
    * `id3tags --help`
  0. This release uses the Original AtomicParsley 0.9 and not the bitbucket fork
  0. Known Issues:
    * Compiling atomicparsley generates a lot of Warning messages (Ignore)
    * This was tested with a limited number of files (fetched from internet), 
    any help in obtaining some non-contraband stuff would be more than welcome
    * id3tags is a limited release, PRIV tags are not clumped together as a
    collection

Project Goals
====

  * [MediaTags Project Specification](http://coolaj86.info/articles/mediatags.html)
  * [m4atags Specification](http://coolaj86.info/articles/example-of-verbose-output-from-mediatags.html)
