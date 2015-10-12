#include <algorithm> // sort
#include <fstream>   // ofstream
#include <vector>

#include <wx/msgdlg.h>

#include "ibidi_export.hpp"

void ibidiExport(const std::string& fileName,
   const std::map<std::string, Trackee>& trackees)
{
   std::vector<int> goodTrackees{};
   std::string badTrackees{};

   for (const auto& pair : trackees)
   {
      const std::string trackeeID = std::get<0>(pair);

      int index;

      try {
         index = std::stoi(trackeeID);
      }
      catch (std::invalid_argument) {
         badTrackees += trackeeID + ", ";
         continue;
      }
      catch (std::out_of_range) {
         badTrackees += trackeeID + ", ";
         continue;
      }

      if (index <= 0 || std::to_string(index) != trackeeID) {
         badTrackees += trackeeID + ", ";
      }
      else {
         goodTrackees.push_back(index);
      }
   }

   std::sort(goodTrackees.begin(), goodTrackees.end());

   std::ofstream oStream{fileName}; // RAII
   oStream << "--\ttrack\tslice\tx\ty\n";

   unsigned i = 1;
   for (int index : goodTrackees)
   {
      const Trackee& trackee = trackees.at(std::to_string(index));
      // trackee.getTrack() returns a weak_ptr. weak_ptr::lock() returns a shared_ptr.
      const Track track = *trackee.getTrack().lock();

      unsigned j = 0;
      for (; j < track.size(); ++j)
      {
         oStream << i + j << '\t' << index << '\t' << j + 1 << '\t' << track[j].x << '\t'
                 << track[j].y << '\n';
      }
      i += j;
   }

   if (!badTrackees.empty()) {
      ::wxMessageBox("Some trackees were skipped because of incompatible identifiers: " +
         badTrackees.erase(badTrackees.size() - 2), "Warning", wxOK | wxCENTRE,
         nullptr);
   }
}

// vim: tw=90 sw=3 et
