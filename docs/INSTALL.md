Installation
============

MSA is executed from the command line, and can be executed in a variety of environments.

Dependencies
------------

You will need the following dependencies:

* make OR some other compatible Makefile reader
* g++ OR clang OR msvc OR other compatible C++ compiler

Usually, these can be installed on your system through its package manager.

In Debian-based systems:

```
$ sudo apt-get install make g++
```

MSA can also be built on Android devices. [Termux](https://play.google.com/store/apps/details?id=com.termux)
is a free terminal that is fully-featured even on non-rooted devices and even provides a package management
system and repository. To install the dependencies, do:

```
$ apt install clang make
```

Building & Running
------------------
Once the dependencies are installed, download the source code and navigate to the project folder and execute
the make script:

```
$ make
```

To build with debug symbols, build the debug target:

```
$ make debug
```

Finally, run the project by executing the `moe-serifu` executable:

```
$ ./moe-serifu
```
