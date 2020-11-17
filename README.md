# MPL

Mathematical Programming Language

How to install:

1) Edit "Makefile"

For the lines:

    install: mpl stdlib.mpl
            cp mpl /usr/local/bin
	    cp stdlib.mpl /usr/local/share/mpl

Change "/usr/local/bin" to where you want the file executible file, "mpl", to be installed.
Change "/usr/local/share/mpl" to the directory you want the "stdlib.mpl" to be installed.

2) Edit "mpl.cc"

For the line:

    const std::string stdlib = "/usr/local/share/mpl/stdlib.mpl";

change the the string "/usr/local/share/mpl/stdlib.mpl" to use the path you used in the Makefile

3) Make program

Enter:

    make

If your system does not have a latex package installed, install one.

4) Install program

Enter:

    sudo make install

5) Enjoy!







