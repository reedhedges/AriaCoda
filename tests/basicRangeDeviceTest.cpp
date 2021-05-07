
/*
  This ArRangeDevice subclass simply repeatedly provides a small number of fixed
  readings in a few different subsets that can be turned on/off to verify
  correct behavior of how ArRangeDevice and ArRangeBuffer store and process
  readings.
 
  The patterns of readings are:
    1. A line of ten readings in front of 0,0 at Y=10meters, spanning from X=-10m to X=+10m.
    2. The left 5 of those readings are duplicated (normally should be omitted from range buffer.)
    3. A line of ten readings in front of 0,0 at Y=5meters, spanning from X=0 to X=+10m.  These oclude the right 5 readings of the first line.
    4. Two readings very close to 0,0.  If the robot is at 0,0 these would be "inside" the robot and should normally be omitted from range buffer.
    5. A reading directly in front of 0,0 at Y=260mm.  This is very close to the very front of, but about 10mm outside a p3dx. 

 This test starts the test range device, logs the readings, turns some off, and
logs them again after a short wait which is long enough that the old readings
should have been removed from the buffer.  It cycles through some permutations of the above, waiting several seconds in
between (so old readings should have been cleared from the buffers), and logs
the readings.    

  The logs of readings may be compared to basicRangeBufferTestKnownGood.csv as an
approval/regression test.
*/
