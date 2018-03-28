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

void updateMaxMin(const ArPose& p, double &xmax, double &xmin, double &ymax, double &ymin)
{
  if(p.getX() > xmax) xmax = p.getX();
  if(p.getY() > ymax) ymax = p.getY();
  if(p.getX() < xmin) xmin = p.getX();
  if(p.getY() < ymin) ymin = p.getY();
}

int main(int argc, char **argv)
{
  Aria::init();

  bool autocenter = false;
  ArPose offset;

  if(argc == 1 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help") || argc == 3 || argc > 4)
  {
    ArLog::log(ArLog::Normal, "Usage: %s <mapfile> [<X offset> <Y offset>]", argv[0]);
    ArLog::log(ArLog::Normal, "Example: %s example.map -2031 +29\n\t(This recenters the map by shifting it in -X (left) by 2031 mm then in +Y (up) by 29 mm)", argv[0]);
    ArLog::log(ArLog::Normal, "Example: %s example.map\n\t(This recenters the map automatically according to the max/min obstacle positions)", argv[0]);
    Aria::exit(1);
  }

  const char *mapfile = argv[1];

  if(argc == 4) 
  {
    offset.setX(atol(argv[2]));
    offset.setY(atol(argv[3]));
  }
  else
  {
    autocenter = true;
  }
    
  ArMap armap;
  if (!armap.readFile(mapfile))
  {
    ArLog::log(ArLog::Normal, "Error: Could not open map file '%s' to convert", mapfile);
    Aria::exit(2);
  }

  std::vector<ArPose> *points = armap.getPoints();
  std::vector<ArLineSegment> *lines = armap.getLines();
  std::list<ArMapObject*> *objs = armap.getMapObjects();

  if(autocenter)
  {
    double xmax, ymax, xmin, ymin;
    if(points->begin() != points->end())
    {
      xmax = points->begin()->getX();
      ymax = points->begin()->getY();
      xmin = points->begin()->getX();
      ymin = points->begin()->getY();
    }
    else if(lines->begin() != lines->end())
    {
      xmax = lines->begin()->getEndPoint1().getX();
      ymax = lines->begin()->getEndPoint1().getY();
      xmin = lines->begin()->getEndPoint1().getX();
      ymin = lines->begin()->getEndPoint1().getY();
    }
    else if(objs->begin() != objs->end())
    {
      xmax = (*(objs->begin()))->getPose().getX();
      ymax = (*(objs->begin()))->getPose().getY();
      xmin = (*(objs->begin()))->getPose().getX();
      ymin = (*(objs->begin()))->getPose().getY();
    }
    else
      xmax = ymax = xmin = ymin = 0.0;

    for(std::vector<ArPose>::iterator i = points->begin(); i != points->end(); ++i)
    {
      updateMaxMin(*i, xmax, xmin, ymax, ymin);
    }
    for(std::vector<ArLineSegment>::iterator i = lines->begin(); i != lines->end(); ++i)
    {
      updateMaxMin(i->getEndPoint1(), xmax, xmin, ymax, ymin);
      updateMaxMin(i->getEndPoint2(), xmax, xmin, ymax, ymin);
    }
    for(std::list<ArMapObject*>::iterator i = objs->begin(); i != objs->end(); ++i)
    {
      if((*i)->hasFromTo())
      {
        updateMaxMin((*i)->getFromPose(), xmax, xmin, ymax, ymin);
        updateMaxMin((*i)->getToPose(), xmax, xmin, ymax, ymin);
      }
      else
      {
        updateMaxMin((*i)->getPose(), xmax, xmin, ymax, ymin);
      }
    }

  /*
    const double xmax = max_element(points->begin(), points->end(), &ArPose::compareX)->getX();
    const double ymax = max_element(points->begin(), points->end(), &ArPose::compareY)->getY();
    const double xmin = min_element(points->begin(), points->end(), &ArPose::compareX)->getX();
    const double ymin = min_element(points->begin(), points->end(), &ArPose::compareY)->getY();
  */

    const double xsize = ArMath::fabs(xmax - xmin);
    const double ysize = ArMath::fabs(ymax - ymin);
    offset.setX(-1 * (xmin + xsize/2.0));
    offset.setY(-1 * (ymin + ysize/2.0));
    ArLog::log(ArLog::Normal, "Automatically recentering map. (Max=(%f,%f), Min=(%f,%f), Width=%f, Height=%f, Offset by %f, %f)", xmax, ymax, xmin, ymin, xsize, ysize, offset.getX(), offset.getY());
  }

  ArLog::log(ArLog::Normal, "Offset all data points by (%f, %f)...", offset.getX(), offset.getY());
  for(std::vector<ArPose>::iterator i = points->begin(); i != points->end(); ++i)
  {
    *i += offset;
  }

  ArLog::log(ArLog::Normal, "Offset all line positions by (%f, %f)...", offset.getX(), offset.getY());
  for(std::vector<ArLineSegment>::iterator i = lines->begin(); i != lines->end(); ++i)
  {
    ArPose new1 = i->getEndPoint1() + offset;
    ArPose new2 = i->getEndPoint2() + offset;
    i->newEndPoints(new1, new2);
  }
  
  ArLog::log(ArLog::Normal, "Offset all map object positions by (%f, %f)...", offset.getX(), offset.getY());
  for(std::list<ArMapObject*>::iterator i = objs->begin(); i != objs->end(); ++i)
  {
    ArMapObject *obj = (*i);
    ArPose oldp = obj->getPose();
    ArLog::log(ArLog::Normal, "Moving '%s' from (%f, %f) to (%f, %f)...", obj->getName(), oldp.getX(), oldp.getY(), (oldp+offset).getX(), (oldp+offset).getY());
    obj->setPose( obj->getPose() + offset );
    if(obj->hasFromTo())
    {
      ArLog::log(ArLog::Normal, "...Moving line ends or box corners too...");
      obj->setFromTo( obj->getFromPose() + offset, obj->getToPose() + offset );
    }
/*
    if(obj->getType() == "ForbiddenLine")
    {
      ArLineSegment line = obj->getFromToSegment();
      line.getEndPoint1() += offset;
      line.getEndPoint2() += offset;
      obj->setFromTo( obj->getFromPose() + offset, obj->getToPose() + offset );
*/
      
        
  }
  
  
  ArLog::log(ArLog::Normal, "Saving file...");
  if (!armap.writeFile(mapfile))
  {
    ArLog::log(ArLog::Normal, "Error: Could not save map file '%s'", mapfile);
    Aria::exit(3);
  }
  
  
  if (!armap.writeFile(mapfile))
  {
    ArLog::log(ArLog::Normal, "Error: Could not save map file '%s'", mapfile);
    Aria::exit(3);
  }

  ArLog::log(ArLog::Normal, "Saved map %s.", mapfile);

  Aria::exit(0);
}



