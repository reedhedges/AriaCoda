function Aria_Download_Setup(hDlg, hSrc)
%DOWNLOADSETUP Summary of this function goes here
%   Detailed explanation goes here

if strcmp(slConfigUIGetVal(hDlg, hSrc,'DL2Robot'),'on')
    slConfigUISetVal(hDlg, hSrc,'ExtMode','on')
    slConfigUISetVal(hDlg, hSrc,'ExtModeTransport',0)
    ipadd = ['''' slConfigUIGetVal(hDlg, hSrc,'RobotIPAddr') ''''];
    verbose = ', 0 ,';
    portnum = num2str(slConfigUIGetVal(hDlg, hSrc,'RobotPortNum'));
    slConfigUISetVal(hDlg, hSrc,'ExtModeMexArgs',[ipadd verbose portnum]);
else 
    slConfigUISetVal(hDlg, hSrc,'ExtMode','off')
end
