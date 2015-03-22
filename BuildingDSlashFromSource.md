# Building DSlash from Source #

## What you need ##
  * Linux. DSlash is not currently tested on other platforms.
  * A subversion client. These instructions assume svn.
  * A suitable build environment (make, gcc, build-essentials or whatever).

## Steps ##
  1. Get the latest DSlash source code: `svn export http://dslash.googlecode.com/svn/trunk/ dslash`
  1. Enter the source directory: `cd dslash`
  1. Compile: `make`
  1. Show Help: `./dslash -h`
  1. Trim a ROM file: `./dslash infile.nds outfile.nds`