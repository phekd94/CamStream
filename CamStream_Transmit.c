
//=============================================================================

#include "CamStream_TLPI.h"

#include "CamStream_Print.h"
#include "CamStream_Transmit.h"
#include "CamStream_Commands.h"
#include "CamStream_IO.h"

//-----------------------------------------------------------------------------

// file
#include <fcntl.h>		// flags for open
#include <sys/stat.h>		// permission for open

//-----------------------------------------------------------------------------

#ifndef SDL2_DISABLE

// gui
#include <SDL2/SDL.h>

#endif // SDL2_DISABLE

//-----------------------------------------------------------------------------

// inet
#include "CamStream_InetSockets.h"
#include <signal.h> // !!!

//=============================================================================

#define STR_PREF		"[trn]: "

//=============================================================================

// inet
char * 		CamStream_Transmit_inet_Service = "50000";
char * 		CamStream_Transmit_inet_Address = "10.2.0.113";
						// "10.1.0.95"
						// "10.2.0.113"

//-----------------------------------------------------------------------------

// file
char * 		CamStream_Transmit_file_OutName = "output.raw";
						//"output.h264"
						//"output.raw"
						//"output.mjpeg"
						//"output.rgb"

//=============================================================================

// file
static int			fd_file			= -1;

//-----------------------------------------------------------------------------

// inet
// FIXME: another way ?
// static int	fd_inet				= -1;
int		CamStream_Transmit_inet_fd	= -1;

//-----------------------------------------------------------------------------

#ifndef SDL2_DISABLE

// gui
static SDL_Window		*window			= NULL;
static SDL_Surface		*screen			= NULL;
static Boolean			SDL_Init_bool		= FALSE;

#endif // SDL2_DISABLE

//=============================================================================
//=============================================================================

// file

/*!
	\defgroup file_transmit Transmit in file
	 \ingroup Transmit
	\brief Send frame in file
	\version 1.3
	\note Error description output in 'stderr' stream using 
	'ErrorFunctions' module

	A set of functions that allows you to send frame to file.
	@{
*/

//=============================================================================

/*!
	\brief Initialization the transmit to file module
	\param[in] width Image width in pixels (don't use this)
	\param[in] height Image height in pixels (don't use this)
	\return 0 - success, -1 - error
*/
static int 
Init_file(
		int width 	__attribute__((unused)), 
		int height 	__attribute__((unused))
	)
{
	mode_t filePerms;
	int openFlags;

	openFlags = O_CREAT | O_WRONLY | O_TRUNC;
	// TODO: right permision for output file
	filePerms = S_IRUSR | S_IRGRP | S_IROTH | // read for all
			S_IWUSR | S_IWGRP | S_IWOTH;  // write for all

	fd_file = open(CamStream_Transmit_file_OutName, openFlags, filePerms);
	if (fd_file == -1) {
		errMsg(STR_PREF "open %s", CamStream_Transmit_file_OutName);
		return -1;
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "open %s", 
			CamStream_Transmit_file_OutName);
		return 0;
	}
}

//=============================================================================

/*!
	\brief Delete the transmit to file module
	\return 0 - success, -1 - error
*/
static int 
Delete_file(void)
{
	if (fd_file != -1) {
		if (close(fd_file) == -1) {
			fd_file = -1;
			errMsg(STR_PREF "close %s", 
			       CamStream_Transmit_file_OutName);
			return -1;
		} else {
			fd_file = -1;
			CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
				"close %s", CamStream_Transmit_file_OutName);
			return 0;
		}
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"fd_file == -1");
		return 0;
	}
}

//=============================================================================

/*!
	\brief Send frame to file
	\param[in] ptr Array of pointers to colour parts (e.g. Y: index 0; U: 
	index 1; V: index 2)
	\param[in] size Size of send frame
	\param[in] width Image width in pixels (don't use this)
	\param[in] height Image height in pixels (don't use this)
	\param[in] c Flag of add size to out stream (1 - add, 0 - don't add)
	\param[in] col Method of storing colors (don't use this)
	\return 0 - success, -1 - error
*/
static int 
SendFrame_file(
		unsigned char **ptr, 
		unsigned int size, 
		int width 			__attribute__((unused)), 
		int height 			__attribute__((unused)), 
		int c, 
		enum TransmColorPacked col 	__attribute__((unused))
	)
{
	// TODO: use 'col' for write all color
	return WriteToFd(fd_file, ptr[0], size, c, TRANSMIT_TYPE_NONE);
}

//=============================================================================

/*! @} */

//=============================================================================
//=============================================================================

#ifndef SDL2_DISABLE

// gui (SDL2)

/*!
	\defgroup gui_transmit Transmit in gui
	 \ingroup Transmit
	\brief Send frame in gui using SDL2
	\version 1.3
	\note Error description output in 'stderr' stream using 
	'ErrorFunctions' module

	A set of functions that allows you to send frame to gui.
	@{
*/

//=============================================================================

/*!
	\brief Initialization the transmit to gui module
	\param[in] width Image width in pixels
	\param[in] height Image height in pixels
	\return 0 - success, -1 - error
*/
static int 
Init_gui(int width, int height)
{
	// Initialize SDL2
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Init_bool = FALSE;
		CamStream_Print(TRUE, STR_PREF "SDL_Init: %s", SDL_GetError());
		return -1;
	}
	SDL_Init_bool = TRUE;
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "SDL_Init");

	// Create an application window
	window = SDL_CreateWindow(
		"Video from camera",		// window title
		SDL_WINDOWPOS_UNDEFINED,	// initial x position
		SDL_WINDOWPOS_UNDEFINED,	// initial y position
		width,				// width, in pixels
		height,				// height, in pixels
		SDL_WINDOW_OPENGL		// flags
	);

	// Check that the window was successfully created
	if (window == NULL) {
		CamStream_Print(TRUE, STR_PREF "SDL_CreateWindow: %s", 
			SDL_GetError());
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "SDL_CreateWindow");

	// but instead of creating a renderer, 
	// we can draw directly to the screen
	// !!! DO NOT FREE IN APP !!!
	screen = SDL_GetWindowSurface(window);

	if (screen == NULL) {
		CamStream_Print(TRUE, STR_PREF "SDL_GetWindowSurface: %s", 
			SDL_GetError());
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
		"SDL_GetWindowSurface");

	return 0;
}

//=============================================================================

/*!
	\brief Delete the transmit to gui module
	\return 0 - success, -1 - error
*/
static int 
Delete_gui(void)
{
	// Close and destroy the window
	if (window != NULL) {
		SDL_DestroyWindow(window);
		window = NULL;
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
				"SDL_DestroyWindow");
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"window == NULL");
	}

	// Clean up
	if (SDL_Init_bool == TRUE) {
		SDL_Quit();
		SDL_Init_bool = FALSE;
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "SDL_Quit");
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
				"SDL_Init_bool == FALSE");
	}

	return 0;
}

//=============================================================================

/*!
	\brief YUV to RRB
	\param[in] y Y component
	\param[in] u U component
	\param[in] v V component
	\return 4 bytes; 1st byte: blue; 2nd byte: green; 3th byte: red; 
	4th byte: 0  
*/
static unsigned int 
yuv2rgb(double y, double u, double v)
{
	static int 		rs;
	static int 		gs;
	static int 		bs;
	static unsigned int	r;
	static unsigned int	g;
	static unsigned int	b;
	static unsigned int 	ret;

	rs = (int)(y + 1.4075 * (v - 128.0));
	gs = (int)(y - 0.3455 * (u - 128.0) - (0.7169 * (v - 128.0)));
	bs = (int)(y + 1.7790 * (u - 128.0));
	
	if (rs > 255)
		r = 255;
	else if (rs < 0)
		r = 0;
	else
		r = (unsigned int)rs;
	
	if (gs > 255)
		g = 255;
	else if (gs < 0)
		g = 0;
	else
		g = (unsigned int)gs;

	if (bs > 255)
		b = 255;
	else if (bs < 0)
		b = 0;
	else
		b = (unsigned int)bs;
	
	ret 	= 0u;
	ret 	=	(b 		& 0x000000FF) | 
			((g << 8) 	& 0x0000FF00) | 
			((r << 16) 	& 0x00FF0000);

	return ret;
}

//-----------------------------------------------------------------------------

/*!
	\brief Send frame to gui
	\param[in] ptr Array of pointers to colour parts (e.g. Y: index 0; U: 
	index 1; V: index 2)
	\param[in] size Size of send frame
	\param[in] width Image width in pixels
	\param[in] height Image height in pixels
	\param[in] c Flag of add size to out stream (don't use this)
	\param[in] col Method of storing colors
	\return 0 - success, -1 - error
*/
static int 
SendFrame_gui(
		unsigned char **ptr, 
		unsigned int size 	__attribute__((unused)), 
		int width, 
		int height, 
		int c 			__attribute__((unused)),
		enum TransmColorPacked col
	)
{
	static int 			i, j;
	static unsigned int 		pix_fr;
	static double 			u, v, y_1, y_2;

	unsigned int *pixs = (unsigned int*)screen->pixels;

	switch (col)
	{
		case TRANSMIT_COL_GRAY_YUV_PACKED:
			for (i = 0, j = 0; i < width * height; ++i, j += 2) {
				pix_fr = (unsigned int)(ptr[0][j]);
				if (pix_fr > 255u)
					pix_fr = 255u;
				pixs[i] = 0;
				pixs[i] = (pix_fr & 0x000000FF) | 
					((pix_fr << 8) & 0x0000FF00) | 
					((pix_fr << 16) & 0x00FF0000);
			}
			break;
		case TRANSMIT_COL_COLOR_YUV_PACKED:
			for (i = 0, j = 0; i < width * height; 
							i += 2, j += 4) {
				y_1 	= (double)ptr[0][j];
				u 	= (double)ptr[0][j + 1];
				y_2 	= (double)ptr[0][j + 2];
				v 	= (double)ptr[0][j + 3];
				pixs[i] 	= yuv2rgb(y_1, u, v);
				pixs[i + 1] 	= yuv2rgb(y_2, u, v);
			}
			break;
		case TRANSMIT_COL_GRAY:
			for (i = 0; i < width * height; ++i) {
				pix_fr = (unsigned int)(ptr[0][i]);
				if (pix_fr > 255u)
					pix_fr = 255u;
				pixs[i] = 0;
				pixs[i] = (pix_fr & 0x000000FF) | 
					((pix_fr << 8) & 0x0000FF00) | 
					((pix_fr << 16) & 0x00FF0000);
			}
			break;
		case TRANSMIT_COL_COLOR_YUV_FFMPEG:
			for (i = 0, j = 0; i < width * height; i += 2, ++j) {
				y_1 	= (double)ptr[0][i];
				u 	= (double)ptr[1][j];
				y_2 	= (double)ptr[0][i + 2];
				v 	= (double)ptr[2][j];
				pixs[i] 	= yuv2rgb(y_1, u, v);
				pixs[i + 1] 	= yuv2rgb(y_2, u, v);
			}
			break;
		default:
			CamStream_Print(TRUE, STR_PREF 
		"'col' parameter should be from 'enum TransmColorPacked'");
			return -1;
			break;
	}
	//---------------------------------------------------------------------
	if (SDL_UpdateWindowSurface(window) < 0) {
		CamStream_Print(TRUE, STR_PREF 
				"SDL_UpdateWindowSurface: %s", SDL_GetError());
		return -1;
	}
	// CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
	//		"SDL_UpdateWindowSurface");
	//---------------------------------------------------------------------
	return 0;
}

//=============================================================================

/*! @} */

#endif // DL2_DISABLE

//=============================================================================
//=============================================================================

// inet

/*!
	\defgroup inet_transmit Transmit in inet
	 \ingroup Transmit
	\brief Send frame in inet
	\version 1.3
	\note Error description output in 'stderr' stream using 
	'ErrorFunctions' module

	A set of functions that allows you to send frame to inet.
	@{
*/

//=============================================================================

/*!
	\brief Initialization the transmit to inet module
	\param[in] width Image width in pixels (don't use this)
	\param[in] height Image height in pixels (don't use this)
	\return 0 - success, -1 - error
*/
static int 
Init_inet(
		int width 	__attribute__((unused)), 
		int height 	__attribute__((unused))
	)
{
	// TODO: signal SIGPIPE ignore (if write to socket which close in 
	// oher end)
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		errMsg(STR_PREF "signal SIGPIPE ignore");
		return -1;
	}

	CamStream_Transmit_inet_fd = inetConnect(
				CamStream_Transmit_inet_Address, 
				CamStream_Transmit_inet_Service, 
				SOCK_STREAM);
	if (CamStream_Transmit_inet_fd == -1) {
		errno = ENOSYS;
		errMsg(STR_PREF "inetConnect for address %s, port %s", 
				CamStream_Transmit_inet_Address, 
				CamStream_Transmit_inet_Service);
		return -1;
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
				"connect to server (%s, %s)",
				CamStream_Transmit_inet_Address, 
				CamStream_Transmit_inet_Service);
	}
	return 0;
}

//=============================================================================

/*!
	\brief Delete the transmit to inet module
	\return 0 - success, -1 - error
*/
static int 
Delete_inet(void)
{
	if (CamStream_Transmit_inet_fd != -1) {
		if (close(CamStream_Transmit_inet_fd) != 0) {
			CamStream_Transmit_inet_fd = -1;
			errMsg(STR_PREF "close");
			return -1;
		}
		CamStream_Transmit_inet_fd = -1;
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "close");
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"CamStream_Transmit_inet_fd == -1");
	}
	return 0;
}

//=============================================================================

/*!
	\brief Send frame to inet
	\param[in] ptr Array of pointers to colour parts (e.g. Y: index 0; U: 
	index 1; V: index 2)
	\param[in] size Size of send frame
	\param[in] width Image width in pixels (don't use this)
	\param[in] height Image height in pixels (don't use this)
	\param[in] c Flag of add size to out stream (1 - add, 0 - don't add)
	\param[in] col Method of storing colors (don't use this)
	\return 0 - success, -1 - error
*/
static int 
SendFrame_inet(
		unsigned char **ptr, 
		unsigned int size, 
		int width 			__attribute__((unused)), 
		int height 			__attribute__((unused)), 
		int c, 
		enum TransmColorPacked col 	__attribute__((unused))
	)
{
	return WriteToFd(CamStream_Transmit_inet_fd, ptr[0], size, 
			 c, TRANSMIT_TYPE_FRAME);
}

//=============================================================================

/*! @} */

//=============================================================================
//=============================================================================

// stubs

static int 
Init_stub(
		int width 	__attribute__((unused)), 
		int height 	__attribute__((unused))
	)
{
	return 0;
}

static int 
Delete_stub(void)
{
	return 0;
}

static int 
SendFrame_stub(
		unsigned char **ptr 		__attribute__((unused)), 
		unsigned int size 		__attribute__((unused)),
		int width 			__attribute__((unused)), 
		int height 			__attribute__((unused)), 
		int c 				__attribute__((unused)), 
		enum TransmColorPacked col 	__attribute__((unused))
	)
{
	return 0;
}

//=============================================================================
//=============================================================================
//=============================================================================

struct trn_ops trn_ops_file = {
	.CamStream_Transmit_Init 	= Init_file,
	.CamStream_Transmit_Delete 	= Delete_file,
	.CamStream_Transmit_SendFrame 	= SendFrame_file,
};

//-----------------------------------------------------------------------------

#ifndef SDL2_DISABLE

struct trn_ops trn_ops_gui = {
	.CamStream_Transmit_Init 	= Init_gui,
	.CamStream_Transmit_Delete 	= Delete_gui,
	.CamStream_Transmit_SendFrame 	= SendFrame_gui,
};

#endif // SDL2_DISABLE

//-----------------------------------------------------------------------------

struct trn_ops trn_ops_inet = {
	.CamStream_Transmit_Init 	= Init_inet,
	.CamStream_Transmit_Delete 	= Delete_inet,
	.CamStream_Transmit_SendFrame 	= SendFrame_inet,
};

//-----------------------------------------------------------------------------

struct trn_ops trn_ops_stub = {
	.CamStream_Transmit_Init 	= Init_stub,
	.CamStream_Transmit_Delete 	= Delete_stub,
	.CamStream_Transmit_SendFrame 	= SendFrame_stub,
};

//=============================================================================

