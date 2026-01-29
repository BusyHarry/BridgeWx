// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <wx/wx.h>

#include <wx/sizer.h>
#include <wx/image.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/dc.h>

#include "showStartImage.h"

static bool LoadImageFromResource(const wxString& resName, const wxString& resType, wxImage& image);

class MyImagePanel : public wxPanel
{
    wxImage     m_image;
    wxBitmap    m_resizedBM;
    int         m_width;
    int         m_height;

public:
    MyImagePanel(wxWindow* parent, const wxString& resName, wxBitmapType format);
    ~MyImagePanel(){}

    void paintEvent(wxPaintEvent & evt);
    void paintNow();
    void OnSize(wxSizeEvent& event);
    void render(wxDC& dc);

    // some useful events
    /*
    void mouseMoved(wxMouseEvent& event);
    void mouseDown(wxMouseEvent& event);
    void mouseWheelMoved(wxMouseEvent& event);
    void mouseReleased(wxMouseEvent& event);
    void rightClick(wxMouseEvent& event);
    void mouseLeftWindow(wxMouseEvent& event);
    void keyPressed(wxKeyEvent& event);
    void keyReleased(wxKeyEvent& event);
    */

    DECLARE_EVENT_TABLE()
};


BEGIN_EVENT_TABLE(MyImagePanel, wxPanel)
// some useful events
/*
EVT_MOTION(MyImagePanel::mouseMoved)
EVT_LEFT_DOWN(MyImagePanel::mouseDown)
EVT_LEFT_UP(MyImagePanel::mouseReleased)
EVT_RIGHT_DOWN(MyImagePanel::rightClick)
EVT_LEAVE_WINDOW(MyImagePanel::mouseLeftWindow)
EVT_KEY_DOWN(MyImagePanel::keyPressed)
EVT_KEY_UP(MyImagePanel::keyReleased)
EVT_MOUSEWHEEL(MyImagePanel::mouseWheelMoved)
*/

// catch paint events
EVT_PAINT(MyImagePanel::paintEvent)
//Size event
EVT_SIZE(MyImagePanel::OnSize)
END_EVENT_TABLE()


// some useful events
/*
void MyImagePanel::mouseMoved(wxMouseEvent& event) {}
void MyImagePanel::mouseDown(wxMouseEvent& event) {}
void MyImagePanel::mouseWheelMoved(wxMouseEvent& event) {}
void MyImagePanel::mouseReleased(wxMouseEvent& event) {}
void MyImagePanel::rightClick(wxMouseEvent& event) {}
void MyImagePanel::mouseLeftWindow(wxMouseEvent& event) {}
void MyImagePanel::keyPressed(wxKeyEvent& event) {}
void MyImagePanel::keyReleased(wxKeyEvent& event) {}
*/

static wxString ConvertType2String(wxBitmapType format)
{
    switch(format)
    {
        case wxBITMAP_TYPE_JPEG:
        case wxBITMAP_TYPE_JPEG_RESOURCE:
            return "JPG";
        case wxBITMAP_TYPE_PNG_RESOURCE:
        case wxBITMAP_TYPE_PNG:
            return "PNG";
        default:
            return "???";
    }
}   // ConvertType2String()

MyImagePanel::MyImagePanel(wxWindow* a_parent, const wxString& a_resName, wxBitmapType a_format) :
    wxPanel(a_parent)
{
    // load the file... ideally add a check to see if loading was successful
    if (!m_image.LoadFile(a_resName, a_format))
    {
        //    bool  bResult = LoadImageFromResource("BRIDGE_JPG", "JPG", m_image);
        if (!LoadImageFromResource(a_resName, ConvertType2String(a_format), m_image))
        {
            wxBitmap png(a_resName, a_format);
            m_image =  png.ConvertToImage();
            if (!m_image.IsOk()) {/*???*/ }
        }
    }

    m_width  = -1;
    m_height = -1;
}   // MyImagePanel()

/*
* Called by the system or by wxWidgets when the panel needs
* to be redrawn. You can also trigger this call by
* calling Refresh()/Update().
*/

void MyImagePanel::paintEvent(wxPaintEvent & )
{
    // depending on your system you may need to look at double-buffered dcs
    wxPaintDC dc(this);
    render(dc);
}   // paintEvent()

/*
* Alternatively, you can use a clientDC to paint on the panel
* at any time. Using this generally does not free you from
* catching paint events, since it is possible that e.g. the window
* manager throws away your drawing when the window comes to the
* background, and expects you will redraw it when the window comes
* back (by sending a paint event).
*/
void MyImagePanel::paintNow()
{
    // depending on your system you may need to look at double-buffered dcs
    wxClientDC dc(this);
    render(dc);
}   // paintNow()

/*
* Here we do the actual rendering. I put it in a separate
* method so that it can work no matter what type of DC
* (e.g. wxPaintDC or wxClientDC) is used.
*/
void MyImagePanel::render(wxDC&  a_dc)
{
    int newW, newH;
    a_dc.GetSize( &newW, &newH );

    if( newW != m_width || newH != m_height )
    {
        m_resizedBM = wxBitmap( m_image.Scale( newW, newH /*, wxIMAGE_QUALITY_HIGH*/ ) );
        m_width     = newW;
        m_height    = newH;
        a_dc.DrawBitmap( m_resizedBM, 0, 0, false );
    }
    else
    {
        a_dc.DrawBitmap( m_resizedBM, 0, 0, false );
    }
}   // render()

/*
* Here we call refresh to tell the panel to draw itself again.
* So when the user resizes the image panel the image should be resized too.
*/
void MyImagePanel::OnSize(wxSizeEvent& a_event)
{
    Refresh();
    //skip the event.
    a_event.Skip();
}   // OnSize()

static const char* bridges[]=
{   // bridge-pics included as resource
      "BRIDGE_JPG_X0"
    , "BRIDGE_JPG_X1"
    , "BRIDGE_JPG_X2"
    , "BRIDGE_JPG_X3"
    , "BRIDGE_JPG_X4"
    , "BRIDGE_JPG_X5"
};

void ShowStartImage(wxWindow* a_pParent)
{
    wxInitAllImageHandlers();

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    // then simply create like this
#if 1
    UINT rand = wxDateTime::UNow().GetMillisecond() % (sizeof(bridges)/sizeof(bridges[0]));
    auto drawPane = new MyImagePanel( a_pParent, bridges[rand], wxBITMAP_TYPE_JPEG_RESOURCE);
    //auto drawPane = new MyImagePanel( a_pParent, "BRIDGE_JPG", wxBITMAP_TYPE_JPEG_RESOURCE);
#else
    auto drawPane = new MyImagePanel( pParent, "BRIDGE_PNG", wxBITMAP_TYPE_PNG_RESOURCE);
#endif
    //    auto drawPane = new MyImagePanel( this, wxT("D:/bridge/BridgeWx/bridge2.jpg"), wxBITMAP_TYPE_JPEG);
    sizer->Add(drawPane, 1, wxEXPAND);

    a_pParent->SetSizer(sizer);
}   // ShowStartImage()

#include <wx/mstream.h>
//#include <wx/msw/private.h>
// https://forums.wxwidgets.org/viewtopic.php?t=18230&highlight=png
wxMemoryInputStream *GetResourceInputStream(const wxString& a_resource_name, const wxString& a_resource_type)
{
//    HMODULE aa = GetModuleHandle(nullptr);
//    HMODULE bb = wxGetInstance();
    HMODULE nil = nullptr;   // equals the HMODULE of the executable!

    HRSRC hrsrc=FindResource(nil, a_resource_name, a_resource_type);
    if(hrsrc==nullptr) return nullptr;

    HGLOBAL hglobal=LoadResource(nil, hrsrc);
    if(hglobal==nullptr) return nullptr;

    void *data=LockResource(hglobal);
    if(data==nullptr) return nullptr;

    DWORD datalen=SizeofResource(nil, hrsrc);
    if(datalen<1) return nullptr;

    return new wxMemoryInputStream(data, datalen);
}   // GetResourceInputStream()

bool HasResource(const wxString& a_resource_name, const wxString& a_resource_type)
{
    HMODULE nil = nullptr;   // equals the HMODULE of the executable!
    HRSRC hrsrc=FindResource(nil, a_resource_name, a_resource_type);
    if(hrsrc==nullptr) return false;

    HGLOBAL hglobal=LoadResource(nil, hrsrc);
    if(hglobal==nullptr) return false;

    return true;
}   // HasResource()

bool LoadImageFromResource(const wxString& a_resName, const wxString& a_resType, wxImage& a_image)
{
    bool bResult = false;
    wxMemoryInputStream* stream = GetResourceInputStream(a_resName, a_resType);
    if(stream != nullptr)
    {
        bResult = a_image.LoadFile(*stream);
        delete stream;
    }
    return bResult;
}   // LoadImageFromResource()
