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

#include "Aria.h"

/* This program puts the LINES from an ArMap into a very minimal DXF file.  

   Usage: 
      ./convertArMapToDXF [-h|--help] [mapfile] [outputfile]
   [mapfile] and [outputfile] are optional. If omitted, then stdin and stdout 
   are used instead.
   

   TODO:  

    * Include Points from the ArMap on a separate layer
    * Include Goals and other map objects on separate layers with labels and
      appropriate metadata
    * Set useful properties in the DXF headers and layer sections.

      -> If you would like to contribute any improvements, send them to the
         ARIA users forum or support@mobilerobots.com. <-

  Output was tested in Autodesk DWG TrueView 2013.
*/

int main(int argc, char **argv)
{
  Aria::init();
  ArLog::init(ArLog::StdErr, ArLog::Normal, "", false, false, false);

  if((argc > 1 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) || !strcmp(argv[1], "-help")) || argc > 3)
  {
    ArLog::log(ArLog::Normal, "Usage:\n\t%s [mapfile] [outputfile]", argv[0]);
    ArLog::log(ArLog::Normal, "[mapfile] and [outputfile] are optional. If omitted, then stdin and stdout are used instead.");
    Aria::exit(1);
  }

  const char *mapfile = "/dev/stdin";
  if(argc > 1)
    mapfile = argv[1];

  const char *outfile = NULL;
  if(argc > 2)
    outfile = argv[2];

  ArMap armap;
  if (!armap.readFile(mapfile))
  {
    ArLog::log(ArLog::Terse, "Error: Could not open map file '%s' to convert", mapfile);
    Aria::exit(2);
  }

  //std::vector<ArPose> *points = armap.getPoints();
  std::vector<ArLineSegment> *lines = armap.getLines();
  //std::list<ArMapObject*> *objs = armap.getMapObjects();

  FILE *outfp = NULL;
  if(outfile)
    outfp = fopen(outfile, "w");
  else
    outfp = stdout;
  if(!outfp)
  {
    ArLog::logErrorFromOS(ArLog::Terse, "Error: Could not open output file (%s)", outfile?outfile:"stdout");
    Aria::exit(3);
  }

  time_t now = time(NULL);
  fprintf(outfp, "999\nConverted from %s at %s", mapfile, ctime(&now));
  fprintf(outfp, "999\n\tMap Lines Data:\n0\nSECTION\n2\nENTITIES\n");
  size_t n = 0;
  for(std::vector<ArLineSegment>::const_iterator i = lines->begin(); i != lines->end(); ++i)
  {
    // 8 is handleID
    // 10 is startx, 20 is starty, 11 is endx, 21 is endy
    fprintf(outfp, "0\nLINE\n8\n0\n10\n%f\n20\n%f\n11\n%f\n21\n%f\n",
      i->getX1(), i->getY1(),
      i->getX2(), i->getY2());
	++n;
  }
  fprintf(outfp, "0\nENDSEC\n");

  // TODO points
  // POINT entity has:
  // 10=x, 20=y, 39=thickness

  // TODO goals and other objects as labelled points or boxes on other layers
  // for goals with heading and other points with headings, include arrow
  // pointing in direction of heading

  fprintf(outfp, "0\nEOF\n");
  fclose(outfp);
  ArLog::log(ArLog::Normal, "\nconvertArMapToDXF: wrote %lu lines to %s.\nDone.\n", n, outfile);
  Aria::exit(0);
}



