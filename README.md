n-touch
=======

The n-touch is a versatile little application that is useful for creating and 
managing numbered files. Contains header-only library and commandline app too!

TL;DR
-----

	ntouch --help          # Shows help message
	ntouch                 # Shows help message
	ntouch alma.txt        # Creates alma0.txt - if exists creates alma1.txt instead etc.
	ntouch alma.txt 4      # Inserts alma4.txt as empty - shift earlier as alma5.txt etc.
	ntouch -lr 3 asdf.log  # Logrotate: asdf0.log .. asdf2.log ... asdf0.log ...
	ntouch -lr 3 a.log 2   # Logrotate with insert: mix of -lr and the insert num.

	The operation always prints the filename to stdout so you know what to open!

What is included 
----------------

* There is a header-only library for using the same from c/c++ code easily. I think its useful.

* Otherwise this is just a simple commandline app that installs with `make; sudo make install`.

Use cases
---------

* Simple log rotation for small scrpts or C programs

* Automatic unique file creation for small scripts or C programs

* File sequences: you can think of the files as a vector or list with an insert operator as ntouch insert mode, delete operator as rm, push_back operator as ntouch without number parameter. Also length can be calculated with ntouch+rm if needed.

* etc.
