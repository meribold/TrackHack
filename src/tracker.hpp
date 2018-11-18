#ifndef TRACKER_H
#define TRACKER_H

#include <algorithm> // find(), find_if()
#include <cmath>     // pow()
#include <cstddef>   // size_t
#include <memory>    // shared_ptr

#include "movie.hpp"   // defines Frame
#include "trackee.hpp"

class Tracker
{
   public:

   template <typename Map>
   void track(Map& trackees, const Movie&);

   void track(Trackee&, const Movie&);

   private:

   Point trackDown(Trackee&, std::shared_ptr<const Bitmap>, const Point& adjacentPoint);

   // The last parameter denotes the auxiliaryPoint's distance (in frames) to the Bitmap.
   Point trackDown(Trackee&, std::shared_ptr<const Bitmap>, const Point& adjacentPoint,
                   const Point& auxiliaryPoint, unsigned proximity);

   // Higher is nicer; negative values are possible (but not so nice).
   int niceness(const Point& point, unsigned char intensity, const Point& adjacentPoint,
      const Point& auxiliaryPoint, unsigned speedCap, unsigned distanceCap);
};

template <typename Map>
inline void Tracker::track(Map& trackees, const Movie& movie)
{
   for (auto keyTrackeePair : trackees)
   {
      track(std::get<1>(keyTrackeePair), movie);
   }
}

inline void Tracker::track(Trackee& trackee, const Movie& movie)
{
   std::shared_ptr<Track> track = trackee.track;

   auto last = std::find_if(track->begin(), track->end(), [](const Point& point) -> bool {
         return point != Point{-1, -1};
      }
   );
   if (last != track->end())
   {
      auto first = std::find(track->begin(), last, Point{-1, -1});
      for (auto i = last; i != first;)
      {
         --i; *i = trackDown(trackee, movie.getFrame(i - track->begin()).getBitmap(),
            *(i + 1));
      }

      first = std::find(last, track->end(), Point{-1, -1});
      last = std::find_if(first, track->end(), [](const Point& point) -> bool {
            return point != Point{-1, -1};
         }
      );
      while (last != track->end())
      {
         auto i = last;
         while (first != i)
         {
            *first = trackDown(trackee, movie.getFrame(
               first - track->begin()).getBitmap(), *(first - 1), *i, i - first);
            ++first;
            if (first != i) {
               --i;
               *i = trackDown(trackee, movie.getFrame(i - track->begin()).getBitmap(),
                  *(i + 1), *(first - 1), i - first + 1);
            }
            else {
               break;
            }
         }
         first = std::find(last, track->end(), Point{-1, -1});
         last = std::find_if(first, track->end(), [](const Point& point) -> bool {
               return point != Point{-1, -1};
            }
         );
      }
      for (;first != last; ++first)
      {
         *first = trackDown(trackee, movie.getFrame(first - track->begin()).getBitmap(),
            *(first - 1));
      }
   }
}

inline Point Tracker::trackDown(Trackee& trackee, std::shared_ptr<const Bitmap> bitmap,
   const Point& adjacentPoint)
{
   int firstRow    = unsigned(adjacentPoint.y) < trackee.speedCap ?
                        0 : adjacentPoint.y - trackee.speedCap;
   int lastRow     = adjacentPoint.y + trackee.speedCap < bitmap->height ?
                        adjacentPoint.y + trackee.speedCap : bitmap->height - 1;
   int firstColumn = unsigned(adjacentPoint.x) < trackee.speedCap ?
                        0 : adjacentPoint.x - trackee.speedCap;
   int lastColumn  = adjacentPoint.x + trackee.speedCap < bitmap->width ?
                        adjacentPoint.x + trackee.speedCap : bitmap->width - 1;

   Point peakPoint = adjacentPoint;
   unsigned char intensityPeak = (*bitmap)[peakPoint.y][peakPoint.x];

   std::size_t squaredDisplacement, squaredSpeedCap = std::pow(trackee.speedCap, 2);

   for (int row = firstRow; row <= lastRow; ++row)
   {
      for (int column = firstColumn; column <= lastColumn; ++column)
      {
         squaredDisplacement = std::pow(adjacentPoint.x - column, 2) +
                               std::pow(adjacentPoint.y - row, 2);
         if (squaredDisplacement <= squaredSpeedCap)
         {
            if ((*bitmap)[row][column] > intensityPeak)
            {
               peakPoint = Point{column, row};
               intensityPeak = (*bitmap)[row][column];
            }
            else if ((*bitmap)[row][column] == intensityPeak && squaredDisplacement <
               std::pow(adjacentPoint.x - peakPoint.x, 2) +
               std::pow(adjacentPoint.y - peakPoint.y, 2))
            {
               peakPoint = Point{column, row};
            }
         }
      }
   }
   return peakPoint;
}

inline Point Tracker::trackDown(Trackee& trackee, std::shared_ptr<const Bitmap> bitmap,
   const Point& adjacentPoint, const Point& auxiliaryPoint, unsigned proximity)
{
   int firstRow    = unsigned(adjacentPoint.y) < trackee.speedCap ?
                        0 : adjacentPoint.y - trackee.speedCap;
   int lastRow     = adjacentPoint.y + trackee.speedCap < bitmap->height ?
                        adjacentPoint.y + trackee.speedCap : bitmap->height - 1;
   int firstColumn = unsigned(adjacentPoint.x) < trackee.speedCap ?
                        0 : adjacentPoint.x - trackee.speedCap;
   int lastColumn  = adjacentPoint.x + trackee.speedCap < bitmap->width ?
                        adjacentPoint.x + trackee.speedCap : bitmap->width - 1;

   Point preliminaryPoint = adjacentPoint;
   int jolliestNiceness = -255; // That's not very nice at all.

   std::size_t squaredDisplacement, squaredSpeedCap = std::pow(trackee.speedCap, 2);

   for (int row = firstRow; row <= lastRow; ++row)
   {
      for (int column = firstColumn; column <= lastColumn; ++column)
      {
         squaredDisplacement = std::pow(adjacentPoint.x - column, 2) +
                               std::pow(adjacentPoint.y - row, 2);
         if (squaredDisplacement <= squaredSpeedCap)
         {
            int contendersNiceness = niceness(Point{column, row}, (*bitmap)[row][column],
               adjacentPoint, auxiliaryPoint, trackee.speedCap,
               trackee.speedCap * proximity);

            if (contendersNiceness > jolliestNiceness)
            {
               preliminaryPoint = Point{column, row};
               jolliestNiceness = contendersNiceness;
            }
         }
      }
   }
   return preliminaryPoint;
}

inline int Tracker::niceness(const Point& point, unsigned char intensity,
   const Point& adjacentPoint, const Point& auxiliaryPoint, unsigned speedCap,
   unsigned distanceCap)
{
   double squaredDistance    = std::pow(point.x - auxiliaryPoint.x, 2) +
                               std::pow(point.y - auxiliaryPoint.y, 2);
   double squaredDistanceCap = std::pow(distanceCap, 2);

   double priorDistance      = std::sqrt(std::pow(adjacentPoint.x - auxiliaryPoint.x, 2) +
                                         std::pow(adjacentPoint.y - auxiliaryPoint.y, 2));
   double gainedDistance     = priorDistance - std::sqrt(squaredDistance);

   // When sqaredDistance approaches squaredDistanceCap a full proximityBonus is given and
   // a pitch-black pixel can be as good as a white one and vice versa; if squaredDistance
   // is higher than squaredDistanceCap the user should reconsider the speed cap.
   int proximityBonus =
      255. * gainedDistance / speedCap * squaredDistance / squaredDistanceCap;

   return intensity + proximityBonus;
}

#endif //TRACKER_H
