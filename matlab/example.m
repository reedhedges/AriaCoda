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

arrobot_disconnect
clear all

% initialize with default arguments: (connect to robot connected to this
% computer via COM1 serial port)
aria_init 

% initialize aria to connect to a remote computer (e.g. a robot with a wifi
% interface instead of onboard computer, or a simulator running on another
% computer):
% aria_init -rh 10.0.125.32


% connect to the robot:
arrobot_connect

% make the robot drive in a small circle:
arrobot_setvel(300)
arrobot_setrotvel(35)

disp 'use arrobot_stop to stop'
