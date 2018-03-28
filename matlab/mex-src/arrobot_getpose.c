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
	if ( nrhs != 0 ) 
    {
        mexErrMsgIdAndTxt("arrobot_getpose","This function takes no input arguments.");
        return;
    }
    
    if (nlhs < 1 || nlhs > 3)
    {
        mexErrMsgIdAndTxt("arrobot_getpose", "Output must be assigned to one matrix, two values [x, y] or three values [x, y, th]");
        return;
    }
      
    if(nlhs == 1)
    {
        double *op;
        arpose rp;
        plhs[0] = mxCreateDoubleMatrix(1, 3, mxREAL);
        op = mxGetPr(plhs[0]);
        rp = arrobot_getpose();
        op[0] = rp.x;
        op[1] = rp.y;
        op[2] = rp.th;
        return;
    }
    
    if(nlhs >= 2)
    {
         arpose rp = arrobot_getpose();
         plhs[0] = mxCreateDoubleScalar(rp.x);
         plhs[1] = mxCreateDoubleScalar(rp.y);
         if(nlhs > 2)
             plhs[2] = mxCreateDoubleScalar(rp.th);
    }
}
	
