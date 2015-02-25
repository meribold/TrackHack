TrackHack
=========

Very basic (but fast), semi-automatic single particle tracking software. Designed to track cells in
sequences of phase shift images obtained using digital holographic microscopy. Only grayscale
bitmaps are supported as input.

I didn't work on TrackHack since september 2012 and it's pretty incomplete. Some UI elements are
only dummies. It's written in C++ and I used to build it on MS Windows with MinGW. It uses some
Boost libraries (Filesystem, Regex, Thread) and wxWidgets.

For Windows, you can download the most recent executable along with some required libraries
[here](https://www.dropbox.com/s/98tsk3fnto9k5gw/TrackHack.zip?dl=0). It should look something like this:

![Screenshot showing the GUI of TrackHack on Windows 7](https://www.dropbox.com/s/ufdf6d4655lh22h/TrackHackScreenshot.png?dl=1 "Trackees are added by entering a name in the text box and hitting return. The right list box shows the frames in which the user marked the selected cell's position (to correct its trajectory).")
