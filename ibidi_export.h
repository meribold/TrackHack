#ifndef IBIDI_EXPORT_H
#define IBIDI_EXPORT_H

#include <map>
#include <string>

#include "trackee.h"

void ibidiExport(const std::string&, const std::map<std::string, Trackee>&);

#endif //IBIDI_EXPORT_H

// vim: tw=90 sw=3 et
