#include <wx/image.h>

#include "bitmap.hpp"
#include "frame.hpp"

Frame::Frame(Frame&& frame) : dir{frame.dir}, filename{std::move(frame.filename)} {}

Frame::Frame(const std::string* dir, const boost::filesystem::path& filename) :
   dir{dir}, filename{filename}, bitmap{} {}

Frame& Frame::operator=(Frame&& frame)
{
   filename = std::move(frame.filename);
   dir = frame.dir;
   return *this;
}

std::shared_ptr<const Bitmap> Frame::getBitmap(bool load) const
{
   auto bitmap = this->bitmap.lock();
   if (!bitmap && load)
   {
      // Code that does not allocate and initialize memory for a temporary RGB image would
      // be potentially faster and more elegant; the CImg or OpenCV library might be
      // a suitable replacement for wxWidgets' image loading facilities.
      wxImage image{*dir + getFilename(), wxBITMAP_TYPE_ANY};
      unsigned char* imageData = image.GetData();
      std::size_t pixelCount = image.GetWidth() * image.GetHeight();

      bitmap = std::make_shared<Bitmap>(image.GetWidth(), image.GetHeight());
      for (std::size_t i = 0; i < pixelCount; ++i)
      {
         bitmap->pixels[i] = imageData[3 * i];
      }
   }
   return bitmap; // nullptr if the bitmap wasn't loaded already and the load flag was
                  // explicitly set to false
}

void Frame::setBitmap(std::shared_ptr<const Bitmap> bitmap) const
{
   this->bitmap = bitmap;
}

// vim: tw=90 sw=3 et
