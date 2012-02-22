/*
 * Copyright (c) 2004, 2005  Andrei Vasiliu
 * 
 * 
 * This file is part of MudBot.
 * 
 * MudBot is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * MudBot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with MudBot; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define WINMAIN_ID "$Name: Release_6 $ $Id: winmain.c,v 3.1 2005/12/08 18:02:56 andreivasiliu Exp $"

/* Enable all things we need, from the Windows headers. */

//#define WINVER 0x0500

//#define _WIN32_IE 0x0500
#define _WIN32_IE 0x0300

#include <windows.h>
#include <winsock2.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <signal.h>

#include "header.h"

#include "winres.h"

#define UDM_WINSOCK_SELECT WM_USER + 999

#define TAB_CONSOLE 0
#define TAB_MODULES 1
#define TAB_MINIMIZE 2

char *winmain_id = WINMAIN_ID "\r\n";

int CreateMainWindow( HINSTANCE hInstance );
int CreateEditorWindow( HINSTANCE hInstance );
LRESULT CALLBACK MainWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK EditorWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

extern DESCRIPTOR *descs, *current_descriptor;
extern DESCRIPTOR *server, *client, *control;
extern MODULE *modules;
extern TIMER *timers;

extern char client_hostname[1024], server_hostname[1024];
extern int main_version_major, main_version_minor;
extern int logging;
extern int a_on;
extern int atcpoff;

extern char *mb_section;
extern char *desc_name;
extern char *desc_section;

int sock_list[256];
int iconized;
char *editor_buffer[3];
char *composer_contents;

/* Main Window Handles. */
HWND hwndMain;
HWND hwndEdit;
HWND hwndTab;
HWND hwndMods;

/* Editor Window Handles. */
HWND hwndEMain;
HWND hwndEEdit;
HWND hwndERebar;
HWND hwndEToolbar;

/* Colours for the Edit window. */
COLORREF crEditFg, crEditBg;
HBRUSH hbrEditBg;

/* Main instance handle. */
HINSTANCE hInstance;


void AddText( char *string )
{
   static char buf[4096];
   static int closed;
   int lines;
   
   if ( !strncmp( string, "cleanandclearitmessage",22 ) ) {
    closed = 0;
    SetWindowText( hwndEdit, NULL );
    strcpy(buf,"");    
    }
    
   if ( closed )
     return;
   
   if ( strlen( buf ) > 2048 )
     {
	strcat( buf, "Too many messages! Disabling the console.\r\n" );
	closed = 1;
     }
   else if (strncmp( string, "cleanandclearitmessage",22 )) 
     strcat( buf, string );
   else
   {
   strcat(buf, "Terminal Cleared.\r\n");
   }
   if ( !hwndEdit )
     return;
   
   SendMessage( hwndEdit, WM_SETTEXT, 0, (LPARAM) buf );
   
   /* Scroll all the way down. */
   lines = SendMessage( hwndEdit, EM_GETLINECOUNT, 0, 0 );
   SendMessage( hwndEdit, EM_LINESCROLL, 0, (LPARAM) lines );
   
   UpdateWindow( hwndMain );
}


void debugf( char *string, ... )
{
   void logff( char *type, char *string, ... );
   char buf [ 4096 ];
   char winbuf [ 4096 ], *p, *b;
   
   va_list args;
   va_start( args, string );
   vsnprintf( buf, 4096, string, args );
   va_end( args );
   
   p = winbuf, b = buf;
   while( *b )
     {
	if ( *b == '\n' )
	  *(p++) = '\r', *(p++) = '\n';
	else
	  *(p++) = *b;
	b++;
     }
   *(p++) = '\r';
   *(p++) = '\n';
   *p = 0;
   
   AddText( winbuf );
   
   if ( logging )
     logff( "debug", buf );
}


void win_sig_segv_handler( int sig )
{
   FILE *fl;
   extern char *debug[6];
   void crash_report( FILE *fl );
   int i;
   
   if ( logging )
     {
	debugf( "Segmentation fault received!" );
	debugf( "History:" );
	for ( i = 0; i < 6 && debug[i]; i++ )
	  debugf( " (%d) %s", i, debug[i] );
     }
   
   fl = fopen( "crash.log", "w" );
   
   if ( !fl )
     return;
   
   crash_report( fl );
   
   fclose( fl );
   
   MessageBox( NULL, "Segmentation Fault signal! File 'crash.log' written.", "MudBot", 0 );
   exit( 1 );
}



int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
   void mudbot_init( int port );
   
   MSG msg;
   
   hInstance = hInst;
   
   /* Initialize WSA. */
   WORD wVersionRequested;
   WSADATA wsaData;
   wVersionRequested = MAKEWORD( 1, 0 );
   
   if ( WSAStartup( wVersionRequested, &wsaData ) )
     {
	return 0;
     }
   
   if ( LOBYTE( wsaData.wVersion ) != 1 ||
	HIBYTE( wsaData.wVersion ) != 0 )
     {
	WSACleanup( );
	return 0;
     }
   /* End of WSA. */
   
   if ( !CreateMainWindow( hInstance ) )
     return 0;
   
   if ( !CreateEditorWindow( hInstance ) )
     return 0;
   
   ShowWindow( hwndMain, nShowCmd );
   
   mudbot_init( 123 );
   
   if ( !control )
     {
	MessageBox( NULL, "At least one port to listen on must be defined!", "MudBot Warning", 0 );
     }
   
   signal( SIGSEGV, win_sig_segv_handler );
   
   AddText( "Entering main loop... All ready.\r\n" );
   
   /* Main Loop. */
   while( GetMessage( &msg, NULL, 0, 0 ) )
     {
	TranslateMessage( &msg );
	DispatchMessage( &msg );
     }
   
   WSACleanup( );
   
   return msg.wParam;
}

int CreateMainWindow( HINSTANCE hInstance )
{
   WNDCLASSEX wndClass;
   RECT rcClient;
   TCITEM tab;
   INITCOMMONCONTROLSEX icc;
   HICON hIcon;
   HIMAGELIST iLarge, iSmall;
   char title[256];
   
   /*** Register our Main Window Class. ***/
   
   wndClass.cbSize = sizeof(WNDCLASSEX);
   wndClass.style = CS_DBLCLKS;
   wndClass.lpfnWndProc = MainWndProc;
   wndClass.cbClsExtra = 0;
   wndClass.cbWndExtra = 0;
   wndClass.hInstance = hInstance;
   wndClass.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( ID_MAIN_ICON ) );
   wndClass.hCursor = LoadCursor (NULL, IDC_ARROW);
   wndClass.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
   wndClass.lpszMenuName = NULL;
   wndClass.lpszClassName = "MainWindowClass";
   wndClass.hIconSm = NULL;
   
   if ( !RegisterClassEx( &wndClass ) )
     {
	MessageBox( NULL, "Unable to register the Main Window class!", "Window Creation Error", MB_ICONERROR );
	return 0;
     }
   
   /*** Initialize the Common Controls we need. ***/
   icc.dwSize = sizeof( icc );
   icc.dwICC = ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES;
   
   if ( !InitCommonControlsEx( &icc ) )
     {
	MessageBox( NULL, "Unable to initialize the needed Common Controls!"
		    " (Tab Control Classes and List View Classes)", "Window Creation Error", MB_ICONERROR );
	return 0;
     }
   
   /*** Create our Main Window. ***/
   
   sprintf( title, "Midnight v%d.%d", main_version_major, main_version_minor );
   
   hwndMain = CreateWindowEx( 0, "MainWindowClass", title,
			      WS_OVERLAPPEDWINDOW,
			      CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
			      NULL, NULL, hInstance, NULL );
   
   if ( !hwndMain )
     {
	MessageBox( NULL, "Unable to create the Main Window!", "Window Creation Error", MB_ICONERROR );
	return 0;
     }
   
   /*** Create the main Tab Control. ***/
   
   GetClientRect( hwndMain, &rcClient );
   
   hwndTab = CreateWindowEx( 0, WC_TABCONTROL, "",
			     WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
			     0, 0, rcClient.right, rcClient.bottom,
			     hwndMain, NULL, hInstance, NULL );
   
   if ( !hwndTab )
     {
	MessageBox( NULL, "Unable to create the Tab Control window!", "Window Creation Error", MB_ICONERROR );
	return 0;
     }
   
   /* And the tabs. */
   
   tab.mask = TCIF_TEXT;
   tab.iImage = -1;
   
   tab.pszText = "Console";
   TabCtrl_InsertItem( hwndTab, TAB_CONSOLE, &tab );
   
   tab.pszText = "Modules";
   TabCtrl_InsertItem( hwndTab, TAB_MODULES, &tab );
   
   tab.pszText = "Iconize";
   TabCtrl_InsertItem( hwndTab, TAB_MINIMIZE, &tab );
   
   /*** Create the Edit Window. ***/
   
   GetClientRect( hwndTab, &rcClient );
   TabCtrl_AdjustRect( hwndTab, FALSE, &rcClient );
   
   hwndEdit = CreateWindowEx( WS_EX_CLIENTEDGE, "EDIT", "",
			      WS_BORDER | WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY,
			      rcClient.left, rcClient.top, rcClient.right, rcClient.bottom,
			      hwndMain, NULL, hInstance, NULL );
   
   if ( !hwndEdit )
     {
	MessageBox( NULL, "Unable to create the main Console window!", "Window Creation Error", MB_ICONERROR );
	return 0;
     }
   
   /* Initialize its colours. */
   
   crEditFg = RGB( 0,255,0 );
   crEditBg = RGB( 0,0,0 );
   hbrEditBg = CreateSolidBrush( crEditBg );
   
   /*** Create the modules List View. ***/
   
   hwndMods = CreateWindowEx( WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
			      WS_BORDER | WS_CHILD | LVS_ICON,
			      rcClient.left, rcClient.top, rcClient.right, rcClient.bottom,
			      hwndMain, NULL, hInstance, NULL );
   
   if ( !hwndMods )
     {
	MessageBox( NULL, "Unable to create the List View window!", "Window Creation Error", MB_ICONERROR );
	return 0;
     }
   
   /* Create the Icon Lists for the List View. */
   
   iLarge = ImageList_Create( GetSystemMetrics( SM_CXICON ),
			      GetSystemMetrics( SM_CYICON ), ILC_MASK, 1, 1 );
   iSmall = ImageList_Create( GetSystemMetrics( SM_CXSMICON ),
			      GetSystemMetrics( SM_CYSMICON ), ILC_MASK, 1, 1 );
   
   hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( ID_MOD_ICON ) );
   ImageList_AddIcon( iLarge, hIcon );
   ImageList_AddIcon( iSmall, hIcon );
   
   DestroyIcon( hIcon );
   
   /* Assign the Icon Lists to the module List View. */
   
   ListView_SetImageList( hwndMods, iLarge, LVSIL_NORMAL );
   ListView_SetImageList( hwndMods, iSmall, LVSIL_SMALL );
   
   return 1;
}


int CreateEditorWindow( HINSTANCE hInstance )
{
   INITCOMMONCONTROLSEX icc;
   WNDCLASSEX wndClass;
   RECT rcClient;
//   REBARINFO rbi;
//   REBARBANDINFO rbBand;
//   TBBUTTON tbb[2];
//   DWORD dwBtnSize;
   HGDIOBJ hObj;
//   int iSend, iClear, i;
   
   /*** Register our Editor Window Class. ***/
   
   wndClass.cbSize = sizeof( WNDCLASSEX );
   wndClass.style = CS_DBLCLKS;
   wndClass.lpfnWndProc = EditorWndProc;
   wndClass.cbClsExtra = 0;
   wndClass.cbWndExtra = 0;
   wndClass.hInstance = hInstance;
   wndClass.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( ID_MAIN_ICON ) );
   wndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
   wndClass.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
   wndClass.lpszMenuName = MAKEINTRESOURCE( ID_EDITOR_MENU );
   wndClass.lpszClassName = "EditorWindowClass";
   wndClass.hIconSm = NULL;
   
   if ( !RegisterClassEx( &wndClass ) )
     {
	MessageBox( NULL, "Unable to register the Main Window class!", "Window Creation Error", MB_ICONERROR );
	return 0;
     }
   
   /*** Initialize the Common Controls we need. ***/
   icc.dwSize = sizeof( icc );
   icc.dwICC = ICC_BAR_CLASSES | ICC_COOL_CLASSES;
   
   if ( !InitCommonControlsEx( &icc ) )
     {
	MessageBox( NULL, "Unable to initialize the needed Common Controls!"
		    " (Toolbar Classes and Rebar Classes)", "Window Creation Error", MB_ICONERROR );
	return 0;
     }
   
   /*** Create the Editor's Main Window. ***/
   
   hwndEMain = CreateWindowEx( 0, "EditorWindowClass", "Editor",
			       WS_OVERLAPPEDWINDOW,
			       CW_USEDEFAULT, CW_USEDEFAULT, 700, 500,
			       NULL, NULL, hInstance, NULL );
   
   if ( !hwndEMain )
     {
	MessageBox( NULL, "Unable to create the Editor's Main Window!", "Window Creation Error", MB_ICONERROR );
	return 0;
     }
   
   /*** Create the Editor's Edit Window. ***/
   
   GetClientRect( hwndEMain, &rcClient );
   
   hwndEEdit = CreateWindowEx( WS_EX_CLIENTEDGE, "EDIT", "",
			       WS_BORDER | WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
			       rcClient.left, rcClient.top, rcClient.right, rcClient.bottom,
			       hwndEMain, NULL, hInstance, NULL );
   
   if ( !hwndEEdit )
     {
	MessageBox( NULL, "Unable to create the Editor's Edit Window!", "Window Creation Error", MB_ICONERROR );
	return 0;
     }
   
   /* Set the Font to Fixed. */
   
   hObj = GetStockObject( ANSI_FIXED_FONT );
   
   if ( hObj )
     SendMessage( hwndEEdit, WM_SETFONT, (WPARAM) hObj, FALSE );
   
#if 0
   /*** Create the Editor's Toolbar Window. ***/
   
   hwndEToolbar = CreateWindowEx( 0, TOOLBARCLASSNAME, NULL,
				  WS_CHILD | WS_VISIBLE | CCS_ADJUSTABLE | TBSTYLE_FLAT,
				  0, 0, 0, 0,
				  hwndEMain, NULL, hInstance, NULL );
   
   SendMessage(hwndEToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof( TBBUTTON ), 0 );
   
   /* Load the strings. */
   
   iSend = SendMessage( hwndEToolbar, TB_ADDSTRING, 0, (LPARAM) (LPSTR) "Send" );
   iClear = SendMessage( hwndEToolbar, TB_ADDSTRING, 0, (LPARAM) (LPSTR) "Clear" );
   
   /* Load the buttons. */
   
   i = 0;
   
   tbb[i].iBitmap = I_IMAGENONE;
   tbb[i].idCommand = 0;
   tbb[i].fsState = TBSTATE_ENABLED;
   tbb[i].fsStyle = BTNS_BUTTON;
   tbb[i].dwData = 0;
   tbb[i++].iString = iSend;
   
   tbb[i].iBitmap = I_IMAGENONE;
   tbb[i].idCommand = 0;
   tbb[i].fsState = TBSTATE_ENABLED;
   tbb[i].fsStyle = BTNS_BUTTON;
   tbb[i].dwData = 0;
   tbb[i++].iString = iClear;
   
   SendMessage( hwndEToolbar, TB_ADDBUTTONS, (WPARAM) i,
		(LPARAM) (LPTBBUTTON) &tbb );
   
   SendMessage( hwndEToolbar, TB_AUTOSIZE, 0, 0 );
   
   /*** Create the Editor's Rebar Window. ***/
   
   hwndERebar = CreateWindowEx( WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL,
				WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | RBS_VARHEIGHT | CCS_NODIVIDER,
				0, 0, 100, 100,
				hwndEMain, NULL, hInstance, NULL );
   
   if ( !hwndERebar )
     {
	MessageBox( NULL, "Unable to create the Editor's Rebar Window!", "Window Creation Error", MB_ICONERROR );
	return 0;
     }
   
   /* Initialize common members. */
   
   rbi.cbSize = sizeof( REBARINFO );
   rbi.fMask  = 0;
   rbi.himl   = (HIMAGELIST) NULL;
   
   SendMessage( hwndERebar, RB_SETBARINFO, 0, (LPARAM) &rbi );
   
   rbBand.cbSize = sizeof( REBARBANDINFO );
   rbBand.fMask  = RBBIM_COLORS | RBBIM_TEXT | RBBIM_BACKGROUND | 
     RBBIM_STYLE | RBBIM_CHILD  | RBBIM_CHILDSIZE | 
     RBBIM_SIZE;
   rbBand.fStyle = RBBS_CHILDEDGE | RBBS_FIXEDBMP;
   
   //    rbBand.hbmBack = LoadBitmap( hInstance, MAKEINTRESOURCE( IDB_BACKGRND ) );
	
   dwBtnSize = SendMessage( hwndEToolbar, TB_GETBUTTONSIZE, 0, 0 );
   
   rbBand.lpText = "Toolbar";
   rbBand.hwndChild = hwndEToolbar;
   rbBand.cxMinChild = 0;
   rbBand.cyMinChild = HIWORD( dwBtnSize );
   rbBand.cx = 200;
   
   SendMessage( hwndERebar, RB_INSERTBAND, (WPARAM) -1, (LPARAM) &rbBand );
#endif
   
   return 1;
}


void win_show_editor( )
{
   if ( !hwndEMain )
     if ( !CreateEditorWindow( hInstance ) )
       return;
   
   ShowWindow( hwndEMain, SW_SHOW );
   SetActiveWindow( hwndEMain );
}



void win_composer_contents( char *string )
{
   MENUITEMINFO miiMenu;
   
   if ( composer_contents )
     free( composer_contents );
   
   composer_contents = strdup( string );
   
   miiMenu.cbSize = sizeof(MENUITEMINFO);
   miiMenu.fMask = MIIM_STATE;
   miiMenu.fState = MFS_ENABLED;
   SetMenuItemInfo( GetMenu( hwndEMain ), ID_EDM_LOADCOMPOSER, FALSE, &miiMenu);
}



void LoadComposer( )
{
   char buf[65536];
   char *p, *b;
   
   p = composer_contents;
   b = buf;
   
   /* Convert both "\n" and "\r\n" to "\r\n" */
   
   while( *p )
     {
	if ( *p == '\r' && *(p+1) == '\n' )
	  {
	     *(b++) = *(p++);
	     *(b++) = *(p++);
	     continue;
	  }
	
	if ( *p == '\n' )
	  *(b++) = '\r';
	
	*(b++) = *(p++);
     }
   
   *b = 0;
   
   SendMessage( hwndEEdit, WM_SETTEXT, 0, (LPARAM) buf );
}



void MakeTrayTooltip( char *buf )
{
   sprintf( buf, "Midnight v%d.%d (Client: %s - Server: %s)",
	    main_version_major, main_version_minor,
	    client ? ( client_hostname[0] ? client_hostname : "unknown" ) : "not connected",
	    server ? ( server_hostname[0] ? server_hostname : "unknown" ) : "not connected" );
   buf[63] = 0;
}



void Iconize( )
{
   NOTIFYICONDATA nid;
   char buf[1024];
   
   /* Create a System Tray icon. */
   
   MakeTrayTooltip( buf );
   
   nid.cbSize = sizeof( NOTIFYICONDATA );
   nid.hWnd = hwndMain;
   nid.uID = 0;
   nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
   nid.uCallbackMessage = WM_USER;
   nid.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( ID_MAIN_ICON ) );
   lstrcpy( nid.szTip, buf );
   Shell_NotifyIcon( NIM_ADD, &nid );
   
   iconized = 1;
}



char *win_str_error( int error )
{
   switch( error )
     {
      case WSAEINTR:
	return "Interrupted function call";
      case WSAEACCES:
	return "Permission denied";
      case WSAEFAULT:
	return "Bad address";
      case WSAEINVAL:
	return "Invalid argument";
      case WSAEMFILE:
	return "Too many open files";
      case WSAEWOULDBLOCK:
	return "Resource temporarily unavailable";
      case WSAEINPROGRESS:
	return "Operation now in progress";
      case WSAEALREADY:
	return "Operation already in progress";
      case WSAENOTSOCK:
	return "Socket operation on nonsocket";
      case WSAEDESTADDRREQ:
	return "Destination address required";
      case WSAEMSGSIZE:
	return "Message too long";
      case WSAEPROTOTYPE:
	return "Protocol wrong type for socket";
      case WSAENOPROTOOPT:
	return "Bad protocol option";
      case WSAEPROTONOSUPPORT:
	return "Protocol not supported";
      case WSAESOCKTNOSUPPORT:
	return "Socket type not supported";
      case WSAEOPNOTSUPP:
	return "Operation not supported";
      case WSAEPFNOSUPPORT:
	return "Protocol family not supported";
      case WSAEAFNOSUPPORT:
	return "Address family not supported by protocol family";
      case WSAEADDRINUSE:
	return "Address already in use";
      case WSAEADDRNOTAVAIL:
	return "Cannot assign requested address";
      case WSAENETDOWN:
	return "Network is down";
      case WSAENETUNREACH:
	return "Network is unreachable";
      case WSAENETRESET:
	return "Network dropped connection on reset";
      case WSAECONNABORTED:
	return "Software caused connection abort";
      case WSAECONNRESET:
	return "Connection reset by peer";
      case WSAENOBUFS:
	return "No buffer space available";
      case WSAEISCONN:
	return "Socket is already connected";
      case WSAENOTCONN:
	return "Socket is not connected";
      case WSAESHUTDOWN:
	return "Cannot send after socket shutdown";
      case WSAETIMEDOUT:
	return "Connection timed out";
      case WSAECONNREFUSED:
	return "Connection refused";
      case WSAEHOSTDOWN:
	return "Host is down";
      case WSAEHOSTUNREACH:
	return "No route to host";
      case WSAEPROCLIM:
	return "Too many processes";
      case WSASYSNOTREADY:
	return "Network subsystem is unavailable";
      case WSAVERNOTSUPPORTED:
	return "Winsock.dll version out of range";
      case WSANOTINITIALISED:
	return "Successful WSAStartup not yet performed";
      case WSAEDISCON:
	return "Graceful shutdown in progress";
      case WSATYPE_NOT_FOUND:
	return "Class type not found";
      case WSAHOST_NOT_FOUND:
	return "Host not found";
      case WSATRY_AGAIN:
	return "Nonauthoritative host not found";
      case WSANO_RECOVERY:
	return "This is a nonrecoverable error";
      case WSANO_DATA:
	return "Valid name, no data record of requested type";
      case WSA_INVALID_HANDLE:
	return "Specified event object handle is invalid";
      case WSA_INVALID_PARAMETER:
	return "One or more parameters are invalid";
      case WSA_IO_INCOMPLETE:
	return "Overlapped I/O event object not in signaled state";
      case WSA_IO_PENDING:
	return "Overlapped operations will complete later";
      case WSA_NOT_ENOUGH_MEMORY:
	return "Insufficient memory available";
      case WSA_OPERATION_ABORTED:
	return "Overlapped operation aborted";
      case WSASYSCALLFAILURE:
	return "System call failure";
     }
   
   return "Unknown error";
}


void win_update_descriptors( )
{
   DESCRIPTOR *d;
   int i;
   
   for ( i = 0; i < 256; i++ )
     if ( sock_list[i] )
       {
	  WSAAsyncSelect( i, hwndMain, 0, 0 );
	  sock_list[i] = 0;
       }
   
   i = 0;
   
   for ( d = descs; d; d = d->next )
     {
	if ( d->fd < 1 )
	  continue;
	
	if ( d->callback_in )
	  WSAAsyncSelect( d->fd, hwndMain, UDM_WINSOCK_SELECT,
			  ( d->callback_out ? FD_WRITE : 0 ) | FD_READ | FD_CLOSE | FD_ACCEPT );
	
	sock_list[i++] = d->fd;
     }
   
   /* Update the System Tray icon. */
   if ( iconized )
     {
	char buf[1024];
	NOTIFYICONDATA nid;
	
	MakeTrayTooltip( buf );
	
	nid.cbSize = sizeof( NOTIFYICONDATA );
	nid.hWnd = hwndMain;
	nid.uID = 0;
	nid.uFlags = NIF_ICON | NIF_TIP;
	nid.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( ID_MAIN_ICON ) );
	lstrcpy( nid.szTip, buf );
	Shell_NotifyIcon( NIM_MODIFY, &nid );
     }
}


void win_update_modules( )
{
   MODULE *mod;
   LVITEM lvi;
   int i = 0;
   
   if ( !hwndMods )
     return;
   
   ListView_DeleteAllItems( hwndMods );
   
   lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
   lvi.state = 0;
   lvi.stateMask = 0;
   
   for ( mod = modules; mod; mod = mod->next )
     {
	lvi.iItem = i++;
	lvi.iImage = 0;
	lvi.iSubItem = 0;
	lvi.pszText = mod->name;
	
	ListView_InsertItem( hwndMods, &lvi );
     }
   
   UpdateWindow( hwndMods );
}


void check_descriptors( )
{
   void check_timers( );
   struct timeval pulsetime;
   fd_set in_set;
   fd_set out_set;
   fd_set exc_set;
   DESCRIPTOR *d, *d_next;
   int maxdesc = 0;
   
   mb_section = "Checking timers";
   /* Check all timers. */
   check_timers( );
   mb_section = NULL;
   
   FD_ZERO( &in_set );
   FD_ZERO( &out_set );
   FD_ZERO( &exc_set );
   
   /* What descriptors do we want to select? */
   for ( d = descs; d; d = d->next )
     {
	if ( d->fd < 1 )
	  continue;
	
	if ( d->callback_in )
	  FD_SET( d->fd, &in_set );
	if ( d->callback_out )
	  FD_SET( d->fd, &out_set );
	if ( d->callback_exc )
	  FD_SET( d->fd, &exc_set );
	
	if ( maxdesc < d->fd )
	  maxdesc = d->fd;
     }
   
   /* Don't block, return immediately. */
   pulsetime.tv_sec = 0;
   pulsetime.tv_usec = 0;
   
   if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &pulsetime ) == SOCKET_ERROR )
     {
	debugf( "check_descriptors: select error" );
     }
   
   /* Go through the descriptor list. */
   for ( d = descs; d; d = d_next )
     {
	d_next = d->next;
	
	/* Do we have a valid descriptor? */
	if ( d->fd < 1 )
	  continue;
	
	desc_name = d->name;
	
	current_descriptor = d;
	
	desc_section = "Reading from...";
	if ( d->callback_in && FD_ISSET( d->fd, &in_set ) )
	  (*d->callback_in)( d );
	
	if ( !current_descriptor )
	  continue;
	
	desc_section = "Writing to...";
	if ( d->callback_out && FD_ISSET( d->fd, &out_set ) )
	  (*d->callback_out)( d );
	
	if ( !current_descriptor )
	  continue;
	
	desc_section = "Exception handler at...";
	if ( d->callback_exc && FD_ISSET( d->fd, &exc_set ) )
	  (*d->callback_exc)( d );
     }
   desc_name = NULL;
}



void UpdateTimer( )
{
   void check_timers( );
   
   check_timers( );
   
   if ( timers )
     SetTimer( hwndMain, 0, 200, NULL );
}


void SendBuffer( int ole )
{
   void clientff( char *string, ... );
   void send_to_server( char *string );
   const char sb_atcp[] = { IAC, SB, ATCP, 0 };
   const char se[] = { IAC, SE, 0 };
   char buf[16384];
   char buf2[16384+16];
   char *pbuf, *p, *b;
   int bytes;
   
   if ( !hwndEEdit )
     return;
   /* comment out this, work from here */
   if ( !server )
     {
        MessageBox( hwndEMain, "Not connected!", "Send Buffer", 0 );
        return;
     }
   
   if ( ole && !a_on && !atcpoff )
     {
        MessageBox( hwndEMain, "Not properly authenticated with ATCP. Reconnect.", "Set OLE", 0 );
        return;
     }
   
   bytes = SendMessage( hwndEEdit, WM_GETTEXT, (WPARAM) 16384, (LPARAM) buf );
   
   /* Convert all "\r\n"'s to "\n". */
   pbuf = malloc( bytes );
   p = pbuf;
   b = buf;
   while( *b )
     {
	if ( *b == '\r' )
	  {
	     b++;
	     continue;
	  }
	
	*(p++) = *(b++);
     }
   /* And don't let it end without a new line. */
   if ( bytes > 0 && *(b-1) != '\n' )
     *(p++) = '\n';
   *p = 0;
   
   if ( client )
     {
        if ( !ole )
	  clientff( C_B "Sending %d bytes...\n" C_0, bytes );
        else
	  clientff( C_B "Sent %d bytes. Check *echo, to verify.\n" C_0, bytes );
     }
   
   if ( ole )
     {
        sprintf( buf2, "%solesetbuf\n%s%s", sb_atcp, pbuf, se );
        send_to_server( buf2 );
     }
   else
     {
        send_to_server( pbuf );
     }
   
   free( pbuf );
}


void SaveBuffer( int b )
{
   MENUITEMINFO miiMenu;
   UINT menu_handle[] = { ID_EDM_LOADB1, ID_EDM_LOADB2, ID_EDM_LOADB3 };
   char buf[16384];
   int bytes;
   
   if ( editor_buffer[b] )
     free( editor_buffer[b] );
   
   bytes = SendMessage( hwndEEdit, WM_GETTEXT, (WPARAM) 16384, (LPARAM) buf );
   
   editor_buffer[b] = malloc( bytes + 1 );
   
   strcpy( editor_buffer[b], buf );
   
   miiMenu.cbSize = sizeof(MENUITEMINFO);
   miiMenu.fMask = MIIM_STATE;
   miiMenu.fState = MFS_ENABLED;
   SetMenuItemInfo( GetMenu( hwndEMain ), menu_handle[b], FALSE, &miiMenu);
}



void LoadBuffer( int b )
{
   SendMessage( hwndEEdit, WM_SETTEXT, 0, (LPARAM) editor_buffer[b] );
}


#ifndef AW_BLEND
# define AW_BLEND 0x00080000
#endif
#ifndef AW_HIDE
# define AW_HIDE 0x00010000
#endif

void BlendWindow( HWND hwndMain, DWORD dwTime, DWORD dwFlags )
{
   HINSTANCE dll;
   BOOL (*func_AnimateWindow)( HWND hwnd, DWORD dwTime, DWORD dwFlags );
   
   dll = LoadLibrary( "user32.dll" );
   
   if ( dll <= (HINSTANCE) HINSTANCE_ERROR )
     {
	return;
     }
   
   func_AnimateWindow = (void *) GetProcAddress( dll, "AnimateWindow" );
   
   if ( !func_AnimateWindow )
     {
	FreeLibrary( dll );
	return;
     }
   
   (func_AnimateWindow)( hwndMain, dwTime, dwFlags );
   FreeLibrary( dll );
}


LRESULT CALLBACK MainWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
   switch( msg )
     {
      case WM_CLOSE:
	if ( server )
	  {
	     if ( MessageBox( hwndMain, "Midnight is still connected to a server. Are you sure you want to quit?", "Confirm Close", MB_OKCANCEL ) != IDOK )
	       break;
	  }
	else if ( client )
	  {
	     if ( MessageBox( hwndMain, "Closing Midnight will disconnect the current Client. Are you sure you want to quit?", "Confirm Close", MB_OKCANCEL ) != IDOK )
	       break;
	  }
	
	BlendWindow( hwndMain, 400, AW_HIDE | AW_BLEND );
	DestroyWindow( hwndMain );
	
	break;
	
      case WM_DESTROY:
	PostQuitMessage( 0 );
	break;
	
	//		case WM_PAINT:
	//			UpdateWindow( hwndTab );
	//			UpdateWindow( hwndEdit );
	//			UpdateWindow( hwndMods );
	//			break;
	
      case WM_SIZE:
	  {
	     RECT rc;
	     HDWP hdwp;
	     
	     /* Resize the Tab Control. */
	     GetClientRect( hwndMain, &rc );
	     
	     hdwp = BeginDeferWindowPos( 2 );
	     
	     DeferWindowPos( hdwp, hwndTab, NULL, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOZORDER );
	     
	     /* Resize the Edit window. */
	     TabCtrl_AdjustRect( hwndTab, FALSE, &rc );
	     
	     DeferWindowPos( hdwp, hwndEdit, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0 );
	     DeferWindowPos( hdwp, hwndMods, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0 );
	     
	     EndDeferWindowPos( hdwp );
	  }
	break;
	
      case WM_GETMINMAXINFO:
	((MINMAXINFO*)lParam)->ptMinTrackSize.x = 300;
	((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
	break;
	
      case WM_CTLCOLORSTATIC:
	if ( (HWND) lParam == hwndEdit )
	  {
	     SetBkColor( (HDC) wParam, crEditBg );
	     SetTextColor( (HDC) wParam, crEditFg );
	     return (LRESULT) hbrEditBg;
	  }
	break;
	
      case WM_CREATE:
	break;
	
      case WM_TIMER:
	UpdateTimer( );
	if ( !timers )
	  KillTimer( hwndMain, 0 );
	break;
	
      case WM_NOTIFY:
	switch ( ( ((LPNMHDR) lParam)->code ) )
	  {
	   case TCN_SELCHANGE:
	       {
		  int page;
		  
		  page = TabCtrl_GetCurSel( hwndTab );
		  
		  if ( page == TAB_CONSOLE || page == TAB_MINIMIZE )
		    {
		       if ( page == TAB_MINIMIZE )
			 {
			    ShowWindow( hwndMain, SW_MINIMIZE );
			    ShowWindow( hwndMain, SW_HIDE );
			    
			    Iconize( );
			    TabCtrl_SetCurSel( hwndTab, TAB_CONSOLE );
			 }
		       ShowWindow( hwndEdit, SW_SHOW );
		       ShowWindow( hwndMods, SW_HIDE );
		    }
		  else if ( page == TAB_MODULES )
		    {
		       ShowWindow( hwndEdit, SW_HIDE );
		       ShowWindow( hwndMods, SW_SHOW );
		    }
	       }
	     break;
	  }
	break;
	
      case WM_USER:
	if ( lParam == WM_LBUTTONUP )
	  {
	     NOTIFYICONDATA nid;
	     
	     nid.cbSize = sizeof( NOTIFYICONDATA );
	     nid.hWnd = hwndMain;
	     nid.uID = 0;
	     nid.uFlags = 0;
	     Shell_NotifyIcon( NIM_DELETE, &nid );
	     
	     iconized = 0;
	     
	     ShowWindow( hwndMain, SW_SHOW );
	     ShowWindow( hwndMain, SW_RESTORE );
	     //                  AnimateWindow( hwndMain, 100, AW_CENTER | AW_ACTIVATE );
	  }
	break;
	
      case UDM_WINSOCK_SELECT:
	  {
	     int event  = WSAGETSELECTEVENT( lParam );
//	     int wsaerr = WSAGETSELECTERROR( lParam );
	     
	     switch( event )
	       {
		case FD_READ: break;
		case FD_ACCEPT: break;
		case FD_CLOSE: break;
		default: break;
	       }
	     
	     check_descriptors( );
	     
	     UpdateTimer( );
	  }
	break;
	
      default:
	return DefWindowProc( hwnd, msg, wParam, lParam );
     }
   
   return 0;
}


LRESULT CALLBACK EditorWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
   switch( msg )
     {
      case WM_CLOSE:
	ShowWindow( hwndEMain, SW_HIDE );
	break;
        
      case WM_DESTROY:
	hwndEMain = NULL;
	UnregisterClass( "EditorWindowClass", hInstance );
	break;
        
      case WM_SIZE:
	  {
	     RECT rc;
	     
	     GetClientRect( hwndEMain, &rc );
	     
	     MoveWindow( hwndEEdit, 0, 0, rc.right, rc.bottom, TRUE );
	     
	     UpdateWindow( hwndEMain );
	  }
	break;
	
      case WM_COMMAND:
	switch( LOWORD( wParam ) )
	  {
	   case ID_EDM_CLOSE:
	     ShowWindow( hwndEMain, SW_HIDE );
	     break;
	     
	   case ID_EDM_SENDNORMAL:
	     SendBuffer( 0 );
	     break;
	     
	   case ID_EDM_SENDSETOLE:
	     SendBuffer( 1 );
	     break;
	     
	   case ID_EDM_CLEAR:
	     if ( hwndEEdit )
	       SendMessage( hwndEEdit, WM_SETTEXT, 0, (LPARAM) "" );
	     break;
	     
	   case ID_EDM_SAVEFILE:
	     MessageBox( hwndEMain, "Not implemented yet. Sorry!", "Save to File", 0 );
	     break;
	     
	   case ID_EDM_SAVEB1:
	     SaveBuffer( 0 );
	     break;
	   case ID_EDM_SAVEB2:
	     SaveBuffer( 1 );
	     break;
	   case ID_EDM_SAVEB3:
	     SaveBuffer( 2 );
	     break;
	     
	   case ID_EDM_LOADFILE:
	     MessageBox( hwndEMain, "Not implemented yet. Sorry!", "Load from File", 0 );
	     break;
	     
	   case ID_EDM_LOADB1:
	     LoadBuffer( 0 );
	     break;
	   case ID_EDM_LOADB2:
	     LoadBuffer( 1 );
	     break;
	   case ID_EDM_LOADB3:
	     LoadBuffer( 2 );
	     break;
	     
	   case ID_EDM_LOADCOMPOSER:
	     LoadComposer( );
	     break;
	  }
        
      default:
	return DefWindowProc( hwnd, msg, wParam, lParam );
     }
   
   return 0;
}

/* A poor implementation of gettimeofday, just for local use. */

int gettimeofday( struct timeval *tv, void *tz )
{
   LARGE_INTEGER frequency, counter;
   
   tv->tv_sec = 0;
   tv->tv_usec = 0;
   
   if ( !QueryPerformanceFrequency( &frequency ) )
     return -1;
   
   /* Too big for us? */
   if ( frequency.HighPart )
     return -1;
   
   if ( !frequency.LowPart )
     return -1;
   
   if ( !QueryPerformanceCounter( &counter ) )
     return -1;
   
   tv->tv_sec = counter.LowPart / frequency.LowPart;
   
   tv->tv_usec = ( counter.LowPart % frequency.LowPart ) * 1e6 / frequency.LowPart;
   
   return 0;
}
