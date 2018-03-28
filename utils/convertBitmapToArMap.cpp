/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016-2018 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/

#define USAGE_TEXT "\n"\
"Usage: convertBitmapToArMap [-resolution|-r <resolution>] [-color|-c <color>] [-input|-i <inputfile>] [-output|-o <outputfile>] [-help|-h]\n"\
"\n"\
"Supported input file formats are: PPM, PBM, PGM, PNM.\n"\
"\n"\
"Example: convertBitmapToArMap -r 10 -i example.pbm -o example.map\n"\
"\n"\
"This program converts a bitmap file to a MobileRobots Inc. ArMap (.map) file.\n"\
"You can specify the resolution of the bitmap (pixels to milimeters ratio)\n"\
"on the command line using the \"-resolution\" or \"-r\" option. If omitted, a\n"\
"ratio of 1 will be asumed (1 pixel per milimeter). An obstacle point will\n"\
"be placed in the ArMap for each pixel in the bitmap, spaced according to\n"\
"its resolution. I.e., a pixel at row R column C will result in one data point\n"\
"at position (R * resolution), (C * resolution).\n"\
"All nonblack pixels in the bitmap are used as datapoints, unless the \"-color\"\n"\
"or \"-c\" is given, in which case only that color value (0..256 for pgm, 0 or\n"\
"1 for pbm, 0..765 for pnm/ppm) is considered.  Specify the input file with\n"\
"\"-input\" or \"-i\" (or just give the file) and output file with -output or -o.\n"\
"If no input is given, stdin is used. If no output file is given, the input\n"\
"file with \".map\" extension is used, or stdout if stdin is the input file.\n"

#include <string.h>
#include "ArMap.h"

extern "C" {
#include <pam.h>
}


void usage() 
{
    puts(USAGE_TEXT);
}

/* Macro expanding to a boolean expression matching variations of a command line argument. short_arg
 * and long_arg must be literal string constants (ignored if "")
 */
#define command_argument_match(arg_given, short_arg, long_arg) \
  ( (strlen(short_arg) > 0 && strcmp(arg_given, "-" short_arg) == 0) || \
    (strlen(long_arg) > 0 && strcmp(arg_given, "--" long_arg) == 0) || \
    (strlen(long_arg) > 0 && strcmp(arg_given, "-" long_arg) == 0)  )

int main(int argc, char **argv)
{
    int backgroundColor = 0;
    bool useExactColor = false; // If true, don't use != backgroundColor, instead it must equal exactColor 
    int exactColor = 0;
    double resolution = 1.0;
    struct pam bitmap;
    tuple *tuplerow;
    FILE *infp = 0;
    char *inputfilename = 0;
    char *outputfilename = 0;

    // check for help before pnm_init because it checks and gives an unhelpful
    // message.
    for(int i = 1; i < argc; ++i)
    {
        if(command_argument_match(argv[i], "h", "help")) {
            usage();
            exit(0);
        }
    }

    // init libnetpbm
    pnm_init(&argc, argv); 

    // check for command line arguments
    for(int i = 1; i < argc; ++i)
    {
        if(command_argument_match(argv[i], "r", "resolution")) {
            if(++i < argc) {
                resolution = atof(argv[i]);
            } else {
                usage();
                exit(1);
            }
        }
        else if(command_argument_match(argv[i], "c", "color")) {
            if(++i < argc) {
                useExactColor = true;
                exactColor = atoi(argv[i]);
            } else {
                usage(); exit(1);
            }
        }
        else if(command_argument_match(argv[i], "i", "input")) {
            printf("input found. i==%d\n", i);
            if(++i < argc) {
                printf("input has following arg. i==%d, argc==%d\n", i, argc);
                printf("argv[%d]==%s\n", i, argv[i]);
                inputfilename = argv[i];
            } else {
                usage(); exit(1);
            }
        }
        else if(command_argument_match(argv[i], "o", "output")) {
            if(++i < argc) {
                outputfilename = argv[i];
            } else {
                usage(); exit(1);
            }
        }
        else if(argv[i][0] != '-')
        {
            inputfilename = argv[i];
        }
    }

    puts("Use --help command-line option for help.");

    // Open files
    if(inputfilename == NULL)
    {
        puts("Input file is stdin.");
        infp = stdin;
    }
    else
    {
        printf("Input file is: %s\n", inputfilename);
        infp = fopen(inputfilename, "r");
        if(!infp)
        {
            printf("Error opening input file: %s\n", inputfilename);
            exit(2);
        }
    }
    if(outputfilename == NULL)
    {
        if(inputfilename == NULL)
        {
            printf("Output file is out.map.\n");
            outputfilename = "out.map";
        }
        else
        {
            std::string s(inputfilename);
            s += ".map";
            // TODO replace .p?m instead of just appending .map
            outputfilename = strdup(s.c_str());
            printf("Output file is: %s\n", outputfilename);
        }
    }

            
        
    // Read bitmap
    puts("Processing input file...");
    pnm_readpaminit(infp, &bitmap, sizeof(bitmap));

    printf("Input bitmap is %d X %d pixels.\n", bitmap.width, bitmap.height);
    double mapwidth = (double)bitmap.width * resolution;
    double mapheight = (double)bitmap.height * resolution;
    printf("Resolution: %f mm/pixel (so map will be %f X %f mm or %f X %f meters).\n", 
      resolution, mapwidth, mapheight, mapwidth/1000.0, mapheight/1000.0);

    tuplerow = pnm_allocpamrow(&bitmap);
    std::vector<ArPose> points;
    for (int row = 0; row < bitmap.height; row++) 
    {
        pnm_readpamrow(&bitmap, tuplerow);
        for (int column = 0; column < bitmap.width; ++column) 
        {
            int color = backgroundColor;
            for (unsigned int plane = 0; plane < bitmap.depth; ++plane) 
            {
                color += tuplerow[column][plane];
            }
            double x = row * resolution;
            double y = column * resolution;
            //printf("color=%d ", color);
            if(useExactColor)
            {
                if(color == exactColor)
                {
                    ArPose p(x, y, 0.0);
                    //printf("[%0.2f,%0.2f] ", p.getX(), p.getY());
                    points.push_back(p);
                }
            }
            else if(color != backgroundColor)
            {
                ArPose p(x, y, 0.0);
                //printf("[%0.2f,%0.2f] ", p.getX(), p.getY());
                points.push_back(p);
            }
        }
        //puts("");
    }
    pnm_freepamrow(tuplerow);
    puts("Done.");

    // Write ArMap
    ArMap armap;
    armap.setPoints(&points);
    if(!armap.writeFile(outputfilename))
    {
        printf("Error writing map to output file: %s\n", outputfilename);
        exit(3);
    }
    printf("Wrote map to %s\n", outputfilename);
}
