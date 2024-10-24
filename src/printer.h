// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#if !defined _PRINTER_H_
#define _PRINTER_H

class wxArrayString;
class wxString;

namespace prn
{
bool        EnumeratePrinters(wxArrayString& printerNames   );  // get an array of available printers
bool        PrintAFile      (const wxString& fileName, const wxString& title=ES);  // print an ascii CP437 file: conversion is done
void        SetFont         (const wxString& pFont          );  // set the printerfont to use from now on, if 0, you get a selectfont dialog
//bool      GetPrinterName  (wxString*       pName          );  // get active printername, if  0, you get a select printer dialog
bool        PrintLine       (const wxString& line           );  // prints a line, no conversion done
bool        PrintCharacter  (wxChar          chr            );  // prints a char, no conversion done
void        SetPrinterName  (const wxString& printerName    );  // sets the name of the printer to use.
void        SetLinesPerPage (int             lines          );  // sets the maximum number of lines on a page
void        EndPrint        ();                                 // signal end of print session
bool        BeginPrint      (const wxString& title=ES);         // signal start of print session with optional title
const wxString&    GetFont  ();                                 // get the current selected printerfont
bool        SelectPrinter   ();                                 // selects a printer through the windows printselect dialog
void        SelectFont      ();                                 // selects a font through the windows fontselect dialog
UINT        GetLinesPerPage ();                                 // gets the actual maxLines/page
UINT        GetCharsPerLine ();                                 // gets actual max chars on a line
bool        IsPrintToFile   ();                                 // true, if we are printing to a file (not a printer)

namespace table
{   //simple table-printing: texts and horizontal/vertical lines
struct Line
{
    Line() { begin = { 0,0 }; end = { 0,0 }; }
    Line(const wxPoint& a_begin, const wxPoint& a_end){begin = a_begin; end = a_end;}
    wxPoint     begin;          // beginpoint in chars/lines from left-top of page
    wxPoint     end;            // endpoint of line
//    wxPoint     offsetInChar;   // startpoint within a char in pixels
};

struct Text
{
    wxPoint     begin;         // in chars/lines from left-top of page
    wxString    text;
};

struct TableInfo    // use oldstyle arrays for fixed layout && vectors for dynamic layouts
{
    wxPoint origin;     // offset from top left of page where table starts
    Line*   lines;      // array of lines to draw
    Text*   texts;      // array of texts to print
    size_t  lineCount;  // number of lines (pointed to by lines)
    size_t  textCount;  // number of texts (pointed to by texts)
    std::vector<Line>   linesV; // runtime calculated lines
    std::vector<Text>   textsV; // runtime calculated texts
};

}   // namespace table
void PrintTable (const table::TableInfo& table);

}   // end namespace prn
#endif
