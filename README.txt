YAZ++ - A C++ library for YAZ

$Id: README.txt,v 1.13 2003-10-16 13:40:41 adam Exp $
 

Introduction
------------

YAZ++ is a C++ layer for YAZ and implements the ANSI Z39.50
protocol for information retrieval (client and server side).
The YAZ++ homepage is: http://www.indexdata.dk/yaz++/

YAZ++ uses the same license as YAZ - see LICENSE file for details.

Documentation
-------------

Directory sub contains documentation in HTML and PDF. You can
also read it online at http://www.indexdata.dk/yaz++/

Overview
--------

YAZ++ builds a programmers' library libyaz++.lib and a few
example applications:

  yaz-my-client      basic client
  yaz-my-server      basic server
  yaz-proxy          not-so-basic proxy server

Directory structure of the YAZ++ package:

  -- src (C++ library and proxy source)
  -- zoom (C++ source for ZOOM)
  -- include/yaz++ (C++ headers) 
  -- lib (compiled libraries)
  -- win (Windows build files)
  -- doc (DocBook-format documentation)


Installation, Unix
------------------

Make sure you have a C and C++ compiler available. gcc and g++ work fine.

Before compilation can take place YAZ must be installed. It goes, roughly,
like this:

  $ cd yaz-<version>
  $ ./configure
  $ make
  $ su
  # make install
  $ ^D
  $ cd ..

The YAZ proxy uses a configuration file in XML and libxml2 is required
in order for the config facility to work. Many systems already have
libxml2 installed. In that, case remember to get the devel version
of that as well (not just the runtime). Libxml2 can also be compiled
easily on most systems. See http://www.xmlsoft.org/ for more
information.

Then, build YAZ++:

  $ cd yaz++-<version>
  $ ./configure
  $ make

If you do have libxml2 installed and configure above does not find it,
use option --with-xml2=PREFIX to specify its location. yaz++ configure
looks for PREFIX/bin/xml2-config.

Installation, Windows
---------------------

YAZ++ for WIN32 should run on Windows 95/98/2K and Windows NT 4.0.
Yaz++ was built using Microsoft Visual C++ 6.0. Other compilers
should work but makefile/project files will have to be created for
those compilers.

  Workspace yazxx.dsw includes the projects
    yazxx.dsp       -   builds yazxx.dll
    yazclient.dsp   -   builds yazmyclient.exe
    yazserver.dsp   -   builds yazmyserver.exe
    yazproxy.dsp    -   builds yazproxy.exe


About the proxy
---------------

For the proxy the actual target is determined in by the OtherInfo
part of the InitRequest. We've defined an OID for this which we call
PROXY. The OID is 1.2.840.10003.10.1000.81.1. 

  OtherInformation   ::= [201] IMPLICIT SEQUENCE OF SEQUENCE{
    category           [1]   IMPLICIT InfoCategory OPTIONAL, 
    information        CHOICE{
      characterInfo            [2]  IMPLICIT InternationalString,
      binaryInfo               [3]  IMPLICIT OCTET STRING,
      externallyDefinedInfo    [4]  IMPLICIT EXTERNAL,
      oid                      [5]  IMPLICIT OBJECT IDENTIFIER}}
--
  InfoCategory ::= SEQUENCE{
      categoryTypeId   [1]   IMPLICIT OBJECT IDENTIFIER OPTIONAL,
      categoryValue    [2]   IMPLICIT INTEGER}

The InfoCategory is present with categoryTypeId set to the PROXY OID
and categoryValue set to 0. The information in OtherInformation uses
characterInfo to represent the target using the form target[:port][/db].

For clients that don't set the PROXY OtherInformation, a default
target can be specified using option -t for proxy.

Example:
  We start the proxy so that it listens on port 9000. The default
  target is Bell Labs Library unless it is overridden by a client in
  the InitRequest.

     $ ./yaz-proxy -t z3950.bell-labs.com/books @:9000

  The client is started and talks to the proxy without specifying
  a target. Hence this client will talk to the Bell Labs server.

     $ ./yaz-client localhost:9000

  The client is started and it specifies the actual target itself.

     $ ./yaz-client -p localhost:9000 bagel.indexdata.dk/gils

  For ZAP the equivalent would be
     proxy=localhost:9000
     target=bagel.indexdata.dk/gils
  Simple, huh!


ZOOM-C++
--------

The ZOOM library in this distribution implements the ZOOM C++ binding
described on the ZOOM web-site at 
	http://zoom.z3950.org/bind/cplusplus/index.html
It provides a simple but powerful API for constructing Z39.50 client
applications.  See the documentation in doc/zoom.xml for much more
information.
