/* -*- mode: C; c-file-style: "gnu"; fill-column: 78; -*-
   graphtools.h - public header for the graphtools library.

   Copyright (C) 2012 Embecosm Limited

   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embecosm graphtools

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef GRAPHTOOLS__H
#define GRAPHTOOLS__H

#include <graphviz/cgraph.h>

/*! A structure for attribute matching. */
typedef struct attr_match
{
  char              *name;
  int                matchit;		/*!< Bool whether to match/not match */
  int                kind;		/*!< AGRAPH, AGNODE or AGEDGE */
  Agsym_t           *attr;		/*!< attribute in old graph */
  char              *value;		/*!< value to match */
  struct attr_match *next;		/*!< Hold as linked list */
} Attr_match_t;

/*! A structure to hold all the attributes of a graph.

    Separate null terminated vectors for graph, node and edge attributes
    to allow fast iteration of each. */
typedef struct all_attr
{
  Agsym_t **graph_attrs;	/*!< Vector of edge attributes in graph */
  Agsym_t **node_attrs;		/*!< Vector of node attributes in graph */
  Agsym_t **edge_attrs;		/*!< Vector of edge attributes in graph */
} All_attr_t;


/* Externally visible functions. Commented in the implementation source */

void clone_attributes (Agraph_t   *og,
		       Agraph_t   *ng,
		       All_attr_t *old_attrs,
		       All_attr_t *new_attrs,
		       int         kind);

Agnode_t *clone_node  (Agraph_t   *g,
		       Agnode_t   *nodep,
		       All_attr_t *old_attrs,
		       All_attr_t *new_attrs);

Agedge_t *clone_edge (Agraph_t   *g,
		      Agedge_t   *edgep,
		      Agnode_t   *tailp,
		      Agnode_t   *headp,
		      All_attr_t *old_attrs,
		      All_attr_t *new_attrs);

int  check_attributes (void         *objp,
		       int           kind,
		       Attr_match_t *amp);

#endif
