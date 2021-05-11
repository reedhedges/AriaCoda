/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "AriaCoda", "index.html", [
    [ "AriaCoda reference manual", "index.html", [
      [ "Contents", "index.html#toc", [
        [ "Additional tools in the ARIA toolbox:", "index.html#tocToolbox", null ],
        [ "Helpful information about C++ development with ARIA:", "index.html#tocDevHelp", null ],
        [ "Advanced Usage:", "index.html#tocAdv", null ]
      ] ],
      [ "Introduction", "index.html#intro", [
        [ "What is ARIA? How does it relate to other MobileRobots software?", "index.html#whouses", null ],
        [ "Java and Python", "index.html#javapython", null ],
        [ "Matlab", "index.html#matlab", null ]
      ] ],
      [ "License and Sharing", "index.html#licensing", null ],
      [ "The ARIA Package", "index.html#AriaPackage", [
        [ "ARIA/", "index.html#arpack", null ],
        [ "Other ARIA Files of Note", "index.html#arpackother", null ]
      ] ],
      [ "Documentation and Coding Convention", "index.html#codingConventions", null ],
      [ "ARIA-Robot Client-Server Relationship", "index.html#arCliServ", null ],
      [ "Robot Communication", "index.html#commClasses", [
        [ "Connecting with a Robot or the Simulator", "index.html#connectRobot", null ]
      ] ],
      [ "Specifying Details about Robot and Device Connections", "index.html#HardwareConfiguration", [
        [ "Robot Parameter Files", "index.html#RobotParameterFiles", null ],
        [ "Program Runtime Options", "index.html#RuntimeOptions", null ]
      ] ],
      [ "ArRobot", "index.html#robot", [
        [ "Client Commands and Server Information Packets", "index.html#commands", null ],
        [ "Packet Handlers", "index.html#packetHandlers", null ],
        [ "Command Packets", "index.html#CommandPackets", null ],
        [ "Robot Synchronization Cycle", "index.html#syncRobot", null ],
        [ "State Reflection", "index.html#stateReflection", null ],
        [ "Robot Callbacks", "index.html#callback", null ]
      ] ],
      [ "Controlling the robot with Commands and Actions", "index.html#ClientCommands", [
        [ "Direct Commands", "index.html#robotDirectCommands", null ],
        [ "Motion Command Functions", "index.html#robotMotionCommands", null ],
        [ "Actions", "index.html#actions", null ],
        [ "Action Desired", "index.html#actionDesired", null ],
        [ "The Action Resolver", "index.html#resolvers", null ],
        [ "Predefined Actions", "index.html#predefinedActions", null ],
        [ "Mixing Actions", "index.html#actionInteractions", null ],
        [ "Action Groups", "index.html#actionGroups", null ]
      ] ],
      [ "Range Devices", "index.html#rangeDevices", null ],
      [ "Functors", "index.html#functors", null ],
      [ "Keyboard and Joystick Input", "index.html#userInput", null ],
      [ "Threading", "index.html#threading", [
        [ "Thread Syncronizing Objects", "index.html#syncObject", null ],
        [ "Asynchronous Task Class", "index.html#asynctasks", null ]
      ] ],
      [ "Global Data", "index.html#aria", null ],
      [ "Device and Accessory Interface Classes", "index.html#devices", null ],
      [ "Utility Classes", "index.html#utility", null ],
      [ "ArConfig", "index.html#arconfig", null ],
      [ "Shared Info Groups", "index.html#arinfogroup", null ],
      [ "Maps", "index.html#maps", null ],
      [ "Sockets", "index.html#sockets", null ],
      [ "ArNetworking", "index.html#ArNetworking", null ],
      [ "Sound and Speech", "index.html#sound", null ],
      [ "Emacs", "index.html#emacs", null ],
      [ "Non-everyday use of C++", "index.html#noneverydayC", [
        [ "Standard Template Library", "index.html#stl", null ],
        [ "Default Arguments", "index.html#defaultArgs", null ],
        [ "Constructor Chaining", "index.html#constructorChaining", null ],
        [ "Chars and Strings, Win workaround", "index.html#charsAndStrings", null ],
        [ "AREXPORT", "index.html#arexport", null ],
        [ "Exceptions", "index.html#exceptions", null ]
      ] ],
      [ "Topics and Esoterica", "index.html#Advanced", [
        [ "Piecemeal Use of ARIA", "index.html#pieceMealUse", null ],
        [ "Connecting with a Robot or the Simulator the hard way", "index.html#hardConnectRobot", null ],
        [ "Opening the Connection", "index.html#openDevice", null ],
        [ "Robot Client-Server Connection", "index.html#devConnect", null ],
        [ "Connection Read, Write, Close and Timestamping", "index.html#connrw", null ]
      ] ]
    ] ],
    [ "Robot Parameter Files", "ParamFiles.html", [
      [ "Device Parameters", "ParamFiles.html#ParamFileDeviceParams", [
        [ "General Robot Parameters", "ParamFiles.html#ParamFileGeneralParams", null ],
        [ "Laser Rangefinding Devices", "ParamFiles.html#ParamFileLaserParams", null ],
        [ "GPS Devices", "ParamFiles.html#ParamFileGPSParams", null ],
        [ "Other Accessory Device Flags and Options", "ParamFiles.html#ParamFileOtherDeviceParams", null ]
      ] ]
    ] ],
    [ "Command Line Option Summary", "CommandLineOptions.html", [
      [ "ArRobotConnector", "CommandLineOptions.html#ArRobotConnector", null ],
      [ "ArLaserConnector", "CommandLineOptions.html#ArLaserConnector", null ],
      [ "ArPTZConnector", "CommandLineOptions.html#ArPTZConnector", null ],
      [ "ArGPSConnector", "CommandLineOptions.html#ArGPSConnector", null ],
      [ "ArSonarConnector", "CommandLineOptions.html#ArSonarConnector", null ],
      [ "ArBatteryConnector", "CommandLineOptions.html#ArBatteryConnector", null ],
      [ "ArLCDConnector", "CommandLineOptions.html#ArLCDConnector", null ]
    ] ],
    [ "Laser Scan Log File Format", "LaserLogFileFormat.html", null ],
    [ "Map File Format", "MapFileFormat.html", [
      [ "Defining Custom Map Objects", "MapFileFormat.html#MapCustomObjects", null ]
    ] ],
    [ "Deprecated List", "deprecated.html", null ],
    [ "Todo List", "todo.html", null ],
    [ "Modules", "modules.html", "modules" ],
    [ "Examples", "examples.html", "examples" ]
  ] ]
];

var NAVTREEINDEX =
[
"CommandLineOptions.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';