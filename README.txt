YAZ++ - A C++ library for YAZ

$Id: README.txt,v 1.7 2000-10-26 21:31:50 adam Exp $
 
o Introduction

YAZ++ is a C++ layer for YAZ and implements the ANSI Z39.50
protocol for information retrieval (client - and server side).
YAZ homepage is: http://www.indexdata.dk/yaz/

YAZ++ uses the same license as YAZ - see LICENSE file for details.

o Overview

YAZ++ builds a programmers' library libyaz++.lib and a few
example applications:
  yaz-client++       basic client
  yaz-server++       basic server
  yaz-proxy          proxy server

Description in HTML format of object model can be found in the sub
directory doc. The top-page of the documentaion is index.html. The
documentation was auto-generated from YAZ++ source using doc++.

Directory structure of the YAZ++ package.

  -- src (C++ source)
  -- include (C++ headers) 
  -- doc (documentation)
  -- win (Windows build files)

o Installation, Unix

Make sure you have a C - and C++ compiler available. gcc and
g++ works fine.

Before compilation can take place YAZ must be installed. It goes, roughly,
like this:

  $ cd yaz-<version>
  $ ./configure
  $ make
  $ cd ..

Then, build YAZ++:

  $ cd yaz++-<version>
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
