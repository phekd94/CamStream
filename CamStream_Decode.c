
//=============================================================================

#include "CamStream_TLPI.h"

#include "CamStream_Print.h"
#include "CamStream_Decode.h"

//-----------------------------------------------------------------------------

#ifndef FFMPEG_DISABLE

// ffmpeg and ffmpeg deprecated
#include <libavcodec/avcodec.h>

#endif // FFMPEG_DISABLE

//=============================================================================

#define STR_PREF		"[dec]: "

//=============================================================================

#ifndef FFMPEG_DISABLE

// ffmpeg and ffmpeg deprecated
char * 		CamStream_Decode_ffmpeg_CodecName = "h264";
int 		CamStream_Decode_ffmpeg_CodecID = AV_CODEC_ID_H264;

#endif // FFMPEG_DISABLE

//=============================================================================

#ifndef FFMPEG_DISABLE

// ffmpeg
static const AVCodec 		*codec			= NULL;
static AVCodecContext 		*c			= NULL;
static AVPacket			*pkt			= NULL;
static AVFrame			*frame			= NULL;
static AVCodecParserContext	*parser			= NULL;

#endif // FFMPEG_DISABLE

//=============================================================================
//=============================================================================

#ifndef FFMPEG_DISABLE

// ffmpeg

/*!
	\defgroup ffmpeg_decode Decode using ffmpeg
	 \ingroup Decode
	\brief Decode frame using ffmpeg
	\version 1.3
	\note Error description output in 'stderr' stream using 
	'ErrorFunctions' module

	A set of functions that allows you to decode frame using ffmpeg.
	@{
*/

//=============================================================================

/*!
	\brief Initialization the decode module using ffmpeg
	\return 0 - success, -1 - error
*/
static int 
Init_ffmpeg(void)
{
	int ret;
	//---------------------------------------------------------------------
	pkt = av_packet_alloc();
	if (pkt == NULL) {
		errno = ENOSYS;
		errMsg(STR_PREF "av_packet_alloc");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "av_packet_alloc");
	//---------------------------------------------------------------------
	codec = avcodec_find_decoder(CamStream_Decode_ffmpeg_CodecID);
	if (codec == NULL) {
		errno = ENOSYS;
		errMsg(STR_PREF "avcodec_find_decoder for '%s'(%d)", 
			CamStream_Decode_ffmpeg_CodecName, 
			CamStream_Decode_ffmpeg_CodecID);
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
		"avcodec_find_decoder");
	//---------------------------------------------------------------------
	parser = av_parser_init(codec->id);
	if (parser == NULL) {
		errno = ENOSYS;
		errMsg(STR_PREF "av_parser_init for codec->id = %d", 
			codec->id);
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "av_parser_init");
	//---------------------------------------------------------------------
	c = avcodec_alloc_context3(codec);
	if (c == NULL) {
		errno = ENOSYS;
		errMsg(STR_PREF "avcodec_alloc_context3");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
		"avcodec_alloc_context3");
	//---------------------------------------------------------------------

	// TODO: flags
	if(codec->capabilities & AV_CODEC_FLAG_TRUNCATED) {
		c->flags |= AV_CODEC_FLAG_TRUNCATED;
	}

	//---------------------------------------------------------------------
	ret = avcodec_open2(c, codec, NULL);
	if (ret < 0) {
		errno = ENOSYS;
		errMsg(STR_PREF "avcodec_open2");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "avcodec_open2");
	//---------------------------------------------------------------------
	frame = av_frame_alloc();
	if (frame == NULL) {
		errno = ENOSYS;
		errMsg(STR_PREF "av_frame_alloc");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "av_frame_alloc");
	//---------------------------------------------------------------------
	return 0;
}

//=============================================================================

/*!
	\brief Delete the decode module using ffmpeg
	\return 0 - success, -1 - error
*/
static int 
Delete_ffmpeg(void)
{
	if (frame != NULL) {
		av_frame_free(&frame);
		frame = NULL;
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"av_frame_free");
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"frame == NULL");
	}

	if (parser != NULL) {
		av_parser_close(parser);
		parser = NULL;
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"av_parser_close");
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"parser == NULL");
	}

	if (pkt != NULL) {
		av_packet_free(&pkt);
		pkt = NULL;
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"av_packet_free");
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"pkt == NULL");
	}
	
	if (c != NULL) {
		avcodec_free_context(&c);
		c = NULL;
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"avcodec_free_context");
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "c == NULL");
	}
	
	return 0;
}

//=============================================================================

/*!
	\brief Decode frame using ffmpeg
	\param[in] ptr Pointer to frame for decode
	\param[in] size Size of frame for decode
	\param[out] ptr_res Array of pointers to colour parts 
	(e.g. Y: index 0; U: index 1; V: index 2)
	\param[out] size_res Size of decode frame
	\return 0 - success, -1 - error, -2 - EAGAIN
	\note Some codec use several frame therefore can return EAGAIN
	\warning Memory for result buffers allocates here
*/
static int 
DecFrame_ffmpeg(unsigned char *ptr, unsigned int size, 
			unsigned char **ptr_res, unsigned int *size_res)
{
	
	// NOTE: DECODE using 'av_parser_parse2'; 'avcodec_send_packet';
	// 'avcodec_receive_frame'
	
	int ret;
	//static int ii = 0;
	//---------------------------------------------------------------------
	// NOTE: see doc (AV_INPUT_BUFFER_PADDING_SIZE)
	ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size, ptr, size,  
				AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
	if (ret < 0) {
		errno = ENOSYS;
		errMsg(STR_PREF "av_parser_parse2");
		return -1;
	}
	//---------------------------------------------------------------------
	//CamStream_Print(FALSE, STR_PREF ANSI_COLOR_CYAN "%d", ii++);
	//CamStream_Print(FALSE, STR_PREF ANSI_COLOR_CYAN "input  : %d", size);
	//CamStream_Print(FALSE, STR_PREF ANSI_COLOR_CYAN "return : %d", ret);
	//CamStream_Print(FALSE, STR_PREF ANSI_COLOR_CYAN 
	//	"pkt    : %d", pkt->size);
	//=====================================================================
	// decode
	if (pkt->size > 0) {
	//---------------------------------------------------------------------
		// NOTE: see doc (AV_INPUT_BUFFER_PADDING_SIZE); depend of 
		//	av_parser_parse2 buffer in ptr
		ret = avcodec_send_packet(c, pkt);
		if (ret < 0) {
			if (ret == AVERROR(EAGAIN)) {
				// NOTE: this means should be call 
				// avcodec_receive_frame,
				// but don't do it here
				errno = ENOSYS;
				errMsg(STR_PREF "avcodec_send_packet and "
					"EAGAIN: user must read output frame");
				return -1;
			}
			if (ret == AVERROR(ENOMEM)) {
				errno = ENOSYS;
				errMsg(STR_PREF "avcodec_send_packet: ENOMEM");
				return -1;
			}
			errno = ENOSYS;
			errMsg(STR_PREF "avcodec_send_packet");
			return -1;
		}
		//-------------------------------------------------------------
		// FIXME: for me 'while' is not correct
		//while (ret >= 0) {
			// NOTE: frame allocates by encoder and call 
			// av_frame_unref(frame)
			ret = avcodec_receive_frame(c, frame);
			if (ret < 0) {
				if (ret == AVERROR(EAGAIN)) {
					// NOTE: in this state require new 
					// input; don't do it here, but 
					// return -2 for get new input
					CamStream_Print(FALSE, STR_PREF 
							ANSI_COLOR_YELLOW 
					"avcodec_receive_frame: EAGAIN");
					ptr_res[0] = NULL;
					*size_res = 0;
					return -2;
				}
				errno = ENOSYS;
				errMsg(STR_PREF "avcodec_receive_frame");
				return -1;
			}
			ptr_res[0] = frame->data[0];
			ptr_res[1] = frame->data[1];
			ptr_res[2] = frame->data[2];
			*size_res = (frame->width) * (frame->height);

			//CamStream_Print(FALSE, STR_PREF ANSI_COLOR_CYAN 
			//		"pix_fmt: %d", frame->format);
			//CamStream_Print(FALSE, STR_PREF ANSI_COLOR_CYAN 
			//		"res w  : %d", frame->width);
			//CamStream_Print(FALSE, STR_PREF ANSI_COLOR_CYAN 
			//		"res h  : %d", frame->height);
		//}
		//-------------------------------------------------------------
	//---------------------------------------------------------------------
	}
	else {
		ptr_res[0] = NULL;
		*size_res = 0;
	}
	//=====================================================================
	return 0;
}

//=============================================================================

/*!
	\brief This is last call after end of decode stream (flush encode)
	\param[out] ptr_res Array of pointers to colour parts 
	(e.g. Y: index 0; U: index 1; V: index 2)
	\param[out] size_res Size of decode frame (0 if empty data)
	\return 0 - success, -1 - error
*/
static int 
EndDec_ffmpeg(unsigned char **ptr_res, unsigned int *size_res)
{
	int ret;
	ret = avcodec_send_packet(c, NULL);
	if (ret < 0) {
		errno = ENOSYS;
		errMsg(STR_PREF "flush: avcodec_send_packet");
		return -1;
	}

	// NOTE: must be avcodec_receive_frame after flush (read a last data)
	// but don't do it here

	ptr_res[0] = NULL;
	*size_res = 0u;

	return 0;
}

//=============================================================================

/*! @} */

#endif // FFMPEG_DISABLE

//=============================================================================
//=============================================================================

#ifndef FFMPEG_DISABLE

// ffmpeg deprecated

/*!
	\defgroup ffmpeg_decode_deprecated Decode using ffmpeg deprecated 
	functions
	 \ingroup Decode
	\brief Decode frame using ffmpeg
	\version 1.3
	\note Error description output in 'stderr' stream using 
	'ErrorFunctions' module

	A set of functions that allows you to decode frame using ffmpeg 
	deprecated functions.
	@{
*/

//=============================================================================

/*!
	\brief Decode frame using ffmpeg deprecated functions
	\param[in] ptr Pointer to frame for decode
	\param[in] size Size of frame for decode
	\param[out] ptr_res Array of pointers to colour parts 
	(e.g. Y: index 0; U: index 1; V: index 2)
	\param[out] size_res Size of decode frame
	\return 0 - success, -1 - error, -2 - EAGAIN
	\note Some codec use several frame therefore can return EAGAIN
	\warning Memory for result buffers allocates here
*/
static int 
DecFrame_ffmpeg_deprecated(unsigned char *ptr, unsigned int size, 
			unsigned char **ptr_res, unsigned int *size_res)
{
	// NOTE: DECODE using 'avcodec_decode_video2'
	// https://ffmpeg.org/doxygen/trunk/decoding__encoding_8c-source.html
	// NOTE: only when 'size' will be equal 'len' (return from 
	// 'avcodec_decode_video2')

	int len, got_frame;
	//---------------------------------------------------------------------
	pkt->size = (int)size;
	pkt->data = (u_int8_t*)ptr;
	//---------------------------------------------------------------------
	len = avcodec_decode_video2(c, frame, &got_frame, pkt);
	if (len < 0) {
		errno = ENOSYS;
		errMsg(STR_PREF "avcodec_decode_video2");
		return -1;
	} else if (len == 0) {
		// NOTE: in the state no frame could be decompressed;
		// this code don't support it.
		errno = ENOSYS;
		errMsg(STR_PREF "avcodec_decode_video2: return == 0");
		return -1;
	}
	//---------------------------------------------------------------------
	if (got_frame != 0) {
		ptr_res[0] = (unsigned char *)frame->data[0];
		ptr_res[1] = (unsigned char *)frame->data[1];
		ptr_res[2] = (unsigned char *)frame->data[2];
		*size_res = (unsigned int)(frame->width * frame->height);
	} else {
		// NOTE: in the state no frame could be decompressed;
		// this code don't support here.
		errno = ENOSYS;
		errMsg(STR_PREF "avcodec_decode_video2: got_frame == 0");
		return -1;
	}
	//---------------------------------------------------------------------
 	return 0;
}

//=============================================================================

/*!
	\brief This is last call after end of decode stream (flush encode)
	\param[out] ptr_res Array of pointers to colour parts 
	(e.g. Y: index 0; U: index 1; V: index 2)
	\param[out] size_res Size of decode frame (0 if empty data)
	\return 0 - success, -1 - error
*/
static int 
EndDec_ffmpeg_deprecated(unsigned char **ptr_res, unsigned int *size_res)
{
	ptr_res[0] = NULL;
	*size_res = 0u;

	return 0;
}

//=============================================================================

/*! @} */

#endif // FFMPEG_DISABLE

//=============================================================================
//=============================================================================

// stubs

static int 
Init_stub(void)
{
	return 0;
}

static int 
Delete_stub(void)
{
	return 0;
}

static int 
DecFrame_stub(unsigned char *ptr, unsigned int size, 
			unsigned char **ptr_res, unsigned int *size_res)
{
	ptr_res[0] = ptr;
	*size_res = size;
	return 0;
}

static int 
EndDec_stub(unsigned char **ptr_res, unsigned int *size_res)
{
	ptr_res[0] = NULL;
	*size_res = 0u;
	return 0;
}

//=============================================================================
//=============================================================================
//=============================================================================

#ifndef FFMPEG_DISABLE

struct dec_ops dec_ops_ffmpeg = {
	.CamStream_Decode_Init 		= Init_ffmpeg,
	.CamStream_Decode_Delete 	= Delete_ffmpeg,
	.CamStream_Decode_DecFrame	= DecFrame_ffmpeg,
	.CamStream_Decode_EndDec	= EndDec_ffmpeg,
};

//-----------------------------------------------------------------------------

struct dec_ops dec_ops_ffmpeg_deprecated = {
	.CamStream_Decode_Init 		= Init_ffmpeg,
	.CamStream_Decode_Delete 	= Delete_ffmpeg,
	.CamStream_Decode_DecFrame	= DecFrame_ffmpeg_deprecated,
	.CamStream_Decode_EndDec	= EndDec_ffmpeg_deprecated,
};

#endif // FFMPEG_DISABLE

//-----------------------------------------------------------------------------

struct dec_ops dec_ops_stub = {
	.CamStream_Decode_Init 		= Init_stub,
	.CamStream_Decode_Delete 	= Delete_stub,
	.CamStream_Decode_DecFrame	= DecFrame_stub,
	.CamStream_Decode_EndDec	= EndDec_stub,
};

//=============================================================================
