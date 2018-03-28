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


/* MEX function for Matlab */

#include "mex.h"
#include "ariac.h"


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  arpose p;
  if(nrhs == 1)
  {
    p.x = mxGetPr(prhs[0])[0];
    p.y = mxGetPr(prhs[0])[1];
    p.th = mxGetPr(prhs[0])[2];
  }
  else if(nrhs == 2)
  {
    p.x = *mxGetPr(prhs[0]);
    p.y = 0.0;
    p.th = *mxGetPr(prhs[1]);
  } 
  else if(nrhs == 3)
  {
    p.x = *mxGetPr(prhs[0]);
    p.y = *mxGetPr(prhs[1]);
    p.th = *mxGetPr(prhs[2]);
  }
	else
  {
		mexErrMsgIdAndTxt("arrobot_setvels", "One vector input {x, y, th}, or two inputs (x, th), or three inputs (x, y, th) required.");
		return;
	}
  arrobot_setvels(p);
}
	
