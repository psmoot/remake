/* $Id: buildargv.c,v 1.2 2006/02/27 03:17:51 rockyb Exp $ */
/* Create and destroy argument vectors (argv's)
   Copyright (C) 1992, 2001 Free Software Foundation, Inc.
   Written by Fred Fish @ Cygnus Support

   Copyright (C) 2008 R. Bernstein rocky@gnu.org

This file is part of the libiberty library.
Libiberty is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

Libiberty is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with libiberty; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* rocky: Below we only need/use the freeargv and buildargv routine
   from the above.  Some small adaption was made to include outside of
   libiberty.
*/
#include <ctype.h>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#include <string.h>
#include <stdlib.h>
#include "buildargv.h"

#ifndef EOS
#define EOS '\0'
#endif

#define INITIAL_MAXARGC 8	/* Number of args + NULL in initial argv */
#define ISBLANK(ch) ((ch) == ' ' || (ch) == '\t')

/*

@deftypefn Extension void freeargv (char **@var{vector})

Free an argument vector that was built using @code{buildargv}.  Simply
scans through @var{vector}, freeing the memory for each argument until
the terminating @code{NULL} is found, and then frees @var{vector}
itself.

@end deftypefn

*/

void freeargv (vector)
char **vector;
{
  register char **scan;

  if (vector != NULL)
    {
      for (scan = vector; *scan != NULL; scan++)
	{
	  free (*scan);
	}
      free (vector);
    }
}

/*

@deftypefn Extension char** buildargv (char *@var{sp})

Given a pointer to a string, parse the string extracting fields
separated by whitespace and optionally enclosed within either single
or double quotes (which are stripped off), and build a vector of
pointers to copies of the string for each field.  The input string
remains unchanged.  The last element of the vector is followed by a
@code{NULL} element.

All of the memory for the pointer array and copies of the string
is obtained from @code{malloc}.  All of the memory can be returned to the
system with the single function call @code{freeargv}, which takes the
returned result of @code{buildargv}, as it's argument.

Returns a pointer to the argument vector if successful.  Returns
@code{NULL} if @var{sp} is @code{NULL} or if there is insufficient
memory to complete building the argument vector.

If the input is a null string (as opposed to a @code{NULL} pointer),
then buildarg returns an argument vector that has one arg, a null
string.

@end deftypefn

The memory for the argv array is dynamically expanded as necessary.

In order to provide a working buffer for extracting arguments into,
with appropriate stripping of quotes and translation of backslash
sequences, we allocate a working buffer at least as long as the input
string.  This ensures that we always have enough space in which to
work, since the extracted arg is never larger than the input string.

The argument vector is always kept terminated with a @code{NULL} arg
pointer, so it can be passed to @code{freeargv} at any time, or
returned, as appropriate.

*/

char **buildargv (input)
     const char *input;
{
  char *arg;
  char *copybuf;
  int squote = 0;
  int dquote = 0;
  int bsquote = 0;
  int argc = 0;
  int maxargc = 0;
  char **argv = NULL;
  char **nargv;

  if (input != NULL)
    {
      copybuf = (char *) alloca (strlen (input) + 1);
      /* Is a do{}while to always execute the loop once.  Always return an
	 argv, even for null strings.  See NOTES above, test case below. */
      do
	{
	  /* Pick off argv[argc] */
	  while (ISBLANK (*input))
	    {
	      input++;
	    }
	  if ((maxargc == 0) || (argc >= (maxargc - 1)))
	    {
	      /* argv needs initialization, or expansion */
	      if (argv == NULL)
		{
		  maxargc = INITIAL_MAXARGC;
		  nargv = (char **) malloc (maxargc * sizeof (char *));
		}
	      else
		{
		  maxargc *= 2;
		  nargv = (char **) realloc (argv, maxargc * sizeof (char *));
		}
	      if (nargv == NULL)
		{
		  if (argv != NULL)
		    {
		      freeargv (argv);
		      argv = NULL;
		    }
		  break;
		}
	      argv = nargv;
	      argv[argc] = NULL;
	    }
	  /* Begin scanning arg */
	  arg = copybuf;
	  while (*input != EOS)
	    {
	      if (ISBLANK (*input) && !squote && !dquote && !bsquote)
		{
		  break;
		}
	      else
		{
		  if (bsquote)
		    {
		      bsquote = 0;
		      *arg++ = *input;
		    }
		  else if (*input == '\\')
		    {
		      bsquote = 1;
		    }
		  else if (squote)
		    {
		      if (*input == '\'')
			{
			  squote = 0;
			}
		      else
			{
			  *arg++ = *input;
			}
		    }
		  else if (dquote)
		    {
		      if (*input == '"')
			{
			  dquote = 0;
			}
		      else
			{
			  *arg++ = *input;
			}
		    }
		  else
		    {
		      if (*input == '\'')
			{
			  squote = 1;
			}
		      else if (*input == '"')
			{
			  dquote = 1;
			}
		      else
			{
			  *arg++ = *input;
			}
		    }
		  input++;
		}
	    }
	  *arg = EOS;
	  argv[argc] = strdup (copybuf);
	  if (argv[argc] == NULL)
	    {
	      freeargv (argv);
	      argv = NULL;
	      break;
	    }
	  argc++;
	  argv[argc] = NULL;

	  while (ISBLANK (*input))
	    {
	      input++;
	    }
	}
      while (*input != EOS);
    }
  return (argv);
}
