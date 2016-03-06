##### Bugs
*   It doesn't object to loading files that don't look like bitmaps (based on extension)
    and crashes when failing to display them.

##### Issues
*   Don't use `-Wno-deprecated-declarations`.  The warnings I get might be caused by [this
    GCC bug][1].  Test after upgrading GCC.
*   Don't use `-Wno-old-style-cast`.  Maybe there's a way to suppress warnings caused by
    library headers.
*   Don't start boxing when closing the context menu by left-clicking.

##### Enhancements
*   Cancel boxing with right click.
*   Sort left list box?
*   Use `wxRichTextStyleListBox` and `wxRichTextCtrl`; no identifiers on `TrackPanel`?
*   Can we draw bitmaps faster?  Draw to device context?  Use `wxGraphicsBitmap`?
*   Reposition the list boxes depending on whether more free space is available at the
    bottom and top or left and right of the `TrackPanel`?
*   Use static linking for Windows builds?
*   Create an installer for Windows?

##### Features
*   Make trackee speed caps changeable from the UI.
*   Allow selecting and configuring tracking algorithms from the UI?
*   Add *Play* button.
*   Load existing tracks with movie.  Requires saving which positions were user-supplied.
    It's probably best to save everything in a new file and not to auto-save (and create a
    save button again).
*   Allow capture in regex to extract indices from file name?

##### Meta
*   Add GIF movie to `README.md`
*   Add information such as the compilation date and options and the SHA-1 of the latest
    commit to the About Dialog.

[1]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65974

<!--- vim: set tw=90 sts=4 sw=4 et spell: -->
