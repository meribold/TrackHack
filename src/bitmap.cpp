#include "bitmap.hpp"

Bitmap::Bitmap(std::size_t width, std::size_t height) : width{width}, height{height}
{
   const auto size = width * height;
   pixels = new Byte[size];
}

Bitmap::~Bitmap()
{
   delete[] pixels;
}

unsigned char* Bitmap::operator[](std::size_t row) const
{
   return pixels + row * width;
}

// vim: tw=90 sw=3 et
