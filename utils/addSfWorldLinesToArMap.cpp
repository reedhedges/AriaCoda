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

/**
   This program adds the lines from a world file to a map file and
   saves it as the map file.
 **/


int main(int argc, char **argv)
{
  Aria::init();
  char *worldName;
  char *oldMapName;
  char *newMapName;

  if (argc != 3 && argc != 4)
  {
    ArLog::log(ArLog::Normal, "Usage: %s <World> <Map>", argv[0]);
    ArLog::log(ArLog::Normal, "Example: %s columbia.wld columbia.map (this opens columbia.wld and takes the lines from that and adds it to columbia.map", argv[0]);
    ArLog::log(ArLog::Normal, "Usage: %s <oldWorld> <oldMap> <newMap>", argv[0]);
    ArLog::log(ArLog::Normal, "Example: %s columbia.wld columbia.map newColumbia.map (this opens columbia.wld and takes the lines from that and opens columbia.map and and saves the combined data to newColumbia.map)", argv[0]);

    exit(1);
  }

  worldName = argv[1];
  oldMapName = argv[2];
  if (argc == 4)
    newMapName = argv[3];
  else if (argc == 3)
    newMapName = argv[2];
  
  FILE *file;
  if ((file = ArUtil::fopen(worldName, "r")) == NULL)
  {
    ArLog::log(ArLog::Normal, "Could not open world file '%s' to convert", worldName);
    exit(1);
  }
  
  char line[10000];
  
  std::vector<ArLineSegment> lines;

  // read until the end of the file
  while (fgets(line, sizeof(line), file) != NULL)
  {
    ArArgumentBuilder builder;
    builder.add(line);
    if (builder.getArgc() == 4 && builder.isArgInt(0) && 
	builder.isArgInt(1) && builder.isArgInt(2) && 
	builder.isArgInt(3))
    {
      lines.push_back(
	      ArLineSegment(builder.getArgInt(0), builder.getArgInt(1),
			    builder.getArgInt(2), builder.getArgInt(3)));
    }
  }

    
  ArMap armap;
  if (!armap.readFile(oldMapName))
  {
    ArLog::log(ArLog::Normal, "Could not open map file '%s' to convert", worldName);
    exit(1);
  }
  armap.setLines(&lines);
  
  if (!armap.writeFile(newMapName))
  {
    ArLog::log(ArLog::Normal, "Could not save new map file '%s'", newMapName);
    exit(1);
  }

  ArLog::log(ArLog::Normal, "Added lines of '%s' world file and to map '%s' and saved it as '%s'.", worldName, oldMapName, newMapName);
  exit(0);
}



