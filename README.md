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

Standard GNU autotools install. See INSTALL for details.

    mkdir bd
    cd bd
    ../configure
    make
    make install

Use --prefix with configure to adjust the installation location.

Usage
=====

Up to date summary can be obtained by

    subgraph --help
    connect --help

Verilator (http://www.veripool.org/wiki/verilator) generates DOT graphs at
various stages during compilations. These graphs use both colors and style of
nodes and edges to indicate particular properties.

For example the gate optimization netlists use nodes in blue for variables and
yellow for logic, with dotted style for variables and logic that are
"consumers".
