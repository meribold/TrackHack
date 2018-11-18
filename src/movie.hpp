#ifndef MOVIE_H
#define MOVIE_H

#include <cstddef> // size_t
#include <deque>
#include <memory>  // shared_ptr
#include <string>
#include <vector>

#include <boost/regex.hpp> // Note: switch to <regex> from the stdlib when upgrading to a
                           // future release of GCC.

#define BOOST_THREAD_USE_LIB
#include <boost/thread.hpp> // thread

#include "frame.hpp"

class Movie
{
   public:

   Movie() = default;
   Movie(const Movie&) = delete;
   Movie(Movie&&);
   Movie(const std::string& directory, const std::string& regEx);

   ~Movie();

   Movie& operator=(const Movie&) = delete;
   Movie& operator=(Movie&&);

   const std::string& getDir() const;
   const Frame& getFrame(std::size_t) const;
   std::string getFilename(std::size_t) const; // includes directory
   std::size_t getSize() const;
   std::size_t size() const;

   private:

   void populateBuffer();

   std::string* dir;
   std::vector<Frame> frames;

   std::deque<std::shared_ptr<const Bitmap>> bitmapBuffer;

   bool terminateThread;
   boost::mutex terminateFlagAccess;
   boost::thread thread;
};

inline const std::string& Movie::getDir() const {
   return *dir;
}

inline const Frame& Movie::getFrame(std::size_t i) const {
   return frames[i];
}

inline std::string Movie::getFilename(std::size_t i) const {
   return *dir + frames[i].getFilename();
}

inline std::size_t Movie::getSize() const {
   return frames.size();
}

inline std::size_t Movie::size() const {
   return frames.size();
}

#endif //MOVIE_H
