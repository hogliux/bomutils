bomutils
========

*Open source tools to create bill-of-materials files used in Mac OS X installers*

*bomutils* are a set of tools to create Mac OS X installer packages on foreign OSes (windows, linux, etc.). These tools can be used as part of the cross-compilation process. In particular, it includes an open source version of the mkbom tool which is distributed as a closed-source version on Mac OS X Developer Tools.

Build Instructions
------------------
1. Copy the appropriate makefile: 'cp Makefile.system Makefile' where 'system' must be replaced with either 'unix' (for linux and unix type systems) or 'win' for Windows
2. Compile the code by executing: 'make'
3. Tools are available in the build/bin directory
4. Install the tools by executing: 'sudo make install'

Usage
-----
To create a bom file from unix type OSes, follow the following steps.

1. Put the installation payload into a directory. We assume the name of the directory is 'base'
2. Execute 'ls4mkbom base > filelist.txt' to create a list of the contents of the base folder.
3. Edit filelist.txt and modify the user and group ids to match the appropriate values on Mac OS X (typically the root(0)/wheel(80) user/group is used on Mac OS X). For example, an entry in filelist.txt may look like this:

    ./Applications   40755	0/80

4. Use mkbom to create the bom file by invoking 'mkbom -i filelist.txt Bom'

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
