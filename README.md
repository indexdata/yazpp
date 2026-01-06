## YAZ++ AKA yazpp - A C++ library for YAZ and a ZOOM C++ API.

YAZ++ is a set of libraries and header files that make it easier to
use the YAZ toolkit from C++, together with some utilities written
using these libraries. It includes an implementation of the C++ binding
for ZOOM.

YAZ++ and ZOOM C++ use the same BSD-like license as YAZ - see [LICENSE](LICENSE)
file for details.

## Documentation

The "doc" directory contains documentation in HTML and PDF.
You can also read it online at https://www.indexdata.com/yazpp

## Overview

YAZ++ builds a programmers' library libyaz++.lib and a few
applications:

*  yaz-my-client      basic client
*  yaz-my-server      basic server
*  zclient            basic ZOOM client

Directory structure of the YAZ++ package:

* [src](src) C++ library
* [zoom](zoom) C++ source for ZOOM
* [include/yazpp](include/yazpp) C++ headers
* [win](win) Windows build files
* [doc](doc) DocBook-format documentation

## ZOOM-C++

The ZOOM library in this distribution implements the ZOOM C++ binding
described on the
[ZOOM C++ binding web-site](https://zoom.z3950.org/bind/cplusplus/).

It provides a simple but powerful API for constructing Z39.50 client
applications.  See the documentation in doc/zoom.xml for much more
information.
