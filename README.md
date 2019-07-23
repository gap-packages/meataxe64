[![Build Status](https://travis-ci.org/gap-packages/meataxe64.svg?branch=master)](https://travis-ci.org/gap-packages/meataxe64)
[![Code Coverage](https://codecov.io/github/gap-packages/meataxe64/coverage.svg?branch=master&token=)](https://codecov.io/gh/gap-packages/meataxe64)

# The meataxe64 GAP package

This package provides a low-level GAP bindings to Richard
Parker's [meataxe64](https://meataxe64.wordpress.com).

## Documentation

Some information and documentation can be found in the manual, available
as PDF `doc/manual.pdf` or as HTML `doc/chap0_mj.html`, or on the package
homepage at

<http://gap-packages.github.io/meataxe64/>

## Requirements

This package contains C (and assembler) code which must be compiled
before it can used. This code is only usable on 64 bit x86_64 systems
and has only been tested under OS X and Linux operating systems. It
will definitely not work on 32 bit systems or non-x86 processors and
probably will not work under Windows.

It depends on the GAPDoc, datastructures and AutoDoc packages which
you will need to have installed (note that datastructures also needs
to be compiled).

## Installation

Clone the GIT repository and then, from the top directory do

```
./autogen.sh
./configure --with-gap-root=<path to your GAP root directory>
make
```

To compile the documentation use

```
gap ./makedoc.g
```

from the top directory (you will need the AutoDoc and GAPDoc packages
and a LaTeX installation).

To test the system try

```
gap tst/testinstall.g (can take a minute or so)
gap tst/testall.g (takes about 20 times as long as testinstall)
```

## Bug reports and feature requests

Please submit bug reports and feature requests via our GitHub issue tracker:

  <https://github.com/gap-packages/meataxe64/issues>


# License

For details see the files COPYRIGHT.md and LICENSE.


