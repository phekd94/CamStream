
/*!
	\defgroup Transmit Frame transmit
	\brief Send frame and send/get command
	\version 1.3
	\note Error description output in 'stderr' stream using 
	'ErrorFunctions' module

	A set of functions that allows you to send frame and send/get command.
	@{
	Usage example (no error checks):
	\code
	unsigned char *frame_ptr[3];
	unsigned int frame_size;
	int width = 1920;
	int height = 1080;

	struct trn_ops tr = trn_ops_gui;

	trn.CamStream_Transmit_Init();
	for(;;) {
		...
		trn.CamStream_Transmit_SendFrame(frame_ptr, frame_size, 
						width, height, 1, 
						TRANSMIT_COL_GRAY_YUV_PACKED);
		...
	}
	trn.CamStream_Transmit_Delete();
	\endcode
*/

//=============================================================================

#ifndef CAM_STREAM_TRANSMIT_H
#define CAM_STREAM_TRANSMIT_H

//=============================================================================

/**
 *	Enumeration with color layout
 */
enum TransmColorPacked {
	TRANSMIT_COL_GRAY_YUV_PACKED = 0,	/**< YUV packed; 
						selecting only Y (gray) */
	TRANSMIT_COL_COLOR_YUV_PACKED,		/**< YUV packed; 
						selecting all components */
	TRANSMIT_COL_GRAY,			/**< Only one component - 
						gray */
	TRANSMIT_COL_COLOR_YUV_FFMPEG		/**< All components (YUV) 
						from ffmpeg (different array
						index) */
};

//=============================================================================

/**
 *	Structure with function pointers to operation for each task
 */
struct trn_ops {
	/*@{*/
	int (*CamStream_Transmit_Init)(int, int);	/**< Pointer to init 
							module function */
	int (*CamStream_Transmit_Delete)(void); 	/**< Pointer to delete 
							module function */
	int (*CamStream_Transmit_SendFrame)(unsigned char **, unsigned int, 
					int, int, int, enum TransmColorPacked); 
							/**< Pointer to send 
							frame function */
	/*@}*/
};

//-----------------------------------------------------------------------------

extern struct trn_ops 	trn_ops_file;	/**< Operations for transmit to 
					file */


#ifndef SDL2_DISABLE

extern struct trn_ops 	trn_ops_gui;	/**< Operations for transmit to 
					gui */

#endif // SDL2_DISABLE

extern struct trn_ops 	trn_ops_inet;	/**< Operations for transmit to 
					inet */
extern struct trn_ops 	trn_ops_stub;	/**< Stub operations to transmit */

//=============================================================================

extern char * 	CamStream_Transmit_inet_Service;
extern char * 	CamStream_Transmit_inet_Address;
extern char * 	CamStream_Transmit_file_OutName;

// FIXME: another way ?
extern int	CamStream_Transmit_inet_fd;

#endif // CAM_STREAM_TRANSMIT_H

//=============================================================================

/*! @} */
