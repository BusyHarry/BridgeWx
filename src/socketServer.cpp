// Copyright(c) 2024-present, BusyHarry/h.levels & BridgeWx contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "wx/socket.h"

#include "schemaInfo.h"
#include "slipServer.h"

enum
{
    // id for sockets
    SERVER_ID = 23456,
    SOCKET_ID
};

//typedef wxIPV6address IPaddress;
typedef wxIPV4address IPaddress;

void SlipServer::CreateNetworkWatcher(bool a_bCreate)
{
    if ( a_bCreate )
    {
        if ( nullptr == m_pSocketServer )
        {   // only create watcher if non exists
            Add2Log(_("Creating networkwatcher"), true);
            m_numClients = 0;
            // Create the address - defaults to localhost:0 initially
            IPaddress addr;
            addr.Service(SERVER_PORT);
            Add2Log(FMT(_("Creating server at %s:%u"), addr.IPAddress(), addr.Service()), true);
            // Create the socket
            m_pSocketServer = new wxSocketServer(addr);

            // We use IsOk() here to see if the server is really listening
            if ( !m_pSocketServer->IsOk() )
            {
                Add2Log(_("Could not listen at the specified port!"), true);
                return;
            }

            IPaddress addrReal;
            if ( !m_pSocketServer->GetLocal(addrReal) )
            {
                Add2Log(_("ERROR: couldn't get the address we bound to"), true);
            }
            else
            {
                Add2Log(FMT(_("Server listening at %s:%u"),
                    addrReal.IPAddress(), addrReal.Service()), true);
            }

            // Setup the event handler and subscribe to connection events
            m_pSocketServer->SetEventHandler(*this, SERVER_ID);
            m_pSocketServer->SetNotify(wxSOCKET_CONNECTION_FLAG);
            m_pSocketServer->Notify(true);
            Bind(wxEVT_SOCKET, &SlipServer::OnServerEvent, this, SERVER_ID);
            Bind(wxEVT_SOCKET, &SlipServer::OnSocketEvent, this, SOCKET_ID);
        }
    }
    else
    {   // delete wanted
        if ( m_pSocketServer )
        {
            Add2Log(_("Deleting networkwatcher"), true);
            Unbind(wxEVT_SOCKET, &SlipServer::OnServerEvent, this, SERVER_ID);
            Unbind(wxEVT_SOCKET, &SlipServer::OnSocketEvent, this, SOCKET_ID);
            delete m_pSocketServer;
            m_pSocketServer = nullptr;
        }
    }
}   // CreateNetworkWatcher()

void SlipServer::OnSocketEvent(wxSocketEvent& event)
{
    wxString msg = "OnSocketEvent: ";
    wxSocketBase *pSock = event.GetSocket();

    // First, print a message
    switch(event.GetSocketEvent())
    {
        case wxSOCKET_INPUT : msg.Append("wxSOCKET_INPUT"); break;
        case wxSOCKET_LOST  : msg.Append("wxSOCKET_LOST"); break;
        default             : msg.Append(_("Unexpected event!")); break;
    }

    Add2Log(msg, true);

    // Now we process the event
    switch(event.GetSocketEvent())
    {
        case wxSOCKET_INPUT:
        {
            // We disable input events, so that the test doesn't trigger
            // wxSocketEvent again.
            pSock->SetNotify(wxSOCKET_LOST_FLAG);
            unsigned char chr = 0;
            pSock->Read(&chr, 1);
            auto lastCount = pSock->LastReadCount(); MY_UNUSED(lastCount);
            switch (chr)
            {
                case SERVER_MSG_ID: SocketGetInput(pSock); break;
                default:
                    Add2Log(FMT(_("Unknown messageId(0x%02X) received from client"), chr), true);
                    break;
            }

            // Enable input events again.
            pSock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
            break;
        }
        case wxSOCKET_LOST:
        {
            m_numClients--;

            // Destroy() should be used instead of delete wherever possible,
            // due to the fact that wxSocket uses 'delayed events' (see the
            // documentation for wxPostEvent) and we don't want an event to
            // arrive to the event handler (the frame, here) after the socket
            // has been deleted. Also, we might be doing some other thing with
            // the socket at the same time; for example, we might be in the
            // middle of a test or something. Destroy() takes care of all
            // this for us.

            Add2Log(_("Deleting socket."), true);
            pSock->Destroy();
            break;
        }
        default:
        ;
    }
}   // OnSocketEvent()

void SlipServer::OnServerEvent(wxSocketEvent& event)
{
    wxString msg = "OnServerEvent: ";
    wxSocketBase *sock;

    switch(event.GetSocketEvent())
    {
        case wxSOCKET_CONNECTION : msg.Append("wxSOCKET_CONNECTION"); break;
        default                  : msg.Append(_("Unexpected event!")); break;
    }

    Add2Log(msg, true);

    // Accept new connection if there is one in the pending
    // connections queue, else exit. We use Accept(false) for
    // non-blocking accept (although if we got here, there
    // should ALWAYS be a pending connection).

    sock = m_pSocketServer->Accept(false);

    if (sock)
    {
        IPaddress addr;
        if ( !sock->GetPeer(addr) )
        {
            Add2Log(_("New connection from unknown client accepted."), true);
        }
        else
        {
            Add2Log(FMT(_("New client connection from %s:%u accepted"),
                addr.IPAddress(), addr.Service()), true);
        }
    }
    else
    {
        Add2Log(_("Error: couldn't accept a new connection"), true);
        return;
    }

    sock->SetEventHandler(*this, SOCKET_ID);
    sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    sock->Notify(true);

    m_numClients++;
}   // OnServerEvent()

void SlipServer::SocketGetInput(wxSocketBase* a_pSock)
{   // <id> is already read, now get the rest: [<id>]<len><msg> all uchars, <len> is length of <msg>
    a_pSock->SetFlags(wxSOCKET_WAITALL);
    unsigned char len = 0;
    a_pSock->Read(&len, 1);
    char inputBuf[255+1];
    a_pSock->Read(inputBuf, len);
    inputBuf[a_pSock->LastReadCount()] = 0;
    wxString input = inputBuf;
    input.Replace("\n", "\\n");
    Add2Log(FMT("%s: '%s'" ,_("received"), input), true);
    auto result = HandleResultLine(input);
    SocketPutResult(a_pSock, result, result == SlipResult::ERROR_NONE ? "" : inputBuf);
    Add2Log(_("Got the data, sending the result"), true);
}   // SocketGetInput()

void SlipServer::SocketPutResult(wxSocketBase* a_pSock, SlipResult a_error, const char a_buf[])
{ // send result buf: <id><err><len><buf>
    const auto MAX_BUF = 256;
    struct {unsigned char id; unsigned char error; unsigned char len; char out[MAX_BUF+1];} sendBuf;
    sendBuf.out[MAX_BUF] = 0;
    sendBuf.id           = SERVER_MSG_ID;
    sendBuf.error        = (unsigned char)a_error;
    if ( a_error == SlipResult::ERROR_NONE )
        sendBuf.out[0] = 0; // no error text returned
    else
        strncpy(sendBuf.out, a_buf, MAX_BUF);
    sendBuf.len = strlen(sendBuf.out);
    a_pSock->Write(&sendBuf, 3 + sendBuf.len);
}   // SocketPutResult()

#if 0
static wxString Ip2S(UINT ip)
{
    return FMT("%u.%u.%u.%u", ip & 0xff, (ip >> 8)&0xff, (ip>>16)&0xff, (ip >> 24)&0xff);
}

void TestIp()
{
    wxIPV4address addr;
    auto full = wxGetFullHostName();
    addr.Hostname(full);
    wxString ipAddr = addr.IPAddress();
    auto hbn = gethostbyname(full);
    struct hostent* hbn_1 = hbn;  MY_UNUSED(hbn_1);
    /*
    char    ** h_aliases;   alias list 
    short   h_addrtype;              host address type 
    short   h_length;                length of address 
    char    * * h_addr_list;
    */
    auto len = hbn->h_length; MY_UNUSED(len);
    auto name = hbn->h_name;  MY_UNUSED(name);
    auto lst = hbn->h_addr_list;
    auto p1 = lst[0]; UINT p1u = *(UINT*)p1; wxString ip1 = Ip2S(p1u);
    auto p2 = lst[1]; UINT p2u = *(UINT*)p2; wxString ip2 = Ip2S(p2u);
    auto p3 = lst[2]; UINT p3u = *(UINT*)p3; wxString ip3 = Ip2S(p3u);
    auto p4 = lst[3]; UINT p4u = *(UINT*)p4; wxString ip4 = Ip2S(p4u);
    //    char ips[1][4] = lst;
}   // TestIp()
#endif
