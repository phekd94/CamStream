
//=============================================================================

/*!
	\defgroup Capture Video capture
	\brief Video capture
	\version 1.3
	\note Error description output in 'stderr' stream using 
	'ErrorFunctions' module
	\warning Should be function where memory allocates for result buffers

	A set of functions that allows you to capture a video.
	@{

	Usage example (no error checks):
	\code
	unsigned char *frame_ptr;
	unsigned int bytesused, msg_type;
	
	struct cap_ops cap = cap_ops_v4l2;
	
	cap.CamStream_Capture_Init();
	cap.CamStream_Capture_Start();
	for (;;) {
		frame_ptr = cap.CamStream_Capture_Get(&bytesused, &msg_type);
		if (frame_ptr != NULL && msg_type == TRANSMIT_TYPE_FRAME)
			work_with_frame();
		cap.CamStream_Capture_ThrowFrame();
	}
	cap.CamStream_Capture_Stop();
	...
	cap.CamStream_Capture_Delete();
	\endcode
*/

//=============================================================================

#ifndef CAM_STREAM_CAPTURE_H
#define CAM_STREAM_CAPTURE_H

//=============================================================================

// FIXME: add doxygen description
enum CamType {
	CAM_TYPE_NONE,
	CAM_TYPE_LOGITECH_USB_YUV,
	CAM_TYPE_LOGITECH_USB_MJPEG,
	CAM_TYPE_CAM_V,
};

//=============================================================================

/**
 *	Structure with function pointers to operation for each task
 */
struct cap_ops {
	/*@{*/
	int (*CamStream_Capture_Init)(void);		/**< Pointer to init 
							module function */
	int (*CamStream_Capture_Delete)(void);		/**< Pointer to delete 
							module function */
	int (*CamStream_Capture_Start)(void);		/**< Pointer to start 
							capture function */
	int (*CamStream_Capture_Stop)(void);		/**< Pointer to stop 
							capture function */
	unsigned char* (*CamStream_Capture_Get)(unsigned int*, unsigned int*); 
							/**< Pointer to get 
							frame function */
	int (*CamStream_Capture_ThrowFrame)(void);	/**< Pointer to throw 
							frame function 
							(pointer got from get 
							frame func will be not 
							valid) */
	/*@}*/
};

//-----------------------------------------------------------------------------

extern struct cap_ops 	cap_ops_v4l2;	/**< Operations to capture using 
					v4l2 */
extern struct cap_ops 	cap_ops_file;	/**< Operations to capture from 
					file */
extern struct cap_ops 	cap_ops_inet;	/**< Operations to capture from 
					inet */
extern struct cap_ops 	cap_ops_stub;	/**< Stub operations to capture */

//=============================================================================

extern char* 		CamStream_Capture_v4l2_DevName;
extern int 		CamStream_Capture_v4l2_BufCount;
extern enum CamType	CamStream_Capture_v4l2_CamType;
extern char* 		CamStream_Capture_file_FileName;
extern char* 		CamStream_Capture_inet_Service;
extern int 		CamStream_Capture_inet_Backlog;

// FIXME: another way
extern int 	CamStream_Capture_inet_fd_client;

//=============================================================================

#endif // CAM_STREAM_CAPTURE_H

//=============================================================================

/*! @} */

//=============================================================================

