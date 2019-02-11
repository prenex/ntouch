n-touch
=======

The n-touch is a versatile little application that is useful for creating and 
managing numbered files. Contains header-only library and commandline app too!

TL;DR
-----

	Usage:
	------
	
	ntouch --help          # Shows help message
	ntouch                 # Shows help message
	ntouch alma.txt        # Creates alma0.txt - if exists creates alma1.txt instead etc.
	ntouch alma.txt 4      # Inserts alma4.txt as empty - shift earlier as alma5.txt etc.
	ntouch -lr 3 asdf.log  # Logrotate: asdf0.log .. asdf2.log ... asdf0.log ...
	ntouch -lr 3 a.log 2   # Logrotate continuing at: Same as logotate but overwrites at 2 here.
	
	Remarks:
	--------
	
	* We return 0 for success and return 1 in case of errors!
	* On success the operation prints the filename to stdout so you know what got created!
	* All parameters should be non-negative and -lr param cannot be zero!
	* The order of parameters do count! It cannot be anything just the above ones!
	* The numbering always happen BEFORE the last dot in the filename (bf extension) 
	  or at the end of the file if there is no 'file extension'!
	* If you provide an insertno with -lr being on too, you can 'overwrite' an elem of the list!

What is included 
----------------

* There is a header-only library for using the same from c/c++ code easily. I think its useful.

* Otherwise this is just a simple commandline app that installs with `make; sudo make install`.

Use cases
---------

* Automatic unique file creation for small scripts or C programs

* Simple log rotation for small scripts or C programs

* File sequences: you can think of the files as a vector or list with an insert operator as ntouch insert mode, delete operator as rm, push_back operator as ntouch without number parameter. Also length can be calculated with ntouch+rm if needed.

* etc.
