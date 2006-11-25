/* options.c
 *
 * Copyright � 2001 Lutz M�ller <lutz@users.sf.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include "actions.h"
#include "globals.h"
#include "i18n.h"
#include "main.h"
#include "options.h"

#include <stdio.h>
#include <string.h>

#define CR(result) {int r = (result); if (r < 0) return r;}

#define SHORT_OPTION  "-"
#define LONG_OPTION   "--"

int option_is_present (char *op, int argc, char **argv) {
        /* checks to see if op is in the command-line. it will */
        /* check for both short and long option-formats for op */

        int x, found=0;
        char s[5], l[20];

        /* look for short/long options and fill them in */
        for (x=0; x<glob_option_count; x++) {
                if ((strcmp(op, option[x].short_id)==0)||
                    (strcmp(op, option[x].long_id)==0)) {
                        sprintf(s, "%s%s", SHORT_OPTION, option[x].short_id);
                        sprintf(l, "%s%s", LONG_OPTION, option[x].long_id);
                        found=1;
                }
        }

        /* Strictly require an option in the option table */
        if (!found)
                return (GP_ERROR_BAD_PARAMETERS);

        /* look through argv, if a match is found, return */
        for (x=1; x<argc; x++)
                if ((strcmp(s, argv[x])==0)||(strcmp(l, argv[x])==0))
                        return (GP_OK);

        return (GP_ERROR_BAD_PARAMETERS);
}

int
verify_options (int argc, char **argv)
{
        int x, y, match, missing_arg, which;
        char s[5], l[24];

        which = 0;

	for (x=1; x<argc; x++) {
		match = 0;
		missing_arg = 0;
		for (y=0; y<glob_option_count; y++) {
			/* Check to see if the option matches */
			sprintf(s, "%s%s", SHORT_OPTION, option[y].short_id);
			sprintf(l, "%s%s", LONG_OPTION, option[y].long_id);

			if ((strlen (option[y].short_id) &&
						!strcmp (s, argv[x])) ||
			    (strlen (option[y].long_id) &&
			     			!strcmp (l, argv[x]))) {

				/* Check to see if the option requires an argument */
				if (strlen(option[y].argument)>0) {
					if (x+1 < argc) {
					   if (
				(strncmp(argv[x+1], SHORT_OPTION, strlen(SHORT_OPTION))!=0) &&
				(strncmp(argv[x+1], LONG_OPTION, strlen(LONG_OPTION))!=0)
					      ) {
						match=1;
						x++;
					   } else {
						which=y;
						missing_arg=1;
					   }
					} else {
					   missing_arg=1;
					   which=y;
					}
				}  else
					match=1;
			}
		}
		if (!match) {
			cli_error_print("Bad option \"%s\": ", argv[x]);
			if (missing_arg) {
				cli_error_print("    Missing argument. You must specify the \"%s\"",
					option[which].argument);
			}   else
				cli_error_print("    unknown option");
			return (GP_ERROR_BAD_PARAMETERS);
		}
	}

	/* Make sure required options are present */
	for (x=0; x<glob_option_count; x++) {
	   if (option[x].required) {
		if (option_is_present(option[x].short_id, argc, argv) != GP_OK) {
			printf("Option %s%s is required.\n",
			 strlen(option[x].short_id)>0? SHORT_OPTION:LONG_OPTION,
			 strlen(option[x].short_id)>0? option[x].short_id:option[x].long_id);
			return (GP_ERROR_BAD_PARAMETERS);
		}
	   }
	}

        return (GP_OK);
}

int
execute_options (int argc, char **argv) {

        int x, y;
	const char *o;

        /* Execute the command-line options */
        for (x = 0; x < glob_option_count; x++) {

		/* If there is no function, skip this option */
		if (!option[x].execute)
			continue;

		/* Did the user use this option? */
                for (y = 1; y < argc; y++) {

			/*
			 * Skip leading "-". We assume that the syntax has
			 * already been verified.
			 */
			o = argv[y];
			while (o[0] == '-')
				o++;

			if ((strlen (option[x].short_id) &&
					!strcmp (o, option[x].short_id)) ||
			    (strlen (option[x].long_id) &&
			     		!strcmp (o, option[x].long_id))) {
				if (strlen (option[x].argument) > 0) {
					CR ((*option[x].execute) (argv[++y]));
				} else {
					CR ((*option[x].execute) (NULL));
				}
			}
                }
        }

        return (GP_OK);
}

void
usage (GPParams *params)
{
        int x=0;
        char buf[128], s[5], l[24], a[16];

	/* Standard licensing stuff */
	print_version_action (params);
        printf (_("Usage:\n"));

	/* Make this 79 characters long. Some languages need the space. */
	printf (_("Short/long options (& argument)        Description\n"
		  "--------------------------------------------------------------------------------\n"));
	/* Run through option and print them out */
	while (x < glob_option_count) {
		/* maybe sort these by short option? can't be an in-place sort.
		   would need to memcpy() to a new struct array. */
		if (strlen(option[x].short_id) > 0)
			sprintf(s, "%s%s ", SHORT_OPTION, option[x].short_id);
		   else
			sprintf(s, " ");

		if (strlen(option[x].long_id) > 0)
			sprintf(l, "%s%s", LONG_OPTION, option[x].long_id);
		   else
			sprintf(l, " ");

		if (strlen(option[x].argument) > 0)
			sprintf(a, "%s", option[x].argument);
		   else
			sprintf(a, " ");
		sprintf(buf, " %-4s %s %s", s, l, a);
		/* The format line is made translatable so that some
		   languages can make the table a bit tighter. Make
		   sure you only use 79 characters (since #80 is
		   needed for the line break). */
		printf(_("%-38s %s\n"), buf, _(option[x].description));
		x++;
	}

	/* Make this 79 characters long. Some languages need the space. */
	printf (_("--------------------------------------------------------------------------------\n"
		  "[Use double-quotes around arguments]        [Picture numbers begin with one (1)]\n"));
}


/*
 * Local Variables:
 * c-file-style:"linux"
 * indent-tabs-mode:t
 * End:
 */