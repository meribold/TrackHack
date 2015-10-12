#ifndef BITMAP_H
#define BITMAP_H

#include <cstddef> // size_t

typedef unsigned char Byte;

struct Bitmap
{
   Bitmap() = default;

   Bitmap(std::size_t width, std::size_t height);

   ~Bitmap();

   Byte* operator[](std::size_t) const;

   std::size_t width, height;

   Byte* pixels;
};

#endif //BITMAP_H

// vim: tw=90 sts=3 sw=3 et
