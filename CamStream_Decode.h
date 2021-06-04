
/*!
	\defgroup Decode Frame decode
	\brief Decode a frame
	\version 1.3
	\note Error description output in 'stderr' stream using 
	'ErrorFunctions' module
	\warning Should be function where memory allocates for result buffers
	\warning Simple and deprecated operations structs using commom Init 
	and Delete functions therefore don't use their together

	A set of functions that allows you to decode a frame.
	@{

	Usage example (no error checks):
	\code
	unsigned char *frame_ptr;
	unsigned char *frame_colors_ptrs[3];
	int frame_size;
	int ret;

	struct dec_ops dec = dec_ops_ffmpeg;

	dec.CamStream_Decode_Init();
	for(;;) {
		...
		ret = dec.CamStream_Decode_DecFrame(frame_ptr, frame_size, 
					frame_colors_ptrs, &frame_size);
		// IT CORRUPTED frame_size
		if (ret == -2)
			continue;
		...
	}
	dec.CamStream_Decode_DecFrame(&frame_ptr, &frame_size);
	...
	dec.CamStream_Decode_Delete();
	\endcode
*/

//=============================================================================

#ifndef CAM_STREAM_DECODE_H
#define CAM_STREAM_DECODE_H

//=============================================================================

/**
 *	Structure with function pointers to operation for each task
 */
struct dec_ops {
	/*@{*/
	int (*CamStream_Decode_Init)(void);		/**< Pointer to init 
							module function */
	int (*CamStream_Decode_Delete)(void); 		/**< Pointer to delete 
							module function */
	int (*CamStream_Decode_DecFrame)(unsigned char *, unsigned int, 
					unsigned char **, unsigned int *); 	
							/**< Pointer to decode 
							frame function */
	int (*CamStream_Decode_EndDec)(unsigned char **, unsigned int *); 
							/**< Pointer to call 
							last decode frame with 
							flush decoder */
	/*@}*/
};

//-----------------------------------------------------------------------------

#ifndef FFMPEG_DISABLE

extern struct dec_ops 	dec_ops_ffmpeg;	/**< Operations to encode using 
					ffmpeg */
extern struct dec_ops 	dec_ops_ffmpeg_deprecated;	/**< Operations to 
					encode using ffmpeg 
					deprecated functions */

#endif // FFMPEG_DISABLE

extern struct dec_ops 	dec_ops_stub;	/**< Stub operations to decode */

//=============================================================================

#endif // CAM_STREAM_DECODE_H

//=============================================================================

/*! @} */
