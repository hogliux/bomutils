bomutils
========

*Open source tools to create bill-of-materials files used in Mac OS X installers*

*bomutils* are a set of tools to create Mac OS X installer packages on foreign OSes (windows, linux, etc.). These tools can be used as part of the cross-compilation process. In particular, it includes an open source version of the mkbom tool which is distributed as a closed-source version on Mac OS X Developer Tools.

Build Instructions
------------------
1. Copy the appropriate makefile: 'cp Makefile.system Makefile' where 'system' must be replaced with either 'unix' (for linux and unix type systems) or 'win' for Windows
2. Compile the code by executing: 'make'
3. Tools are available in the 'build/bin' directory
4. Install the tools by executing: 'sudo make install'

Usage
-----
To create a bom file from unix type OSes, follow the following steps.

1. Put the installation payload into a directory. We assume the name of the directory is 'base'
2. Use mkbom to create the bom file by invoking 'mkbom -u 0 -g 80 base Bom'`

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
Fabian Renn, fabian.renn@gmail.com
http://hogliux.github.io/bomutils

Donations
---------

Creating and maintaining bomutils is time-consuming. If you find bomutils useful, then why not consider donating:

Bitcoin: 1AUYAR1uzs8c3RnpEHM8kqQYN8eXaxdLKi
PayPal: fabian.renn@gmail.com
