This is version mtx64m3, send by Richard Parker to Jon Thackray
on 5.Dec.2015 so that Jon can implement null-space on the correct
basic software.

There are six directories.  bin is empty and is where the compiled
binaries go, doc is documentation, src is the sources, jif contains
the (bash) shell scripts, and test is where the user should go to
use it.  The "getting started" commands are . . .

bash 
cd /mtx64m3/test
source go
makl HAS
compa
. . . start doing things.

'source go' just sets up the environment variables.  'makl' compiles
the subroutine library.  compa compiles all the programs.
makl and compa are shell scripts residing in the 'jif' directory.

There are currently four possible libraries . . .

HAS uses the Haswell x86 instructions, and will not work on anything
    earlier since it makes substantial use of AVX2.

NEH uses only the SSE instructions and should work on any 64-bit chip,
    and has been tested on the oldest one I can lay my hands on.

PIL is specifically tuned for the AMD "piledriver" chip, but should
    work on any 64-bit chip also

GEN uses no assembler at all.  It might even work on a non-x86 chip,
    at least if it is little-endian, but this has not been tried.

Currently the HPMI works mod2 and mod3 in HAS/NEH/PIL, but only
mod2 is implemented in GEN so far.  Hence if GEN is used, mod3
is done by generic methods and is very considerably slower.

The 'compa' routine compiles all the programs, and then copies a
binary version (the "best" one) to the program where it is used.
For example zrn1 and zrn2 do the same job, but zrn2 is multi-core
and hence faster.  zrn2 is therefore copied to zrn for use.  All
that is needed to use zrn1 throughtout is to 
copy $BIN/zrn1 to $BIN/zrn

From my side (Richard) the project of trying a disk-based 'transpose'
function is work-in-progress.  It does not even compile at the moment
and has been taken out of the 'compa' script.

Jon's next task is to convert rank (zrn2.c) into nullspace (znu2.c).
I have tried to reduce the complexity of zrn2 a bit - not as much
as I would like.

val1 checks the full set of Conway polynomials and then runs fieldreg
to check that the basic arithmetic is still working.

val2 runs a fairly long script and checks the log-file is the same
as it was.  This occasionally gives errors if the closure of files
happens in a different order, so that the log-file is not textually
identical.  This problem will get more serious as time goes on, and
a better testing methods will ultimately be needed.
