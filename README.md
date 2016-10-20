pg_read_uncommitted
===================

Features
--------

Allow to easily read uncommitted on even deleted data from tables.

Installation
============

Compiling
---------

The module can be built using the standard PGXS infrastructure. For this to
work, the ``pg_config`` program must be available in your $PATH. Instruction to
install follows::

 # git clone or download and extract a tarball
 cd pg_read_uncommitted
 make
 make install

Usage
=====

The module is now available. You have to load it if you want to use its
functions::

 rjuju=# load 'pg_read_uncommitted'

You can now enable uncommitted visibility::

 rjuju=# set pg_read_uncommitted.enabled = true;

Or view all data, even deleted rows:

 rjuju=# set pg_read_uncommitted.show_deleted = true;

License
=======

pg_read_uncommitted is free software distributed under the PostgreSQL license.

Copyright (c) 2016, Julien Rouhaud.

