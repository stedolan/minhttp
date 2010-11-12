minHTTP
-------

A tiny and surprisingly featureful webserver.


### minhttp.c

The full version, containing some documentation and configuration
instructions. Builds as a single file with gcc or any other C
compiler.

      gcc minhttp.c -o minhttp
      ./minhttp

### minhttp-80x24.c

Smaller version to fit in an 80x24 terminal. Still has the same
features, but reconfiguring is a little fiddly. You'll need to consult
minttp.c to find out what the config variables mean, and the change
the -DVAR=foo as desired. There are no documentation comments in this
version and it requires a trickier set of compiler flags to make it
compile.

Luckily, the program is also a valid shell script which will correctly
compile itself and then run.

      ./minhttp-80x24.c
