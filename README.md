Linear File System
==================

LinFS is a simple filesystem library that stores data in a linear buffer of unknown size.

There are no any limitations or assumptions about the buffer. To access the buffer it uses
[ReaderWriter](https://github.com/HaK1R/linfs/blob/master/lib/utils/reader_writer.h) interface
which in this implementation operates with regular files but the networked version can also be
made (à la [Network File System](https://en.wikipedia.org/wiki/Network_File_System)).

Getting Started
---------------
LinFS is a Makefile project written in pure C++14.

```console
$ git clone https://github.com/HaK1R/linfs.git linfs
$ cd linfs
$ make
```

The project is well covered by tests written using [Boost Test Library](http://www.boost.org/)
and has been tested on Linux and Windows.
```console
$ make build-tests
$ ./tests/run_tests[.exe]
```

Also the latest release is available for downloading [here](https://github.com/hak1r/linfs/releases).
