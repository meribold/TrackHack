#include <fstream> // ofstream

#include <wx/msgdlg.h>

#include "ibidi_export.h"

void ibidiExport(const std::string& fileName,
   const std::map<std::string, Trackee>& trackees)
{
   std::ofstream oStream{fileName}; // RAII

   oStream << "--\ttrack\tslice\tx\ty\n";

   std::string skippedTrackees{};

   unsigned i = 1;
   for (const auto& pair : trackees)
   {
      const std::string trackeeID = std::get<0>(pair);

      int index;
      try {
         index = std::stoi(trackeeID);
      }
      catch (std::invalid_argument) {
         skippedTrackees += trackeeID + ", ";
         continue;
      }
      catch (std::out_of_range) {
         skippedTrackees += trackeeID + ", ";
         continue;
      }

      if (index <= 0 || std::to_string(index) != trackeeID) {
         skippedTrackees += trackeeID + ", ";
      }
      else {
         const Trackee trackee = std::get<1>(pair);
         // trackee.getTrack() returns a weak_ptr. weak_ptr::lock() returns a shared_ptr.
         const Track track = *trackee.getTrack().lock();

         unsigned j = 0;
         for (; j < track.size(); ++j)
         {
            oStream << i + j << '\t' << trackeeID << '\t' << j + 1 << '\t' << track[j].x
                    << '\t' << track[j].y << '\n';
         }
         i += j;
      }
   }

   if (!skippedTrackees.empty()) {
      ::wxMessageBox("Some trackees were skipped because of incompatible identifiers: " +
         skippedTrackees.erase(skippedTrackees.size() - 2), "Warning", wxOK | wxCENTRE,
         nullptr);
   }
}

// vim: tw=90 sw=3 et
