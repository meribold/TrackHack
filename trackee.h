#ifndef TRACKEE_H
#define TRACKEE_H

#include <cstddef> // size_t
#include <memory>  // shared_ptr

#include "track.h"

class Tracker;

class Trackee
{
   public:

   friend class Tracker;

   Trackee() = default;
   Trackee(const Trackee&) = default;
   Trackee(unsigned speedCap, std::size_t frameCount) : speedCap{speedCap},
      track{new Track{frameCount, {-1, -1}}} {}

   Trackee& operator=(const Trackee&) = delete;

   void setPoint(std::size_t index, const Point&);

   std::weak_ptr<const Track> getTrack() const;

   private:

   unsigned speedCap; // the maximum speed in pixels at which the trackee can travel
   std::shared_ptr<Track> track;
};

inline void Trackee::setPoint(std::size_t index, const Point& point)
{
   (*track)[index] = point;
}

inline std::weak_ptr<const Track> Trackee::getTrack() const
{
   return track;
}

#endif //TRACKEE_H

// vim: tw=90 sw=3 et
