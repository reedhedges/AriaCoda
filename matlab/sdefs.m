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

% S-function definitions for selected ArRobot methods in ARIA (via ariac functions)
% for use in Simulink



% Run this in Matlab (select it in the file browser, right click, and choose Run, 
% or run "sdefs" in the command window if the Current Folder is Aria/matlab, 
% or click "Run" in the toolbar if you have this file open in the Matlab file editor.)
% to generate and compile mex interfaces and s-function definitions for 
% ArRobot accessor functions.  
%
% The "S-Functions" generated are for use as blocks in Simulink.
%
% The first time any of these accessor S-functions runs,
% it will load the ARIA library and try connecting to the robot if a
% connection has not yet been made.  Or you can explicitly connect to
% the robot with arrobot_connect(). (See arrobot_connect() in ariac.h or
% documentation in ariac docs).
%
% When compiling, it assumes you are using Visual C++ 2010 as your
% compiler. You must have this compiler installed, and have selected
% it by running the following command in Matlab:
%    mex -setup

% TODO check for system architecture (32-bit i386 or 64-bit amd64) and OS
% (Windows or Linux). For now you will need to change values below to swich
% between 64-bit and 32-bit architecture, and between Linux and Windows
% (you may also need to modify code in ariac.cpp and/or ariac.h too for
% Linux)


% Table of S-function name, ariac function, and description to display with
% block
funcs = {
    {'arrobot_getx', 'double y1 = arrobot_getx()', 'X position of mobile robot (mm)'}
    {'arrobot_gety', 'double y1 = arrobot_gety()', 'Y position of mobile robot (mm)'}
    {'arrobot_getth', 'double y1 = arrobot_getth()', 'Orientation (theta) of mobile robot (degrees)'}
    {'arrobot_setvel', 'void arrobot_setvel(double u1)', 'Send command to set forward/back velocity of mobile robot (mm/sec)'}
    {'arrobot_setrotvel', 'void arrobot_setrotvel(double u1)', 'Send command to set rotational velocity of mobile robot (deg/sec, counterclockwise +)'}
    {'arrobot_setlatvel', 'void arrobot_setlatvel(double u1)', 'Send command to set lateral velocity of mobile robot (Seekur only) (mm/sec, rightwards +)'}
    {'arrobot_setwheelvels', 'void arrobot_setwheelvels(double u1, double u2)', 'Send command to set left, right wheel velocities separately (mm/sec), overrides vel, rotvel (Note: not implemented in Seekur or LX)'}
    {'arrobot_getvel', 'double y1 = arrobot_getvel()', 'Get current forward/back velocity of mobile robot (mm/sec)'}
    {'arrobot_getrotvel', 'double y1 = arrobot_getrotvel()', 'Get current rotational velocity of mobile robot (deg/sec, counterclockwise +)'}
    {'arrobot_stop', 'void arrobot_stop()', 'Send command to stop robot movement'}
    {'arrobot_setdeltaheading', 'void arrobot_setdeltaheading(double u1)', 'Request a heading offset to turn to'}
    {'arrobot_getlatvel', 'double y1 = arrobot_getlatvel()', 'Current lateral velocity of mobile robot (Seekur only) (mm/sec)'}
    {'arrobot_move', 'void arrobot_move(double u1)', 'Request a translation distance (forward/backward) to drive robot to.'}
    {'arrobot_resetpos', 'void arrobot_resetpos()', 'Reset mobile robot odometric pose estimate to 0,0,0 (does not move robot in simulation)'}
    {'arrobot_setpose', 'void arrobot_setpose(double u1, double u2, double u3)', 'Change stored robot pose estimate'}
    {'arrobot_getbatteryvoltage', 'double y1 = arrobot_getbatteryvoltage()', 'Mobile robot battery voltage'}
    {'arrobot_length', 'double y1 = arrobot_length()', 'Length of mobile robot (mm)'}
    {'arrobot_width', 'double y1 = arrobot_width()', 'Width of mobile robot (mm)'}
    {'arrobot_radius', 'double y1 = arrobot_radius()', 'Radius (maximum) of mobile robot (mm)'}
    {'arrobot_getnumsonar', 'int32 y1 = arrobot_getnumsonar()', 'Number of sonar sensors on mobile robot'}
    {'arrobot_getsonarrange', 'double y1 = arrobot_getsonarrange(int32 p1)', 'Get range reading from one sonar sensor (0-indexed) of mobile robot (mm)'}
    {'arrobot_getsonar', 'void arrobot_getsonar(double y1[16])', 'Get range readings of all front sonar on mobile robot (mm)'}
    {'arrobot_getdigin', 'int8 y1 = arrobot_getdigin()', 'Get current state of digital inputs (Pioneer only)'}
    {'arrobot_setdigout', 'void arrobot_setdigout(int8 u1)', 'Set state of digital outputs (Pioneer only)'}
    {'arrobot_isstalled', 'int32 y1 = arrobot_isstalled()', 'Are either motors stalled?'}
    {'arrobot_isleftstalled', 'int32 y1 = arrobot_isleftstalled()', 'Is left motor stalled?'}
    {'arrobot_isrightstalled', 'int32 y1 = arrobot_isrightstalled()', 'Is right motor stalled?'}
    {'arrobot_getnumfrontbumpers', 'int32 y1 = arrobot_num_front_bumpers()',   'Number of front bumpers on mobile robot'}
	{'arrobot_getnumrearbumpers', 'int32 y1 = arrobot_num_rear_bumpers()', 'Number of rear bumpers on mobile robot'}
	{'arrobot_getfrontbumper', 'int32 y1 = arrobot_get_front_bumper(int32 p1)', 'Get range reading from one front bumper (0-indexed) of mobile robot'}
	{'arrobot_getrearbumper', 'int32 y1 = arrobot_get_rear_bumper(int32 p1)', 'Get range reading from one front bumper (0-indexed) of mobile robot'}
    {'arrobot_motors_enabled', 'int32 y1 = arrobot_motors_enabled()', 'Are motors enabled?'}
	{'arrobot_enable_motors', 'void arrobot_enable_motors()', 'Send command to enable or re-enable motors'}
	{'arrobot_disable_motors', 'void arrobot_disable_motors()', 'Send command to disable motors'}

    };


clear arrobot_* aria_* arloginfo

cn = mex.getCompilerConfigurations("C", "Selected").Name;
ver = strsplit(mex.getCompilerConfigurations("C", "Selected").Version, '.')
cv = ver{1};

disp 'Selected compiler is:'
disp (cn)

switch computer
    case 'PCWIN'
        msg = sprintf('Using ARIA and ariac libraries compiled with Visual C++ %s', cv);
	    disp (msg)
        ariadll = sprintf('../bin/AriaVC%s.dll', cv);
        arialib = sprintf('../lib/AriaVC%s.lib', cv);
        ariaclib = sprintf('ariac_vc%s_i386.lib', cv);
        compileopts = {'-DWIN32', '-DMATLAB'};
    case 'PCWIN64'
        msg = sprintf('Using ARIA and ariac libraries compiled with Visual C++ %s', cv);
        disp (msg)
        ariadll = sprintf('../bin64/AriaVC%s.dll', cv);
        arialib = sprintf('../lib64/AriaVC%s.lib', cv);
        ariaclib = sprintf('ariac_vc%s_x64.lib', cv);
        compileopts = {'-DWIN32', '-DMATLAB'};
    case 'GLNX86'
        ariadll = '../lib/libAria.so';
        arialib = '../lib/libAria.so';
        ariaclib = 'libariac.a';
        compileopts = {'-DMATLAB'};
    case 'GLNXA64'
        ariadll = '../lib/libAria.so';
        arialib = '../lib/libAria.so';
        ariaclib = 'libariac_x64.a';
        compileopts = {'-DMATLAB'};
end

copyfile(ariadll, '.')


path(path, pwd) ;
path(path, sprintf('%s/ariac-simulink-library', pwd)) % to be able to find ariac.slx in there
load_system('ariac-simulink-library/ariac')


set_param('ariac', 'lock', 'off')

startxpos = 5;
startypos = 5;
xpos = startxpos;
ypos = startypos;
xposinc = 300;
yposinc = 100;
xsize = 150;
ysize = 70;
for i = 1:length(funcs)
    name = funcs{i}{1};
    spec = funcs{i}{2};
    desc = funcs{i}{3};
    disp(sprintf('Defining %s...', name));
    def = legacy_code('initialize');
    def.SFunctionName = sprintf('%s_sfn', name);
    def.HeaderFiles = {'ariac.h'};
    def.IncPaths = {'../include', '.'};
    def.HostLibFiles = {ariaclib, arialib};
    def.TargetLibFiles = {ariaclib, arialib};
    def.Options.language = 'C';
    def.OutputFcnSpec = spec;
    disp(sprintf('Generating and compiling %s and generating block...', name));
    legacy_code('sfcn_cmex_generate', def)
    legacy_code('rtwmakecfg_generate',def)
    legacy_code('sfcn_tlc_generate',def)
    legacy_code('compile', def, compileopts)
    legacy_code('slblock_generate', def, 'ariac')
	legacy_code('rtwmakecfg_generate',def)
	legacy_code('sfcn_tlc_generate',def)
    set_param(sprintf('ariac/%s_sfn', name), 'Description', desc)
    set_param(sprintf('ariac/%s_sfn', name), 'AttributesFormatSTring', desc)
    
    % layout of blocks in two columns so you can see them
    xpos = xpos + xposinc;
    ypos = ypos + yposinc;
    if xpos >= xposinc + startxpos
        xpos = startxpos;
    end
    set_param(sprintf('ariac/%s_sfn', name), 'position', [xpos ypos xpos+xsize ypos+ysize]);
end

save_system('ariac');
close_system('ariac');





disp 'Done with all arrobot functions.'

% older versions: 
% browser = LibraryBrowser.StandaloneBrowser
% browser.refreshLibraryBrowser
% newer versions:
%browser = LibraryBroser.LibraryBrowser2
%browser.refresh


disp 'You can permanently add the ariac simulink-library to your MATLAB path by right clicking on it in the file browser, choosing Add to Path -> Selected Folders. Or copy it to your MATLAB directory.'
disp 'Then you can create a new model, and access the ariac library in the new model using the library.'
disp 'Or just open ariac-simulink-library/ariac.slx and copy the blocks to another open model.'
clear ariaclib ariadll arialib compileopts def desc funcs i name spec browser startxpos startypos xpos xposinc xsize ysize yposinc cv cn msg ver

