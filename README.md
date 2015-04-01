TrackHack
=========

Very basic (but fast), semi-automatic single particle tracking software. Designed to track
cells in sequences of phase shift images obtained using digital holographic microscopy.
Only grayscale bitmaps are supported as input.

Written in C++ using some Boost libraries (Filesystem, Regex, Thread) and wxWidgets.
Building on GNU/Linux and Windows (using MSYS2 and MinGW-w64) should work.

Developed at the University of Muenster.

<!---
For Windows, you can download the most recent executable along with some required
libraries
[here](https://www.dropbox.com/s/xe9da1712u1ntws/track_hack_2015-04-01.zip?dl=1). It
should look something like this:
-->

![Screenshot showing the GUI of TrackHack on Windows 7](https://www.dropbox.com/s/ufdf6d4655lh22h/TrackHackScreenshot.png?dl=1 "Trackees are added by entering a name in the text box and hitting return. The right list box shows the frames in which the user marked the selected cell's position.")
