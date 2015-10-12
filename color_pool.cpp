#include "color_pool.hpp"

#include <iostream>

const wxColour* ColorPool::peek()
{
   return &colors[next];
}

const wxColour* ColorPool::getColor()
{
   // this->next must be the smallest index so that no color is in use less often than
   // colors[next].
   const wxColour* color = &colors[next];

   int minUses = numUsed[next];

   ++numUsed[next];

   /*
   std::cout << "color " << next << " handed out; times in use: " << numUsed[next]
             << std::endl;
   */

   // Set this->next to the first color that's in use as little as colors[next].
   for (; next < numColors; ++next) {
      if (numUsed[next] == minUses) {
         break;
      }
   }
   // All colors are in use more often than colors[next].
   if (next == numColors) {
      next = 0;
      for (unsigned i = 0; i < numColors; ++i) {
         if (numUsed[i] < numUsed[next]) {
            next = i;
         }
      }
   }

   return color;
}

void ColorPool::returnColor(const wxColour* color)
{
   std::size_t index = color - colors.data();
   --numUsed[index];
   if (numUsed[index] < numUsed[next] || (numUsed[index] == numUsed[next] && index < next))
   {
      next = index;
   }
   /*
   std::cout << "color " << index << " returned; times in use: " << numUsed[index]
             << std::endl;
   */
}

/*
ColorPool::Color::Color(int niceness, wxColour color) :
   wxColour(color), niceness(niceness)
{

}

ColorPool::Color::~Color()
{

}

const ColorPool::Color& ColorPool::getColor()
{

}
*/

//ColorPool colorPool;

// vim: tw=92 sw=3 et
