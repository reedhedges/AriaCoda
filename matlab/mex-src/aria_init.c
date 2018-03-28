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

#include <string.h>
#include "mex.h"
#define MATLAB 1
#include "ariac.h"


void mexLog(const char *s)
{
    mexPrintf("aria: %s\n", s);
}


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{

   int pi;
   int ai;
   char **arg;
   char **argv;
   const char *scriptname;
   size_t n;
   size_t snlen;
   char buf[256];
   
   aria_setloghandler(&mexLog);
   
   argv = (char**)malloc((1+nrhs) * sizeof(char*));
   scriptname = mexFunctionName();
   snlen = strlen(scriptname);
   argv[0] = (char*)malloc(snlen+1);
   strncpy(argv[0], scriptname, snlen);
   mexPrintf("nrhs=%d, argv[0]=%s\n", nrhs, argv[0]);
   for(ai = 1, pi = 0; pi < nrhs; ++pi, ++ai)
   {
       memset(buf, 0, 256);
       if(mxGetString(prhs[pi], buf, 255) != 0)
       {
           mexPrintf("aria_init: error copying argument %d from matlab. not a string or too long. aborting without initializing aria.");
           return;
       }
       mexPrintf("prhs[%d]=%s\n", pi, buf);
       n = strlen(buf);
       argv[ai] = (char*)malloc(n+1);
       strncpy(argv[ai], buf, n+1);
       mexPrintf("argv[%d]=%s\n", ai, argv[ai]);
   }
   
   aria_init_with_args(nrhs+1, argv);
}
	
