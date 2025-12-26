// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

//#include <windows.h> //do not use this if you include msgdlg.h

#include <wx/arrstr.h>
#include <wx/msgdlg.h>

#include "cfg.h"
#include "printer.h"

#if 0
    #define DEBUG_LOCAL                    	// if set, print some info during some actions
#else
    #undef DEBUG_LOCAL                     	// if set, print some info during some actions
#endif


namespace prn
{
    bool EnumeratePrinters(wxArrayString& a_sPrinterNames)
    {
        DWORD               dwNeeded    = 0;
        DWORD               dwReturned  = 0;
        std::unique_ptr<PRINTER_INFO_1> pInfo;
        BOOL                bResult     = 0;    //false;
        UINT                searchFlags = PRINTER_ENUM_LOCAL;

        if (cfg::GetNetworkPrinting())
            searchFlags |= PRINTER_ENUM_CONNECTIONS;

        a_sPrinterNames.clear();    // we have new data, remove old!

                                    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd162931(v=vs.85).aspx
                                    // get size of buffer needed to receive the wanted info
        (void) ::EnumPrinters(
            searchFlags,        // types to search for
            nullptr,            // printername
            1L,                 // printer info level type
            nullptr,            // determine size of info_1 buffer needed to store data
            0L,                 //   "
            &dwNeeded,          // bytes needed for buffer
            &dwReturned);       // number of entries

        if (dwNeeded > 0)
        {
            pInfo.reset(reinterpret_cast<PRINTER_INFO_1 *>(new char[dwNeeded]));
        }

        if ( nullptr != pInfo.get() )
        {   // now we have the wanted buffer for the info
            dwReturned = 0;
            bResult = ::EnumPrinters(
                searchFlags,
                nullptr,
                1L,                // printer info level
                reinterpret_cast<LPBYTE>(pInfo.get()),
                dwNeeded,
                &dwNeeded,
                &dwReturned);
        }

        if (bResult && ( nullptr != pInfo.get() ) )
        {
            // Review the information from all the printers returned by EnumPrinters.
            unsigned int realCount = 0;
            for (DWORD ii=0; ii < dwReturned; ii++)
            {
                #pragma warning(push)
                //#pragma warning(disable:6385)
                const auto& info = pInfo.get()[ii];
                // pInfo[ii].pName contains the printername to use in the CreateDC function call.
                if (   ( wxString(_("Fax"                             )) == info.pName)
                    || ( wxString(_("Microsoft XPS Document Writer"   )) == info.pName)
                    || ( wxString(_("OneNote"                         )) == info.pName)
                   )
                {
                    continue;   // we only want 'real' printers!
                }
                #pragma warning(pop)

                ++realCount;
                a_sPrinterNames.push_back(info.pName);
                MyLogMessage(_("Printer %u found, flags: 0x%08x, name: <%s>"), realCount, (unsigned int)info.Flags, info.pName);
            }
        }

        return bResult;
    }   // EnumPrinters()

#if defined DEBUG_LOCAL
static wxString s_sDebugString;  	// used to get some info, and then show it in a messagebox.
#endif

// some constants used to establish default values that work for me :)
//static const int    sc_iDefaultCodepage     = 437;          // codepage used for translation from ascii <--> wide chars = MS-DOS U.S. English
static const char   sc_cDefaultFont[]       = "Lucida Console";// "Consolas";   // fixed pitch font
// L"DejaVu Sans Mono" L"Lucida Console" 
static const int    sc_iDefaultPointsize    = 11;           // 12+'Consolas' -> 84 chars/line+58 lines/page. 11+'Consolas' -> 95 chars/line+63 lines/page.
static const int    sc_iHorizontalMargin    = 2;            // leftmargin in characters
static const int    sc_iVerticalMargin      = 1;            // topmargin in lines

/*
    This class enables you get Ascii text on a windows printer.
    It acts as a simple lineprinter: only 8 bit text and '\n' and '\f' are supported.
    So special chars in the high area (128-255) are translated via the selected codepage to wide.
    It has a PrintAFile() method that allows you to print a file.
    It has some methods to get 'characters' to the printer:
      1 - BeginPrint()
      2 - lots of PrintCharacter()
          a '\f' is interpreted as 'go to a new page, a '\n' will go to the begin of the next line.
      3 - EndPrint()
    REMARK: all is setup for using ASCII (needed it for an old dos-program when converting it to windows)
*/
class MyLinePrinter
{
public:
    MyLinePrinter();
    ~MyLinePrinter();

    /*
        Sets the name of the printer to be used for next printsession.
        If its different from the current one, it will be initialised.
    */
    void SetPrinterName(const wxString& printerName);

    /*
        Gets the name of the active printer.
        @return the printername
    */
    const wxString& GetPrinterName()const;

    /*
        Adds a character to the linebuffer.
        If its the first char after \f or if its the very first character,
        then a StartPage will be send.
        If its a \n, the line will be printed and the linebuffer will be emptied.
        If its a \f, then an EndPage will be send only if we already had some characters printed on this page.
    */
    bool PrintCharacter( wxChar chr );

    /*
        Prints a complete line of chars.
        It acts asif each char in the supplied buffer is output to the PrintCharacter() method;
    */
    bool PrintLine(const wxString& line);

    /*
        BeginPrint() Empties all buffers and prepares the class for the reception of chars to print.
        @return true if all ok and false on error (printer open unsucsessfull or no name yet).
        If there is no printer choosen yet, a standard windows dialog for selecting a printer is shown.
    */
    bool BeginPrint( const wxString& title );

    /*
        EndPrint() flushes all data and ends this printsession (closes document).
    */
    void EndPrint();

    /*
        Set the font you want to be used as printerfont.
        If the new font is empty, or the parameter == 0, then a standard windowsdialog for
        font selection (fixed pitch chars) is used to get a new font.
        If there is an active printer, it will use the new font from now on.
        REMARK: we always use a font with pointsize 11
    */
    void SetFont(const wxString& font, LOGFONT* pLogFontInfo = nullptr);

    /*
        Get the currently active font.
    */
    const wxString& GetFont() const;

    /*
    Select a font you want to be used as printerfont.
    The standard windowsdialog for font selection (fixed pitch chars) is used to get a new font.
    If there is an active printer, it will use the new font from now on.
    REMARK: we always use a font with pointsize 11
    */
    void SelectFont();

    /*
        Print a file.
    */
    bool PrintAFile(const wxString& fileName, const wxString& title);

    /*
    Will setup a standard windowsdialog for selecting a printer.
    */
    bool SelectPrinter();

    /*
    * Sets the maximum number of lines per page
    */
    void SetLinesPerPage( int lines);

    /*
    * Gets the maximum number of characters on a line for the current font and paper size
    */
    UINT GetCharsPerLine () const{return m_iMaxCharsOnLine; }

    /*
    * Gets the maximum number of lines per page for the current font and paper size
    */
    UINT GetLinesPerPage()const{return (UINT)m_iMaxLinesOnPage; };

    /*
    * will print a table according the supplied info
    */
    void PrintTable(const table::TableInfo& table);

    bool IsPrintToFile() const { return m_bPrint2File; }  // true, if we are printing to a file (not a printer)

private:

    /*
        Closes the current printersession and resets the connection to 0
        @Return true if all ok, false if something went wrong.
    */
    bool PrinterClose();

    /*
        Open a (new) printerconnection. If the printer was already actief, nothing is done.
        If we have a printername, we use that. If not, we setup a standard windows dialog for
        choosing a printer.
        @Return false on error.
    */
    bool    PrinterOpen();

    /*
        If we have a new font, update the printer hdc and our internal values (char height, width, chars/line/page.
    */
    void InitFont(LOGFONT* logFontInfo = nullptr);

    /*
        clear all local data.
    */
    void Clear();
    void Line2Printer();                            //executed when '\n' is found in input or line > maxline
    void PrintPage();                               //executed when '\f' is found in input
    bool OpenDiskfile4Print();                      //open the diskfile, if printing to disk

    HDC                 m_hPrinter;                 // active printer connection, will be closed on destruction or on new printername
    bool                m_bPrinting;                // if we have started, we also have to stop
    bool                m_bPageStarted;             // true if at least 1 line has been put on a (new) page
    int                 m_iPrintjobId;              // id from startdoc, not used here
    int                 m_iLinesOnPage;             // number of lines already printed on current page
    int                 m_iCharWidth;               // the width in pixels of a char in the choosen font
    int                 m_iCharHeight;              // the height in pixels of a char in the choosen font
    int                 m_iHorizontalOffset;        // left margin in pixels of page
    int                 m_iVerticalOffset;          // top margin in pixels of page
    int                 m_iDeviceHeight;            // height in pixels of page of used printer
    int                 m_iDeviceWidth;             // width in pixels of page of used printer
    int                 m_iMaxCharsOnLine;          // maximum chars on a line for this printer/paper/font
    int                 m_iMaxLinesOnPage;          // maximum lines on a page for this printer/paper/font
    int                 m_iDefaultLpp4List;         // lpp when printing to textfile, set by application
    int                 m_iPointSize;               // the pointsize used in creating fonts
    int                 m_iPagesPrinted;            // nr of pages printed for the active session
    bool                m_bPrint2File;              // true, if we are not using a real printer
    FILE*               m_fp;                       // the filepointer when writing to a document
    bool                m_bAutoWrap;                // will wrap text to next line if more chars then m_iMaxCharsOnLine


    HFONT               m_hOriginalFont;            // receives the original font when setting a new one
    HFONT               m_hNewFont;                 // the currently created new font
    wxString            m_sLineBuffer;              // buffer for characters till next \n or \f
    wxString            m_sPrinterName;             // the name of the current active printer
    wxString            m_sPrinterFont;             // the currently used font for the printer
    wxString            m_sPageTitle;               // the title is printed at the top of a (new) page

    #define X(x) ((x)*m_iCharWidth  + m_iHorizontalOffset)  //determine the horizontal print position
    #define Y(y) ((y)*m_iCharHeight + m_iVerticalOffset  )  //determine the vertical print position

};

/*
    This is normally the only one printerclass.
    The methods or called through global "C"-functions (extern "C")
*/
static MyLinePrinter thePrintclass;

MyLinePrinter::MyLinePrinter()
: m_hPrinter            ( 0 )
, m_bPrinting           ( false )
, m_bPageStarted        ( false )
, m_iPrintjobId         ( 0 )
, m_iLinesOnPage        ( 0 )
, m_iCharWidth          ( 0 )
, m_iCharHeight         ( 0 )
, m_iHorizontalOffset   ( 0 )
, m_iVerticalOffset     ( 0 )
, m_iDeviceHeight       ( 0 )
, m_iDeviceWidth        ( 0 )
, m_iMaxCharsOnLine     ( 80 )
, m_iMaxLinesOnPage     ( 64 )
, m_iDefaultLpp4List    ( 64 )
, m_iPointSize          ( sc_iDefaultPointsize )
, m_iPagesPrinted       ( 0 )
, m_bPrint2File         ( false )
, m_fp                  ( nullptr )
, m_bAutoWrap           ( true )
, m_hOriginalFont       ( 0 )
, m_hNewFont            ( 0 )
, m_sPrinterFont        ( sc_cDefaultFont )
{ 
    m_sLineBuffer.reserve(100);         // assume single line always smaller then 100 chars
}   // MyLinePrinter()

MyLinePrinter::~MyLinePrinter()
{
    try
    {   //methods could possibly throw an exception according lint??
        Clear();    //clean everything as needed
        (void)PrinterClose();
        if (m_fp) (void)fclose(m_fp);
    }
    catch (...) { ; }
}   // ~MyLinePrinter()

bool MyLinePrinter::OpenDiskfile4Print()
{
    if (m_bPrint2File && m_fp == nullptr)
    {
        wxString fileName  = cfg::GetActiveMatchPath()+cfg::GetFilePrinterName();
        bool     bWriteBom = !wxFileExists(fileName);
        m_fp = _fsopen(fileName.mb_str(wxConvUTF8), "a", _SH_DENYWR);   // fopen_s() does not allow read for other apps
        if (m_fp == nullptr)
            m_bPrint2File = false;
        else if (bWriteBom)
        {
            const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };    // utf8
            (void)fwrite(bom, 1, sizeof(bom), m_fp);
        }
    }
    return m_bPrint2File;
}   // OpenDiskfile4Print()

bool MyLinePrinter::PrintAFile(const wxString& a_fileName, const wxString& a_title)
{
    if (m_bPrint2File)
    {
        wxString msg = _("Cant print a file to a file!");
        LogMessage(msg);
        wxMessageBox(msg);
        return false;
    }

    bool    bResult = true;
    FILE*   fp      = 0;
    char*   buf     = 0;

    do
    {
        if (a_fileName.IsEmpty())
        {
            bResult = false;   // no filename given?
            break;
        }

        if (!BeginPrint(a_title))
        {
            bResult = false;   // printer error??
            break;
        }

        auto err = fopen_s(&fp, a_fileName.char_str(), "rb"); MY_UNUSED(err);
        if (!fp)
        {
            bResult = false;   // can't open file: no file or not allowed
            break;
        }

        #define BUF_SIZE_ 1024
        try
        {
            buf = new char[BUF_SIZE_];
        } catch (...) { ; }

        if (!buf )
        {
            bResult = false;        // out of memory??
            break;   
        }

        size_t count;
        while (bResult && ( 0 != (count = fread(buf, sizeof(buf[0]), BUF_SIZE_, fp))))
        {
            wxString cnv = Ascii2Unicode(buf, count );
            bResult = PrintLine(cnv);
//            bResult = std::all_of(cnv.begin(), cnv.end(), [this]( wxChar chr ){ return this->PrintCharacter(chr);});
        }
    } while (0);            // execute body only once

    EndPrint();             // print last line/page
    if (fp) (void)fclose(fp);
    delete[] buf;
    m_sPageTitle.clear();
    return bResult;
}   // PrintAFile()

void MyLinePrinter::InitFont(LOGFONT* pLogFontInfo)
{
    if (!PrinterOpen())
    {   // we really do need a printer DC for this to work...
        return;
    }

    LOGFONT lf = {0};

    if (pLogFontInfo == nullptr)
    {
        pLogFontInfo = &lf;
        // Initialize members of the LOGFONT structure.  
//      lf.lfCharSet        = FS_LATIN1 ;
        lf.lfCharSet        = ANSI_CHARSET;
        lf.lfOutPrecision   = OUT_STRING_PRECIS;        // OUT_CHARACTER_PRECIS=2, OUT_STRING_PRECIS=1, OUT_DEFAULT_PRECIS=0, OUT_STROKE_PRECIS=3, OUT_DEVICE_PRECIS=5, OUT_TT_PRECIS=4, OUT_RASTER_PRECIS=6
        lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;  // DEFAULT_PITCH=0, VARIABLE_PITCH=2, or FIXED_PITCH=1 and OR of FF_DECORATIVE=5<<4 FF_DONTCARE=0<<4 FF_MODERN=3<<4 FF_ROMAN=1<<4 FF_SCRIPT=4<<4 FF_SWISS=2<<4
        lf.lfQuality        = DRAFT_QUALITY;            // DEFAULT_QUALITY=0, PROOF_QUALITY=2, and DRAFT_QUALITY=1
        lf.lfWeight         = FW_NORMAL;                // FW_NORMAL=400, FW_THIN =100, FW_EXTRALIGHT=200,FW_LIGHT=300,FW_MEDIUM=500,FW_SEMIBOLD=600,FW_BOLD=700,FW_EXTRABOLD=800,FW_HEAVY=900
        wcscpy(lf.lfFaceName, m_sPrinterFont.c_str());
        //    long pointSize = MulDiv(std::abs(lf.lfHeight), 72, GetDeviceCaps(m_hPrinter, LOGPIXELSY));
        lf.lfHeight         = -MulDiv(m_iPointSize, GetDeviceCaps(m_hPrinter, LOGPIXELSY), 72);   // set font to x point: '-' means: find a compatible font
    }
    else
    {
        long height  =  -MulDiv(std::abs(pLogFontInfo->lfHeight), GetDeviceCaps(m_hPrinter, LOGPIXELSY), 72);   // set font to x point: '-' means: find a compatible font
        pLogFontInfo->lfHeight = (height*11)/15;
    }
    // int pointsize, logheight = abs(lf.lfHeight);
    // pointsize = MulDiv(logheight, 72, GetDeviceCaps(a_hdc, LOGPIXELSY));
    // To convert from points back to logical device coordinates, use
    // logheight = -MulDiv(pointsize, GetDeviceCaps(a_hdc, LOGPIXELSY), 72);

    /*
    lf:
    - lfHeigth          = 'see'below'
    - lfWidth           = 0
    - lfEscapement      = 0
    - lfOrientation     = 0
    - lfWeight          = 400 = FW_NORMAL=400, FW_THIN =100, FW_EXTRALIGHT=200,FW_LIGHT=300,FW_MEDIUM=500,FW_SEMIBOLD=600,FW_BOLD=700,FW_EXTRABOLD=800,FW_HEAVY=900
    - lfCharset         = 0 = ANSI_CHARSET = 0, OEM_CHARSET=255
    - lfOutPrecision    = 3 = OUT_CHARACTER_PRECIS=2, OUT_STRING_PRECIS=1, OUT_DEFAULT_PRECIS=0, OUT_STROKE_PRECIS=3, OUT_DEVICE_PRECIS=5, OUT_TT_PRECIS=4, OUT_RASTER_PRECIS=6
    - lfClipPrecision   = 2 = CLIP_CHARACTER_PRECIS=1, CLIP_MASK=0x0f, CLIP_DEFAULT_PRECIS=0, CLIP_STROKE_PRECIS=2, CLIP_ENCAPSULATE=, CLIP_TT_ALWAYS=2<<4, CLIP_LH_ANGLES=1<<4
    - lfQuality         = 1 = DEFAULT_QUALITY=0, PROOF_QUALITY=2, and DRAFT_QUALITY=1
    - lfPitchAndFamily  = 0x31 = DEFAULT_PITCH=0, VARIABLE_PITCH=2, or FIXED_PITCH=1 and OR of FF_DECORATIVE=5<<4 FF_DONTCARE=0<<4 FF_MODERN=3<<4 FF_ROMAN=1<<4 FF_SCRIPT=4<<4 FF_SWISS=2<<4
    - lfFaceName        = "Consolas" "Courier New" "Fixedsys" "Lucida Console" "Terminal"
    */

    if (m_hNewFont != 0)
    {   // delete previous created font to prevent leaks;
        DeleteObject( m_hNewFont );
    }

    m_hNewFont = CreateFontIndirect(pLogFontInfo);       // Create a logical font based on the user's selection.  

    HFONT tmpFont = reinterpret_cast<HFONT>(SelectObject(m_hPrinter, m_hNewFont));     //activate the new font

    if (m_hOriginalFont == 0)
    {   // first time creation, so keep original for later restore (don't know if needed)
        m_hOriginalFont = tmpFont;
    }

    // now printerinfo is set, re-calc our values
    SIZE        size;
    TEXTMETRIC  textMetrics;
    bool        bTm     = GetTextMetrics(m_hPrinter, &textMetrics);
    bool        bTE     = GetTextExtentPoint32(m_hPrinter, wxT("AE34W"), 5, &size);

    (void)bTm;(void)bTE;  // satisfy lint for not using these vars..
    long chWidth        = textMetrics.tmAveCharWidth;   if (chWidth){}
    long chHeight       = textMetrics.tmHeight;         if (chHeight){}

    m_iCharWidth        = size.cx / 5;
    m_iCharHeight       = size.cy;
    m_iHorizontalOffset = sc_iHorizontalMargin * m_iCharWidth;      // set left margin
    m_iVerticalOffset   = sc_iVerticalMargin * m_iCharHeight;       // set top  margin
    m_iDeviceHeight     = GetDeviceCaps(m_hPrinter, PHYSICALHEIGHT);
    m_iDeviceWidth      = GetDeviceCaps(m_hPrinter, PHYSICALWIDTH);

    int vRes            = GetDeviceCaps(m_hPrinter, VERTRES);       if (vRes){}         // usable nr of lines
    int hRes            = GetDeviceCaps(m_hPrinter, HORZRES);       if (hRes){}         // usable horizontal pixels
    int technology      = GetDeviceCaps(m_hPrinter, TECHNOLOGY);    if (technology){}   // DT_RASPRINTER       2   /* Raster printer  */

    m_iMaxCharsOnLine   = m_iDeviceWidth  / m_iCharWidth  - 2*sc_iHorizontalMargin;
    m_iMaxLinesOnPage   = m_iDeviceHeight / m_iCharHeight - 2*sc_iVerticalMargin;
}   // InitFont()

void MyLinePrinter::SetFont(const wxString& a_font, LOGFONT* pLogFontInfo )
{
    m_sPrinterFont = a_font;
    InitFont(pLogFontInfo);
}   // SetFont()

void MyLinePrinter::SelectFont()
{
    LOGFONT     lf = {0};
    CHOOSEFONT  cf = {0};

    // Initialize members of the CHOOSEFONT structure.  
    cf.lStructSize      = sizeof( cf );
    lf.lfCharSet        = FS_LATIN1 ;
    cf.lpLogFont        = &lf;
    cf.Flags            = CF_PRINTERFONTS | CF_FIXEDPITCHONLY;
    //lint -e{835} : A zero has been given as left argument to operator '<<'
    cf.rgbColors        = RGB(0, 0, 0);
    cf.nFontType        = PRINTER_FONTTYPE;

#ifdef DEBUG_LOCAL
    char    fontBefore[100]     ={0};
    int     nrOfCharsCopied1    = GetTextFace(m_hPrinter, sizeof( fontBefore ), fontBefore); (void)nrOfCharsCopied1;  // for lint
#endif

#if 1
    if (!ChooseFont(&cf)) return;   // user pressed cancel
#else
    while (!ChooseFont(&cf))        // Display the font-dialog box, don't accept cancel!
    {
        MessageBox(0, _T("Invalid font"), 0, MB_OK);
    };
#endif
    SetFont(lf.lfFaceName, &lf); // init system with it

#ifdef DEBUG_LOCAL
    // now compare with what you have put in...
    char fontAfter[100]         = {0};
    int     nrOfCharsCopied2    = GetTextFace(m_hPrinter, sizeof( fontAfter ), fontAfter); (void)nrOfCharsCopied2; // for lint
    char buf[200] = {0};
    sprintf(buf, "Original font: %s, new font: %s, should be: %s\n", fontBefore, fontAfter, m_sPrinterFont.c_str());
    s_sDebugString = buf;
#endif
}   // SelectFont()

bool MyLinePrinter::PrinterClose()
{
    if (m_hPrinter == 0)
    {
        return true;
    }

    if (m_hOriginalFont && m_hNewFont)  // REMARK: '&&' m_hNewFont' only added to satisfy LINT
    {   // release old stuff and restore old situation.
        (void)SelectObject( m_hPrinter, m_hOriginalFont );
        (void)DeleteObject( m_hNewFont );
    }

    bool result     = DeleteDC( m_hPrinter );
    m_hNewFont      = 0;
    m_hOriginalFont = 0;
    m_hPrinter      = 0;
    return result;
}   // PrinterClose()

bool MyLinePrinter::PrinterOpen()
{
    if (m_hPrinter)
    {
        return true;    // printer already opened.
    }

    if (m_sPrinterName.IsEmpty())
    {
        if (!SelectPrinter())
        {
            return false;
        }
    }

    try
    {
        m_hPrinter = CreateDC(wxT("WINSPOOL"), m_sPrinterName.c_str(), 0, 0);
        // gives multiple(6): "Invalid parameter passed to C runtime function."
    } catch(...){m_hPrinter = 0;}

    if (m_hPrinter == 0)
    {
        return false;
    }
    ////////todo: setting an abortproc??
    //:todo: create a font: https://msdn.microsoft.com/en-us/library/windows/desktop/dd183499(v=vs.85).aspx
#if 0
    //********************************************************
    //Logical units are device dependent pixels, so this will create a handle to a logical font that is 48 pixels in height.
    //The width, when set to 0, will cause the font mapper to choose the closest matching value.
    //The font face name will be Impact.
    HFONT hFont;
    RECT rect;
    hFont = CreateFont(48, 0, 0, 0, FW_DONTCARE, false, true, false, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Impact"));
    SelectObject(m_hPrinter, hFont);

    //Sets the coordinates for the rectangle in which the text is to be formatted.
    SetRect(&rect, 100, 100, 700, 200);
    SetTextColor(m_hPrinter, RGB(255, 0, 0));
    DrawText(m_hPrinter, TEXT("Drawing Text with Impact"), -1, &rect, DT_NOCLIP);


    //Logical units are device dependent pixels, so this will create a handle to a logical font that is 36 pixels in height.
    //The width, when set to 20, will cause the font mapper to choose a font which, in this case, is stretched.
    //The font face name will be Times New Roman.  This time nEscapement is at -300 tenths of a degree (-30 degrees)
    hFont = CreateFont(36, 20, -300, 0, FW_DONTCARE, false, true, false, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Times New Roman"));
    SelectObject(m_hPrinter, hFont);

    //Sets the coordinates for the rectangle in which the text is to be formatted.
    SetRect(&rect, 100, 200, 900, 800);
    SetTextColor(m_hPrinter, RGB(0, 128, 0));
    DrawText(m_hPrinter, TEXT("Drawing Text with Times New Roman"), -1, &rect, DT_NOCLIP);


    //Logical units are device dependent pixels, so this will create a handle to a logical font that is 36 pixels in height.
    //The width, when set to 10, will cause the font mapper to choose a font which, in this case, is compressed. 
    //The font face name will be Arial. This time nEscapement is at 250 tenths of a degree (25 degrees)
    hFont = CreateFont(36, 10, 250, 0, FW_DONTCARE, false, true, false, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
    SelectObject(m_hPrinter, hFont);

    //Sets the coordinates for the rectangle in which the text is to be formatted.
    SetRect(&rect, 500, 200, 1400, 600);
    SetTextColor(m_hPrinter, RGB(0, 0, 255));
    DrawText(m_hPrinter, TEXT("Drawing Text with Arial"), -1, &rect, DT_NOCLIP);
//    DeleteObject(hFont);

    //********************************************************
#endif

    InitFont(); // get fontinfo into printer hdc and our own variables

#ifdef DEBUG_LOCAL
    char    fontUsed[1000] ={0};
    int     nrOfCharsCopied = GetTextFace(m_hPrinter, sizeof( fontUsed ), fontUsed); (void)nrOfCharsCopied; // lint..
    char    info[100];
    sprintf(info, "font: '%s', char width=%i, height=%i\n", fontUsed, m_iCharWidth, m_iCharHeight);
    s_sDebugString += info;
    sprintf(info, "max chars/line = %i, max lines/page = %i\n", m_iMaxCharsOnLine, m_iMaxLinesOnPage);
    s_sDebugString += info;
    MessageBox(0, s_sDebugString.c_str(), "Info on the FONT", MB_OK);
#endif

    return true;
}   // PrinterOpen()

bool MyLinePrinter::BeginPrint(const wxString& a_title)
{
    m_iLinesOnPage  = 0;
    m_iPagesPrinted = 0;
    m_sPageTitle    = a_title;

    if (m_bPrint2File)
    {
        return PrintLine('\n' + a_title + '\n');
    }

//    Clear();
    if (!PrinterOpen())
    {   // can't open printer!
        return false;
    }

    DOCINFO di      = {0};
    di.cbSize       = sizeof( di );
    di.lpszDocName  = wxT("wxBridgeHlTc.txt");
    m_iPrintjobId   = StartDoc(m_hPrinter, &di);
    m_bPrinting     = true;

    return true;
}   // BeginPrint()

void  MyLinePrinter::EndPrint()
{
    if (m_bPrint2File)
    {
        int result = fflush(m_fp);
        if (result)
            LogError(_("MyLinePrinter::EndPrint(): flushing error"));
        return;
    }

    // print the available data and close the printing process.
    if (m_bPrinting)
    {
        PrintPage();
        EndDoc(m_hPrinter);
        m_bPrinting = false;
        m_sPageTitle.clear();
    }
}   // EndPrint()

void  MyLinePrinter::Clear()
{
    //remark: we leave the printerconnection open, if there is one! because of performance.
    if (m_bPrinting)
    {
        if (m_bPageStarted)
        {
            PrintPage();
            m_bPageStarted = false;
        }

        EndDoc(m_hPrinter);
        m_bPrinting = false;
    }

    m_sLineBuffer.clear();  // should already be empty.
}   // Clear()

void MyLinePrinter::SetPrinterName(const wxString& a_printerName )
{
    if (m_sPrinterName == a_printerName)
    {
        return;             // nothing to do...
    }

    if (m_fp) (void) fclose(m_fp); m_fp = nullptr;
    (void)PrinterClose();   // may error....
    m_sPrinterName = a_printerName;

    if (cfg::GetFilePrinterName() == a_printerName)
    {
        m_iMaxLinesOnPage = m_iDefaultLpp4List;
        m_bPrint2File = true;
    }
    else
    {
        (void)PrinterOpen();
        m_bPrint2File = false;
    }
}   // SetPrinterName()

void  MyLinePrinter::Line2Printer()
{
    if (m_hPrinter == 0) return;        // there is no open printer yet, so just ignore

    /*
    Caution  Using the MultiByteToWideChar function incorrectly can compromise the security
    of your application. Calling this function can easily cause a buffer overrun because the
    size of the input buffer indicated by lpMultiByteStr equals the number of bytes in the
    string, while the size of the output buffer indicated by lpWideCharStr equals the number
    of characters. To avoid a buffer overrun, your application must specify a buffer size
    appropriate for the data type the buffer receives. For more information,
    see: https://msdn.microsoft.com/en-us/library/windows/desktop/dd374047(v=vs.85).aspx
    */
        if (!m_bPageStarted)
        {   //first line on a page, so we need to tell the printer to prepare a new page
            {
                bool bStatus = (bool)StartPage(m_hPrinter);     MY_UNUSED(bStatus);
            }

            m_bPageStarted = true;
        }

        if (m_iLinesOnPage == 0 && !m_sPageTitle.IsEmpty())
        {   // print header, if available
            wxString header = FMT("              %s %d: ", _("page"), 1+m_iPagesPrinted) + m_sPageTitle;
            (void)TextOut(m_hPrinter, X(0), Y(m_iLinesOnPage), header.c_str(), header.Len());
            ++m_iLinesOnPage;
        }

        m_sLineBuffer.Trim(TRIM_RIGHT);
        if (!m_sLineBuffer.IsEmpty())
        {
            bool bResult = TextOutW(m_hPrinter, X(0), Y(m_iLinesOnPage), m_sLineBuffer.c_str(), m_sLineBuffer.Len());          (void)bResult;
            m_sLineBuffer.clear();
        }

        m_iLinesOnPage++;
        if (m_iLinesOnPage >= m_iMaxLinesOnPage)
        {
            PrintPage();
        }
}   // Line2Printer()

void  MyLinePrinter::PrintPage()
{   // called at end of page: '\f'
    if (!m_sLineBuffer.IsEmpty()) Line2Printer();   // print the last line, if we have something;
    m_bPageStarted  = false;                        // was set in PrintLine()....
    m_iLinesOnPage  = 0;                            // back to first line on new page
    ++m_iPagesPrinted;
    (void)EndPage(m_hPrinter);
}   // PrintPage()

static bool PrintString2File(const wxString& a_string, FILE* a_fp)
{
    const wxScopedCharBuffer buf(a_string.ToUTF8());
    auto length = buf.length();
    return length == fwrite(buf, 1, length, a_fp);
}   // PrintString2File()

bool MyLinePrinter::PrintCharacter(wxChar a_char)
{
    if (m_bPrint2File && OpenDiskfile4Print())
    {
        // utf8 text
        if (a_char < 0x80)
        {   // assume no conversion
            return 1 == fwrite(&a_char, 1, 1, m_fp);
        }

        //convert the single character
        return PrintString2File(a_char, m_fp);
    }

    //if m_bPrint2File but OpenDiskfile4Print() failes, then next test will return false...
    if (!m_bPrinting)
    {
        return false;		// no open printer yet
    }

    switch (a_char)
    {
        case '\n':
            Line2Printer();
            break;
        case '\f':
            PrintPage();
            break;
        case '\r':
            //just ignore
            break;
        default:
            if (m_sLineBuffer.Len() >= static_cast<size_t>(m_iMaxCharsOnLine) && m_bAutoWrap)
            {
                Line2Printer();         // wrap to next line
            }
            m_sLineBuffer += a_char;    // all chars within a line are added to the line buffer
            break;
    }

    return true;
}   // PrintCharacter()

bool MyLinePrinter::PrintLine(const wxString& a_line)
{
    if (m_bPrint2File)
    {
        if (!OpenDiskfile4Print())
            return false;
        return PrintString2File(a_line, m_fp);
    }

    return std::all_of(a_line.begin(), a_line.end(), [this]( wxChar chr ){ return this->PrintCharacter(chr);});
}   // PrintLine()

const wxString& MyLinePrinter::GetPrinterName()const
{
    return m_sPrinterName;
}   // GetPrinterName()

bool MyLinePrinter::SelectPrinter()
{
    HRESULT     result  = S_FALSE;
    PRINTDLGEX  pdlg    = {0};

    // Initialize the PRINTDLGEX structure.
    pdlg.lStructSize    = sizeof( pdlg );
    pdlg.nCopies        = 1;
    pdlg.Flags          =       // Set the flags to determine the printername.
          PD_RETURNDC           // Return a printer device context
        | PD_HIDEPRINTTOFILE    // Don't allow separate print to file.
        | PD_DISABLEPRINTTOFILE // no file print, thats a separate choice
        | PD_NOSELECTION        // Don't allow selecting individual document pages to print.
        | PD_NOPAGENUMS         // no separate pages: we always print all
        | PD_NOCURRENTPAGE      //   ^--
//      | PD_ALLPAGES           // PD_ALLPAGES == 0 ???????????, lint complains about this
        | PD_EXCLUSIONFLAGS
        //      | PD_USEDEVMODECOPIESANDCOLLATE // don't allow change of nrOfCopies
#if 0
        | PD_RETURNDEFAULT      // for getting default printer without dialog
#endif
        ;
    pdlg.ExclusionFlags = PD_EXCL_COPIESANDCOLLATE;
    pdlg.nStartPage     = START_PAGE_GENERAL;
    pdlg.hwndOwner      = GetActiveWindow();

    if (pdlg.hwndOwner == 0)
        pdlg.hwndOwner = GetDesktopWindow();	// not sure if we need this..  yes, we do!

    result              = PrintDlgEx(&pdlg);	// Invoke the printer dialog box to get the wanted printer.
    DWORD resultAction  = pdlg.dwResultAction;	// PD_RESULT_APPLY=2 PD_RESULT_CANCEL=0 PD_RESULT_PRINT=1
    if (( result != S_OK ) || ( resultAction == PD_RESULT_CANCEL ) || ( pdlg.hDevNames == 0 ))
    {
        return false;   //something wrong, or user has cancelled.
    }

    PDEVMODE    pDefmode    = (PDEVMODE)GlobalLock(pdlg.hDevMode);                              (void)pDefmode;
    LPDEVNAMES  pDefNames   = (LPDEVNAMES)GlobalLock(pdlg.hDevNames);
    wxChar*     pDriverName = reinterpret_cast<wxChar*>(pDefNames) + pDefNames->wDriverOffset;  (void)pDriverName;
    wxChar*     pDeviceName = reinterpret_cast<wxChar*>(pDefNames) + pDefNames->wDeviceOffset;
    wxChar*     pOutput     = reinterpret_cast<wxChar*>(pDefNames) + pDefNames->wOutputOffset;  (void)pOutput;

    m_sPrinterName = pDeviceName;   // update member

    GlobalFree(pdlg.hDevMode);
    GlobalFree(pdlg.hDevNames);

    return true;
}   // SelectPrinter()

const wxString& MyLinePrinter::GetFont() const
{
    return m_sPrinterFont;
}   // GetFont()

void MyLinePrinter::SetLinesPerPage(int a_lines)
{
    m_iDefaultLpp4List = a_lines;
    if (m_bPrint2File)
        m_iMaxLinesOnPage = a_lines;
}   // SetLinesPerPage()

void MyLinePrinter::PrintTable(const table::TableInfo& table)
{   // print a set of lines and texts
    if (m_bPrint2File) return;      // not for now
    if (m_hPrinter == 0) return;    // no active printer

    for ( size_t index = 0; index < table.textCount; ++index)
    {   // print all the texts in the textarray
        wxPoint  pos  = table.texts[index].begin + table.origin;
        wxString text = table.texts[index].text;
        if (text.IsEmpty()) continue;
        (void)TextOut(m_hPrinter, X(pos.x), Y(pos.y), text.c_str(), text.Len());
    }
    for (const auto& textIt : table.textsV)
    {   // print all the texts in the textvector
        wxPoint  pos  = textIt.begin + table.origin;
        wxString text = textIt.text;
        if (text.IsEmpty()) continue;
        (void)TextOut(m_hPrinter, X(pos.x), Y(pos.y), text.c_str(), text.Len());
    }

    #define TO_X(pt) (X((pt).x)+m_iCharWidth/2)
    #define TO_Y(pt) (Y((pt).y)+m_iCharHeight/2)

    HPEN pen    = CreatePen(PS_SOLID, 10, RGB(0, 0, 0));
    HPEN oldPen = static_cast<HPEN> (SelectObject(m_hPrinter, pen));
    for (size_t index = 0; index < table.lineCount; ++index)
    {   // draw all the lines in the linearray
        wxPoint pos = table.origin + table.lines[index].begin;
        MoveToEx(m_hPrinter, TO_X(pos), TO_Y(pos), nullptr);
        pos = table.origin + table.lines[index].end;
        LineTo(m_hPrinter, TO_X(pos), TO_Y(pos));
    }
    for (const auto& lineIt : table.linesV)
    {   // draw all the lines in the linevector
        wxPoint pos = table.origin + lineIt.begin;
        MoveToEx(m_hPrinter, TO_X(pos), TO_Y(pos), nullptr);
        pos = table.origin + lineIt.end;
        LineTo(m_hPrinter, TO_X(pos), TO_Y(pos));
    }
    SelectObject(m_hPrinter, oldPen);
}   // PrintTable()

// external callable functions

bool BeginPrint(const wxString& a_title)
{
    return thePrintclass.BeginPrint(a_title);
}   // BeginPrint()

void EndPrint()
{
    thePrintclass.EndPrint();
}   // EndPrint()

void SetPrinterName(const wxString& a_printerName)
{
    thePrintclass.SetPrinterName( a_printerName );
}   // SetPrinterName()

bool PrintCharacter(wxChar a_char)
{
    return thePrintclass.PrintCharacter(a_char);
}   // PrintCharacter()

bool PrintLine(const wxString& a_line)
{
    return thePrintclass.PrintLine(a_line);
}   // PrintLine()

const wxString& GetPrinterName()
{
    return thePrintclass.GetPrinterName();
}   // GetPrinterName()

void SetFont(const wxString& a_font)
{
    thePrintclass.SetFont(a_font);
}   // SetFont()

const wxString& GetFont()
{
    return thePrintclass.GetFont();
}   // GetFont()

void SelectFont()
{
    return thePrintclass.SelectFont();
}   // SelectFont()

bool PrintAFile(const wxString& a_fileName, const wxString& a_title)
{
    return thePrintclass.PrintAFile( a_fileName, a_title );
}   // PrintAFile()

bool SelectPrinter()
{
    return thePrintclass.SelectPrinter();
}   // SelectPrinter()

void SetLinesPerPage(int a_lines)
{
    thePrintclass.SetLinesPerPage( a_lines );
}   // SetLinesPerPage()

UINT GetLinesPerPage ()
{
    return thePrintclass.GetLinesPerPage();
}   // GetLinesPerPage()

void PrintTable(const table::TableInfo& table)
{
    thePrintclass.PrintTable(table);
}   // PrintTable()

unsigned int GetCharsPerLine()
{
    return thePrintclass.GetCharsPerLine();
}   // GetCharsPerLine()

bool IsPrintToFile()
{
    return thePrintclass.IsPrintToFile();
}  // IsPrintToFile()


#if 0
///////////// test stuff
HFONT MyCreateFont(HDC a_hdc, char fontBuf[100])
{
    LOGFONT     lf2     ={0};
    HFONT       hfont;

    ZeroMemory(&lf2, sizeof( lf2 ));
    lf2.lfCharSet           = ANSI_CHARSET;
    lf2.lfOutPrecision      = OUT_STRING_PRECIS;        // OUT_CHARACTER_PRECIS=2, OUT_STRING_PRECIS=1, OUT_DEFAULT_PRECIS=0, OUT_STROKE_PRECIS=3, OUT_DEVICE_PRECIS=5, OUT_TT_PRECIS=4, OUT_RASTER_PRECIS=6
    lf2.lfPitchAndFamily    = FIXED_PITCH | FF_MODERN;  // DEFAULT_PITCH=0, VARIABLE_PITCH=2, or FIXED_PITCH=1 and OR of FF_DECORATIVE=5<<4 FF_DONTCARE=0<<4 FF_MODERN=3<<4 FF_ROMAN=1<<4 FF_SCRIPT=4<<4 FF_SWISS=2<<4
    lf2.lfCharSet           = ANSI_CHARSET;
    lf2.lfHeight            = -MulDiv(12, GetDeviceCaps(a_hdc, LOGPIXELSY), 72);    // always 12 point size!
    lf2.lfQuality           = DRAFT_QUALITY;            // DEFAULT_QUALITY=0, PROOF_QUALITY=2, and DRAFT_QUALITY=1
    strcpy(lf2.lfFaceName, "Consolas");
    hfont                   = CreateFontIndirect(&lf2);
    strcpy(fontBuf, lf2.lfFaceName);
#if 0
    return hfont;
#endif
    CHOOSEFONT  cf      ={0};
    LOGFONT     lf      ={0};

    // Initialize members of the CHOOSEFONT structure.  
    lf.lfCharSet = FS_LATIN1 ;// EASTEUROPE_CHARSET; //OEM_CHARSET;

    cf.lStructSize      = sizeof( CHOOSEFONT );
    cf.hwndOwner        = (HWND)nullptr;
    cf.hDC              = (HDC)nullptr;
    cf.lpLogFont        = &lf;
    cf.iPointSize       = 0;
    cf.Flags            = CF_PRINTERFONTS | CF_FIXEDPITCHONLY;
    cf.rgbColors        = RGB(0, 0, 0);
    cf.lCustData        = 0L;
    cf.lpfnHook         = (LPCFHOOKPROC)nullptr;
    cf.lpTemplateName   = (LPSTR)nullptr;
    cf.hInstance        = (HINSTANCE)nullptr;
    cf.lpszStyle        = (LPSTR)nullptr;
    cf.nFontType        = PRINTER_FONTTYPE;
    cf.nSizeMin         = 0;
    cf.nSizeMax         = 0;
    //    createfont: .fnWeight = FW_NORMAL;  //'normale' dikte vande chars

    // Display the CHOOSEFONT common-dialog box.  

    ChooseFont(&cf);

    int pointsize, logheight = abs(lf.lfHeight);
    (void)pointsize;
    ////    pointsize = MulDiv(logheight, 72, GetDeviceCaps(a_hdc, LOGPIXELSY));

    //To convert from points back to logical device coordinates, use

    ////    logheight = -MulDiv(pointsize, GetDeviceCaps(a_hdc, LOGPIXELSY), 72);

    /*
    lf:
    - lfHeigth          = 'see'below'
    - lfWidth           = 0
    - lfEscapement      = 0
    - lfOrientation     = 0
    - lfWeight          = 400 = FW_NORMAL=400, FW_THIN =100, FW_EXTRALIGHT=200,FW_LIGHT=300,FW_MEDIUM=500,FW_SEMIBOLD=600,FW_BOLD=700,FW_EXTRABOLD=800,FW_HEAVY=900
    - lfCharset         = 0 = ANSI_CHARSET = 0, OEM_CHARSET=255
    - lfOutPrecision    = 3 = OUT_CHARACTER_PRECIS=2, OUT_STRING_PRECIS=1, OUT_DEFAULT_PRECIS=0, OUT_STROKE_PRECIS=3, OUT_DEVICE_PRECIS=5, OUT_TT_PRECIS=4, OUT_RASTER_PRECIS=6
    - lfClipPrecision   = 2 = CLIP_CHARACTER_PRECIS=1, CLIP_MASK=0x0f, CLIP_DEFAULT_PRECIS=0, CLIP_STROKE_PRECIS=2, CLIP_ENCAPSULATE=, CLIP_TT_ALWAYS=2<<4, CLIP_LH_ANGLES=1<<4
    - lfQuality         = 1 = DEFAULT_QUALITY=0, PROOF_QUALITY=2, and DRAFT_QUALITY=1
    - lfPitchAndFamily  = 0x31 = DEFAULT_PITCH=0, VARIABLE_PITCH=2, or FIXED_PITCH=1 and OR of FF_DECORATIVE=5<<4 FF_DONTCARE=0<<4 FF_MODERN=3<<4 FF_ROMAN=1<<4 FF_SCRIPT=4<<4 FF_SWISS=2<<4
    - lfFaceName        = "Consolas" "Courier New" "Fixedsys" "Lucida Console" "Terminal"
    */
    // set font to 12 points: '-' means: find a compatible font
    lf.lfHeight = -MulDiv(12, GetDeviceCaps(a_hdc, LOGPIXELSY), 72);

    // Create a logical font based on the user's  
    // selection and return a handle identifying  
    // that font.  
    strcpy(fontBuf, lf.lfFaceName);
    hfont = CreateFontIndirect(&lf);
    return ( hfont );
}
#endif

}
