% Adept MobileRobots Robotics Interface for Applications (ARIA)
% Copyright (C) 2004-2005 ActivMedia Robotics LLC
% Copyright (C) 2006-2010 MobileRobots Inc.
% Copyright (C) 2011-2015 Adept Technology, Inc.
% Copyright (C) 2016-2018 Omron Adept Technologies, Inc.
% 
%      This program is free software; you can redistribute it and/or modify
%      it under the terms of the GNU General Public License as published by
%      the Free Software Foundation; either version 2 of the License, or
%      (at your option) any later version.
% 
%      This program is distributed in the hope that it will be useful,
%      but WITHOUT ANY WARRANTY; without even the implied warranty of
%      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%      GNU General Public License for more details.
% 
%      You should have received a copy of the GNU General Public License
%      along with this program; if not, write to the Free Software
%      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
% 
% If you wish to redistribute ARIA under different terms, contact 
% Adept MobileRobots for information about a commercial version of ARIA at 
% robots@mobilerobots.com or 
% Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
% Adept MobileRobots Robotics Interface for Applications (ARIA)
% Copyright (C) 2004-2005 ActivMedia Robotics LLC
% Copyright (C) 2006-2010 MobileRobots Inc.
% Copyright (C) 2011-2015 Adept Technology, Inc.
% Copyright (C) 2016 Omron Adept Technologies, Inc.
% 
%      This program is free software; you can redistribute it and/or modify
%      it under the terms of the GNU General Public License as published by
%      the Free Software Foundation; either version 2 of the License, or
%      (at your option) any later version.
% 
%      This program is distributed in the hope that it will be useful,
%      but WITHOUT ANY WARRANTY; without even the implied warranty of
%      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%      GNU General Public License for more details.
% 
%      You should have received a copy of the GNU General Public License
%      along with this program; if not, write to the Free Software
%      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
% 
% If you wish to redistribute ARIA under different terms, contact 
% Adept MobileRobots for information about a commercial version of ARIA at 
% robots@mobilerobots.com or 
% Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960

funcs = {
  'aria_init',  
  'aria_shutdown',
  'arrobot_command',
  'arrobot_connect', 
  'arrobot_disconnect', 
  'arloginfo', 
  'arrobot_setvel', 
  'arrobot_setrotvel', 
  'arrobot_getx', 
  'arrobot_gety', 
  'arrobot_getth', 
  'arrobot_stop',
  'arrobot_length',
  'arrobot_width',
  'arrobot_radius',
  'arrobot_getsonarrange',
  'arrobot_getnumsonar',
  'arrobot_getdigin',
  'arrobot_setlatvel',
  'arrobot_getbatteryvoltage',
  'arrobot_getvel',
  'arrobot_getrotvel',
  'arrobot_getlatvel',
  'arrobot_setdeltaheading',
  'arrobot_resetpos',
  'arrobot_isstalled',
  'arrobot_isleftstalled',
  'arrobot_isrightstalled',
  'arrobot_setwheelvels',
  'arrobot_setlatvel',
  'arrobot_setvels',
  'arrobot_getpose',
  'arrobot_setpose',
  'arrobot_move',
  'arrobot_motorsenabled',
  'arrobot_enable_motors',
  'arrobot_disable_motors'
}

% Unload old mex functions etc.  Note, if you add a new mex function above that
% does not match these patterns, add it to this list or a pattern that
% matches it.
clear aria_* arloginfo arrobot_*

 
% Set filenames and options depending on current platform
compilername = mex.getCompilerConfigurations('C', 'Selected').Name
msg = join(["The C compiler currentler selected for mex is: ", compilername, ". Use mex -setup to change"]);
disp (msg);
compilerversion = strsplit(mex.getCompilerConfigurations("C", "Selected").Version, '.');
def = '';
disp compilerversion
switch computer
  case 'PCWIN'
    vcver = compilerversion{1}
    ariainstdir = 'C:/Program Files/MobileRobots/Aria'
    ariaclink = sprintf("-L. -lariac_vc%s_i386", vcver)
    arialink = sprintf('-L../lib -L../bin -lAriaVC%s', vcver)
    def = '-DWIN32 -win32'
    ariadll = sprintf('../bin/AriaVC%s.dll', vcver)
    username = getenv('USERNAME')
    hostname = getenv('COMPUTERNAME')
    msg = sprintf("You are on Windows 32 bit. Will attempt to use AriaVC%s.dll and ariac_vc%s_i386.lib, built in Release configuration for win32 platform with %s.", vcver, vcver, compilername);
    disp (msg)
  case 'PCWIN64'
    vcver = compilerversion{1}
    ariainstdir = 'C:/Program Files/MobileRobots/Aria'
    ariaclink = sprintf("-L. -lariac_vc%s_x64", vcver)
    def = '-DWIN32'
    arialink = sprintf("-L../lib64 -L../bin64 -lAriaVC%s", vcver)
    ariadll = sprintf('../bin64/AriaVC%s.dll', vcver)
    username = getenv('USERNAME')
    hostname = getenv('COMPUTERNAME')
    msg = sprintf("You are on Windows 64 bit. Will attempt to use AriaVC%s.dll and ariac_vc%s_x64.lib, built in Release configuration for x64 platform with %s.", vcver, vcver, compilername);
    disp (msg)
  case 'GLNX86'
    ariainstdir = '/usr/local/Aria'
    ariaclink = '-L. -lariac'
    arialink = '-L../lib -lAria'
    ariadll = '../lib/libAria.so'
    username = getenv('USER')
    hostname = getenv('HOSTNAME')
    disp 'You are on Linux 32 bit. Will attempt to use libAria.so and libariac.so.';
  case 'GLNXA64'
    ariainstdir = '/usr/local/Aria'
    ariaclink = '-L. -lariac'
    arialink = '-L../lib -lAria'
    ariadll = '../lib/libAria.so'
    username = getenv('USER')
    hostname = getenv('HOSTNAME')
    disp 'You are on Linux 64 bit. Will attempt to use libAria.so and libariac.so.';
  case 'MACI64'
    disp 'Sorry, not set up for Mac yet. You will need to get ARIA and ariac built on Mac, then edit makemex.m to set the appropriate mex compilation flags under the MACI64 computer type case instead of displaying this message..';
    return
  otherwise
    disp 'error, unrecognized system type. (what kind of computer and OS are you on?)';
    computer
  return
end

% Put a copy of the ARIA runtime library into the current directory
msg =  sprintf("Copying ARIA DLL %s into current directory so Matlab can easily find it...", ariadll);
disp(msg);
disp 'If this fails, make sure that the DLL file was built and that you have permission to write files to this directory.'
eval(['copyfile ' ariadll ' .'])

% Compile all the mex functions listed above:
disp 'Compiling mex functions...'
for i = 1:length(funcs);
  disp(funcs{i});
  cmd = sprintf('mex -g %s -DMATLAB -I. %s %s mex-src/%s.c', def, ariaclink, arialink, funcs{i});
  cmd;
  eval(cmd);
end;

disp 'Writing build info to file buildinfo.txt'
fp = fopen('buildinfo.txt', 'w');
fprintf(fp, 'ARIA Matlab Interface built %s by %s@%s\nMatlab %s %s\nUsed ARIA DLL %s with link flags %s %s\n', date, username, hostname, version, computer, ariadll, ariaclink, arialink);
fprintf(fp, 'Compiler was %s', compilername)
fclose(fp);

clear cmd def funcs i hostname username fp vcver compilername compilerversion msg
clear ariadll ariaclink arialink ariainstdir

disp 'done'
