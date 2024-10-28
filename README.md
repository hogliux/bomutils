bomutils
========

_Open source tools to create bill-of-materials files used in Mac OS X installers_

_bomutils_ are a set of tools to create Mac OS X installer packages on foreign OSes (Windows, Linux, etc.). These tools can be used as part of the cross-compilation process. In particular, it includes an open source version of the _mkbom_ tool which on macOS is distributed as a closed-source Developer Tool.

> [!CAUTION]
> This project has matured and is not being actively maintained.\
> \
> Please note that, while bomutils still produce and consume valid BOM formats as of 2024, there is no guarantee for security updates, bug fixes, added features or support for future formats to appear in this repository.

Build Instructions
------------------
1. On UNIX-like OSes, compile the code by executing: `make all`;\
   on Mingw64 for Windows, use `make -f Makefile.win all` instead;\
   there is no support for a native Windows build.
2. Tools become available in the `build/bin` directory after the build.
3. Install the tools by executing the install target on the same Makefile as a privileged user.
   

Usage
-----
To create a Bom file on UNIX type OSes, follow these steps:

1. Put the installation payload into a directory, e.g. `pkgdir`
2. Use _mkbom_ to create the Bom file by invoking `mkbom -u 0 -g 80 pkgdir Bom`

Docker
------
A [`Dockerfile`](./Dockerfile) is provided so that you can build a Docker image with:

```
make docker-image
```

This will build a local Docker image called `bomutils`:

```
$ docker images | egrep '^REPO|bomutils'
REPOSITORY     TAG                 IMAGE ID            CREATED             SIZE
bomutils       latest              b337a65c9450        4 minutes ago       7.18MB
```

Here's an example of running `mkbom` and `lsbom` using this image:

```
$ docker run -it --rm -v $(pwd):/pwd -w /pwd bomutils mkbom src/ Bom

$ docker run -it --rm -v $(pwd):/pwd -w /pwd bomutils lsbom Bom
.	40755	0/0
./bom.h	100644	0/0	4326	3041715736
./crc32.cpp	100644	0/0	4744	188989394
./crc32.hpp	100644	0/0	1042	3230108458
./crc32_poly.hpp	100644	0/0	4609	3805068363
./dumpbom.cpp	100644	0/0	11885	2443865403
./ls4mkbom.cpp	100644	0/0	1851	4279638821
./lsbom.cpp	100644	0/0	14439	3082545787
./mkbom.cpp	100644	0/0	17568	1328881612
./printnode.cpp	100644	0/0	3719	3615238253
./printnode.hpp	100644	0/0	1098	2509017999
```

Note that the Docker image itself comes with a `Bom` (why not?), which you can inspect with:

```
$ docker run -it --rm bomutils lsbom /Bom
.	40755	0/0
./usr	40755	0/0
./usr/bin	40755	0/0
./usr/bin/dumpbom	100755	0/0	1771008	3672370744
./usr/bin/ls4mkbom	100755	0/0	1766912	2953923935
./usr/bin/lsbom	100755	0/0	1783296	1267751980
./usr/bin/mkbom	100755	0/0	1857056	3038373640
```

Documentation
-------------
For full documentation it is best to follow the tutorial at http://hogliux.github.io/bomutils/tutorial.html

Acknowledgments
----------------
Joseph Coffland and Julian Devlin for initial contributions. Baron Roberts for numerous improvements (symlink support, code clean-up, ...)

Contact
-------
@hogliux
