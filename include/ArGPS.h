/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016-2018 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/

#ifndef ARGPS_H
#define ARGPS_H

#include "ariaTypedefs.h"
#include "ArFunctor.h"
#include "ariaUtil.h"
#include "ArMutex.h"
#include "ArNMEAParser.h"
#include "ArGPSCoords.h"
#include <math.h>
#include <string>
#include <vector>

class ArDeviceConnection; // for pointer in ArGPS

/** @brief GPS Device Interface 
 *
 *  Connects to GPS device over a serial port or other device connection and reads data.
 *  Supports GPS devices sending standard NMEA format data 
 *  (specifically the GPRMC, GPGGA, GPGSA, GPGRME, and optionally GPGSV, PGRMZ, PGRME, 
 *  HCHDG/T/M and GPHDG/T/M messages). 
 *  If your GPS device supports several data formats or modes, select
 *  NMEA output in its configuration.
 *
 *  The preferred method of creating and setting up a new ArGPS object is to use
 *  ArGPSConnector, which creates an instance of ArGPS or a subclass, and
 *  creates and opens its device connection, based on command-line parameters.
 *  (To manually create an ArGPS object, create an ArDeviceConnection instance
 *  and call setDeviceConnection(), then open that device connection and call
 *  connect().
 *
 *  For either method, to get new data from the GPS, must call read() or readWithLock() periodically, 
 *  ideally at a rate equal to or faster than your GPS sends data (usually one second). 
 *  You can do this from a Sensor Intetrpretation Task in ArRobot, or a seperate thread. 
 *  If you are calling read() from a loop in a new thread, 
 *
 *  Here is an example of calling readWithLock() from a sensor interpretation
 *  task. The integer argument given to the functor constructor is a milisecond timeout that
 *  is passed to readWithLock() and prevents it from blocking too long if it doesn't read any data.
 *  It is important to do this in a robot task, or the robot task cycle will be
 *  blocked and cause problems.
 *  @code
 *    ArRetFunctor1C<ArGPS, int, unsigned int> gpsReadFunc(myGPS, &ArGPS::readWithLock, 10);
 *    myRobot->addSensorInterpretationTask("GPS read", 100, &gpsReadFunc);
 *  @endcode
 *
 *  If you use your own loop or thread, then it ought to include a call to ArUtil::sleep() for at least several hundred 
 *  miliseconds to avoid starving other threads, since read() will return
 *  immediately if there is no data to read rather than blocking.
 *
 *  For each piece of data provided by this class, there is a flag indicating
 *  whether it was received from the GPS and set. Not all GPS models return all
 *  kinds of information, or it may be disabled in some way in a GPS's internal
 *  configuration, or the GPS may not yet have started sending the data (e.g.
 *  still acquiring satellites).  Also, not all data will be received by one call to read(),
 *  and especially immediately after connecting and starting to read data, it 
 *  may take a few seconds for data to be obtained. Furthermore, it may take
 *  some time for the GPS to calculate data with full accuracy.
 *
 *  @sa @ref gpsExample.cpp
 *  @sa @ref gpsRobotTaskExample.cpp
 *
 *  This class is not inherently thread safe. Stored data is updated by read(), so 
 *  if accessing from multiple threads, call lock() before calling any data
 *  accessor methods (methods starting with "get"), or read(), and call unlock() 
 *  when done.   You can also call readWithLock() to do a locked read in one
 *  function call. 
 *
 *  @note ArGPS only provides access to the data reported by a GPS. The position
 *  reported by a GPS is in degrees on the surface of the earth (WGS84 datum), not in the
 *  cartesian coordinate system used by the robot odometry or ArMap. You can use 
 *  the subclasses of Ar3DPoint (ArLLACoords, etc) to convert between different
 *  geographical coordinate systems, which may help you match GPS coordinates to
 *  the robot pose coordinate system.
 
   @note Generally one device per connection is assumed. If multiple 
   devices are chained, all sending messages on one connection, messages 
   from different devices are not distinguished.  This is OK if the devices
   send different types of messages.  (However a particular message handler
   function in a subclass of ArGPS could check the message prefix. No 
   message handlers in ArGPS or subclasses provided in ARIA do this however.)

   @sa ArGPSMapCoords
   @sa ArGPSCoordConverter

  @ingroup OptionalClasses
   @ingroup DeviceClasses
 */
class ArGPS {

public:
    AREXPORT ArGPS();

    virtual ~ArGPS() { }

    /** @brief Set device connection to use */
    void setDeviceConnection(ArDeviceConnection* deviceConn) { myDevice = deviceConn; }

    /** @brief Return device connection in use (or NULL if none) */
    ArDeviceConnection* getDeviceConnection() const { return myDevice; }


    /** @brief Check that the device connection (e.g. serial port) is open, and
     *  that data is being received from GPS.
     *
     *  Subclasses may override this method so that  device-specific
     *  initialization commands may be sent.
     *
     *  @return false if there is no device connection or the device connection
     *  is not open, or if there is an error sending device initialization
     *  commands, or if no data is received after calling read() every 100 ms
     *  for @a connectTimeout ms. Otherwise, return true.
     *
     *  @sa blockingConnect()
     */
    AREXPORT virtual bool connect(unsigned long connectTimeout = 20000);

    /** Same as connect(). See connect().  */
    bool blockingConnect(unsigned long connectTimeout = 20000) { return connect(connectTimeout); }

public:

    
    /** @brief Flags to indicates what the read() method did. 
     *  i.e. If nothing was done, then the
     *  result will be 0. To check a read() return result @a result to see if data was updated, use
     *  (result & ReadUpdated). To check if there was an error, use (result &
     *  ReadError). 
     *
     *  These happen to match the flags in ArNMEAParser.
     */
    enum {
      ReadFinished = ArNMEAParser::ParseFinished,
      ReadError = ArNMEAParser::ParseError, 
      ReadData = ArNMEAParser::ParseData,
      ReadUpdated = ArNMEAParser::ParseUpdated
    } ReadFlags;

    /** @brief Read some data from the device connection, and update stored data as complete messages are received. 
     * @param maxTime If nonzero, return when this time limit is reached, even if there is still data available to read. If zero, then don't return until all available data has been exhausted or an error occurs. 
     *  Be careful setting this parameter to 0: read() could block for 
     *  an arbitrary amount of time, even forever if for some reason data is recieved from
     *  the device faster than read() can read and parse it. 
     * @return A mask of ReadFlags codes, combined with bitwise or (|), or 0 if no attempt to read from the device occured (for example because the @a maxTime timeout was reached before the first attempt to read occured).  The flags will include
     * ReadError if there was as error reading from the device connection,
     * ReadData if some data was read,
     * ReadUpdated if data was read and a full message was successfully read and
     * stored data was updated in ArGPS,
     * ReadFinished if all available data was read.
     */
    AREXPORT virtual int read(unsigned long maxTime = 0);

    /** Calls lock(), calls read(maxTime), then calls unlock(). Note, this could
     * end up keeping ArGPS locked until @a maxTime is reached, or for any amount
     * of time if @a maxTime is 0, so watch out for that. */
    int readWithLock(unsigned int maxTime) { lock(); int r = read(maxTime); unlock(); return r; }

    /** Locks a mutex object contained by this class.
     *  No other method (except readWithLock()) in ArGPS locks or unlocks this
     *  mutex, it is provided for you to use when accessing ArGPS from multiple
     *  threads.
     */
    void lock() { myMutex.lock(); }

    /** Unlocks a mutex object contained by this class.
     *  No other method (except readWithLock()) in ArGPS locks or unlocks this
     *  mutex, it is provided for you to use when accessing ArGPS from multiple
     *  threads.
     */
    void unlock() { myMutex.unlock(); }
    

    /** @brief Set whether checksum sent with NMEA messages is ignored */
    void setIgnoreChecksum(bool ignore) { myNMEAParser.setIgnoreChecksum(ignore); }

    /** @brief Log last received data using ArLog. */
    AREXPORT void logData() const;

    /** Print basic navigation data on one line to standard output, with no newline at end. */
    AREXPORT void printData(bool labels = true) const;

    AREXPORT void printDataLabelsHeader() const;

    /** Data accessors
     * @brief Access the last received data from the GPS */
    // @{
 
    typedef enum {  
        NoFix, BadFix, GPSFix, DGPSFix, PPSFix, 
        RTKinFix, FloatRTKinFix, DeadReckFix, 
        ManualFix, SimulatedFix, UnknownFixType,
        OmnistarConverging = FloatRTKinFix, 
        OmnistarConverged = RTKinFix
     } FixType;

    class Data {
    public:
        AREXPORT Data();
        double latitude; ///< (from NMEA GPRMC)
        double longitude; ///< (from NMEA GPRMC)
        bool havePosition; ///< (from NMEA GPRMC)
        ArTime timeGotPosition;   ///< Local computer time when ArGPS class received the GPRMC position message from the GPS. 
        double speed; ///< (From NMEA GPRMC, if provided)
        bool haveSpeed; ///< (From NMEA GPRMC)
        ArTime GPSPositionTimestamp;   ///< Timestamp provided by GPS device along with latitude and longitude. (from NMEA GPRMC message)
        ArGPS::FixType fixType; ///< (from NMEA GPGGA)
        unsigned short numSatellitesTracked;
        double altitude;    ///< receiver provides this based on GPS data.  meters above sea level. (from NMEA GPGGA)
        bool haveAltitude; //< (from NMEA GPGGA)
        double altimeter;   ///< from seperate altimeter (if receiver provides PGRMZ message). meters above sea level.
        bool haveAltimeter;
        unsigned short DGPSStationID; ///< (from NMEA GPGGA)
        bool haveDGPSStation; ///< (from NMEA GPGGA)
        double garminPositionError; ///< Error in meters, only some GPS devices provide this
        bool haveGarminPositionError; ///< Error in meters, only some GPS devices provide this (PGRME)
        double garminVerticalPositionError; ///< Error in meters, only some GPS devices provide this (PGRME)
        bool haveGarminVerticalPositionError; ///< Error in meters, only some GPS devices provide this (PGRME)
        double compassHeadingMag; ///< (from HCDHM message, if device provides it)
        double compassHeadingTrue; ///< (from HCHDT, if device provides it)
        bool haveCompassHeadingMag; ///< (from HCDHM message, if device provides it)
        bool haveCompassHeadingTrue; ///< (from HCHDT message, if device provides it)
        unsigned long compassMagCounter;  ///< Incremented whenever @a compassHeadingMag is updated with new data
        unsigned long compassTrueCounter;  ///< Incremented whenever @a compassHeadingMag is updated with new data
        bool haveHDOP; ///< Horizontal dilution of precision (from NMEA GPGGA)
        double HDOP; ///< Horizontal dilution of precision (from NMEA GPGGA)
        bool haveVDOP; ///< Vertical dilution of precision (from NMEA GPGGA)
        double VDOP; ///< Vertical dilution of precision (from NMEA GPGGA)
        bool havePDOP; ///< Combined dilution of precision (from NMEA GPGGA)
        double PDOP; ///< Combined dilution of precision (from NMEA GPGGA)
        bool qualityFlag;   ///< Some GPS devices set this to false if data quality is below some thresholds.
        double meanSNR;   ///< Mean of satellite signal-noise ratios (dB)
        bool haveSNR; ///< (from NMEA GPGSV)
        double beaconSignalStrength;  ///< dB (from NMEA GPMSS)
        double beaconSNR; ///< dB  (from NMEA GPMSS)
        double beaconFreq; ///< kHz (from NMEA GPMSS)
        unsigned short beaconBPS; ///< Bits/sec (from NMEA GPMSS)
        unsigned short beaconChannel; ///< (from NMEA GPMSS)
        bool haveBeaconInfo; ///< (from NMEA GPMSS)
        double inputsRMS; ///< (from NMEA GPGST)
        bool haveInputsRMS; ///< (from NMEA GPGST)
        ArPose errorEllipse; ///< Ellipse shows standard deviation, in meters. Orientation is degrees from true north. (from NMEA GPGST)
        bool haveErrorEllipse; ///< (from NMEA GPGST)
        ArPose latLonError; ///< Std.deviation, meters. Theta is unused. May only be provided by the GPS in certain fix modes. Note, values could be inf or nan (GPS sends these in some situations). Use isinf() and isnan() to check.
        bool haveLatLonError;
        double altitudeError; ///< Std. deviation, meters. Note, value could be inf or nan (GPS sends these in some situations). use isinf() and isnan() to check.
        bool haveAltitudeError;
    };

    /** Access all of the internally stored data directly. @see ArGPS::Data  */
    const ArGPS::Data& getCurrentDataRef() const { return myData; } 

    /** (from NMEA GPGGA) */
    FixType getFixType() const { return myData.fixType; }
    /** (from NMEA GPGGA) */
    AREXPORT const char* getFixTypeName() const;
    static AREXPORT const char* getFixTypeName(FixType type);

    /** (from NMEA GPRMC) */
    AREXPORT bool havePosition() const { return myData.havePosition; }
    /** (from NMEA GPRMC) */
    AREXPORT bool haveLatitude() const { return myData.havePosition; }
    /** (from NMEA GPRMC) */
    AREXPORT bool haveLongitude() const { return myData.havePosition; }

    /** @return latitude in decimal degrees. 
        (from NMEA GPRMC) */
    double getLatitude() const { return myData.latitude; }

    /** @return longitude in decimal degrees. 
        (from NMEA GPRMC) */
    double getLongitude() const { return myData.longitude; }

    /// @return latitude, longitude and altitude
    ArLLACoords getLLA() const { return ArLLACoords(myData.latitude, myData.longitude, myData.altitude); }

    /// @return latitude and longitude in an ArPose object. Theta will be 0.
    ArPose getAsPose() const { return ArPose(myData.latitude, myData.longitude); }

    /** @return copy of an ArTime object set to the time that ArGPS read and received latitude and longitude data from the GPS. 
         */
    ArTime getTimeReceivedPosition() const { return myData.timeGotPosition; }
  

    /** (from NMEA GPRMC) */
    bool haveSpeed() const { return myData.haveSpeed; }

    /** @return GPS' measured speed converted to meters per second, if provided
        (from NMEA GPRMC, if provided)
        */
    double getSpeed() const { return myData.speed; }

    /** Timestamp provided by GPS device along with position. (from NMEA GPRMC) */
    ArTime getGPSPositionTimestamp() const { return myData.GPSPositionTimestamp; }

    int getNumSatellitesTracked() const { return (int) myData.numSatellitesTracked; }
    /** (from NMEA GPGGA) */
    bool haveDGPSStation() const { return myData.haveDGPSStation; }
    /** (from NMEA GPGGA) */
    unsigned short getDGPSStationID() const { return myData.DGPSStationID; }

    /** @return whether GPS provided a distance error estimation (from a
     * Garmin-specific message PGRME, most GPS receivers will not provide this) */
    bool haveGarminPositionError() const { return myData.haveGarminPositionError; }
    /** GPS device's error estimation in meters (from a Garmin-specific message PGRME,
 * most GPS receivers will not provide this)*/
    double getGarminPositionError() const { return myData.garminPositionError; }
    /** @return whether GPS provided an altitude error estimation (from a
     * Garmin-specific message PGRME, most GPS receivers will not provide this) */
    bool haveGarminVerticalPositionError() const { return myData.haveGarminVerticalPositionError; }
    /** @return An altitude error estimation (from a Garmin-specific message PGRME,
 * most GPS receivers will not provide this) */
    double getGarminVerticalPositionError() const { return myData.garminVerticalPositionError; }

    /** Have a compass heading value relative to magnetic north.
        @note The GPS or compass device must be configured to send HCHDM messages 
        to receive compass data. Only some GPS receivers support this. 
    */
    bool haveCompassHeadingMag() const { return myData.haveCompassHeadingMag; }
    /** Have a compass heading value relative to true north (using GPS/compass
        device's configured declination).
        @note The GPS or compass device must be configured to send HCHDT messages 
        to receive compass data. Only some GPS receivers support this. 
    */
    bool haveCompassHeadingTrue() const { return myData.haveCompassHeadingTrue; }
    /** Heading from magnetic north
        @note The GPS or compass device must be configured to send HCHDM messages 
        to receive compass data. Only some GPS receivers support this. 
    */
    double getCompassHeadingMag() const { return myData.compassHeadingMag; }
    /** Heading from true north
        @note The GPS or compass device must be configured to send HCHDT messages 
        to receive compass data. Only some GPS receivers support this. 
    */
    double getCompassHeadingTrue() const { return myData.compassHeadingTrue; }


    /** Manually set compass value. */
    void setCompassHeadingMag(double val) { 
      myData.haveCompassHeadingMag = true;
      myData.compassHeadingMag = val; 
      myData.compassMagCounter++; 
    }

    /** Manually set compass value. */
    void setCompassHeadingTrue(double val) { 
      myData.haveCompassHeadingTrue = true;
      myData.compassHeadingTrue = val; 
      myData.compassMagCounter++; 
    }

    /** Manually set compass value. */
    void setCompassHeadingMagWithLock(double val) { lock(); setCompassHeadingMag(val); unlock(); }
    /** Manually set compass value. */
    void setCompassHeadingTrueWithLock(double val) { lock(); setCompassHeadingTrue(val); unlock(); }

    /// Altitude above sea level calculated from satellite positions (see also haveAltimiter()) (from NMEA GPGGA, if provided)
    bool haveAltitude() const { return myData.haveAltitude; }
    /// Altitude above sea level (meters), calculated from satellite positions (see also getAltimiter()) (from NMEA GPGGA, if provided)
    double getAltitude() const { return myData.altitude; }

    /// Some receivers may have an additional altitude from an altimiter (meters above sea level) (from PGRMZ, if receiver provides it)
    bool haveAltimeter() const { return myData.haveAltimeter; }
    /// Some receivers may have an additional altitude from an altimiter (meters above sea level) (from PGRMZ, if receiver provides it)
    double getAltimeter() const { return myData.altimeter; }

    /** (from NMEA GPGGA) */
    bool haveHDOP() const { return myData.haveHDOP; }
    /** (from NMEA GPGGA) */
    double getHDOP() const { return myData.HDOP; }
    /** (from NMEA GPGGA) */
    bool haveVDOP() const { return myData.haveVDOP; }
    /** (from NMEA GPGGA) */
    double getVDOP() const { return myData.VDOP; }
    /** (from NMEA GPGGA) */
    bool havePDOP() const { return myData.havePDOP; }
    /** (from NMEA GPGGA) */
    double getPDOP() const { return myData.PDOP; }

    /** (from NMEA GPGSV) */
    bool haveSNR() const { return myData.haveSNR; }
    /// dB (from NMEA GPGSV)
    double getMeanSNR() const { return myData.meanSNR; }

    /** Whether we have any DGPS stationary beacon info  (from NMEA GPMSS) */
    bool haveBeaconInfo() const { return myData.haveBeaconInfo; }
    /** DGPS stationary beacon signal strength (dB) (from NMEA GPMSS) */
    double getBeaconSignalStrength() const { return myData.beaconSignalStrength; }  
    /** DGPS stationary beacon signal to noise (dB) (from NMEA GPMSS) */
    double getBeaconSNR() const { return myData.beaconSNR; }  
    /** DGPS stationary beacon frequency (kHz) (from NMEA GPMSS) */
    double getBeaconFreq() const { return myData.beaconFreq; }
    /** DGPS stationary beacon bitrate (bits per second) (from NMEA GPMSS) */
    unsigned short getBecaonBPS() const { return myData.beaconBPS; }
    /** DGPS stationary beacon channel (from NMEA GPMSS) */
    unsigned short getBeaconChannel() const { return myData.beaconChannel; }

    /** Whether we have a position error estimate (as standard deviations in latitude and longitude) (from NMEA GPGST) */
    bool haveErrorEllipse() const { return myData.haveErrorEllipse; }
    /** Standard deviation of position error (latitude and longitude), meters. Theta in ArPose is orientation of ellipse from true north, Y is the length of the major axis on that orientation, X the minor.
        (from NMEA GPGST)
        @note Values may be inf or NaN (if GPS supplies "Inf" or "NAN")
    */
    ArPose getErrorEllipse() const {return myData.errorEllipse; }
    
    /** Whether we have latitude or longitude error estimates  (from NMEA GPGST) */
    bool haveLatLonError() const { return myData.haveLatLonError; }
    /** Standard deviation of latitude and longitude error, meters. 
        Theta value in ArPose is unused. 
        @note May only be provided by GPS in certain fix modes
          (e.g. Trimble AgGPS provides it in Omnistar and RTK modes, but not in GPS
          or DGPS modes).
        @note Values may be inf or NaN (if GPS supplies "Inf" or "NAN")
        (from NMEA GPGST)
    */
    ArPose getLatLonError() const { return myData.latLonError; }
    /** @copydoc getLatLonError() */
    double getLatitudeError() const { return myData.latLonError.getX(); }
    /** @copydoc getLatLonError() */
    double getLongitudeError() const { return myData.latLonError.getY(); }

    bool haveAltitudeError() const { return myData.haveAltitudeError; }
    /// Standard deviation of altitude error, meters. (from NMEA GPGST, if provided)
    double getAltitudeError() const { return myData.altitudeError; }
    
    /// (from NMEA GPGST)
    bool haveInputsRMS() const { return myData.haveInputsRMS; }
    /// (from NMEA GPGST)
    double getInputsRMS() const { return myData.inputsRMS; }

    

    /** Set a handler for an NMEA message. Mostly for internal use or to be used
     * by related classes, but you could use for ususual or custom messages
     * emitted by a device that you wish to be handled outside of the ArGPS
     * class. 
     * @param message Message identifier, without the prefix (i.e. without the "GP" prefix, or other prefix.)
     * @param handhler Handler function to parse and handle the message
     * @sa ArNMEAParser::addHandler
     * @sa ArNMEAParser::Handler
     * @sa ArNMEAParser
     */
    void addNMEAHandler(const char *message, ArNMEAParser::Handler *handler) { myNMEAParser.addHandler(message, handler); }
    void removeNMEAHandler(const char *message) { myNMEAParser.removeHandler(message); }
    void replaceNMEAHandler(const char *message, ArNMEAParser::Handler *handler) { 
      myNMEAParser.removeHandler(message);
      myNMEAParser.addHandler(message, handler); 
    }

protected:
    /** Block until data is read from GPS.
        Waits by calling read() every 100 ms for @a timeout ms.
     */
    AREXPORT bool waitForData(unsigned long timeout);

    /** Subclasses may override to send device initialization/configuration
     * commands and set up device-specific message handlers. (Default behavior
     * is to do nothing and return true.)
     */
    virtual bool initDevice() { return true; }

     
    /* Most recent data values received, to return to user */
    Data myData;

    /* Utility to read a double floating point number out of a std::string, if possible.
     * @return true if the string was nonempty and @a target was modified.
     */
    bool readFloatFromString(const std::string& str, double* target, double(*convf)(double) = NULL) const;

    /* Utility to read an unsigned short integer out of a std::string, if possible.
     * @return true if the string was nonempty and @a target was modified.
     */
    bool readUShortFromString(const std::string& str, unsigned short* target, unsigned short (*convf)(unsigned short) = NULL) const;


    /* Utility to read a double from a member of a vector of strings, if it exists. */
    bool readFloatFromStringVec(const std::vector<std::string>* vec, size_t i, double* target, double (*convf)(double) = NULL) const;

    /* Utility to read a double from a member of a vector of strings, if it exists. */
    bool readUShortFromStringVec(const std::vector<std::string>* vec, size_t i, unsigned short* target, unsigned short (*convf)(unsigned short) = NULL) const;

    /* Utility to convert DDDMM.MMMM to decimal degrees */
    static double gpsDegminToDegrees(double degmin);

    /* Utility to convert US nautical knots to meters/sec */
    static double knotsToMPS(double knots);
 
    /** Convert meters per second to miles per hour */
    static double mpsToMph(const double mps) { return mps * 2.23693629; }

    /* Utility to convert meters to US feet */
    static double metersToFeet(double m) { return m * 3.2808399; }

    /* Utility to convert US feet  to meters */
    static double feetToMeters(double f) { return f / 3.2808399; }
    


    /* Mutex */
    ArMutex myMutex;


    /* Connection info */
    ArDeviceConnection *myDevice;
    bool myCreatedOwnDeviceCon;
    ArRetFunctorC<bool, ArGPS> myParseArgsCallback; 
    ArArgumentParser* myArgParser;
    
    /* NMEA Parser */
    ArNMEAParser myNMEAParser;

    /* GPS message handlers */

    void handleGPRMC(ArNMEAParser::Message msg);
    ArFunctor1C<ArGPS, ArNMEAParser::Message> myGPRMCHandler;

    void handleGPGGA(ArNMEAParser::Message msg);
    ArFunctor1C<ArGPS, ArNMEAParser::Message> myGPGGAHandler;

    void handlePGRME(ArNMEAParser::Message msg);
    ArFunctor1C<ArGPS, ArNMEAParser::Message> myPGRMEHandler;

    void handlePGRMZ(ArNMEAParser::Message msg);
    ArFunctor1C<ArGPS, ArNMEAParser::Message> myPGRMZHandler;

    void handleHCHDx(ArNMEAParser::Message msg);
    ArFunctor1C<ArGPS, ArNMEAParser::Message> myHCHDxHandler;

    void handleGPGSA(ArNMEAParser::Message msg);
    ArFunctor1C<ArGPS, ArNMEAParser::Message> myGPGSAHandler;

    void handleGPGSV(ArNMEAParser::Message msg);
    ArFunctor1C<ArGPS, ArNMEAParser::Message> myGPGSVHandler;

    /* For calculating SNR averages based on multiple GPGSV messages. */
    unsigned int mySNRSum;
    unsigned short mySNRNum;

    void handleGPMSS(ArNMEAParser::Message msg);
    ArFunctor1C<ArGPS, ArNMEAParser::Message> myGPMSSHandler;

    void handleGPGST(ArNMEAParser::Message msg);
    ArFunctor1C<ArGPS, ArNMEAParser::Message> myGPGSTHandler;

    /* Set an ArTime object using a time read from a string as decimal seconds (SSS.SS) */
    bool readTimeFromString(const std::string& s, ArTime* time) const;

    /** Parse a GPRMC message (in @a msg) and place results in provided
     * variables. (Can be used by subclasses to store results of GPRMC differently
     * than normal.)
     * @since Aria 2.7.2
     */
    void parseGPRMC(const ArNMEAParser::Message &msg, double &latitudeResult, double &longitudeResult, bool &qualityFlagResult, bool &gotPosition, ArTime &timeGotPositionResult, ArTime &gpsTimestampResult, bool &gotSpeedResult, double &speedResult);

};


class ArRobotPacket;
class ArRobot;

/// @since Aria 2.7.4
class ArSimulatedGPS : public virtual ArGPS
{
  bool myHaveDummyPosition;
  ArRetFunctor1C<bool, ArSimulatedGPS, ArRobotPacket*> mySimStatHandlerCB;
  ArRobot *myRobot;
public:
  AREXPORT ArSimulatedGPS(ArRobot *robot = NULL);
  AREXPORT virtual ~ArSimulatedGPS();
  void setDummyPosition(double latitude, double longitude) {
    myData.latitude = latitude;
    myData.havePosition = true;
    myData.longitude = longitude;
    if(!myData.haveHDOP) myData.HDOP = 1.0;
    myData.haveHDOP = true;
    if(!myData.haveVDOP) myData.VDOP = 1.0;
    myData.haveVDOP = true;
    if(!myData.havePDOP) myData.PDOP = 1.0;
    myData.havePDOP = true;
    myData.fixType = SimulatedFix;
    myHaveDummyPosition = true;
  }
  void clearDummyPosition() {
    clearPosition();
    myHaveDummyPosition = false;
  }
  void clearPosition() {
    myData.havePosition = false;
    myData.latitude = 0;
    myData.longitude = 0;
    myData.altitude = 0;
    myData.HDOP = 0;
    myData.VDOP = 0;
    myData.PDOP = 0;
    myData.fixType = NoFix;
  }
  void setDummyPosition(double latitude, double longitude, double altitude) {
    myData.altitude = altitude;
    setDummyPosition(latitude, longitude);
  }
  AREXPORT void setDummyPosition(ArArgumentBuilder *args); 
  void setDummyPositionFromArgs(ArArgumentBuilder *args) { setDummyPosition(args); } // non-overloaded function can be used in functors
  AREXPORT virtual bool connect(unsigned long connectTimeout = 10000);
  virtual bool initDevice() { return true; }
  virtual int read(unsigned long maxTime = 0) {
    if(myHaveDummyPosition)
    {
      myData.timeGotPosition.setToNow();
    }
    return ReadUpdated | ReadFinished;
  }
private:
#ifndef SWIG
  bool handleSimStatPacket(ArRobotPacket *pkt); 
#endif
};


/** Contains conversion of latitude and longitude to flat map coordinates in one
 *  object with one convert() function returning a Ar3DPoint object; can
 *  internally obtain current GPS position from an ArGPS object; and can
 *  automatically use the first latitude and longitude coordinate received as
 *  the origin point of the output coordinate system. May simplify composition with other software 
 *  in some cases where you want to use single function calls and single objects
 *  rather than storing several variables (e.g. via ArFunctor object, filters,
 *  etc.), and also do not have a GPS position yet to use for the origin at time of
 *  initialization.
 */
class ArGPSCoordConverter
{
public:
  ArGPSCoordConverter(ArGPS *_gps = NULL) : gps(_gps), haveOrigin(false) ,
    myGetCurrentFunc(this, &ArGPSCoordConverter::getCurrent),
    myGetCurrent2DPoseFunc(this, &ArGPSCoordConverter::getCurrent2DPose)
  {}
  ArGPSCoordConverter(double &originLat, double &originLon, double &originAlt, ArGPS *_gps = NULL) : 
    gps(_gps),
    haveOrigin(true),
    origin(originLat, originLon, originAlt),
    mapCoords(origin),
    myGetCurrentFunc(this, &ArGPSCoordConverter::getCurrent),
    myGetCurrent2DPoseFunc(this, &ArGPSCoordConverter::getCurrent2DPose)
  {}
  ArGPSCoordConverter(ArLLACoords &originlla, ArGPS *_gps)  :
    gps(_gps),
    haveOrigin(true),
    origin(originlla),
    mapCoords(origin),
    myGetCurrentFunc(this, &ArGPSCoordConverter::getCurrent),
    myGetCurrent2DPoseFunc(this, &ArGPSCoordConverter::getCurrent2DPose)
  {}
  Ar3DPoint convert(const ArLLACoords &lla) 
  {
    double x, y, z;
    if(!haveOrigin) 
    {
      origin = lla; //ArLLACoords(lla.getLatitude(), lla.getLongitude(), lla.getAltitude());
      mapCoords = ArMapGPSCoords(origin);
      haveOrigin = true;
      ArLog::log(ArLog::Normal, "ArGPSConverter: initialized origin to %.2f, %.2f\n", lla.getLatitude(), lla.getLongitude());
    }
    mapCoords.convertLLA2MapCoords(lla, x, y, z);
    //printf("GPSConv: %.2f, %.2f -> %.2f, %.2f\n", lla.getLatitude(), lla.getLongitude(), x, y);
    return Ar3DPoint(x, y, z);
  }
  Ar3DPoint convert(double &lat, double &lon, double &alt) 
  {
    return convert(ArLLACoords(lat, lon, alt));
  }
  Ar3DPoint getCurrent() 
  {
    if(gps && gps->havePosition())
      return convert(gps->getLLA());
    return Ar3DPoint();
  }
  ArRetFunctor<Ar3DPoint> *getCurrentFunc() { return &myGetCurrentFunc; }
  ArPose getCurrent2DPose()
  {
    Ar3DPoint p = getCurrent();
    return ArPose(p.getX(), p.getY());
  }
  ArRetFunctor<ArPose> *getCurrent2DPoseFunc() { return &myGetCurrent2DPoseFunc; }
private:
  ArGPS *gps;
  bool haveOrigin;
  ArLLACoords origin;
  ArMapGPSCoords mapCoords;
  ArRetFunctorC<Ar3DPoint, ArGPSCoordConverter> myGetCurrentFunc;
  ArRetFunctorC<ArPose, ArGPSCoordConverter> myGetCurrent2DPoseFunc;
};

#endif // ifdef ARGPS_H
