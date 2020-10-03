Big Ring Indoor Video Cycling
=============================

![screenshot](https://cloud.githubusercontent.com/assets/420742/9442540/bebbd2b4-4a7c-11e5-95b5-17388583c665.jpg)

Big Ring Indoor Video Cycling can be used in combination with a bicycle, indoor trainer and [ANT+](http://www.thisisant.com) power, cadence or heart rate sensors.

Current Status
--------------

Indoor Cycling currently does the following:

* Works on Linux
* Get information from ANT+ Sensors
    - ANT+ FE-C Smart Trainers.
    - Power meters
    - Cadence sensors (including cadence from power meters)
    - Speed sensors
    - Combined Cadence & Speed Sensors
    - Heart rate sensors
* Show current values for:
    - Power, directly measured from Power meter or derived from speed and power curve of trainer
    - Cadence
    - Heart Rate
* Play a Real Life Video with video frames mapped to the distance
  the cyclist has travelled. The distance is determined by calculating
  (approximating) the speed of the cyclist several times per second,
  and changing the travelled distance according to the speed.

Building
--------

1. Create a build directory, for instance next to the source directory.
2. Run qmake <source directory> from the build directory.
3. make
4. the `big-ring` executable will be located in the bin/ directory inside the build directory.

File/Device Permissions
-----------------------

To be able to send data to the ANT+ USB sticks, the user needs permissions. On Linux systems with *udev*, this can established by putting the following rules in the udev configuration.

	SUBSYSTEM=="tty", ATTRS{idVendor}=="0fcf", ATTRS{idProduct}=="1004", SYMLINK+="garmin-usb1", MODE="666"
	SUBSYSTEMS=="usb" ATTR{idVendor}=="0fcf", ATTR{idProduct}=="1008", MODE="666"

Usage
-----

Run `bin\big-ring`. The program will start and try to find your videos. If no video folder has been configured yet, the program will ask you to configure it. The files will be parsed and when ready, the list of videos will be populated. Using the preferences window, the user can configure the ANT+ sensors. Choose a video, and a course. 

License
-------

Big Ring Indoor Video Cycling is distributed under the GNU General Public License, version 3 or later. See LICENSE.txt for details.
