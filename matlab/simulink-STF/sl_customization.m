function sl_customization(cm)
% SL_CUSTOMIZATION for Arduino PIL connectivity config

% Copyright 2008-2010 The MathWorks, Inc.

disp('Adding tcpip external mode to test target');
cm.ExtModeTransports.add('Aria.tlc', 'tcpip','ext_comm', 'Level1');

    
