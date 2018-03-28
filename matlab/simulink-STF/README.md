

Creating the Custom Simulink System Target File
===============================================

If you have the optional Simulink Coder component, you can compile a Simulink system 
and deploy it to  the robot's onboard computer via SSH, while viewing and starting/stopping
system from the Simulink GUI on a remote laptop or other computer over wifi.  

On Windows, Putty is used to transfer files and communications over SSH.

Currently, only Windows is supported on onboard computer as well as user laptop or other remote computer.

TLC Files: 	Aria.tlc,
			Aria_srmain.tlc
			Aria_file_process.tlc
MATLAB Files: 	sl_customization.m
			Aria_Download_Setup.m
Aria_select_callback_handler.m
			Aria_make_rtw_hook.m
Putty Command:  PLINK.exe
			PSCP.exe

Aria.tlc:
---------

The System Target File (STF) sets the protocol for code generation. In order to create a custom target it is required to have a license to Embedded Coder. 

The file Aria.tlc is based off of the STF `ert.tlc`. The first two lines of the file are Header delariations this gives Simulink information about the STF's description, and what template makefile, mkake command, and external mex wrapper to use for this target. 

The next few lines deals with assigning token's to be used by the TLC's such as what language the code should generate and its format. The last %include statement gives the codegenentry.tlc for the STF. This does most of the code generation. 

This first segment of the STF was taken from `ert.tlc` with little modification to the code. 

The comments below represent the RTW options for the STF. These options are contained between the keywords `BEGIN_RTW_OPTIONS` and `END_RTW_OPTIONS`. The first segment of these options creates a structure that defines the custom STF pane for the Aria.tlc entitled 'Aria Target Options'. 

It is here that user inputs specific to the target can be supplied to the build process within the `tlcvariable`. The `makevariable` can also be used to define a tokens for the template makefile. 

The final segment below the 'Configure code generation settings' header specify auxiliary items regarding the STF like build directory suffix, what this STF 'Aria' is derived from, and the function that gets executed when the STF is selected in the configuration parameter's  code generation pane. 

Aria_srmain.tlc: 
----------------

This custom tlc creates the main file in order to execute the Simulink model properly on the target. Conditional statements were included such that a user can have the option of adding in external mode and data logging or take it out of the program. 

If both external mode and data logging are not selected then the default example main file is generated. 

The main file also adds the `timer.h` header file. This allows for the timer object for the robot. The timer code slows down the simulation to real time speeds. The main part of the timer code was inspired from the MatlabCentral file Simulink Real-Time Execution, by Guy Rouleau. The file can be found via the following link:

http://www.mathworks.com/matlabcentral/fileexchange/21908-simulink%C2%AE-real-time-execution

The timing code is done within the `rt_onestep` function. Also, the macro `CLOCKS_PER_SEC` is assigned to a variable and is not reused throughout. This is because this macro was causing the simulation to slow too much and result in poor performance of the model on the robot. 

The main file also does the initialization and clean up for the robot object. This is done in the main file so the function `aria_init` can access the command line inputs before they are parsed for external mode.

Aria_file_process.tlc:
----------------------

This TLC dictates how the generated code gets assembled. This file for the Aria target selects the `Aria_srmain.tlc` file as the generated executables main file. 

Currently the main `Aria_srmain` file is used for all configurations. However the if condition can be used to specify different main files depending on what configuration parameters are set. 

sl_customization:
-----------------

This file customizes the Simulink engine and layout. Allowing for files to get
registered to a system target file (e.g. system objects used for pil mode) as
well as add custom buttons to the Simulink UI. The `sl_customization` for the
Aria STF register's the mex wrapper `ext_comm` to the Aria Target. 

Although the STF specifies the `ext_comm` mex file in the header definition, it is still nessessary to specify this in the `sl_customizations` file or TCPIP external mode will not be available for the Target. 

Aria_Download_Setup.m:
----------------------

This file is the call back used within the Aria STF rtwoptions, whenever the configurations need to update the options for the external mode. 

The function queries the download to robot checkbox in the 'Aria Target Options' pane and based on its value switches external mode on or off and updates the target information for the transport layer (e.g. IP address) 

Aria_select_callback_handler:
-----------------------------

This file is the callback used whenever the System Target file is selected in the Configuration Parameters > Code Generation pane. When the Aria STF is selected this callback is used to update the Configuration Parameters to set default values and enable/disable parameters that can be modified. 

Going through the `Aria_select_callback_handler.m file`, the first operation that the callback takes is that it adds in the Aria Libraries and include directories into the custom code section. This will allow for the S-Functions to be able to link to the proper Pioneer drivers during the build process. 

Ideally, this modification should be included in a template make file; however, there was some linking issues when I tried to make a custom template make file so I had to use the custom code section to get around this issue. 

After setting the custom includes and libraries for the target, the select file enables external mode sets the transport layer to TCPIP, and places a default IP address for the robot into the `com_ext` arguments. 

The file then goes to set configurations for model reference, adds an optimization flag for production hardware, and sets up the target language for code generation to be C code. 

Finally, the callback sets the library extension (i.e. lib) that will be used for the linking process, and changes the Custom File Template to the `Aria_file_process.tlc`. This is so the correct main file is generated for the executable. 

Aria_make_rtw_hook.m:
---------------------

This file allows for the download and execution of the generated code onto the Pioneer target. The file is setup to be able to add in callbacks throughout the build process. For the purpose of downloading and executing the generated code on the Pioneer most of the code is contained within the `after_make` callback. 

This is just after the linking process is completed and the EXE file has been generated. The function `Download2Target` queries the necessary information about the target and model. Once done the files are first downloaded to the target. This is done via a `SYSTEM` command that invokes the Putty utility `PSCP`. 

This will place the model and the two libraries, `AriaVC10.dll` and `ariac_vc10_i386.dll` in the user directory. (The two libraries are optional so long as these libraries can be found on the robot computers's system path. )


After the file are downloaded to the robot onboard computer, the other Putty utility `PLINK` is then used to create a SSH connection to the robot computer and pipe commands to it. 

The first set of `PLINK` commands invoke the `CACLS` function on the onboard computer. This is done to set the user permissions to full. By default, Windows will set permissions for files transferred via SSH to read/write only (Even if permissions are set to full on the Host). As such the files will not be able to execute after being sent to the target until the permissions are set to allow execution of the files. 

The final plink command starts the model executable with the two options `-w` and `-port`. These options start the executable in a waiting state and what TCPIP port to open for communication respectively. This final PLINK call also creates a new thread by having an `&` operator at the end. This is due to the `-w` flag. Since the program will wait for a signal from Simulink to start it will not return a value to MATLAB as such both processes wait on each other. With the separate processes setup then external mode can be initiated from Simulink. 

The other function invoked in the `exit` callback closes the command prompt spawned from the previous `PLINK` command in the `after_make` callback. The function does this by getting the process list and scaning the result for `cmd.exe`. Once found, the function takes the second to last instance of that process and terminates it since the command window is no longer needed once the executable is running on the target. 
