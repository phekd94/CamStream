
/*!
	\defgroup Encode Frame encode
	\brief Encode a frame
	\version 1.3
	\note Error description output in 'stderr' stream using 
	'ErrorFunctions' module
	\warning Should be function where memory allocates for result buffers

	A set of functions that allows you to encode a frame.
	@{

	Usage example (no error checks):
	\code
	unsigned char *frame_ptr;
	unsigned int frame_size;

	struct enc_ops enc = enc_ops_ffmpeg;

	enc.CamStream_Encode_Init(bitrate, framerate, width, height, gop, b);
	for (;;) {
		...
		enc.CamStream_Encode_EncFrame(frame_ptr, frame_size, 
						&frame_ptr, &frame_size);
		// IT CORRUPTED frame_ptr and frame_size
		...
	}
	enc.CamStream_Encode_EndEnc(&frame_ptr, &frame_size);
	...
	enc.CamStream_Encode_Delete();
	\endcode
*/

//=============================================================================

#ifndef CAM_STREAM_ENCODE_H
#define CAM_STREAM_ENCODE_H

//=============================================================================

/**
 *	Structure with function pointers to operation for each task
 */
struct enc_ops {
	/*@{*/
	int (*CamStream_Encode_Init)(int, int, int, int, int, int); 
							/**< Pointer to init 
							module function */
	int (*CamStream_Encode_Delete)(void); 		/**< Pointer to delete 
							module function */
	int (*CamStream_Encode_EncFrame)(unsigned char *, unsigned int, 
					unsigned char **, unsigned int *); 
							/**< Pointer to encode 
							frame function */
	int (*CamStream_Encode_EndEnc)(unsigned char **, unsigned int *); 
							/**< Pointer to call 
							last encode frame with 
							flush encoder and add 
							end code to buffer */
	/*@}*/
};

//-----------------------------------------------------------------------------

#ifndef FFMPEG_DISABLE

extern struct enc_ops 	enc_ops_ffmpeg;	/**< Operations to encode using 
					ffmpeg */

#endif // FFMPEG_DISABLE

#ifndef DMAI_DISABLE

extern struct enc_ops 	enc_ops_dmai;	/**< Operations to encode using 
					dmai */

#endif // DMAI_DISABLE

extern struct enc_ops 	enc_ops_stub;	/**< Stub operations to encode */

//=============================================================================

#endif // CAM_STREAM_ENCODE_H

//=============================================================================

/*! @} */

//=============================================================================

