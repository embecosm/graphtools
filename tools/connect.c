/* Simple DOT graph manipulation program.

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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   *************************************************************************

   Take DOT files (as generated by Verilator) and find the subgraph connecting
   two nodes. 

   For Usage see the usage () function below.

   When considering directed graphs, remember the GraphViz cgraph library
   uses the following naming:

     tail -> head */

#include "config.h"

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graphtools.h"


/*! All the attributes held by original and new graphs. */
static All_attr_t old_attrs;		/*!< Attributes in old graph */
static All_attr_t new_attrs;		/*!< Attributes in new graph */

/*! A global structure holding all the options. */
static struct
{
  char *label_from;		/*!< Prefix of the start node label */
  char *label_to;		/*!< Prefix of the end node label */
  char *output;			/*!< Output filename ("-" for stdout) */
  char *input;			/*!< Input filename ("-" for stdin) */
  FILE *fd_out;			/*!< Descriptor for opened output file */
  FILE *fd_in;			/*!< Descriptor for opened input file */
} opts = 
  {
    .label_from = "",
    .label_to   = "",
    .output     = "-",
    .input      = "-"
  };

/*! A type for records maintaining a count. */
typedef struct countrec {
  Agrec_t  header;		/*!< Standard record header */
  int      count;		/*!< a count for this node */
} Countrec_t;


/*! Print details of usage

    Used for help or error.

    @param[in] prog  Name of the program causing the error.
    @param[in] fd    Where to print the details (stdout for help, sterr for
                     errors. */
static void
print_usage (char *prog,
	     FILE *fd)
{
  fprintf (fd, "Usage: %s [-h] | -l <label> [-o <outfile>] [<file>]\n\n", prog);
  fprintf (fd, "    (no arguments)\n");
  fprintf (fd, "    -h\n");
  fprintf (fd, "    --help\n");
  fprintf (fd, "        Print this help message and exit\n\n");
  fprintf (fd, "    -v\n");
  fprintf (fd, "    --version\n");
  fprintf (fd, "        Print the program version and exit\n\n");
  fprintf (fd, "    -f <label>\n");
  fprintf (fd, "    -label-from=<label>\n");
  fprintf (fd, "        Start from the node with this label prefix\n\n");
  fprintf (fd, "    -t <label>\n");
  fprintf (fd, "    -label-to=<label>\n");
  fprintf (fd, "        Start from the node with this label prefix\n\n");
  fprintf (fd, "    -o <file>\n");
  fprintf (fd, "        Output file, use stdout if not specified or '-'\n\n");
  fprintf (fd, "    <file>\n");
  fprintf (fd, "        Input file, use stdin if not specified or '-'\n");

}	/* print_usage () */


/*! Parse the arguments.

    Also open the files.

    @param [in] argc  Number of arguments
    @param [in] argv  Vector of arguments */
static void
parse_args (int    argc,
	    char **argv)
{
  static struct option  long_options[] =
    {
      {"help",       no_argument,       0, 'h'},
      {"version",    no_argument,       0, 'v'},
      {"label-from", required_argument, 0, 'f'},
      {"label-to",   required_argument, 0, 't'},
      {"output",     required_argument, 0, 'o'},
      {NULL,         0,                 0, 0}};
  int  c;
  int  have_err = 0;

  if (1 == argc)
    {
      /* No args, print usage. */
      print_usage (basename (argv[0]), stdout);
      exit (0);
    }

  while (-1 != (c = getopt_long (argc, argv, ":hvf:t:o:", long_options, NULL)))
    {
      switch (c)
	{
	case 'h':
	  print_usage (basename (argv[0]), stdout);
	  exit (0);
	  
	case 'v':
	  printf ("%s\n", PACKAGE_STRING);
	  exit (0);
	  break;

	case 'f':
	  opts.label_from = optarg;
	  break;

	case 't':
	  opts.label_to = optarg;
	  break;

	case 'o':
	  opts.output = optarg;
	  break;

	case ':':
	  fprintf (stderr, "ERROR: Missing argument to %s\n", argv[optind - 1]);
	  have_err = 1;
	  break;

	case '?':
	  fprintf (stderr, "ERROR: Unrecognized option %s\n", argv[optind - 1]);
	  have_err = 1;
	  break;

	default:
	  abort ();
	}
    }

  switch (argc - optind)
    {
    case 0:
      opts.input = "-";
      break;				/* None specified, use stdin. */

    case 1:
      opts.input = argv[optind];
      break;

    default:
      fprintf (stderr, "ERROR: Multiple input files specified\n");
      have_err = 1;
      break;
    }

  if (have_err)
    {
      print_usage (basename (argv[0]), stderr);
      exit (1);
    }

  /* Open the files. */
  if (0 == strcmp (opts.input, "-"))
    {
      opts.fd_in = stdin;
    }
  else
    {
      if (NULL == (opts.fd_in = fopen (opts.input, "r")))
	{
	  fprintf (stderr, "ERROR: failed to open \"%s\": %s\n", opts.input,
		  strerror (errno));
	  exit (1);
	}
    }  

  if (0 == strcmp (opts.output, "-"))
    {
      opts.fd_out = stdout;
    }
  else
    {
      if (NULL == (opts.fd_out = fopen (opts.output, "w")))
	{
	  fprintf (stderr, "ERROR: failed to open \"%s\": %s\n", opts.output,
		  strerror (errno));
	  exit (1);
	}
    }  
}	/* parse_args () */


/*! Initialize the count record for each node.

    An initial value of -1 indicates this node has not been visited.

    @param[in] g  Graph containing the nodes to be initialized. */
static void
init_count (Agraph_t *g)
{
  /* Initialize a record holding the count for all nodes. */
  aginit (ng, AGNODE, "count", sizeof (Countrec_t), AG_MTF_SOFT);

  /* Iterate over all nodes setting the count to 1. */
  for (nodep = agfstnode (g); nodep != NULL; nodep = agnxtnode (g, nodep))
    {
      Countrec_t *recp = (Countrec_t *) aggetrec (nodep, "count", AG_MTF_SOFT);
      recp->count = -1;
    }
}	/* init_count () */


/*! Recursively add nodes to a sub-tree.

    We look at nodes that go from the start node to the end node. If a node is
    a connection point to the end nde we add it to the new graph.

    @param [in] og         Old graph
    @param [in] ng         New graph
    @param [in] nodep      Node we are currently at
    @param [in] endp       End node

    @return  The number of routes to the end node from the current node. */
static Agnode_t *
add_nodes (Agraph_t     *og,
	   Agraph_t     *ng,
	   Agnode_t     *nodep,
	   Agnode_t     *endp)
{
  Countrec_t *recp = (Countrec_t *) aggetrec (nodep, "count", AG_MTF_SOFT);

  /* If we have seen this node before, just return its count. */
  if (recp->count != -1)
    {
      return recp->count;
    }

  /* If this node is the end node, set its count to 1, add the node, together
     with any edges */

  /* Need to recurse here. */
  return 0

}
  
/*! Main program

    @param [in] argc  Number of arguments
    @param [in] argv  Vector of arguments */
int
main(int    argc,
     char **argv)
{

  /* Parse the arguments. */
  parse_args (argc, argv);

  /* Read the graph *before* we parse the other arguments, so attributes are
     available for us to look up. */
  Agraph_t *g;				/* The whole graph */
  char     *gname = "";			/* Its name */

  if (NULL == (g = agread(opts.fd_in, NULL)))
    {
      fprintf (stderr, "ERROR: Failed to read graph\n");
      exit (1);
    }
  else
    {
      gname = agnameof (g);
    }

  /* Find the desired start and end node label prefixes. */
  Agnode_t *startp = NULL;
  Agnode_t *endp = NULL;
  Agnode_t *nodep;

  for (nodep = agfstnode (g); nodep != NULL; nodep = agnxtnode (g, nodep))
    {
      char *label = agget (nodep, "label");
      if (0 == strncmp (label, opts.label_from, strlen(opts.label_from)))
	{
	  startp = nodep;
	  break;
	}
      if (0 == strncmp (label, opts.label_to, strlen(opts.label_to)))
	{
	  endp = nodep;
	  break;
	}
    }

  if (!startp)
    {
      fprintf (stderr, "ERROR: Failed to find start node %s\n",
	       opts.label_from);
      exit (1);
    }

  if (!endp)
    {
      fprintf (stderr, "ERROR: Failed to find end node %s\n", opts.label_to);
      exit (1);
    }

  /* Create a new graph, add all the attributes from the old graph, labelled
     appropriately and with an initial count of -1 on each node. */
  Agraph_t *ng;
  char     *ngname  = malloc (snprintf (NULL, 0, "%s_subgraph", gname) + 1);
  char     *nglabel = malloc (snprintf (NULL, 0, "%s -> %s", opts.label_from,
					opts.label_to) + 1);

  sprintf (ngname, "%s_subgraph", gname);
  ng = agopen (ngname, g->desc, NULL);

  clone_attributes (g, ng, &old_attrs, &new_attrs);

  sprintf(nglabel, "%s -> %s", opts.label_from, opts.label_to);
  label_extend_graph (ng, nglabel);

  init_count (ng);

  /* Start at the root node and find all the nodes deriving from it, adding
     those nodes and the edges connecting them to the graph. Then write out
     the graph. */
  int num_routes    = add_nodes (g, ng, startp, endp);
  char *route_count = malloc (snprintf (NULL, 0, "%d routes", num_routes) + 1);
  sprintf(route_count, "%d routes", num_routes);
  label_extend_graph (ng, route_count);

  agwrite (ng, stdout);

  exit (0);

}	/* main () */
