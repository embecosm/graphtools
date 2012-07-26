Graphtools
==========

A collection of tools for manipulating GraphViz graphs, particularly intended
for use with Verilator

Prequisites
===========

No pre-built binaries yet, so you'll need developer versions of libraries (for
headers).

Ensure you have the GraphViz (www.graphviz.org/) development tools
installed. For example on Debian/Ubuntu/Linux Mint

    sudo apt-get install graphviz-dev

and on Red Hat/Fedora/CentOS

    sudo yum install graphviz-devel

Installing
==========

No _makefile_ yet. Single line compile

    gcc subgraph.c -lcgraph -o subgraph

Install by copying the binary (_subgraph_) to your preferred location

Usage
=====

Up to date summary can be obtained by

    subgraph -h

Verilator uses both colors and style of nodes and edges to indicate particular
properties. For example the gate optimization netlists use nodes in blue for
variables and yellow for logic, with dotted style for variables and logic that
are "consumers".
