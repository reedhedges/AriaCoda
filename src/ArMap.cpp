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
#include "ArExport.h"
#include "ariaOSDef.h"
#include "ariaInternal.h"
#include "ArMap.h"


#include "ArLog.h"


/**
@page MapFileFormat Map File Format

ARIA's map file format is used to store data that defines a map of a space 
in which the robot can operate.  A map file can be loaded, accessed, and 
saved using an ArMap object.

The robot map file is in ASCII text, and may be viewed or edited in any
text editor.
Map file names conventionally end in the suffix ".map".

A map file contains several types of information, including:
<ul>
 <li> Header information which includes the boundaries of the map, various
   data counts, and the resolution (in mm) at which the map was made. </li>
 <li> Optional metadata that can be used to define custom object types.
   (This is explained in greater detail below.)  </li>
 <li> Map objects, such as goals and forbidden lines. </li>
 <li> Map data: lines and points (in mm). </li>
</ul> 

A formal description of the map syntax
follows in <a href="http://www.rfc-editor.org/rfc/rfc4234.txt">augmented 
Backus-Naur Format (ABNF)</a>.
All blank lines in the map file should be ignored. As an exception to ABNF, literal
strings given below <em>are</em> case-sensitive.

<p>
A map is an introductory line (e.g. "2D-Map") followed by the metadata 
section, followed by some number of data sections:
</p>
<pre>
ARMAP                = (MapIntro NEWLINE) (MetadataSection) (*DataSection)
MapIntro             = "2D-Map" / "2D-Map-Ex" / "2D-Map-Ex<2,3,4>"
</pre>
<p>
Most of the features described below apply to all of the MapIntro values.
Exceptions are noted in the feature descriptions.
</p>
<p>
The MetadataSection section provides information about the map data, adds 
objects (Cairns) and also provides storage of application-specific/custom 
information.
</p>
<pre>
MetadataSection      = *MetadataLine
MetadataLine         = MDKey ":" *(WS MDVal) NEWLINE
MDKey                = StringID
MDVal                = Integer / Float / StringID / KeyValPair 
</pre>

<p>
Most metadata lines fall into one of two categories: a simple list of numeric tokens,
or a StringID followed by a list of either numeric tokens or a set of KeyValuePair
tokens.
</p>

<p>
The DataSections contain data that was recorded with sensors (e.g. a Laser Rangefinder
for the "DATA" section) and which represent more or less permanent, solid objects
detectable by a robot's range sensors.  (This data can be used for localization
and path planning, for instance.)  The DATA section is a collection of points
detected by a high-resolution sensor like the LRF. LINES abstracts the world into
a set of flat surfaces.
</p>
<pre>
DataSection          = (LineIntro NEWLINE *LineLine) / (PointIntro NEWLINE *PointLine)
LineLine             = XPos WS YPos WS XPos WS YPos NEWLINE
PointLine            = XPos WS YPos NEWLINE
; If MapIntro is "2D-Map":
  LineIntro            = "LINES" 
  PointIntro           = "DATA"
; If MapIntro is "2D-Map-Ex" or "2D-Map-Ex2":
  LineIntro            = "LINES" / SensorName "_LINES"
  PointIntro           = "DATA" / SensorName "_DATA"
  SensorName           = StringID
</pre>


<p>MetadataSection Details</p>

<p>
"Cairn" is a common instance of MDKey. A "Cairn" metadata entry looks like this:
</p>
<pre>
MetaDataLine         =/ Cairn  NEWLINE
Cairn                = "Cairn:" WS CairnType WS XPos WS YPos WS Theta WS InternalName WS IconName WS Label [WS TypeSpecificData]
CairnType            = StringID
XPos                 = Integer
YPos                 = Integer
Theta                = Integer
InternalName         = QuotedString
IconName             = QuotedString
Label                = QuotedString
TypeSpecificData     = *(WS MDKey)
</pre>

<p>
"MapInfo" is another common instance of MDKey. A "MapInfo" entry can describe custom
map object types for your application beyond the usual Cairn types (see above).
</p>
<pre>
MetaDataLine         =/ MapInfo NEWLINE
MapInfo              = "MapInfo:" WS MapInfoClass WS *(KeyValuePair)
MapInfoClass         = StringID
</pre>

<p>Data types:</p>
<pre>
KeyValPair           = (StringID "=" MDVal) /  QUOTE ALNUM "=" Text QUOTE
Integer              = ["-"] *1(DIGIT)
Float                = ["-"] *1(DIGIT | ".")
StringID             = *1 ALNUM     ; One or more alphanumeric characters (no whitespace)
QuotedText           = QUOTE Text QUOTE
Text                 = *(ALNUM / WS / PUNCTUATION)
DIGIT                = ("0"-"9")
ALPHA                = ("a"-"z" / "A"-"Z")
ALNUM                = ALPHA / DIGIT
WS                   = *(" ")       ; Any number of ASCII space characters (incl. 0)
QUOTE                = %d34         ; ASCII double quote mark (")
NEWLINE              = %d10         ; ASCII newline (\n)
PUNCTUATION          = %d32-%d47 / %d58-%d64 / %d92-%d96 / %d123-%d126
ANY                  = %d32-%d126   ; Any ASCII text
</pre>



<p>

In addition to Cairn and MapInfo, other Common IDs for <i>MDKey</i> are: 
  <dl>
    <dt><code>Sources</code></dt>

      <dd>If multiple sensors were used to compile the points and lines 
          in the DataSection, they are listed here.  The first sensor is 
          the default sensor; the data for it are not prefixed (i.e. 
          plain <code>MinPos</code>, <code>MaxPos</code>, <code>DATA</code>, and <code>LINES</code>).  The data for any additional 
          sensors are prefixed by the sensor name (i.e.
<i>SensorName</i><code>MinPos</code>,
          <i>SensorName</i><code>MaxPos</code>, <i>SensorName</i><code>_DATA</code>,
<i>SensorName</i><code>_LINES</code>).  
          (2D-Map-Ex and 2D-Map-Ex2 feature.)
      </dd>

    <dt><code>MinPos</code> or <i>SensorName</i><code>MinPos</code></dt>
      <dd>"Minimum" value in DATA (defines opposite corner to MaxPos of a bounding box)</dd>
    <dt><code>MaxPos</code> or <i>SensorName</i><code>MaxPos</code></dt> 
      <dd>"Maximum" value in DATA (defines opposite corner to MinPos of a bounding box)</dd>
    <dt><code>NumPoints</code> or <i>Sensor</i><code>NumPoints</code></dt>
      <dd>Number of entries in the DATA section. (Note, it is recommended that
you do not rely on this value if possible; instead simply count the number of
lines in the DATA section)</dd>
    <dt><code>LineMinPos</code> or <i>SensorName</i><code>LineMinPos</code></dt>
      <dd>"Minimum" value in LINES (defines opposite corner to LineMaxPos of a bounding box)</dd>
    <dt><code>LineMaxPos</code> or <i>SensorName</i><code>LineMaxPos</code></dt>
      <dd>"Maximum" value in LINES (defines opposite corner to LineMinPos of a bounding box)</dd>
    <dt><code>NumLines</code> or <i>SensorName</i><code>NumLines</code></dt>
      <dd>Number of entries in the LINES section. (Note, it is recommended that
you do not rely on this value if possible; instead simply count the number of
lines in the LINES section)</dd>
    <dt><code>Resolution</code> or <i>SensorName</i><code>Resolution</code></dt>
      <dd>Grid resolution of the DATA points and the LINE endpoint positions.</dd>
    <dt><code>PointsAreSorted</code> or <i>SensorName</i><code>PointsAreSorted</code></dt>
      <dd><code>true</code> if the points DATA are sorted, <code>false</code> if not.</dd>
    <dt><code>LinesAreSorted</code> or <i>SensorName</i><code>PointsAreSorted</code></dt>
      <dd><code>true</code> if the LINES data are sorted, <code>false</code> if not.</dd>
    <dt><code>Display</code> or <i>SensorName</i><code>Display</code></dt>
      <dd>Text to display in user interface etc. when referring to this sensor
data source.</dd>
    <dt><code>OriginLatLonAlt</code></dt>
      <dd>Latitude, longitude and altitude of the map's center (0, 0) point in
the WGS84 datum. (Only present in "outdoor" maps made for use with MOGS.)</dd>
    <dt><code>Cairn</code></dt>
      <dd>Defines a special object in the map with semantic meaning. See below.</dd>
    <dt><code>MapInfo</code></dt>
      <dd>Describes custom cairn types. See below.</dd>
    <dt><code>CustomInfo</code></dt>
      <dd>Placeholder for custom application data which will be maintained 
          if the map is edited by Mapper3 or MobilePlanner. (2D-Map-Ex2 feature.)</dd>
  </dl>
  New values may be added in the future, or used only by some applications.
</p>


<p>
Common <i>CairnType</i> values are: 
  <dl>
    <dt><code>Goal</code></dt> 
      <dd>A named goal. <i>Theta</i> should be ignored.  The name of the goal is provided in <i>Label</i>.</dd>
    <dt><code>GoalWithHeading</code></dt>
      <dd>A named goal. <i>Theta</i> indicates a final heading. The name of the goal is provided in <i>Label</i>.</dd>
    <dt><code>RobotHome</code></dt>
      <dd>A possible starting position of a robot.</dd>
    <dt><code>Dock</code></dt>
      <dd>A position and heading at which a docking maneuver may be initiated</dd>
    <dt><code>ForbiddenLine</code></dt>
      <dd>Specifies a line that any automatic navigation procedure should avoid crossing.
      This Cairn type has the following <i>TypeSpecificData</i>, which defines the endpoints
      of the line:
<pre>
TypeSpecificData     =/ ForbiddenLineData
ForbiddenLineData    =  XPos WS YPos WS XPos WS YPos
</pre>
      The normal Cairn pose is not used for <code>ForbiddenLine</code>.
      </dd>
    <dt><code>ForbiddenArea</code></dt>
      <dd>Specifies a rectangular area that any automatic navigation procedure should avoid entering.
      This Cairn type has the following <i>TypeSpecificData</i>, which defines the upper-left and
      lower-right opposing corners of the rectangle:
<pre>
TypeSpecificData     =/ ForbiddenAreaData
ForbiddenAreaData    =  XPos WS YPos WS XPos WS YPos
</pre>
      The normal Cairn pose for <code>ForbiddenArea</code> defines an offset of
      the geometric center of the area, plus a rotation of the  area.
      (Typically, <i>XPos</i> and <i>YPos</i> will always be <code>0</code> for <code>ForbiddenArea</code>, but <i>Theta</i> may be
       used to provide the rotation of the rectangular area).
      </dd>
   </dl>

</p>

<p>The <i>InternalName</i> and <i>IconName</i> tokens in <i>Cairn</i> data 
are not currently used. Typically, <i>InternalName</i> is simply an empty quoted 
string ("") and <i>IconName</i> is the placeholder value <code>"ICON"</code>.  
You should preserve them when reading and writing map files though, as they may 
be used in the future.  
</p>
<p>Note, It is be better to calculate maximum, minimum, and number of 
points or lines based on the data in the map, if possible, rather than 
relying on the metadata header.
</p>
<p>So what the heck is a "cairn"?   The word is from the Scottish Gaelic, Old Irish
and Welsh "carn" and Middle English "carne".  A cairn is a pile of stones,
placed in the landscape as a memorial, navigation aid, or other marker. So we
use it to indicate a semantically meaningful object placed at some point by the 
human mapmaker (rather than something detectable by the robot).
</p>
<p>
  Currently used <i>MapInfoClass</i> keywords include:
  <dl>
    <dt><code>GoalType</code></dt> <dd>define a custom goal subtype</dd>
    <dt><code>DockType</code></dt> <dd>define a custom dock subtype</dd>
    <dt><code>LocationType</code></dt> <dd>define a custom other generic poses on the map</dd>
    <dt><code>BoundaryType</code></dt> <dd>define a custom line on the map</dd>
    <dt><code>SectorType</code></dt> <dd>defines a custom rectangular area (which may be rotated)</dd>
  </dl>
  The following ParamNames are valid for all <i>MapInfoClass</i> keywords:
  <dl>
    <dt><code>Name=</code><i>Text</i></dt> <dd>Name of the type that is being defined.
    <dt><code>Label=</code><i>Text</i></dt> <dd>Label that is displayed for the type in a GUI, etc.</dd>
    <dt><code>Desc=</code><i>Text</i></dt> <dd>Longer description of the type that is displayed in a GUI, etc.</dd>
  </dl>
For more information about the use of <code>MapInfo</code> metadata, see the discussion above.
</p>

@section MapCustomObjects Defining Custom Map Objects
 * 
 * In addition to the standard map object types,  is also possible to define 
 * additional types of objects in a map file using 
 * the "MapInfo" metadata section.  For example, if you wished to program
 * some special behavior that would only occur upon reaching certain goals,
 * you could define a new goal type as follows:
 * <pre>
 *    MapInfo: GoalType Name=SpecialGoal "Label=Special" "Desc=Doing special stuff" Heading=Required Shape=VBars "Color0=0xff0000"
 * </pre>
 * The new goal type will appear be available in Mapper3 and MobilePlanner in 
 * a drop-down menu. Instances in the map will also be displayed by MobileEyes. 
 * 
 * 
 * Please read the following information carefully if you plan to use this 
 * feature.
 * 
 * Each MapInfo line is of the format:
 * <pre>
 *       MapInfo: <i>Keyword</i> ([ParamName=ParamValue] )*  ; A keyword followed by a space-separated list of Key=Value or "Key=Value" tokens.
 * </pre>
 *
 * </p><p>
 * The following values for <i>Keyword</i> are currently supported:
 *  - <code>GoalType</code> : defines a goal subtype
 *  - <code>DockType</code> : defines a dock subtype
 *  - <code>LocationType</code> : defines another kind of point in the map
 *  - <code>BoundaryType</code> : defines a line object in the map
 *  - <code>SectorType</code> : defines a rectangular area of some kind in the map (which may be rotated)
 *  - <code>ArgDesc</code>: defines a parameter for the preceding type (2D-Map-Ex2 feature)
 * .
 * 
 * The available parameters depend on the Keyword.  Unless otherwise specified,
 * parameters are optional.  
 * 
 * If a parameter value contains a space, then both the parameter name and
 * value must be enclosed within quotes.  For example:
 * <pre>
 *      "Label=Normal Goal"
 * </pre>
 * Neither the name nor the value can contain the special characters #,
 * %, or ".
 * 
 * The following ParamNames are valid for all keywords except
 * <code>ArgDesc</code>:
 *  - <code>Name=</code><i>String</i> : Text name of the type that is being defined.  (Required.)
 *  - <code>Label=</code><i>String</i> : Label that is displayed for the type in popup menus, etc.    
 *  - <code>Desc=</code><i>String</i> : Description of the type that is displayed in tool tips, etc.
 *  - <code>Vis=[AlwaysOn|DefaultOn|DefaultOff|AlwaysOff]</code> : Specifies the 
 *    visibility of the associated item type. The default is
 *    <code>DefaultOn</code>. 
 *    (This parameter is currently primarily supported for <code>IsData=1</code> items only.
 *    See <code>BoundaryType</code>.  In particular, if <code>DefaultOff</code> is specified for a 
 *    non-data-item, there currently is no way in Mapper3, MobilePlanner or MobileEyes to
 *    subsequently show it.)
 * .
 * 
 * For <code>GoalType</code>, <code>DockType</code>, and
 * <code>LocationType</code>, the following ParamNames are 
 * also supported:
 * <ul>
 *  <li> <code>Shape=</code>[Plain|Cross|HBars|Triangle|T|U|VBars|DockTarget|Cart] : Shape of the icon used
 *    to draw the pose. (The default is <code>Plain</code>.)   
 *    <ul>
 *      <li> <code>Plain</code>: The default shape, a filled square
 *      <li> <code>Cross</code>: A cross shape
 *      <li> <code>HBars</code>: A square shape containing horizontal bars or stripes
 *      <li> <code>Label</code>: A location that simply marks a text label expected to be
 *        used with <code>LocationType</code> and a fixed size font (<code>FtFix</code>, see below)
 *      <li> <code>Triangle</code>: A Triangle
 *      <li> <code>T</code>: A "T" shape
 *      <li> <code>U</code>: A "U" shape
 *      <li> <code>VBars</code>: A square shape containing vertical bars or stripes
 *      <li> <code>DockTarget</code>: A special icon that resembles the physical dock / charger
 *      <li> <code>Cart</code>: A special icon that resembles the physical cart
 *    </ul>
    </li>
 *  <li> <code>Size=</code><i>Integer</i> : Width/height of the displayed icon in mm.  
 *  <li> <code>Color</code><i>0|1|2</i><code>=</code><i>ColorVal</i> : Colors with which to draw the icon.
 *    (In general, <code>Color0</code> is the primary icon color, <code>Color1</code> is the heading 
 *    color, and <code>Color2</code> is the background/outline color.)   <i>ColorVal</i>
 *    is a hexadecimal number starting with <code>0x</code>, and followed
 *    by two digits for the red component, two digits for green, and two
 *    digits for blue. For example, <code>0xff00ff</code>.
 *  <li> <code>FtFix=</code><i>Integer</i> : A boolean.  Set to 0 if the label font should be scaled 
 *    when the map is zoomed (the default) set to 1 if the label should remain
 *    a fixed size.  Note that the label font will only be scaled if the platform 
 *    supports it.  Also note that if this is set to 1, then the <code>FtSize</code> <em>must</em>
 *    also be specified.
 *  <li> <code>FtSize=</code><i>Integer</i> : Point size of the displayed font.  This will vary greatly 
 *    depending on the value of <code>FtFix</code>.  If the font is scaled, then
 *    the point size is expressed in mm and <em>must</em> be a multiple 
 *    of 100. If the font is fixed, then the point size is in pixels.  
 *    (If the font should be scaled, but the platform does not support it,
 *    then a best guess is made as to the desired size.)
 *  </ul>
 *
 * In addition, the following ParamName is supported only for 
 * <code>GoalType</code>s:
  <ul>
 *  <li> <code>Heading=[Required|Optional|Never]</code> : Whether a heading is required
 *    to be given with this goal, is optional, or is irrelevant.
 * </ul>
 *
 * For <code>BoundaryType</code>, the following ParamNames are also supported:
 * <ul>
 *  <li> <code>NameRequired=</code><i>0|1</i> : Whether the item must be named
 *  (1=true, 0=false)
 *  <li> <code>Shape=[Plain|Measure|Dash] </code> Shape or decoration of the line. 
 *    (The default is <code>Plain</code>.)  
 *  <li> <code>Color</code><i>0|1|2</i><code>=</code><i>ColorVal</i>: Color with which to draw the line.
 *  <li> <code>IsData=</code><i>0|1</i> : Set to 1 to indicate that the item is inherently 
 *    part of the map data.  The default is 0 to indicate user-created
 *    items. 
 *  <li> <code>FtFix=</code><i>Integer</i> : A boolean.  Set to 0 if the label font should be scaled 
 *    when the map is zoomed (the default) set to 1 if the label should remain
 *    a fixed size.  Note that the label font will only be scaled if the platform 
 *    supports it.  Also note that if this is set to 1, then the <code>FtSize</code> <em>must</em>
 *    also be specified.
 *  <li> <code>FtSize=</code><i>Integer</i> : Point size of the displayed font.  This will vary greatly 
 *    depending on the value of <code>FtFix</code>.  If the font is scaled, then
 *    the point size is expressed in mm and <em>must</em> be a multiple 
 *    of 100. If the font is fixed, then the point size is in pixels.  
 *    (If the font should be scaled, but the platform does not support it,
 *    then a best guess is made as to the desired size.)
 * </ul>
 *
 * For <code>SectorType</code>, the following ParamNames are also supported:
 * <ul>
 *  <li> <code>NameRequired=</code><i>0|1</i> : Whether the item must be named
 *  <li> <code>Shape=[Plain|
 *                 Arrow|FillArrow|GradArrow|
 *                 Door|
 *                 Elevator|
 *                 Stairs|
 *                 Circle|FillCircle|
 *                 Cross|FillCross|
 *                 Diamond|FillDiamond|
 *                 Octagon|FillOctagon|
 *                 PrefWayDriveOnLeft|FillPrefWayDriveOnLeft|GradPrefWayDriveOnLeft|
 *                 PrefWayDriveOnRight|FillPrefWayDriveOnRight|GradPrefWayDriveOnRight|
 *                 Star|FillStar|
 *                 Triangle|FillTriangle|
 *                 TwoWayDriveOnLeft|FillTwoWayDriveOnLeft|GradTwoWayDriveOnLeft|
 *                 TwoWayDriveOnRight|FillTwoWayDriveOnRight|GradTwoWayDriveOnRight|
 *                 Dash] 
 *    </code> 
 *		Shape of the icon used to draw the rectangle. The default is <code>Plain</code>.
 *    The <code>Dash</code> shape means that the rectangle is drawn with a dashed outline.  
 *    The <code>Door</code>, <code>Elevator</code>, and <code>Stairs</code> shapes designate
 *    a special icon to be drawn in the rectangle.  All other shapes refer to a repeating
 *    pattern of geometric shapes.  If the <code>Fill</code> prefix is specified, then the 
 *    geometric shape is filled with Color1.  If the <code>Grad</code> prefix is specified
 *    (when available), then the geometric shape is filled with a gradient, shaded pattern 
 *    from Color0 to Color1.
 *  <li> <code>Color</code><i>0|1</i><code>=</code><i>ColorVal</i>: Colors with which to draw the 
 *     rectangle. (In general, <code>Color0</code> is the primary rectangle color, 
 *     <code>Color1</code> is the shape/icon color.) 
 * </ul>
 *  
 * 
 * <i>Important Note</i>: if a map defines special <code>GoalType</code> or
 * <code>DockType</code> items, 
 * then it must define <b>all</b> possible Cairn types, including the 
 * default "Goal", "GoalWithHeading", and "Dock" types if you want those
 * types to remain available.
 *
 * The MapInfo ArgDesc lines may be used to define one or more configurable
 * parameters for a custom map object type.  (2D-Map-Ex2 feature.)  
 * The ArgDesc must appear after its parent type definition and can contain 
 * the following parameters:
 * <ul>
 *   <li> <code>Name=</code><i>String</i> : The text name of the parameter being defined.  This 
 *     must be the first item in the line (after ArgDesc) and must be unique 
 *     for the given parent. (Required)
 *   <li> <code>Parent=</code><i>String</i> : The text name of the parent map object type (e.g.
 *     <code>SpecialGoal</code>, <code>RobotHome</code>, ...).  This must be the second item in the line,
 *     immediately following the Name.  (Required)
 *   <li> <code>Priority=</code><i>Important|Normal|Detailed</i> : The priority of the parameter
 *     (Required)
 *   <li> <code>Type=</code><i>int|double|string|bool</i> : The type of the parameter's value 
 *     (Required)
 *   <li> <code>Value=</code><i>String</i> : An optional initial value for the parameter (as 
 *     appropriate for the type)
 *   <li> <code>Min=</code><i>Number</i> : An optional minimum value for the parameter; valid only
 *     if Type=int or Type=double
 *   <li> <code>Max=</code><i>Number</i> : An optional maximum value for the parameter; valid only
 *     if Type=int or Type=double
 *   <li> <code>Display=</code><i>String</i> : An optional display hint that may be used by the 
 *     client to improve display of the parameter.  See ArConfigArg::setDisplayHint()
 *     for a list of the currently supported display hints.
 * </ul>
 * 
 * If a cairn instance with parameters is defined in the map file, then the 
 * parameter values are stored in a special CairnInfo Metadatasection line.
 * (2D-Map-Ex2 feature.)
 *
**/

AREXPORT ArMap::ArMap(const char *baseDirectory,
		                  bool addToGlobalConfig, 
		                  const char *configSection,
		                  const char *configParam,
		                  const char *configDesc,
                      bool ignoreEmptyFileName,
                      ArPriority::Priority priority,
                      const char *tempDirectory,
		      int configProcessFilePriority) :

  myMutex(),
  myBaseDirectory((baseDirectory != NULL) ? baseDirectory : ""),
  myFileName(),
  myReadFileStat(),

  myConfigParam((configParam != NULL) ? configParam : ""),
  myIgnoreEmptyFileName(ignoreEmptyFileName),
  myIgnoreCase(false),

  myConfigProcessedBefore(false),
  myConfigMapName(),
  myForceMapLoad(false),

  myCurrentMap(new ArMapSimple(baseDirectory, tempDirectory)),
  myLoadingMap(NULL),

  myIsQuiet(false),

  myProcessFileCB(this, &ArMap::processFile)
{
  myMutex.setLogName("ArMap::myMutex");
  myConfigMapName[0] = '\0';
  myProcessFileCB.setName("ArMap");

  if (addToGlobalConfig)
  {
    ArLog::log(ArLog::Verbose,
               "ArMap: adding parameter \"%s\" to ArConfig section \"%s\"",
               configParam, configSection);

    myConfigMapName[0] = '\0';
    myConfigProcessedBefore = false;
    myForceMapLoad = false;
    const char *displayHint = "RobotFile:Map Files (*.map)|*.map";

    Aria::getConfig()->addParam(ArConfigArg(configParam, 
                                            myConfigMapName, 
					                                  configDesc,
					                                  sizeof(myConfigMapName)), 
				                        configSection, 
				                        priority,
                                displayHint);
    Aria::getConfig()->addProcessFileWithErrorCB(&myProcessFileCB, 
						 configProcessFilePriority);
  }

} // end ctor
   
AREXPORT ArMap::ArMap(const ArMap &other) :
  myMutex(),
  myBaseDirectory((other.getBaseDirectory() != NULL) ? other.getBaseDirectory() : ""),
  myFileName((other.getFileName() != NULL) ? other.getFileName() : ""),
  myReadFileStat(other.getReadFileStat()),

  myConfigParam(""),
  myIgnoreEmptyFileName(false),
  myIgnoreCase(false),

 // myMapChangedHelper(NULL),

  myConfigProcessedBefore(false), // TODO This is not always init in ArMap
  myConfigMapName(),
  myForceMapLoad(false),


  myCurrentMap(new ArMapSimple(*other.myCurrentMap)),
  myLoadingMap(NULL),

  myIsQuiet(false),

  //myCurrentMapChangedCB(this, &ArMap::handleCurrentMapChanged),
  myProcessFileCB(this, &ArMap::processFile)
{
  myMutex.setLogName("ArMap::myMutex");
  myConfigMapName[0] = '\0';
  myProcessFileCB.setName("ArMap");

//  myMapChangedHelper = new ArMapChangedHelper();

//  myCurrentMap->addMapChangedCB(&myCurrentMapChangedCB);

  // Do not add copy to config

} // end copy ctor

AREXPORT ArMap &ArMap::operator=(const ArMap &other)
{
  if (this != &other) {
  
    lock();
   
    myBaseDirectory = ((other.getBaseDirectory() != NULL) ? 
                              other.getBaseDirectory() : "");
    myFileName      = ((other.getFileName() != NULL) ? 
                              other.getFileName() : "");
    myReadFileStat  = other.getReadFileStat();


    /**
    myConfigParam 
    myIgnoreEmptyFileName(false),
    myIgnoreCase(false),

    myMapChangedHelper(NULL),

    myConfigProcessedBefore(false), // TODO This is not always init in ArMap
    myConfigMapName(),
    myForceMapLoad(false),
    **/

    *myCurrentMap = *other.myCurrentMap;
    
    delete myLoadingMap;
    myLoadingMap = NULL;
    
    if (other.myLoadingMap != NULL) {
      myLoadingMap = new ArMapSimple(*other.myLoadingMap);
    }

    unlock();

    /**
    myIsQuiet(false),

    myCurrentMapChangedCB(this, &ArMap::handleCurrentMapChanged),
    myProcessFileCB(this, &ArMap::processFile)
    **/
  }
  return *this;

} // end operator=


AREXPORT ArMap::~ArMap(void)
{ 
  delete myLoadingMap;
  //myLoadingMap = NULL;

  delete myCurrentMap;
  //myCurrentMap = NULL;

//  delete myMapChangedHelper;
//  myMapChangedHelper = NULL;

} // end dtor 


AREXPORT ArMapInterface *ArMap::clone()
{
  // TODO: There is currently an issue with creating another ArMap (and I 
  // can't remember exactly what it is).  Creating a simple copy appears
  // to be sufficient at the moment.

  //ArMap *copy = new ArMap(*this);
  ArMapSimple *copy = new ArMapSimple(*myCurrentMap);

  return copy;
}


AREXPORT bool ArMap::set(ArMapInterface *other)
{
  if (other == NULL) {
    return false;
  }
  // TODO: What about mapChanged and times?

  bool isSuccess = myCurrentMap->set(other);
  return isSuccess;

} // end method set


AREXPORT void ArMap::clear()
{
  myCurrentMap->clear();
}


AREXPORT std::list<std::string> ArMap::getScanTypes() const
{
  return myCurrentMap->getScanTypes();
}
   
AREXPORT bool ArMap::setScanTypes(const std::list<std::string> &scanTypeList)
{
  return myCurrentMap->setScanTypes(scanTypeList);
}


AREXPORT struct stat ArMap::getReadFileStat() const
{
  return myCurrentMap->getReadFileStat();
}
  
AREXPORT void ArMap::addPreWriteFileCB(ArFunctor *functor,
                                       ArListPos::Pos position)
{
  myCurrentMap->addPreWriteFileCB(functor, position);

} // end method addPreWriteFileCB

AREXPORT void ArMap::remPreWriteFileCB(ArFunctor *functor)
{
  myCurrentMap->remPreWriteFileCB(functor);

} // end method remPreWriteFileCB

AREXPORT void ArMap::addPostWriteFileCB(ArFunctor *functor,
                                        ArListPos::Pos position)
{
  myCurrentMap->addPostWriteFileCB(functor, position);

} // end method addPostWriteFileCB

AREXPORT void ArMap::remPostWriteFileCB(ArFunctor *functor)
{
  myCurrentMap->remPostWriteFileCB(functor);

} // end method remPostWriteFileCB

AREXPORT void ArMap::addPreParseFileCB(ArFunctor1<std::string> *functor)
{
  //myCurrentMap->addPreParseFileCB(functor);
  myPreParseCBList.push_back(functor);

} // end method addPreParseFileCB

AREXPORT void ArMap::remPreParseFileCB(ArFunctor1<std::string> *functor)
{
  //myCurrentMap->remPreParseFileCB(functor);
  myPreParseCBList.remove(functor);
} // end method remPreParseFileCB

AREXPORT bool ArMap::readFile(const char *fileName) { 
  return readFile(fileName, NULL, 0, NULL, 0); 
}

AREXPORT bool ArMap::readFile(const char *fileName, char *errorBuffer, size_t errorBufferLen) { 
  return readFile(fileName, errorBuffer, errorBufferLen, NULL, 0); 
}

AREXPORT bool ArMap::readFile(const char *fileName, 
			                               char *errorBuffer, 
                                     size_t errorBufferLen,
                                     unsigned char *md5DigestBuffer,
                                     size_t md5DigestBufferLen)
{
  // TODO 
  ArLog::log(ArLog::Normal,
             "ArMap::readFile() %s",
             fileName);

  lock();

  // Little worried that this is not going to work... Might want to do more of a 
  // copy ctor type of action.... (This would leave the actual memory address intact.)

  if (myLoadingMap != NULL) {
    delete myLoadingMap;
    myLoadingMap = NULL;
  }
  myLoadingMap = new ArMapSimple(myBaseDirectory.c_str(),
                                 myCurrentMap->getTempDirectory(), 
                                 "ArMapLoading::myMutex");
  myLoadingMap->setQuiet(myIsQuiet);

  std::string realFileName = ArMapInterface::createRealFileName
                                                  (myBaseDirectory.c_str(),
                                                   fileName,
                                                   myIgnoreCase);
  myLoadingMap->setSourceFileName(NULL, // TODO
                                  realFileName.c_str());
  for (std::list<ArFunctor1<std::string>*>::iterator iter = myPreParseCBList.begin();
       iter != myPreParseCBList.end(); iter++) {
    myLoadingMap->addPreParseFileCB(*iter);
  }

  bool isSuccess = myLoadingMap->readFile(fileName, 
                                          errorBuffer, 
                                          errorBufferLen,
                                          md5DigestBuffer,
                                          md5DigestBufferLen);

  if (isSuccess) {

    ArTime copyTime;

    *myCurrentMap = *myLoadingMap;
    //myCurrentMap->set(myLoadingMap);

    int elapsed = copyTime.mSecSince();

    ArLog::log(ArLog::Normal,
               "ArMap::readFile() took %i msecs to copy loading map",
               elapsed);

    myFileName = fileName;
    myReadFileStat = myCurrentMap->getReadFileStat();

    delete myLoadingMap;
    myLoadingMap = NULL;
  }

  ArLog::log(myCurrentMap->getMapChangedLogLevel(),
             "ArMap::readFile() Calling mapChanged()");	
  mapChanged();
  ArLog::log(myCurrentMap->getMapChangedLogLevel(),
             "ArMap::readFile() Finished mapChanged()");

  unlock();
  return isSuccess;

} // end method readFile


AREXPORT bool ArMap::writeFile(const char *fileName, 
                                     bool internalCall,
                                     unsigned char *md5DigestBuffer,
                                     size_t md5DigestBufferLen,
                                     time_t fileTimestamp)
{ 
  if (!internalCall) {
    ArLog::log(ArLog::Verbose,
	       "ArMap::writeFile(%s) about to lock",
	       fileName);
    lock();
    ArLog::log(ArLog::Verbose,
	       "ArMap::writeFile(%s) locked",
	       fileName);

  }

  bool isSuccess = myCurrentMap->writeFile(fileName, 
                                           true, // ?? TODO
                                           md5DigestBuffer,
                                           md5DigestBufferLen,
                                           fileTimestamp);
  
  if (isSuccess) {
    myReadFileStat = myCurrentMap->getReadFileStat();
  }

  if (!internalCall)
    unlock();
  return true;

} // end method writeFile
  

AREXPORT bool ArMap::calculateChecksum(unsigned char *md5DigestBuffer,
                                              size_t md5DigestBufferLen)
{
  return myCurrentMap->calculateChecksum(md5DigestBuffer,
                                         md5DigestBufferLen);
}


AREXPORT const char *ArMap::getBaseDirectory(void) const
{ 
  return myBaseDirectory.c_str();

} // end method getBaseDirectory


AREXPORT const char *ArMap::getFileName(void) const 
{ 
  return myFileName.c_str();

} // end method getFileName


AREXPORT void ArMap::setIgnoreEmptyFileName(bool ignore)
{ 
  myIgnoreEmptyFileName = ignore;
  myCurrentMap->setIgnoreEmptyFileName(ignore); // ?? TODO

} // end method setIgnoreEmptyFileName

AREXPORT bool ArMap::getIgnoreEmptyFileName(void)
{ 
  return myIgnoreEmptyFileName;

} // end method getIgnoreEmptyFileName

AREXPORT void ArMap::setIgnoreCase(bool ignoreCase)
{ 
  myIgnoreCase = ignoreCase;
  myCurrentMap->setIgnoreCase(ignoreCase); // ?? TODO

} // end method setIgnoreCase

AREXPORT bool ArMap::getIgnoreCase(void) 
{ 
  return myIgnoreCase;

} // end method getIgnoreCase


AREXPORT void ArMap::setBaseDirectory(const char *baseDirectory)
{ 
  if (baseDirectory != NULL) {
    myBaseDirectory = baseDirectory;
  }
  else {
    myBaseDirectory = "";
  }
  myCurrentMap->setBaseDirectory(baseDirectory); // ?? TODO

} // end method setBaseDirectory


AREXPORT const char *ArMap::getTempDirectory(void) const
{
  return myCurrentMap->getTempDirectory();
}

AREXPORT void ArMap::setTempDirectory(const char *tempDirectory)
{
  myCurrentMap->setTempDirectory(tempDirectory);
}


AREXPORT void ArMap::setSourceFileName(const char *sourceName,
                                              const char *fileName,
                                              bool isInternalCall)
{
  if (isInternalCall) {
    lock();
  }
  myCurrentMap->setSourceFileName(sourceName, fileName, true);
  if (isInternalCall) {
    unlock();
  }
}


AREXPORT bool ArMap::getMapId(ArMapId *mapIdOut,
                                     bool isInternalCall)
{
  if (isInternalCall) {
    lock();
  }
  bool isSuccess = myCurrentMap->getMapId(mapIdOut, true);
  if (isInternalCall) {
    unlock();
  }
  return isSuccess;
}
  
  
AREXPORT ArArgumentBuilder *ArMap::findMapObjectParams
                                          (const char *mapObjectName)
{
  return myCurrentMap->findMapObjectParams(mapObjectName);
}

AREXPORT bool ArMap::setMapObjectParams(const char *mapObjectName,
                                        ArArgumentBuilder *params,
                                        ArMapChangeDetails  *changeDetails)
{
  return myCurrentMap->setMapObjectParams(mapObjectName, 
                                          params,
                                          changeDetails);
}


AREXPORT std::list<ArArgumentBuilder *> *ArMap::getRemainder()
{
  return myCurrentMap->getRemainder();
}

AREXPORT void ArMap::setQuiet(bool isQuiet)
{ 
  myIsQuiet = isQuiet;
  myCurrentMap->setQuiet(isQuiet);

} // end method setQuiet
	

AREXPORT void ArMap::mapChanged(void)
{ 
  myCurrentMap->mapChanged();

} // end method mapChanged

AREXPORT void ArMap::mapChanged(bool invokePathPlanningCB,
                                bool invokeLocalizationCB)
{
  myCurrentMap->mapChanged(invokePathPlanningCB, invokeLocalizationCB);

} // end method mapChanged


AREXPORT void ArMap::addMapChangedCB(ArFunctor *functor, 
				     int position)
{ 
  myCurrentMap->addMapChangedCB(functor, position);

} // end method addMapChangedCB

AREXPORT void ArMap::remMapChangedCB(ArFunctor *functor)
{ 
  myCurrentMap->remMapChangedCB(functor);

} // end method remMapChangedCB

AREXPORT void ArMap::addMapChangedPathPlanningCB(ArFunctor *functor,
                     int position)
{
  myCurrentMap->addMapChangedPathPlanningCB(functor, position);

} // end method addMapChangedPathPlanningCB

AREXPORT void ArMap::remMapChangedPathPlanningCB(ArFunctor *functor)
{
  myCurrentMap->remMapChangedPathPlanningCB(functor);

} // end method remMapChangedPathPlanningCB

AREXPORT void ArMap::addMapChangedLocalizationCB(ArFunctor *functor,
                     int position)
{
  myCurrentMap->addMapChangedLocalizationCB(functor, position);

} // end method addMapChangedPathPlanningCB

AREXPORT void ArMap::remMapChangedLocalizationCB(ArFunctor *functor)
{
  myCurrentMap->remMapChangedLocalizationCB(functor);

} // end method remMapChangedPathPlanningCB

AREXPORT void ArMap::addPreMapChangedCB(ArFunctor *functor,
					int position)
{ 
  myCurrentMap->addPreMapChangedCB(functor, position);

} // end method addPreMapChangedCB

AREXPORT void ArMap::remPreMapChangedCB(ArFunctor *functor)
{ 
  myCurrentMap->remPreMapChangedCB(functor);

} // end method remPreMapChangedCB




AREXPORT void ArMap::setMapChangedLogLevel(ArLog::LogLevel level)
{ 
  myCurrentMap->setMapChangedLogLevel(level);

} // end method setMapChangedLogLevel

AREXPORT ArLog::LogLevel ArMap::getMapChangedLogLevel(void)
{ 
  return myCurrentMap->getMapChangedLogLevel();

} // end method getMapChangedLogLevel


AREXPORT int ArMap::lock()
{ 
  return myMutex.lock();

} // end method lock

AREXPORT int ArMap::tryLock()
{ 
  return myMutex.tryLock();

} // end method tryLock

AREXPORT int ArMap::unlock()
{ 
  return myMutex.unlock();

} // end method unlock

// ---------------------------------------------------------------------------
// ArMapInfoInterface
// ---------------------------------------------------------------------------

AREXPORT std::list<ArArgumentBuilder *> *ArMap::getInfo(const char *infoName)
{ 
  return myCurrentMap->getInfo(infoName);

} // end method getInfo

AREXPORT std::list<ArArgumentBuilder *> *ArMap::getInfo(int infoType)
{ 
  return myCurrentMap->getInfo(infoType);

} // end method getInfo

AREXPORT std::list<ArArgumentBuilder *> *ArMap::getMapInfo(void)
{ 
  return myCurrentMap->getInfo(ArMapInfo::MAP_INFO_NAME);

} // end method getMapInfo
   
AREXPORT int ArMap::getInfoCount() const
{
  return myCurrentMap->getInfoCount();
}

AREXPORT std::list<std::string> ArMap::getInfoNames() const
{
  return myCurrentMap->getInfoNames();
}

AREXPORT bool ArMap::setInfo(const char *infoName,
						                 const std::list<ArArgumentBuilder *> *infoList,
                             ArMapChangeDetails *changeDetails)
{ 
  return myCurrentMap->setInfo(infoName, infoList, changeDetails);

} // end method setInfo


AREXPORT bool ArMap::setInfo(int infoType,
						                        const std::list<ArArgumentBuilder *> *infoList,
                                    ArMapChangeDetails *changeDetails)
{ 
  return myCurrentMap->setInfo(infoType, infoList, changeDetails);

} // end method setInfo

AREXPORT bool ArMap::setMapInfo(const std::list<ArArgumentBuilder *> *mapInfo,
                                ArMapChangeDetails *changeDetails)
{ 
  return myCurrentMap->setInfo(ArMapInfo::MAP_INFO_NAME, mapInfo, changeDetails);
 
} // end method setMapInfo


AREXPORT void ArMap::writeInfoToFunctor
				(ArFunctor1<const char *> *functor, 
			        const char *endOfLineChars)
{ 
  return myCurrentMap->writeInfoToFunctor(functor, endOfLineChars);

} // end method writeInfoToFunctor


AREXPORT const char *ArMap::getInfoName(int infoType)
{ 
  return myCurrentMap->getInfoName(infoType);

} // end method getInfoName

// ---------------------------------------------------------------------------
// ArMapObjectsInterface
// ---------------------------------------------------------------------------

AREXPORT ArMapObject *ArMap::findFirstMapObject(const char *name, 
                                                      const char *type,
                                                      bool isIncludeWithHeading)
{ 
  return myCurrentMap->findFirstMapObject(name, type, isIncludeWithHeading);

} // end method findFirstMapObject


AREXPORT ArMapObject *ArMap::findMapObject(const char *name, 
				                                          const char *type,
                                                  bool isIncludeWithHeading)
{ 
  return myCurrentMap->findFirstMapObject(name, type, isIncludeWithHeading);

} // end method findMapObject

AREXPORT std::list<ArMapObject *> ArMap::findMapObjectsOfType
                                                (const char *type,
                                                 bool isIncludeWithHeading)
{
  return myCurrentMap->findMapObjectsOfType(type, isIncludeWithHeading);
}

AREXPORT std::list<ArMapObject *> *ArMap::getMapObjects(void)
{ 
  return myCurrentMap->getMapObjects();

} // end method getMapObjects


AREXPORT void ArMap::setMapObjects
                              (const std::list<ArMapObject *> *mapObjects,
                               bool isSortedObjects, 
                               ArMapChangeDetails *changeDetails) 
{ 
  return myCurrentMap->setMapObjects(mapObjects, isSortedObjects, changeDetails);

} // end method setMapObjects


AREXPORT void ArMap::writeObjectsToFunctor(ArFunctor1<const char *> *functor, 
			                                     const char *endOfLineChars,
                                           bool isOverrideAsSingleScan,
                                           const char *maxCategory)
{ 
  myCurrentMap->writeObjectsToFunctor(functor, 
                                      endOfLineChars, 
                                      isOverrideAsSingleScan,
                                      maxCategory);

} // end method writeObjectsToFunctor


AREXPORT void ArMap::writeObjectListToFunctor
                                (ArFunctor1<const char *> *functor, 
			                           const char *endOfLineChars)
{ 
  myCurrentMap->writeObjectListToFunctor(functor, endOfLineChars);

} // end method writeObjectListToFunctor

// ---------------------------------------------------------------------------
// ArMapScanInterface
// ---------------------------------------------------------------------------

AREXPORT bool ArMap::hasOriginLatLongAlt()
{ 
  return myCurrentMap->hasOriginLatLongAlt();

} // end method hasOriginLatLongAlt

AREXPORT ArPose ArMap::getOriginLatLong()
{ 
  return myCurrentMap->getOriginLatLong();

} // end method getOriginLatLong

AREXPORT double ArMap::getOriginAltitude()
{ 
  return myCurrentMap->getOriginAltitude();

} // end method getOriginAltitude

AREXPORT void ArMap::setOriginLatLongAlt(bool hasOriginLatLong,
                                         const ArPose &originLatLong,
                                         double altitude,
                                         ArMapChangeDetails *changeDetails)

{
  myCurrentMap->setOriginLatLongAlt(hasOriginLatLong, 
                                    originLatLong, 
                                    altitude,
                                    changeDetails);

} // end method setOriginLatLongAlt

AREXPORT void ArMap::writeSupplementToFunctor(ArFunctor1<const char *> *functor, 
			                                        const char *endOfLineChars)
{ 
  myCurrentMap->writeSupplementToFunctor(functor, endOfLineChars);

} // end method writeSupplementToFunctor


// ---------------------------------------------------------------------------
// ArMapScanInterface
// ---------------------------------------------------------------------------

AREXPORT const char *ArMap::getDisplayString(const char *scanType)
{
  return myCurrentMap->getDisplayString(scanType);
}

AREXPORT std::vector<ArPose> *ArMap::getPoints(const char *scanType)
{ 
  return myCurrentMap->getPoints(scanType);
 
} // end method getPoints

AREXPORT std::vector<ArLineSegment> *ArMap::getLines(const char *scanType)
{ 
  return myCurrentMap->getLines(scanType);

} // end method getLines

AREXPORT ArPose ArMap::getMinPose(const char *scanType)
{ 
  return myCurrentMap->getMinPose(scanType);

} // end method getMinPose

AREXPORT ArPose ArMap::getMaxPose(const char *scanType)
{ 
  return myCurrentMap->getMaxPose(scanType);

} // end method getMaxPose

AREXPORT int ArMap::getNumPoints(const char *scanType)
{ 
  return myCurrentMap->getNumPoints(scanType);

} // end method getNumPoints

AREXPORT ArPose ArMap::getLineMinPose(const char *scanType)
{ 
  return myCurrentMap->getLineMinPose(scanType);

} // end method getLineMinPose

AREXPORT ArPose ArMap::getLineMaxPose(const char *scanType)
{ 
  return myCurrentMap->getLineMaxPose(scanType);

} // end method getLineMaxPose

AREXPORT int ArMap::getNumLines(const char *scanType)
{ 
  return myCurrentMap->getNumLines(scanType);

} // end method getNumLines

AREXPORT int ArMap::getResolution(const char *scanType)
{ 
  return myCurrentMap->getResolution(scanType);

} // end method getResolution


AREXPORT bool ArMap::isSortedPoints(const char *scanType) const
{
  return myCurrentMap->isSortedPoints(scanType);
}
AREXPORT bool ArMap::isSortedLines(const char *scanType) const
{
  return myCurrentMap->isSortedLines(scanType);
}

AREXPORT void ArMap::setPoints(const std::vector<ArPose> *points,
                               const char *scanType,
                               bool isSorted,
                               ArMapChangeDetails *changeDetails)
{ 
  myCurrentMap->setPoints(points, scanType, isSorted, changeDetails);

} // end method setPoints

AREXPORT void ArMap::setLines(const std::vector<ArLineSegment> *lines,
                              const char *scanType,
                              bool isSorted,
                              ArMapChangeDetails *changeDetails)
{ 
  myCurrentMap->setLines(lines, scanType, isSorted, changeDetails);

} // end method setLines

AREXPORT void ArMap::setResolution(int resolution,
                                   const char *scanType,
                                   ArMapChangeDetails *changeDetails)
{ 
  myCurrentMap->setResolution(resolution, scanType, changeDetails);
 
} // end method setResolution



AREXPORT void ArMap::writeScanToFunctor(ArFunctor1<const char *> *functor, 
			                                  const char *endOfLineChars,
                                        const char *scanType)
{ 
  myCurrentMap->writeScanToFunctor(functor, endOfLineChars, scanType);

} // end method writeScanToFunctor


AREXPORT void ArMap::writePointsToFunctor
		(ArFunctor2<int, std::vector<ArPose> *> *functor,
     const char *scanType,
     ArFunctor1<const char *> *keywordFunctor)
{ 
  return myCurrentMap->writePointsToFunctor(functor, scanType, keywordFunctor);

} // end method writePointsToFunctor

AREXPORT void ArMap::writeLinesToFunctor
	   (ArFunctor2<int, std::vector<ArLineSegment> *> *functor,
      const char *scanType,
      ArFunctor1<const char *> *keywordFunctor)
{ 
  return myCurrentMap->writeLinesToFunctor(functor, scanType, keywordFunctor);

} // end method writeLinesToFunctor

AREXPORT void ArMap::writeToFunctor(ArFunctor1<const char *> *functor, 
			                              const char *endOfLineChars)
{ 
  return myCurrentMap->writeToFunctor(functor, endOfLineChars);
 
} // end method writeToFunctor

AREXPORT ArMapInfoInterface *ArMap::getInactiveInfo()
{
  return myCurrentMap->getInactiveInfo();
}

AREXPORT ArMapObjectsInterface *ArMap::getInactiveObjects()
{
  return myCurrentMap->getInactiveObjects();
}

AREXPORT ArMapObjectsInterface *ArMap::getChildObjects()
{
  return myCurrentMap->getChildObjects();
}

// TODO ???????????????????????????????????????

AREXPORT bool ArMap::readDataPoint( char *line)
{ 
  if (myLoadingMap) {
    return myLoadingMap->readDataPoint(line);
  }
  return false;

} // end method readDataPoint

AREXPORT bool ArMap::readLineSegment( char *line)
{ 
  if (myLoadingMap) {
    return myLoadingMap->readLineSegment(line);
  }
  return false;

} // end method readLineSegment


AREXPORT void ArMap::loadDataPoint(double x, double y)
{ 
  if (myLoadingMap) {
    return myLoadingMap->loadDataPoint(x, y);
  }
  // TODO ArLog
  return;

} // end method loadDataPoint


AREXPORT void ArMap::loadLineSegment(double x1, double y1, double x2, double y2)
{ 
  if (myLoadingMap) {
    return myLoadingMap->loadLineSegment(x1, y1, x2, y2);
  }
  // TODO ArLog
  return;

} // end method loadLineSegment



AREXPORT bool ArMap::addToFileParser(ArFileParser *fileParser)
{
  bool isSuccess = false;

  if (myLoadingMap) {
    isSuccess = myLoadingMap->addToFileParser(fileParser) && isSuccess;
  }
  return isSuccess;

} // end method addToFileParser


AREXPORT bool ArMap::remFromFileParser(ArFileParser *fileParser)
{
  if (myLoadingMap) {
    return myLoadingMap->remFromFileParser(fileParser);
  }
  else {
    return false;
  }
}


AREXPORT bool ArMap::parseLine(char *line)
{
  // Normally, myLoadingMap is already constructed (e.g. in readFile). 
  // The mapClient example simply calls parseLine and parsingComplete,
  // thereby bypassing the normal contruction.  And so, it has been 
  // added here.
  if (myLoadingMap == NULL) {

    myLoadingMap = new ArMapSimple(myBaseDirectory.c_str(),
                                   myCurrentMap->getTempDirectory(), 
                                   "ArMapLoading::myMutex");
    myLoadingMap->setQuiet(myIsQuiet);
  }
  
  if (myLoadingMap) {
    return myLoadingMap->parseLine(line);
  }
  // TODO ArLog
  return false;

} // end method parseLine


AREXPORT void ArMap::parsingComplete(void)
{ 
  lock();

  if (myLoadingMap == NULL) {
    ArLog::log(ArLog::Terse,
               "ArMap::parsingComplete() no map is loading");
    unlock();
    return;
  }
  *myCurrentMap = *myLoadingMap;

  myReadFileStat = myCurrentMap->getReadFileStat();

  delete myLoadingMap;
  myLoadingMap = NULL;

  mapChanged();
  unlock();

} // end method parsingComplete


AREXPORT bool ArMap::isLoadingDataStarted()
{ 
  if (myLoadingMap) {
    return myLoadingMap->isLoadingDataStarted();
  }
  return false;

} // end method  isLoadingDataStarted


AREXPORT bool ArMap::isLoadingLinesAndDataStarted()
{ 
  if (myLoadingMap) {
    return myLoadingMap->isLoadingLinesAndDataStarted();
  }
  return false;

} // end method isLoadingLinesAndDataStarted
            


std::string ArMap::createRealFileName(const char *fileName)
{ 
  return myCurrentMap->createRealFileName(fileName);

} // end method createRealFileName

/***
void ArMap::handleCurrentMapChanged()
{
  ArLog::log(ArLog::Normal,
             "ArMap::handleCurrentMapChanged() invoking callbacks");

  myMapChangedHelper->invokeMapChangedCallbacks();

} // end method handleCurrentMapChanged
***/

AREXPORT bool ArMap::refresh()
{
  ArLog::log(ArLog::Normal, "ArMap::refresh()");

  return processFile(NULL, 0);
}

bool ArMap::processFile(char *errorBuffer, size_t errorBufferLen)
{ 
  ArLog::log(ArLog::Normal, "ArMap::processFile() %s",
             myConfigMapName);

  ArUtil::fixSlashes(myConfigMapName, MAX_MAP_NAME_LENGTH);

  struct stat mapFileStat;
  ArUtil::filestat(myConfigMapName, &mapFileStat);
  
  std::string realFileName = createRealFileName(myConfigMapName);
  
  struct stat realMapFileStat;
  ArUtil::filestat(realFileName, &realMapFileStat);
    
  // If file name is empty, clear out all current information
  if (myIgnoreEmptyFileName && myConfigMapName[0] == '\0')
  {
    if (ArUtil::strcmp(myConfigMapName, myFileName.c_str()) == 0)
    {
      ArLog::log(ArLog::Normal, "Using an empty map since file name is still empty");
      return true;
    }

    ArLog::log(ArLog::Normal, "Using an empty map since empty map file name");
    lock();

    myFileName = "";
    myCurrentMap->clear();

    // The clear method will cause mapChanged to be called.
    // mapChanged();
    unlock();
    return true;
  }

  // Reload map if this is the first time, or we need to use a new file, or the
  // existing file has changed.
  if (!myConfigProcessedBefore || myForceMapLoad ||
      ArUtil::strcmp(myConfigMapName, myFileName.c_str()) != 0 ||
      mapFileStat.st_mtime != myReadFileStat.st_mtime)
  {

    ArLog::log(myCurrentMap->getMapChangedLogLevel(), 
               "ArMap::processFile Loading map because configProcessedBefore %d forceMapLoad %d myFileName \"%s\" configFileName \"%s\" mapFileTime %lu readFileTime %lu (Diff %ld)", 
	       myConfigProcessedBefore, myForceMapLoad,
               myFileName.c_str(), myConfigMapName,
	       mapFileStat.st_mtime, myReadFileStat.st_mtime, 
               mapFileStat.st_mtime - myReadFileStat.st_mtime);
    myConfigProcessedBefore = true; 
    myForceMapLoad = false;

    // If successful, the call to readFile will update the myFileName attribute.
    if (readFile(myConfigMapName, errorBuffer, errorBufferLen))
    {
      return true;
    }
    else
    {
      ArLog::log(ArLog::Terse, "ArMap: failed to read new map file \"%s\": %s.", myConfigMapName, errorBuffer);
      // TODO: if !myConfigProcessedBefore then the intial map file name was
      // invalid; should we now clear the map file name in ArConfig?
      return false;
    }
  }

  // Otherwise, nothing to do.
  return true;

} // end method processFile


AREXPORT bool ArMap::readFileAndChangeConfig(const char *fileName)
{
  std::string beforeFileName = myConfigMapName;

  changeConfigMapName(fileName);

  char buf[1024];
  buf[0] = '\0';
  bool ret = processFile(buf, sizeof(buf));

  return ret;

} // end method readFileAndChangeConfig

AREXPORT void ArMap::changeConfigMapName(const char *fileName)
{
  myConfigMapName[0] = '\0';
  if (fileName != NULL) {
    snprintf(myConfigMapName, MAX_MAP_NAME_LENGTH, fileName);
  }

} // end method changeConfigMapName


AREXPORT void ArMap::offsetMapContents(const ArPose& offset)
{
  std::vector<ArPose> *points = getPoints();
  for(std::vector<ArPose>::iterator i = points->begin(); i != points->end(); ++i)
  {
    *i += offset;
  }

  std::vector<ArLineSegment> *lines = getLines();
  for(std::vector<ArLineSegment>::iterator i = lines->begin(); i != lines->end(); ++i)
  {
    const ArPose new1 = i->getEndPoint1() + offset;
    const ArPose new2 = i->getEndPoint2() + offset;
    i->newEndPoints(new1, new2);
  }

  std::list<ArMapObject*> *objs = getMapObjects();
  for(std::list<ArMapObject*>::iterator i = objs->begin(); i != objs->end(); ++i)
  {
    ArMapObject *obj = (*i);
    //const ArPose oldp = obj->getPose();
    //ArLog::log(ArLog::Normal, "Moving '%s' from (%f, %f) to (%f, %f)...", obj->getName(), oldp.getX(), oldp.getY(), (oldp+offset).getX(), (oldp+offset).getY());
    obj->setPose( obj->getPose() + offset );
    if(obj->hasFromTo())
    {
      //ArLog::log(ArLog::Normal, "...Moving line ends or box corners too...");
      obj->setFromTo( obj->getFromPose() + offset, obj->getToPose() + offset );
    }
/*
    if(obj->getType() == "ForbiddenLine")
    {
      ArLineSegment line = obj->getFromToSegment();
      line.getEndPoint1() += offset;
      line.getEndPoint2() += offset;
      obj->setFromTo( obj->getFromPose() + offset, obj->getToPose() + offset );
*/
  }
}

