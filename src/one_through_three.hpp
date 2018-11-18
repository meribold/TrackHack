#ifndef ONE_THROUGH_THREE_H
#define ONE_THROUGH_THREE_H

#include <string>

#include <wx/bitmap.h>
#include <wx/gdicmn.h> // wxPoint, wxRect

#include "movie.hpp"
#include "trackee.hpp"

void oneThroughThree(const wxBitmap&, const wxPoint&, const Movie*,
   const std::string* trackeeKey, const Trackee*);

#endif //ONE_THROUGH_THREE_H
