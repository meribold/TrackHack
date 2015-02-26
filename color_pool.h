#ifndef COLOR_POOL_H
#define COLOR_POOL_H

#include <wx/colour.h>

class ColorPool
{
   public:

   class Color : public wxColour
   {
      public:

      ~Color() {

      }

      private:

      static ColorPool colorPool;
   };

   private:

   // ...
};

#endif //COLOR_POOL_H

// vim: tw=90 sw=3 et
