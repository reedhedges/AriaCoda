'''
   This program puts the LINES from an ArMap into a very minimal DXF file.  

   Usage: 
      python convertArMapToDXF.py [-h|--help] [mapfile] [outputfile]

   [mapfile] and [outputfile] are optional. If omitted, then stdin and stdout 
   are used instead.
   

   TODO:  

    * Include Points from the ArMap on a separate layer
    * Include Goals and other map objects on separate layers with labels and
      appropriate metadata
    * Set useful properties in the DXF headers and layer sections.

      -> If you would like to contribute any improvements, send them to the
         ARIA users forum or support@mobilerobots.com. <-
'''

from AriaPy import *
import sys
import time

Aria.init()
ArLog.init(ArLog.StdErr, ArLog.Normal, '', False, False, False)

if (len(sys.argv) > 1 and (sys.argv[1] == '-h' or sys.argv[1] == '--help')) or len(sys.argv) > 3:
  ArLog.log(ArLog.Normal, 'Usage:\n\t%s [mapfile] [outputfile]' % sys.argv[0])
  ArLog.log(ArLog.Normal, '[mapfile] and [outputfile] are optional. If omitted, then stdin and stdout are used instead.')
  Aria.exit(1)

mapfile = '/dev/stdin'
if len(sys.argv) > 1:
  mapfile = sys.argv[1]

outfile = None
if len(sys.argv) > 2:
  outfile = sys.argv[2]

armap = ArMap()
if ( not armap.readFile(mapfile) ):
  ArLog.log(ArLog.Terse, 'Error: Could not open map file "%s" to convert' % mapfile)
  Aria.exit(2)

lines = armap.getLines()
print lines 
outfp = None
if outfile:
  outfp = open(outfile, 'w')
else:
  outfp = sys.stdout
if not outfp:
  ArLog.logErrorFromOS(ArLog.Terse, 'Error: Could not open output file (%s)' % outfile)
  Aria.exit(3)

outfp.write('999\nConverted from %s at %s\n' % (mapfile, time.ctime()))
outfp.write('999\n\tMap Lines Data:\n0\nSECTION\n2\nENTITIES\n')
for l in lines:
  # 8 is handleID
  # 10 is startx, 20 is starty, 11 is endx, 21 is endy
  outfp.write('0\nLINE\n8\n0\n10\n%f\n20\n%f\n11\n%f\n21\n%f\n' %
    (l.getX1(), l.getY1(),
    l.getX2(), l.getY2()))
outfp.write('0\nENDSEC\n')

# TODO points
# POINT entity has:
# 10=x, 20=y, 39=thickness

# TODO goals and other objects as labelled points or boxes on other layers
# for goals with heading and other points with headings, include arrow
# pointing in direction of heading

outfp.write('0\nEOF\n')
outfp.close()
Aria.exit(0)

