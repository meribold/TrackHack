#ifndef FRAME_H
#define FRAME_H

#include <memory> // shared_ptr, weak_ptr

#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>

#include "bitmap.h"

class Frame
{
   public:

   Frame() = default;
   Frame(const Frame&) = delete;
   Frame(Frame&&);
   explicit Frame(const std::string* dir, const boost::filesystem::path&);

   Frame& operator=(const Frame&) = delete;
   Frame& operator=(Frame&&);

   friend bool operator<(const Frame&, const Frame&);

   std::string getFilename() const;

   // When several Trackee objects ask for the image at the same time they will point to
   // the same object.
   std::shared_ptr<const Bitmap> getBitmap(bool load = true) const;

   void setBitmap(std::shared_ptr<const Bitmap>) const;

   private:

   const std::string*      dir;
   boost::filesystem::path filename;

   // nullptr until getBitmap() is called; once loaded the bitmap will not be deleted
   // until the program is terminated or runs out of memory (in that case it will also be
   // deleted due to termination).
   mutable std::weak_ptr<const Bitmap> bitmap; // mutable because loading of the bitmap is
                                               // deferred
};

inline std::string Frame::getFilename() const {
   return filename.generic_string();
}

inline bool operator<(const Frame& lhs, const Frame& rhs) {
   return lhs.filename < rhs.filename;
}

#endif //FRAME_H

// vim: tw=90 sw=3 et
