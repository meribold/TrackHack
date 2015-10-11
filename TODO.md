#####Bugs
* There's a bug when tracking with no trackees
* It doesn't object to loading files that don't look like bitmaps (based on extension) and
  crashes when failing to display them

#####Issues
* Don't use `-Wno-deprecated-declarations`.  The warnings I get might be caused by [this
GCC bug][1].  Test after upgrading GCC.
* Don't start boxing when closing context menu with left click

#####Enhancements
* Cancel boxing with right click
* Sort left list box?
* Use `wxRichTextStyleListBox` and `wxRichTextCtrl`; no identifiers on `TrackPanel`?
* Can we draw bitmaps faster?  Draw to device context?  Use `wxGraphicsBitmap`?

#####Features
* Make trackee speed caps changeable from the UI
* Allow selecting and configuring tracking algorithms from the UI?
* "Play" button
* Load existing tracks with movie.  Requires saving which positions were user-supplied.
  It's probably best to save everything in a new file and not to auto-save (and create a
  save button again).
* Allow capture in regex to extract indices from file name?

#####Meta
* Use `.hpp` as the filename extension for C++ header files
* Use separate source directory
* Add GIF movie to `README.md`

[1]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65974

<!--- vim: set tw=90 sts=4 sw=4 et spell: -->
