YAZ++ - A C++ library for YAZ

$Id: README.txt,v 1.5 2000-09-13 12:22:32 adam Exp $
 
o Introduction

YAZ is a development toolkit that implements the ANSI Z39.50 protocol.
YAZ homepage is: http://www.indexdata.dk/yaz/
YAZ++ uses the same license as YAZ - see LICENSE file for
details.

YAZ++ is a C++ wrapper on top of that YAZ and provides a relatively
simple interface to YAZ.

o Overview

YAZ++ builds a programmers' library libyaz++.lib and a few
example applications:
  yaz-client   -  small client
  yaz-server   -  small server
  yaz-proxy    -  simple proxy server

Description in HTML format of object model can be found in the sub
directory doc. The top-page of the documentaion is index.html. The
documentation was auto-generated from YAZ++ source using doc++.

Directory structure of the YAZ++ package.

  -- src (C++ source)
  -- include (C++ headers) 
  -- doc (documentation)
  -- unix (UNIX configure script and Makefile)
  -- win (Windows build files)

o Installation, Unix

Make sure you have a C - and C++ compiler available. gcc and
g++ works fine.

Before compilation can take place YAZ must be installed. Unpack
yaz-<version>.tar.gz in the same directory as yaz++. First
build YAZ:

  $ cd yaz-<version>
  $ ./configure
  $ make

Then, build YAZ++:

  $ cd yaz++-<version>
  $ cd unix
  $ ./configure
  $ make

o Installation, Windows

Software is WIN32 and should run on Windows 95/98 and Windows NT 4.0.
Yaz++ was build using Microsoft Visual C++ 6.0. Other compilers should
work but makefile/project files will have to be created for those
compiler environments.

  Workspace yazxx.dsw includes the projects
    yazxx.dsp       -   builds yazxx.dll
    yazclient.dsp   -   builds yazclient.exe
    yazserver.dsp   -   builds yazserver.exe
    yazproxy.dsp    -   builds yazproxy.exe
