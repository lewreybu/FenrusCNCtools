/*
 * (C) Copyright 2019  -  Arjan van de Ven <arjanvandeven@gmail.com>
 *
 * This file is part of FenrusCNCtools
 *
 * SPDX-License-Identifier: GPL-3.0
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

extern "C" {
    #include "toolpath.h"
}

int verbose = 0;
int want_skeleton_path = 0;
int want_inbetween_paths = 0;

static int depth;

void usage(void)
{
	printf("Usage:\n\ttoolpath [-f] [-s] [-l <toollibrary.csv>] [-t <nr] <file.svg>\n");
	exit(EXIT_SUCCESS);
}


int main(int argc, char **argv)
{
    int opt;
    int tool = 102;
    
    read_tool_lib("toollib.csv");
    
    depth = inch_to_mm(0.044);

    while ((opt = getopt(argc, argv, "vfsil:t:d:D:")) != -1) {
        switch (opt)
	{
			case 'v':
				verbose = 1;
				break;
			case 'f':
				enable_finishing_pass();
				printf("Finishing pass enabled\n");
				break;
			case 's':
				want_skeleton_path = 1;
				printf("Skeleton path enabled\n");
				break;
			case 'i':
				want_inbetween_paths = 1;
				printf("Inbetween paths enabled\n");
				break;
			case 'l':
				read_tool_lib(optarg);
				break;	
			case 'd': /* inch */
				depth = inch_to_mm(strtod(optarg, NULL));
				break;
			case 'D': /* metric mm*/
				depth = strtod(optarg, NULL);
				break;
			case 't':
				int arg;
				arg = strtoull(optarg, NULL, 10);
				if (have_tool(arg)) {
					tool = arg;
					push_tool(tool);
				} else {
					printf("Unknown tool requested\n");
					print_tools();
				}
				break;
			
			default:
				usage();
	}
    }
	
    if (optind == argc) {
    	usage();
    }
    
    set_rippem(15000);
    set_retract_height_imperial(0.06);
    set_default_tool(tool);

    for(; optind < argc; optind++) {      
		parse_svg_file(argv[optind]);
		
		process_nesting();
		
		create_toolpaths(-depth);
		consolidate_toolpaths();
		
		write_svg("output.svg");
		write_gcode("output.nc");
    }
    return EXIT_SUCCESS;
}