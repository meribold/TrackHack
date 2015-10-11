* Don't use `-Wno-deprecated-declarations`.  The warnings I get might be caused by [this
GCC bug](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65974).  Test after upgrading GCC.
* There's a bug when tracking with no trackees
* Add GIF movie to `README.md`
* Use separate source directory
* Cancel boxing with right click
* Don't start boxing when closing context menu with left click
* Make trackee speed caps changeable from the UI
* "Play" button
* Sort left list box?
* Load existing tracks with movie.  Requires saving which positions were user-supplied.
It's probably best to save everything in a new file and not to auto-save (and create a
save button again).
* Use `wxRichTextStyleListBox` and `wxRichTextCtrl`; no identifiers on `TrackPanel`?
* Can we draw bitmaps faster?  Draw to device context?  Use `wxGraphicsBitmap`?
* Allow capture in regex to extract indices from file name?
* Allow selecting and configuring tracking algorithms from the UI?

<!--- vim: set tw=90 sts=4 sw=4 et spell: -->
