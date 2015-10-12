#include <algorithm> // min, max
#include <cmath>     // pow
#include <fstream>   // ofstream, ifstream
#include <iomanip>   // setprecision stream manipulator
#include <memory>    // weak_ptr, shared_ptr

#include <wx/dcbuffer.h> // wxBufferedPaintDC
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/graphics.h> // wxGraphicsContext
#include <wx/log.h>      // ...
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>

#include "one_through_three.hpp"

class OneThroughThreeDialog : public wxDialog
{
   public:

   OneThroughThreeDialog(wxWindow* parent, wxWindowID, const wxBitmap&, const
      wxPoint&, const Movie*, const std::string* key, const Trackee*);

   private:

   // average gray level in the ring with the radii from the two sliders
   double averageAnnulus(const Bitmap&, const Point& center);
   void apply();

   void onImagePanelPaint(wxPaintEvent&);
   void onOkay(wxCommandEvent&);          // process a wxEVT_COMMAND_BUTTON_CLICKED
   void onApply(wxCommandEvent&);         // process a wxEVT_COMMAND_BUTTON_CLICKED
   void onSlider(wxCommandEvent&);        // process a wxEVT_COMMAND_SLIDER_UPDATED

   wxPanel* imagePanel;
   wxSlider* slider[2];

   wxBitmap subBitmap;
   wxPoint relativePoint; // in coordinates relative to the origin (top-left) of the sub
                          // bitmap

   const Movie*       movie;
   const std::string* trackeeKey;
   const Trackee*     trackee;
};

namespace {
   typedef OneThroughThreeDialog MyDialog;
}

void oneThroughThree(const wxBitmap& bitmap, const wxPoint& point, const Movie* movie,
   const std::string* key, const Trackee* trackee)
{
   if (point == wxDefaultPosition) return;

   constexpr int sideLength = 256;

   wxPoint topLeft{point.x - sideLength / 2, point.y - sideLength / 2};
   if (topLeft.x < 0) topLeft.x = 0;
   if (topLeft.y < 0) topLeft.y = 0;

   wxPoint bottomRight{point.x + sideLength / 2, point.y + sideLength / 2};
   if (bottomRight.x >= bitmap.GetWidth()) bottomRight.x = bitmap.GetWidth() - 1;
   if (bottomRight.y >= bitmap.GetHeight()) bottomRight.y = bitmap.GetHeight() - 1;

   wxBitmap subBitmap = bitmap.GetSubBitmap(wxRect{topLeft, bottomRight});
   MyDialog* myDialog = new MyDialog{nullptr, wxID_ANY, subBitmap,
      wxPoint{point.x - topLeft.x, point.y - topLeft.y}, movie, key, trackee};

   if (myDialog->ShowModal() == wxID_OK) {
      // ...
   }

   myDialog->Destroy();
}

OneThroughThreeDialog::OneThroughThreeDialog(wxWindow* parent, wxWindowID id, const
   wxBitmap& subBitmap, const wxPoint& relativePoint, const Movie* movie,
   const std::string* key, const Trackee* trackee) :
      wxDialog{parent, id, "OneToThree", wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE},
      imagePanel{new wxPanel{this}},
      slider{new wxSlider{this, wxID_ANY, 0, 0, 96, wxDefaultPosition, wxDefaultSize,
                          wxSL_VERTICAL | wxSL_LABELS | wxSL_INVERSE},
             new wxSlider{this, wxID_ANY, 0, 0, 96, wxDefaultPosition, wxDefaultSize,
                          wxSL_VERTICAL | wxSL_LABELS | wxSL_INVERSE}},
      subBitmap{subBitmap}, relativePoint{relativePoint},
      movie{movie}, trackeeKey{key}, trackee{trackee}
{
   imagePanel->SetBackgroundStyle(wxBG_STYLE_PAINT);
   imagePanel->SetMinClientSize(subBitmap.GetSize());
   imagePanel->SetMaxClientSize(subBitmap.GetSize());

   wxBoxSizer* topSizer = new wxBoxSizer{wxVERTICAL};

   wxBoxSizer* hBox = new wxBoxSizer{wxHORIZONTAL};
   hBox->Add(imagePanel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 8);
   hBox->Add(slider[0], 0, wxEXPAND | wxRIGHT | wxTOP | wxBOTTOM, 8);
   hBox->Add(slider[1], 0, wxEXPAND | wxRIGHT | wxTOP | wxBOTTOM, 8);

   topSizer->Add(hBox, 0, wxALIGN_CENTER_HORIZONTAL);

   wxSizer* buttonSizer = CreateButtonSizer(wxOK | wxCANCEL | wxAPPLY);
   if (buttonSizer) {
      topSizer->Add(CreateSeparatedSizer(buttonSizer), 0,
         wxEXPAND | wxALIGN_RIGHT | wxALL, 8);
   }

   topSizer->SetSizeHints(this);
   SetSizer(topSizer);

   imagePanel->Bind(wxEVT_PAINT, &OneThroughThreeDialog::onImagePanelPaint, this);
   Bind(wxEVT_COMMAND_BUTTON_CLICKED, &OneThroughThreeDialog::onOkay, this, wxID_OK);
   Bind(wxEVT_COMMAND_BUTTON_CLICKED, &OneThroughThreeDialog::onOkay, this, wxID_APPLY);
   Bind(wxEVT_COMMAND_SLIDER_UPDATED, &OneThroughThreeDialog::onSlider, this, wxID_ANY);
}

double OneThroughThreeDialog::averageAnnulus(const Bitmap& bitmap, const Point& center)
{
   unsigned innerRadius = std::min(slider[0]->GetValue(), slider[1]->GetValue());
   unsigned outerRadius = std::max(slider[0]->GetValue(), slider[1]->GetValue());

   int firstRow         = center.y - static_cast<int>(outerRadius);
   unsigned lastRow     = center.y + outerRadius;

   int firstColumn      = center.x - static_cast<int>(outerRadius);
   unsigned lastColumn  = center.x + outerRadius;

   if (firstRow < 0) firstRow = 0;
   if (lastRow >= bitmap.height) lastRow = bitmap.height - 1;

   if (firstColumn < 0) firstColumn = 0;
   if (lastColumn >= bitmap.width) lastColumn = bitmap.width - 1;

   unsigned pixels = 0; // used as denominator
   unsigned sum = 0;

   for (unsigned row = firstRow; row <= lastRow; ++row)
   {
      for (unsigned column = firstColumn; column <= lastColumn; ++column)
      {
         unsigned squaredDistance = std::pow(static_cast<int>(column) - center.x, 2) +
                                    std::pow(static_cast<int>(row) - center.y, 2);

         // Is this point inside the outer circle? If so, is it also outside of the inner
         // circle?
         if (squaredDistance < std::pow(outerRadius, 2) &&
             squaredDistance >= std::pow(innerRadius, 2))
         {
            ++pixels;
            sum += bitmap[row][column];
         }
      }
   }

   return (pixels == 0) ? -1. : static_cast<double>(sum) / static_cast<double>(pixels);
}

void OneThroughThreeDialog::apply()
{
   std::shared_ptr<const Track> track = trackee->getTrack().lock();
   if (track)
   {
      std::ofstream oStream{movie->getDir() + *trackeeKey + "_1to3-plugin-output.txt"};

      for (std::size_t i = 0; i < movie->getSize(); ++i)
      {
         oStream << movie->getFrame(i).getFilename();
         if ((*track)[i] != Point{-1, -1})
         {
            std::shared_ptr<const Bitmap> bitmap = movie->getFrame(i).getBitmap();

            // gray level
            oStream << '\t' << static_cast<int>((*bitmap)[(*track)[i].y][(*track)[i].x]);

            double annulusAverage = averageAnnulus(*bitmap, (*track)[i]);
            oStream << '\t' << std::fixed << std::setprecision(4) << annulusAverage;

            // ...
            {
               std::string logFile = movie->getDir() + movie->getFrame(i).getFilename();
               std::size_t pos = logFile.find("_uw.bmp");
               if (pos != std::string::npos)
               {
                  logFile.replace(pos, 7, ".log");
                  std::ifstream iStream{logFile};

                  if (iStream)
                  {
                     for (std::size_t i = 0; iStream && i < 84; ++i) // ignore 84 lines
                        iStream.ignore(256, '\n');
                     std::string key; iStream >> key;
                     if (key == "unwrapped_range") {
                        iStream.ignore(256, '\n'); // ignore one more line
                        if (iStream) {
                           double unwrappedRange;
                           iStream >> unwrappedRange;
                           oStream << '\t' << std::fixed << std::setprecision(4) <<
                              unwrappedRange;
                        }
                     }
                     iStream.close();
                  }
               }
            } // ...
         }

         oStream << std::endl;
      }
      oStream.close();
   }
}

void OneThroughThreeDialog::onOkay(wxCommandEvent& event)
{
   apply();
   event.Skip();
}

void OneThroughThreeDialog::onApply(wxCommandEvent&)
{
   apply();
}

void OneThroughThreeDialog::onSlider(wxCommandEvent&)
{
   imagePanel->Refresh(false);
}

void OneThroughThreeDialog::onImagePanelPaint(wxPaintEvent&)
{
   //wxBufferedPaintDC dC{event.GetEventObject()};
   wxBufferedPaintDC dC{imagePanel};
   wxGraphicsContext *gC = wxGraphicsContext::Create(dC);

   if (gC)
   {
      gC->DrawBitmap(subBitmap, 0., 0., wxDouble(imagePanel->GetClientSize().GetWidth()),
         wxDouble(imagePanel->GetClientSize().GetHeight()));

      gC->SetPen(wxPen{wxColour{0x00, 0x00, 0x00}});
      gC->SetBrush(wxBrush{wxColour{0xff, 0xa7, 0x1a, 0x80}}); // Day[9] made me do it.

      // signed to avoid underflow of the expression...
      //int radius = slider[0]->GetValue();
      // ...here
      //gC->DrawEllipse(relativePoint.x - radius, relativePoint.y - radius, 2 * radius,
         //2 * radius);
      //radius = slider[1]->GetValue();
      //gC->DrawEllipse(relativePoint.x - radius, relativePoint.y - radius, 2 * radius,
         //2 * radius);

      wxGraphicsPath path = gC->CreatePath();
      path.AddCircle(relativePoint.x, relativePoint.y, slider[0]->GetValue());
      path.AddCircle(relativePoint.x, relativePoint.y, slider[1]->GetValue());
      gC->FillPath(path); // alternatives are gC->StrokePath() and gC->DrawPath();

      delete gC;
   }
}

// vim: tw=90 sw=3 et
