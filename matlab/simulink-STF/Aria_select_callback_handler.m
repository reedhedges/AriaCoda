function Aria_select_callback_handler(hDlg, hSrc)

% Add Custom Files for Code Generation for Pioneer Robot
tlc_dir = fileparts(which('Aria_make_rtw_hook'));
tmp = slConfigUIGetVal(hDlg, hSrc, 'CustomInclude');
cmp = ['"' tlc_dir '\Include\"'];
cmp = regexprep(cmp,'\\','\\\\');
indx = regexp(tmp,cmp,'once');
if isempty(indx)
slConfigUISetVal(hDlg, hSrc, 'CustomInclude', ...
    [tmp ' "' tlc_dir '\Include\"']);
end
tmp = slConfigUIGetVal(hDlg, hSrc, 'CustomInclude');
cmp = ['"' tlc_dir '\lib\"'];
cmp = regexprep(cmp,'\\','\\\\');
indx = regexp(tmp,cmp,'once');
if isempty(indx)
slConfigUISetVal(hDlg, hSrc, 'CustomInclude', ...
    [tmp ' "' tlc_dir '\lib\"']);
end

tmp = slConfigUIGetVal(hDlg, hSrc, 'CustomLibrary');
indx = regexp(tmp,['ariac_vc10_i386.lib']);
if isempty(indx)
slConfigUISetVal(hDlg, hSrc, 'CustomLibrary', ...
    [tmp ' ariac_vc10_i386.lib']);
end
tmp = slConfigUIGetVal(hDlg, hSrc, 'CustomLibrary');
indx = regexp(tmp,['AriaVC10.lib']);
if isempty(indx)
slConfigUISetVal(hDlg, hSrc, 'CustomLibrary', ...
    [tmp ' AriaVC10.lib']);
end
 
slConfigUISetVal(hDlg, hSrc,'ExtMode','on')
slConfigUISetVal(hDlg, hSrc,'ExtModeTransport',0)
slConfigUISetVal(hDlg, hSrc,'ExtModeMexArgs',slConfigUIGetVal(hDlg, hSrc,'RobotIPAddr'));


% The target is model reference compliant
slConfigUISetVal(hDlg, hSrc, 'ModelReferenceCompliant', 'on');
slConfigUISetEnabled(hDlg, hSrc, 'ModelReferenceCompliant', false);

% Hardware being used is the production hardware
slConfigUISetVal(hDlg, hSrc, 'ProdEqTarget', 'on');

% Set the target language to C and disable modification
slConfigUISetVal(hDlg, hSrc, 'TargetLang', 'C');
slConfigUISetEnabled(hDlg, hSrc, 'TargetLang', 0);

% Set the TargetLibSuffix
slConfigUISetVal(hDlg, hSrc, 'TargetLibSuffix', '.lib');

% For real-time builds, we must generate ert_main.c
slConfigUISetVal(hDlg, hSrc, 'ERTCustomFileTemplate', 'Aria_file_process.tlc');


