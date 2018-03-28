ConfigVersion 2.0
;SectionFlags for : 
;  Robot parameter file

Section General settings
;SectionFlags for General settings: 
Class Pioneer            ; general type of robot
Subclass wheelchair-sh   ; specific type of robot
RobotRadius 550.00000    ; radius in mm
RobotDiagonal 300.00000  ; half-height to diagonal of octagon
RobotWidth 680.00000     ; width in mm
RobotLength 1340.00000   ; length in mm of the whole robot
RobotLengthFront 0.00000 ; length in mm to the front of the robot (if this is 0
                         ; (or non existent) this value will be set to half of
                         ; RobotLength)
RobotLengthRear 0.00000  ; length in mm to the rear of the robot (if this is 0
                         ; (or non existent) this value will be set to half of
                         ; RobotLength)
Holonomic true           ; turns in own radius
MaxRVelocity 360         ; absolute maximum degrees / sec
MaxVelocity 2000         ; absolute maximum mm / sec
MaxLatVelocity 0         ; absolute lateral maximum mm / sec
HasMoveCommand true      ; has built in move command
RequestIOPackets false   ; automatically request IO packets
RequestEncoderPackets false ; automatically request encoder packets
SwitchToBaudRate 38400   ; switch to this baud if non-0 and supported on robot

Section Conversion factors
;SectionFlags for Conversion factors: 
AngleConvFactor 0.00153  ; radians per angular unit (2PI/4096)
DistConvFactor 1.00000   ; multiplier to mm from robot units
VelConvFactor 1.00000    ; multiplier to mm/sec from robot units
RangeConvFactor 1.00000  ; multiplier to mm from sonar units
DiffConvFactor 0.00373   ; ratio of angular velocity to wheel velocity (unused
                         ; in newer firmware that calculates and returns this)
Vel2Divisor 20.00000     ; divisor for VEL2 commands
GyroScaler 1.62600       ; Scaling factor for gyro readings

Section Accessories the robot has
;SectionFlags for Accessories the robot has: 
TableSensingIR false     ; if robot has upwards facing table sensing IR
NewTableSensingIR false  ; if table sensing IR are sent in IO packet
FrontBumpers true        ; if robot has a front bump ring
NumFrontBumpers 4        ; number of front bumpers on the robot
RearBumpers true         ; if the robot has a rear bump ring
NumRearBumpers 3         ; number of rear bumpers on the robot

Section IR parameters
;SectionFlags for IR parameters: 
IRNum 0                  ; number of IRs on the robot
;  IRUnit <IR Number> <IR Type> <Persistance, cycles> <x position, mm> <y
 position, mm>

Section Movement control parameters
;  if these are 0 the parameters from robot flash will be used, otherwise these
;  values will be used
;SectionFlags for Movement control parameters: 
SettableVelMaxes false   ; if TransVelMax and RotVelMax can be set
TransVelMax 0            ; maximum desired translational velocity for the robot
RotVelMax 0              ; maximum desired rotational velocity for the robot
SettableAccsDecs true    ; if the accel and decel parameters can be set
TransAccel 0             ; translational acceleration
TransDecel 0             ; translational deceleration
RotAccel 0               ; rotational acceleration
RotDecel 0               ; rotational deceleration
HasLatVel false          ; if the robot has lateral velocity
LatVelMax 0              ; maximum desired lateral velocity for the robot
LatAccel 0               ; lateral acceleration
LatDecel 0               ; lateral deceleration

Section GPS parameters
;SectionFlags for GPS parameters: 
GPSPX 0                  ; x location of gps receiver antenna on robot, mm
GPSPY 0                  ; y location of gps receiver antenna on robot, mm
GPSType standard         ; type of gps receiver (trimble, novatel, standard)
GPSPort COM3             ; port the gps is on
GPSBaud 9600             ; gps baud rate (9600, 19200, 38400, etc.)

Section Compass parameters
;SectionFlags for Compass parameters: 
CompassType robot        ; type of compass: robot (typical configuration), or
                         ; serialTCM (computer serial port)
CompassPort              ; serial port name, if CompassType is serialTCM

Section Sonar parameters
;SectionFlags for Sonar parameters: 
SonarNum 0               ; Number of sonars on the robot.
;  SonarUnit <sonarNumber> <x position, mm> <y position, mm> <heading of disc,
 degrees> <MTX sonar board> <MTX sonar board unit position> <MTX gain> <MTX
 detection threshold> <MTX max range> <autonomous driving sensor flag>

Section SonarBoard_1
;  Information about the connection to this Sonar Board.
;SectionFlags for SonarBoard_1: 
SonarAutoConnect false   ; SonarBoard_1 exists and should be automatically
                         ; connected at startup.
SonarBoardType           ; Type of the sonar board.
SonarBoardPortType       ; Port type that the sonar is on.
SonarBoardPort           ; Port the sonar is on.
SonarBoardPowerOutput    ; Power output that controls this Sonar Board's power.
SonarBaud 0              ; Baud rate for the sonar board communication. (9600,
                         ; 19200, 38400, etc.).
SonarDelay 2             ; range [0, 10],  Sonar delay (in ms).
SonarGain 10             ; range [0, 31],  Default sonar gain for the board,
                         ; range 0-31.
SonarDetectionThreshold 25 ; range [0, 65535],  Default sonar detection
                         ; threshold for the board.
SonarMaxRange 4335       ; range [0, 4335],  Default maximum sonar range for
                         ; the board.

Section Laser parameters
;  Information about the connection to this laser and its position on the
;  vehicle.
;SectionFlags for Laser parameters: 
LaserAutoConnect true    ; Laser_1 exists and should be automatically connected
                         ; at startup.
LaserX -418              ; Location (in mm) of the laser in X (+ front, - back)
                         ; relative to the robot's idealized center of
                         ; rotation.
LaserY 0                 ; Location (in mm) of the laser in Y (+ left, - right)
                         ; relative to the robot's idealized center of
                         ; rotation.
LaserTh 0.00000          ; range [-180, 180],  Rotation (in deg) of the laser
                         ; (+ counterclockwise, - clockwise).
LaserZ 0                 ; minimum 0,  Height (in mm) of the laser from the
                         ; ground. 0 means unknown.
LaserIgnore              ; Angles (in deg) at which to ignore readings, +/1 one
                         ; degree. Angles are entered as strings, separated by
                         ; a space.
LaserFlipped true        ; Laser_1 is upside-down.
LaserType lms2xx         ; Type of laser.
LaserPortType serial     ; Type of port the laser is on.
LaserPort COM2           ; Port the laser is on.
LaserPowerOutput         ; Power output that controls this laser's power.
LaserStartingBaudChoice  ; StartingBaud for this laser. Leave blank to use the
                         ; default.
LaserAutoBaudChoice      ; AutoBaud for this laser. Leave blank to use the
                         ; default.
LaserPowerControlled true ; When enabled (true), this indicates that the power
                         ; to the laser is controlled by the serial port line.
LaserMaxRange 0          ; Maximum range (in mm) to use for the laser. This
                         ; should be specified only when the range needs to be
                         ; shortened. 0 to use the default range.
LaserCumulativeBufferSize 0 ; Cumulative buffer size to use for the laser. 0 to
                         ; use the default.
LaserStartDegrees        ; Start angle (in deg) for the laser. This may be used
                         ; to constrain the angle. Fractional degrees are
                         ; permitted. Leave blank to use the default.
LaserEndDegrees          ; End angle (in deg) for the laser. This may be used
                         ; to constrain the angle. Fractional degreees are
                         ; permitted. Leave blank to use the default.
LaserDegreesChoice       ; Degrees choice for the laser. This may be used to
                         ; constrain the range. Leave blank to use the default.
LaserIncrement           ; Increment (in deg) for the laser. Fractional degrees
                         ; are permitted. Leave blank to use the default.
LaserIncrementChoice     ; Increment choice for the laser. This may be used to
                         ; increase the increment. Leave blank to use the
                         ; default.
LaserUnitsChoice         ; Units for the laser. This may be used to increase
                         ; the size of the units. Leave blank to use the
                         ; default.
LaserReflectorBitsChoice  ; ReflectorBits for the laser. Leave blank to use the
                         ; default.

Section Laser 2 parameters
;  Information about the connection to this laser and its position on the
;  vehicle.
;SectionFlags for Laser 2 parameters: 
LaserAutoConnect false   ; Laser_2 exists and should be automatically connected
                         ; at startup.
LaserX 0                 ; Location (in mm) of the laser in X (+ front, - back)
                         ; relative to the robot's idealized center of
                         ; rotation.
LaserY 0                 ; Location (in mm) of the laser in Y (+ left, - right)
                         ; relative to the robot's idealized center of
                         ; rotation.
LaserTh 0.00000          ; range [-180, 180],  Rotation (in deg) of the laser
                         ; (+ counterclockwise, - clockwise).
LaserZ 0                 ; minimum 0,  Height (in mm) of the laser from the
                         ; ground. 0 means unknown.
LaserIgnore              ; Angles (in deg) at which to ignore readings, +/1 one
                         ; degree. Angles are entered as strings, separated by
                         ; a space.
LaserFlipped false       ; Laser_2 is upside-down.
LaserType                ; Type of laser.
LaserPortType            ; Type of port the laser is on.
LaserPort                ; Port the laser is on.
LaserPowerOutput         ; Power output that controls this laser's power.
LaserStartingBaudChoice  ; StartingBaud for this laser. Leave blank to use the
                         ; default.
LaserAutoBaudChoice      ; AutoBaud for this laser. Leave blank to use the
                         ; default.
LaserPowerControlled true ; When enabled (true), this indicates that the power
                         ; to the laser is controlled by the serial port line.
LaserMaxRange 0          ; Maximum range (in mm) to use for the laser. This
                         ; should be specified only when the range needs to be
                         ; shortened. 0 to use the default range.
LaserCumulativeBufferSize 0 ; Cumulative buffer size to use for the laser. 0 to
                         ; use the default.
LaserStartDegrees        ; Start angle (in deg) for the laser. This may be used
                         ; to constrain the angle. Fractional degrees are
                         ; permitted. Leave blank to use the default.
LaserEndDegrees          ; End angle (in deg) for the laser. This may be used
                         ; to constrain the angle. Fractional degreees are
                         ; permitted. Leave blank to use the default.
LaserDegreesChoice       ; Degrees choice for the laser. This may be used to
                         ; constrain the range. Leave blank to use the default.
LaserIncrement           ; Increment (in deg) for the laser. Fractional degrees
                         ; are permitted. Leave blank to use the default.
LaserIncrementChoice     ; Increment choice for the laser. This may be used to
                         ; increase the increment. Leave blank to use the
                         ; default.
LaserUnitsChoice         ; Units for the laser. This may be used to increase
                         ; the size of the units. Leave blank to use the
                         ; default.
LaserReflectorBitsChoice  ; ReflectorBits for the laser. Leave blank to use the
                         ; default.

Section Battery_1
;  Information about the connection to this battery.
;SectionFlags for Battery_1: 
BatteryAutoConnect false ; Battery_1 exists and should be automatically
                         ; connected at startup.
BatteryType              ; Type of battery.
BatteryPortType          ; Port type that the battery is on.
BatteryPort              ; Port the battery is on.
BatteryBaud 0            ; Baud rate to use for battery communication (9600,
                         ; 19200, 38400, etc.).

Section LCD_1
;  The physical definition of this LCD.
;SectionFlags for LCD_1: 
LCDAutoConnect false     ; LCD_1 exists and should automatically be connected
                         ; at startup.
LCDDisconnectOnConnectFailure false ; The LCD is a key component and is
                         ; required for operation. If this is enabled and there
                         ; is a failure in the LCD communications, then the
                         ; robot will restart.
LCDType                  ; Type of LCD.
LCDPortType              ; Port type that the LCD is on.
LCDPort                  ; Port that the LCD is on.
LCDPowerOutput           ; Power output that controls this LCD's power.
LCDBaud 0                ; Baud rate for the LCD communication (9600, 19200,
                         ; 38400, etc.).

Section PTZ 1 parameters
;  Information about the connection to a pan/tilt unit (PTU) or pan/tilt/zoom
;  control (PTZ) of a camera
;SectionFlags for PTZ 1 parameters: 
PTZAutoConnect false     ; If true, connect to this PTZ by default.
PTZType unknown          ; PTZ or PTU type
PTZInverted false        ; If unit is mounted inverted (upside-down)
PTZSerialPort none       ; serial port, or none if not using serial port
                         ; communication
PTZRobotAuxSerialPort -1 ; Pioneer aux. serial port, or -1 if not using aux.
                         ; serial port for communication.
PTZAddress none          ; IP address or hostname, or none if not using network
                         ; communication.
PTZTCPPort 80            ; TCP Port to use for HTTP network connection
PTZUsername              ; Username, if required by camera
PTZPassword              ; Password, if required by camera

Section PTZ 2 parameters
;  Information about the connection to a pan/tilt unit (PTU) or pan/tilt/zoom
;  control (PTZ) of a camera
;SectionFlags for PTZ 2 parameters: 
PTZAutoConnect false     ; If true, connect to this PTZ by default.
PTZType unknown          ; PTZ or PTU type
PTZInverted false        ; If unit is mounted inverted (upside-down)
PTZSerialPort none       ; serial port, or none if not using serial port
                         ; communication
PTZRobotAuxSerialPort -1 ; Pioneer aux. serial port, or -1 if not using aux.
                         ; serial port for communication.
PTZAddress none          ; IP address or hostname, or none if not using network
                         ; communication.
PTZTCPPort 80            ; TCP Port to use for HTTP network connection
PTZUsername              ; Username, if required by camera
PTZPassword              ; Password, if required by camera

Section PTZ 3 parameters
;  Information about the connection to a pan/tilt unit (PTU) or pan/tilt/zoom
;  control (PTZ) of a camera
;SectionFlags for PTZ 3 parameters: 
PTZAutoConnect false     ; If true, connect to this PTZ by default.
PTZType unknown          ; PTZ or PTU type
PTZInverted false        ; If unit is mounted inverted (upside-down)
PTZSerialPort none       ; serial port, or none if not using serial port
                         ; communication
PTZRobotAuxSerialPort -1 ; Pioneer aux. serial port, or -1 if not using aux.
                         ; serial port for communication.
PTZAddress none          ; IP address or hostname, or none if not using network
                         ; communication.
PTZTCPPort 80            ; TCP Port to use for HTTP network connection
PTZUsername              ; Username, if required by camera
PTZPassword              ; Password, if required by camera

Section PTZ 4 parameters
;  Information about the connection to a pan/tilt unit (PTU) or pan/tilt/zoom
;  control (PTZ) of a camera
;SectionFlags for PTZ 4 parameters: 
PTZAutoConnect false     ; If true, connect to this PTZ by default.
PTZType unknown          ; PTZ or PTU type
PTZInverted false        ; If unit is mounted inverted (upside-down)
PTZSerialPort none       ; serial port, or none if not using serial port
                         ; communication
PTZRobotAuxSerialPort -1 ; Pioneer aux. serial port, or -1 if not using aux.
                         ; serial port for communication.
PTZAddress none          ; IP address or hostname, or none if not using network
                         ; communication.
PTZTCPPort 80            ; TCP Port to use for HTTP network connection
PTZUsername              ; Username, if required by camera
PTZPassword              ; Password, if required by camera

Section PTZ 5 parameters
;  Information about the connection to a pan/tilt unit (PTU) or pan/tilt/zoom
;  control (PTZ) of a camera
;SectionFlags for PTZ 5 parameters: 
PTZAutoConnect false     ; If true, connect to this PTZ by default.
PTZType unknown          ; PTZ or PTU type
PTZInverted false        ; If unit is mounted inverted (upside-down)
PTZSerialPort none       ; serial port, or none if not using serial port
                         ; communication
PTZRobotAuxSerialPort -1 ; Pioneer aux. serial port, or -1 if not using aux.
                         ; serial port for communication.
PTZAddress none          ; IP address or hostname, or none if not using network
                         ; communication.
PTZTCPPort 80            ; TCP Port to use for HTTP network connection
PTZUsername              ; Username, if required by camera
PTZPassword              ; Password, if required by camera

Section PTZ 6 parameters
;  Information about the connection to a pan/tilt unit (PTU) or pan/tilt/zoom
;  control (PTZ) of a camera
;SectionFlags for PTZ 6 parameters: 
PTZAutoConnect false     ; If true, connect to this PTZ by default.
PTZType unknown          ; PTZ or PTU type
PTZInverted false        ; If unit is mounted inverted (upside-down)
PTZSerialPort none       ; serial port, or none if not using serial port
                         ; communication
PTZRobotAuxSerialPort -1 ; Pioneer aux. serial port, or -1 if not using aux.
                         ; serial port for communication.
PTZAddress none          ; IP address or hostname, or none if not using network
                         ; communication.
PTZTCPPort 80            ; TCP Port to use for HTTP network connection
PTZUsername              ; Username, if required by camera
PTZPassword              ; Password, if required by camera

Section PTZ 7 parameters
;  Information about the connection to a pan/tilt unit (PTU) or pan/tilt/zoom
;  control (PTZ) of a camera
;SectionFlags for PTZ 7 parameters: 
PTZAutoConnect false     ; If true, connect to this PTZ by default.
PTZType unknown          ; PTZ or PTU type
PTZInverted false        ; If unit is mounted inverted (upside-down)
PTZSerialPort none       ; serial port, or none if not using serial port
                         ; communication
PTZRobotAuxSerialPort -1 ; Pioneer aux. serial port, or -1 if not using aux.
                         ; serial port for communication.
PTZAddress none          ; IP address or hostname, or none if not using network
                         ; communication.
PTZTCPPort 80            ; TCP Port to use for HTTP network connection
PTZUsername              ; Username, if required by camera
PTZPassword              ; Password, if required by camera

Section PTZ 8 parameters
;  Information about the connection to a pan/tilt unit (PTU) or pan/tilt/zoom
;  control (PTZ) of a camera
;SectionFlags for PTZ 8 parameters: 
PTZAutoConnect false     ; If true, connect to this PTZ by default.
PTZType unknown          ; PTZ or PTU type
PTZInverted false        ; If unit is mounted inverted (upside-down)
PTZSerialPort none       ; serial port, or none if not using serial port
                         ; communication
PTZRobotAuxSerialPort -1 ; Pioneer aux. serial port, or -1 if not using aux.
                         ; serial port for communication.
PTZAddress none          ; IP address or hostname, or none if not using network
                         ; communication.
PTZTCPPort 80            ; TCP Port to use for HTTP network connection
PTZUsername              ; Username, if required by camera
PTZPassword              ; Password, if required by camera

Section Video 1 parameters
;  Information about the connection to a video acquisition device,
;  framegrabber, or camera
;SectionFlags for Video 1 parameters: 
VideoAutoConnect false   ; If true, connect to this device by default.
VideoType unknown        ; Device type
VideoInverted false      ; If image should be flipped (for cameras mounted
                         ; inverted/upside-down)
VideoWidth -1            ; Desired width of image, or -1 for default
VideoHeight -1           ; Desired height of image, or -1 for default
VideoDeviceIndex -1      ; Device index, or -1 for default
VideoDeviceName none     ; Device name (overrides VideoDeviceIndex)
VideoChannel -1          ; Input channel, or -1 for default
VideoAnalogSignalFormat  ; NTSC or PAL, or empty for default. Only used for
                         ; analog framegrabbers.
VideoAddress none        ; IP address or hostname, or none if not using network
                         ; communication.
VideoTCPPort 80          ; TCP Port to use for HTTP network connection
VideoUsername            ; Username, if required by camera
VideoPassword            ; Password, if required by camera

Section Video 2 parameters
;  Information about the connection to a video acquisition device,
;  framegrabber, or camera
;SectionFlags for Video 2 parameters: 
VideoAutoConnect false   ; If true, connect to this device by default.
VideoType unknown        ; Device type
VideoInverted false      ; If image should be flipped (for cameras mounted
                         ; inverted/upside-down)
VideoWidth -1            ; Desired width of image, or -1 for default
VideoHeight -1           ; Desired height of image, or -1 for default
VideoDeviceIndex -1      ; Device index, or -1 for default
VideoDeviceName none     ; Device name (overrides VideoDeviceIndex)
VideoChannel -1          ; Input channel, or -1 for default
VideoAnalogSignalFormat  ; NTSC or PAL, or empty for default. Only used for
                         ; analog framegrabbers.
VideoAddress none        ; IP address or hostname, or none if not using network
                         ; communication.
VideoTCPPort 80          ; TCP Port to use for HTTP network connection
VideoUsername            ; Username, if required by camera
VideoPassword            ; Password, if required by camera

Section Video 3 parameters
;  Information about the connection to a video acquisition device,
;  framegrabber, or camera
;SectionFlags for Video 3 parameters: 
VideoAutoConnect false   ; If true, connect to this device by default.
VideoType unknown        ; Device type
VideoInverted false      ; If image should be flipped (for cameras mounted
                         ; inverted/upside-down)
VideoWidth -1            ; Desired width of image, or -1 for default
VideoHeight -1           ; Desired height of image, or -1 for default
VideoDeviceIndex -1      ; Device index, or -1 for default
VideoDeviceName none     ; Device name (overrides VideoDeviceIndex)
VideoChannel -1          ; Input channel, or -1 for default
VideoAnalogSignalFormat  ; NTSC or PAL, or empty for default. Only used for
                         ; analog framegrabbers.
VideoAddress none        ; IP address or hostname, or none if not using network
                         ; communication.
VideoTCPPort 80          ; TCP Port to use for HTTP network connection
VideoUsername            ; Username, if required by camera
VideoPassword            ; Password, if required by camera

Section Video 4 parameters
;  Information about the connection to a video acquisition device,
;  framegrabber, or camera
;SectionFlags for Video 4 parameters: 
VideoAutoConnect false   ; If true, connect to this device by default.
VideoType unknown        ; Device type
VideoInverted false      ; If image should be flipped (for cameras mounted
                         ; inverted/upside-down)
VideoWidth -1            ; Desired width of image, or -1 for default
VideoHeight -1           ; Desired height of image, or -1 for default
VideoDeviceIndex -1      ; Device index, or -1 for default
VideoDeviceName none     ; Device name (overrides VideoDeviceIndex)
VideoChannel -1          ; Input channel, or -1 for default
VideoAnalogSignalFormat  ; NTSC or PAL, or empty for default. Only used for
                         ; analog framegrabbers.
VideoAddress none        ; IP address or hostname, or none if not using network
                         ; communication.
VideoTCPPort 80          ; TCP Port to use for HTTP network connection
VideoUsername            ; Username, if required by camera
VideoPassword            ; Password, if required by camera

Section Video 5 parameters
;  Information about the connection to a video acquisition device,
;  framegrabber, or camera
;SectionFlags for Video 5 parameters: 
VideoAutoConnect false   ; If true, connect to this device by default.
VideoType unknown        ; Device type
VideoInverted false      ; If image should be flipped (for cameras mounted
                         ; inverted/upside-down)
VideoWidth -1            ; Desired width of image, or -1 for default
VideoHeight -1           ; Desired height of image, or -1 for default
VideoDeviceIndex -1      ; Device index, or -1 for default
VideoDeviceName none     ; Device name (overrides VideoDeviceIndex)
VideoChannel -1          ; Input channel, or -1 for default
VideoAnalogSignalFormat  ; NTSC or PAL, or empty for default. Only used for
                         ; analog framegrabbers.
VideoAddress none        ; IP address or hostname, or none if not using network
                         ; communication.
VideoTCPPort 80          ; TCP Port to use for HTTP network connection
VideoUsername            ; Username, if required by camera
VideoPassword            ; Password, if required by camera

Section Video 6 parameters
;  Information about the connection to a video acquisition device,
;  framegrabber, or camera
;SectionFlags for Video 6 parameters: 
VideoAutoConnect false   ; If true, connect to this device by default.
VideoType unknown        ; Device type
VideoInverted false      ; If image should be flipped (for cameras mounted
                         ; inverted/upside-down)
VideoWidth -1            ; Desired width of image, or -1 for default
VideoHeight -1           ; Desired height of image, or -1 for default
VideoDeviceIndex -1      ; Device index, or -1 for default
VideoDeviceName none     ; Device name (overrides VideoDeviceIndex)
VideoChannel -1          ; Input channel, or -1 for default
VideoAnalogSignalFormat  ; NTSC or PAL, or empty for default. Only used for
                         ; analog framegrabbers.
VideoAddress none        ; IP address or hostname, or none if not using network
                         ; communication.
VideoTCPPort 80          ; TCP Port to use for HTTP network connection
VideoUsername            ; Username, if required by camera
VideoPassword            ; Password, if required by camera

Section Video 7 parameters
;  Information about the connection to a video acquisition device,
;  framegrabber, or camera
;SectionFlags for Video 7 parameters: 
VideoAutoConnect false   ; If true, connect to this device by default.
VideoType unknown        ; Device type
VideoInverted false      ; If image should be flipped (for cameras mounted
                         ; inverted/upside-down)
VideoWidth -1            ; Desired width of image, or -1 for default
VideoHeight -1           ; Desired height of image, or -1 for default
VideoDeviceIndex -1      ; Device index, or -1 for default
VideoDeviceName none     ; Device name (overrides VideoDeviceIndex)
VideoChannel -1          ; Input channel, or -1 for default
VideoAnalogSignalFormat  ; NTSC or PAL, or empty for default. Only used for
                         ; analog framegrabbers.
VideoAddress none        ; IP address or hostname, or none if not using network
                         ; communication.
VideoTCPPort 80          ; TCP Port to use for HTTP network connection
VideoUsername            ; Username, if required by camera
VideoPassword            ; Password, if required by camera

Section Video 8 parameters
;  Information about the connection to a video acquisition device,
;  framegrabber, or camera
;SectionFlags for Video 8 parameters: 
VideoAutoConnect false   ; If true, connect to this device by default.
VideoType unknown        ; Device type
VideoInverted false      ; If image should be flipped (for cameras mounted
                         ; inverted/upside-down)
VideoWidth -1            ; Desired width of image, or -1 for default
VideoHeight -1           ; Desired height of image, or -1 for default
VideoDeviceIndex -1      ; Device index, or -1 for default
VideoDeviceName none     ; Device name (overrides VideoDeviceIndex)
VideoChannel -1          ; Input channel, or -1 for default
VideoAnalogSignalFormat  ; NTSC or PAL, or empty for default. Only used for
                         ; analog framegrabbers.
VideoAddress none        ; IP address or hostname, or none if not using network
                         ; communication.
VideoTCPPort 80          ; TCP Port to use for HTTP network connection
VideoUsername            ; Username, if required by camera
VideoPassword            ; Password, if required by camera
