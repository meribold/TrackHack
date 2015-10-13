#ifndef CREATE_BITMAPS_H
#define CREATE_BITMAPS_H

#include <array>
#include <bitset>
#include <climits> // CHAR_BIT
#include <cstddef> // size_t
#include <cstdint> // std::int32_t etc.
#include <fstream> // ofstream
#include <iomanip> // stream manipulator functions setfill() and setw()
#include <memory>  // unique_ptr
#include <string>

#include <boost/detail/endian.hpp>

template <std::size_t N>
void createBitmaps(std::string name)
{
   constexpr unsigned SCALE_FACTOR = 24;

   // After changing this function to allocate a lot more memory for objects with
   // automatic storage duration, some of that memory started to look kind of nonrandom.
   // Dynamic storage duration seems to result in subjectively much more random memory.
   std::unique_ptr<std::array<std::array<char, 24 * 24 / 8>, N>>
      xBitMapsPointer{new std::array<std::array<char, 24 * 24 / 8>, N>};

   std::array<std::array<char, 24 * 24 / 8>, N>& xBitMaps = *xBitMapsPointer;

   for (std::size_t bitmapNum = 0; bitmapNum < N; ++bitmapNum)
   {
      std::array<char, 24 * 24 / 8>& bits = xBitMaps[bitmapNum];
      for (std::size_t i = 0; i < bits.size(); ++i)
         bits[i] = ~bits[i];

      for (std::size_t i = 0 + 3 + 1; i < 24 - 3 + 1; i += 3)
         bits[i] = bits[i] ^ std::bitset<8>{"01111110"}.to_ulong();
      for (std::size_t i = 24 + 3 + 2; i < 48 - 3 + 2; i += 3)
         bits[i] = bits[i] ^ std::bitset<8>{"01111110"}.to_ulong();
      for (std::size_t i = 48 + 3; i < 72 - 3;) {
         bits[i] = bits[i] ^ std::bitset<8>{"01111110"}.to_ulong(); ++i;
         bits[i] = bits[i] ^ std::bitset<8>{"01111110"}.to_ulong(); ++i;
         bits[i] = bits[i] ^ std::bitset<8>{"01111110"}.to_ulong(); ++i;
      }

      std::stringstream sStream;
      sStream << std::setfill('0') << std::setw(2) << bitmapNum;

      // I was going to use std::int8_t in case there are more than 8 bits in a char, but
      // how can there be an int8_t then?
      #if (CHAR_BIT != 8)
        #error
      #endif

      #ifndef BOOST_LITTLE_ENDIAN
        #error
      #endif

      std::ofstream bitmapStream(name + sStream.str() + ".bmp", std::ios::binary);

      constexpr std::uint32_t dibHeaderSize = 40;

      char pixelArray[24 * SCALE_FACTOR * 24 * SCALE_FACTOR / 8];

      constexpr char colorTable[8] = {
         0x00, 0x00, 0x00, 0x00,
         0x20, 0x20, 0x20, 0x00
      };

      constexpr std::uint32_t fileSize = 14 + dibHeaderSize + sizeof(colorTable) +
         sizeof(pixelArray);
      constexpr std::uint32_t offset   = 14 + dibHeaderSize + sizeof(colorTable);

      const char bitmapFileHeader[14] = {
         0x42, 0x4d, // "BM"
         reinterpret_cast<const char*>(&fileSize)[0],
         reinterpret_cast<const char*>(&fileSize)[1],
         reinterpret_cast<const char*>(&fileSize)[2],
         reinterpret_cast<const char*>(&fileSize)[3],
         0, 0, 0, 0,
         reinterpret_cast<const char*>(&offset)[0],
         reinterpret_cast<const char*>(&offset)[1],
         reinterpret_cast<const char*>(&offset)[2],
         reinterpret_cast<const char*>(&offset)[3],
      };

      // This will be supported soonish!  Remember to change the color table to 3 byte per
      // entry.  See github.com/wxWidgets/wxWidgets/commit/371928415ab3cf79647153f96907838
      /*
      std::uint16_t bitmapWidth   = 24 * SCALE_FACTOR;
      std::uint16_t bitmapHeight  = 24 * SCALE_FACTOR;
      std::uint16_t one           = 1;

      const char bitmapCoreHeader[dibHeaderSize] = {
         reinterpret_cast<const char*>(&dibHeaderSize)[0],
         reinterpret_cast<const char*>(&dibHeaderSize)[1],
         reinterpret_cast<const char*>(&dibHeaderSize)[2],
         reinterpret_cast<const char*>(&dibHeaderSize)[3],
         reinterpret_cast<const char*>(&bitmapWidth)[0],
         reinterpret_cast<const char*>(&bitmapWidth)[1],
         reinterpret_cast<const char*>(&bitmapHeight)[0],
         reinterpret_cast<const char*>(&bitmapHeight)[1],
         reinterpret_cast<const char*>(&one)[0], // Number of color planes, must be 1
         reinterpret_cast<const char*>(&one)[1],
         reinterpret_cast<const char*>(&one)[0], // Number of bits per pixel
         reinterpret_cast<const char*>(&one)[1],
      };
      */

      std::int32_t  bitmapWidth  = 24 * SCALE_FACTOR;
      std::int32_t  bitmapHeight = 24 * SCALE_FACTOR;
      std::uint16_t one          = 1;
      std::uint32_t imageSize    = sizeof(pixelArray);

      const char bitmapInfoHeader[dibHeaderSize] = {
         reinterpret_cast<const char*>(&dibHeaderSize)[0],
         reinterpret_cast<const char*>(&dibHeaderSize)[1],
         reinterpret_cast<const char*>(&dibHeaderSize)[2],
         reinterpret_cast<const char*>(&dibHeaderSize)[3],
         reinterpret_cast<const char*>(&bitmapWidth)[0],
         reinterpret_cast<const char*>(&bitmapWidth)[1],
         reinterpret_cast<const char*>(&bitmapWidth)[2],
         reinterpret_cast<const char*>(&bitmapWidth)[3],
         reinterpret_cast<const char*>(&bitmapHeight)[0],
         reinterpret_cast<const char*>(&bitmapHeight)[1],
         reinterpret_cast<const char*>(&bitmapHeight)[2],
         reinterpret_cast<const char*>(&bitmapHeight)[3],
         reinterpret_cast<const char*>(&one)[0], // Number of color planes, must be 1
         reinterpret_cast<const char*>(&one)[1],
         reinterpret_cast<const char*>(&one)[0], // Number of bits per pixel
         reinterpret_cast<const char*>(&one)[1],
         0, 0, 0, 0,
         reinterpret_cast<const char*>(&imageSize)[0],
         reinterpret_cast<const char*>(&imageSize)[1],
         reinterpret_cast<const char*>(&imageSize)[2],
         reinterpret_cast<const char*>(&imageSize)[3],
         0, 0, 0, 0,
         0, 0, 0, 0,
         2, 0, 0, 0,
         2, 0, 0, 0
      };

      for (unsigned rowNum = 0; rowNum < 24; ++rowNum)
      {
         for (unsigned colNum = 0; colNum < 24; ++colNum)
         {
            unsigned byteNum = (24 * rowNum + colNum) / 8;
            unsigned bitNum  = (24 * rowNum + colNum) % 8;
            bool set = (bits[byteNum] >> bitNum) & 1;
            for (unsigned i = 0; i < SCALE_FACTOR; ++i)
            {
               for (unsigned j = 0; j < SCALE_FACTOR / 8; ++j)
               {
                  int pixelNum = (SCALE_FACTOR * SCALE_FACTOR * 24 * (23 - rowNum) +
                     SCALE_FACTOR * colNum + 24 * SCALE_FACTOR * i) / 8 + j;
                  pixelArray[pixelNum] = set ? 0x00 : 0xff;
               }
            }
         }
      }

      bitmapStream.write(bitmapFileHeader, sizeof(bitmapFileHeader));
      bitmapStream.write(bitmapInfoHeader, dibHeaderSize);
      bitmapStream.write(colorTable, sizeof(colorTable));
      bitmapStream.write(pixelArray, sizeof(pixelArray));
   }
}

#endif //CREATE_BITMAPS_H

// vim: tw=90 sts=3 sw=3 et
