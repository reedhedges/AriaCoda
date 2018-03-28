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
	if(nrhs == 1) {
    double *arr = mxGetPr(prhs[0]);
    p.x = arr[0];
    p.y = arr[1];
    p.th = arr[2];
  } else if(nrhs == 3) {
    p.x = *mxGetPr(prhs[0]);
    p.y = *mxGetPr(prhs[1]);
    p.th = *mxGetPr(prhs[2]);
  } else {
		mexErrMsgIdAndTxt("arrobot_movepose", "One 3-element array (vector) input {x, y, th}, or three inputs (x, y, th), of floating point numbers (doubles) required.");
    return;
	}
  arrobot_movepose(p);
}
	
