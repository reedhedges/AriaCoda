function Aria_make_rtw_hook(hookMethod, modelName, rtwRoot, templateMakefile,buildOpts, buildArgs)
% Aria_MAKE_RTW_HOOK
switch hookMethod
    case 'error'
        % Called if an error occurs anywhere during the build.  If no error occurs
        % during the build, then this hook will not be called.  Valid arguments
        % at this stage are hookMethod and modelName. This enables cleaning up
        % any static or global data used by this hook file.
        disp(['### Build procedure for model: ''' modelName ''' aborted due to an error.']);
    case 'entry'
        % Called at start of code generation process (before anything happens.)
        % Valid arguments at this stage are hookMethod, modelName, and buildArgs.
        disp('### Start Build Procedure for Aria Robot ###')
    case 'before_tlc'
        % Called just prior to invoking TLC Compiler (actual code generation.)
        % Valid arguments at this stage are hookMethod, modelName, and
        % buildArgs
    case 'after_tlc'
        % Called just after to invoking TLC Compiler (actual code generation.)
        % Valid arguments at this stage are hookMethod, modelName, and
        % buildArgs
        
        % This check must be done after the model has been compiled otherwise
        % sample time may not be valid
    case 'before_make'
        % Called after code generation is complete, and just prior to kicking
        % off make process (assuming code generation only is not selected.)  All
        % arguments are valid at this stage.
        
    case 'after_make'
        % Called after make process is complete. All arguments are valid at
        % this stage.
        if strcmp(get_param(modelName,'DL2Robot'),'on') && strcmp(get_param(modelName,'GenCodeOnly'),'off')
            
            Download2Target(modelName);
            
        end
        
    case 'exit'
        % Called at the end of the build process.  All arguments are valid at this
        % stage.
        close_cmd;
end

function Download2Target(modelName)
tlc_dir = fileparts(which('Aria_make_rtw_hook'));
Putty_dir = fullfile(tlc_dir,'Putty');
Path = getenv('path');
setenv('path',[Path ';' Putty_dir]);
ip   = get_param(modelName,'RobotIPAddr');
portnum = get_param(modelName,'RobotPortNum');
user = get_param(modelName,'UserID');
userpw = get_param(modelName,'UserPW');

exeName = [modelName '.exe'];
exeFile = fullfile('..',[modelName '.exe']);
dllFile_01 = fullfile('..','Library','+Includes','ariac_vc10_i386.dll');
dllFile_02 = fullfile('..','Library','+Includes','AriaVC10.dll');
[~,Ping] = system(['ping ' ip]);
disp('### Downloading to Target ###');

if ~isempty(regexp(Ping,'(unreachable)|(Lost = [1-9])|(could not find host)','once'))
    error('Ping to Target Unsucessful Download Aborted')
end

system(['pscp -l ' user ' -pw "' userpw '" "' exeFile '" ' ip ':".\' exeName '"']);
system(['pscp -l ' user ' -pw "' userpw '" "' dllFile_01 '" ' ip ':".\ariac_vc10_i386.dll"']);
system(['pscp -l ' user ' -pw "' userpw '" "' dllFile_02 '" ' ip ':".\AriaVC10.dll"']);
disp('### Download Complete ###');
system(['plink -l ' user ' -pw "' userpw '" ' ip ' "CACLS "' exeName '" /e /p ' user ':F"']);
system(['plink -l ' user ' -pw "' userpw '" ' ip ' "CACLS "ariac_vc10_i386.dll" /e /p ' user ':F"']);
system(['plink -l ' user ' -pw "' userpw '" ' ip ' "CACLS "AriaVC10.dll" /e /p ' user ':F"']);
disp('### Starting Program ###');
system(['plink -l ' user ' -pw "' userpw '" ' ip ' "cmd.exe /C "' exeName ' -w -port ' num2str(portnum) '";exit&']);
disp('### Waiting For Simulink Connection to Start ###');
setenv('path',Path);

function close_cmd()

[response,tasks] = system('tasklist');
cmd = regexp(tasks,'cmd.exe');
if numel(cmd) == 1
    return;
end
trim_task = tasks(cmd(end-1):end);
cmd_pid = regexp(tasks(cmd(end-1):end),'Console','Once');
pid = trim_task(cmd_pid-10:cmd_pid-2);
respone = system(['taskkill /PID ' pid]);