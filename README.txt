YAZ++ - A C++ library for YAZ

$Id: README.txt,v 1.1 1999-01-28 09:41:07 adam Exp $
 
o Introduction

YAZ is development toolkit that implements the ANSI Z39.50 protocol.
YAZ homepage is: http://www.indexdata.dk/yaz

YAZ++ is a C++ wrapper on top of that YAZ. It provides a relatively
simple, high level, interface to YAZ.

o Documentatation

Description of object model can be found in the sub directory doc, file
index.html.

The documentation was auto-generated from source using doc++.

o Installation

Before compilation can take place YAZ must be installed. Unpack
yaz-<version>.tar.gz in the same directory as yaz++. 

For Unix - make sure you have a C++ compiler available. g++ works fine.
  $ cd unix
  $ ./configure
  $ make

For Windows. The software was build using Microsoft Visual C++ 5.0 and
6.0. Other compilers should work but the makefile will have to be
modified.
  > cd win
  > nmake      (Visual C++ makefile not yet created)

