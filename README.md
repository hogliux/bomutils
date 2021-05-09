bomutils
========

_Open source tools to create bill-of-materials files used in Mac OS X installers_

_bomutils_ are a set of tools to create Mac OS X installer packages on foreign OSes (windows, linux, etc.). These tools can be used as part of the cross-compilation process. In particular, it includes an open source version of the mkbom tool which is distributed as a closed-source version on Mac OS X Developer Tools.

Build Instructions
------------------
1. Copy the appropriate makefile: `cp Makefile.system Makefile` where `system` must be replaced with either `unix` (for linux and unix type systems) or `win` for Windows
2. Compile the code by executing: `make all`
3. Tools are available in the `build/bin` directory
4. Install the tools by executing: `sudo make install`

Usage
-----
To create a bom file from unix type OSes, follow the following steps.

1. Put the installation payload into a directory. We assume the name of the directory is `base`
2. Use mkbom to create the bom file by invoking `mkbom -u 0 -g 80 base Bom`

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
