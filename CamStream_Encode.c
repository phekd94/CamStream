
//=============================================================================

#include "CamStream_TLPI.h"

#include "CamStream_Print.h"
#include "CamStream_Encode.h"

//-----------------------------------------------------------------------------

#ifndef FFMPEG_DISABLE

// ffmpeg
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>

#endif // FFMPEG_DISABLE

//-----------------------------------------------------------------------------

#ifndef DMAI_DISABLE

// dmai
#include <xdc/std.h> 		// Void, Char, Int and etc.
#include "../demo.h" 		// CODECHEIGHTALIGN

#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/ce/CERuntime.h>

#include <ti/sdo/dmai/Dmai.h>
#include <ti/sdo/dmai/ce/Venc1.h>
#include <ti/sdo/dmai/VideoStd.h>
#include <ti/sdo/dmai/BufferGfx.h>

#endif // DMAI_DISABLE

//=============================================================================

#define STR_PREF		"[enc]: "

//=============================================================================

#ifndef FFMPEG_DISABLE

// ffmpeg
char * 		CamStream_Encode_ffmpeg_CodecName = "h264";
int		CamStream_Encode_ffmpeg_CodecID = AV_CODEC_ID_H264;
int 		CamStream_Encode_ffmpeg_PixFmt = AV_PIX_FMT_YUVJ422P;
//int 		CamStream_Encode_ffmpeg_ColRange = AVCOL_RANGE_JPEG;

#endif // FFMPEG_DISABLE

//=============================================================================

#ifndef FFMPEG_DISABLE

// ffmpeg
static const AVCodec 		*codec			= NULL;
static AVCodecContext 		*c			= NULL;
static AVPacket			*pkt			= NULL;
static AVFrame			*frame			= NULL;

#endif // FFMPEG_DISABLE

//-----------------------------------------------------------------------------

#ifndef DMAI_DISABLE

// dmai
static char * 			codecName = "h264enc";

// dmai buffers
static Int32 			bufSize;
static Buffer_Handle		hCapBuf			= NULL;
static Buffer_Handle		hDstBuf			= NULL;
static Int8			*dstBufUserPtr;
static Int8			*capBufUserPtr;

// dmai encoder
static Venc1_Handle		hVe1			= NULL;
static Engine_Handle 		hEngine			= NULL;

#endif // DMAI_DISABLE

//=============================================================================
//=============================================================================

#ifndef FFMPEG_DISABLE

// ffmpeg

/*!
	\defgroup ffmpeg_encode Encode using ffmpeg
	 \ingroup Encode
	\brief Encode frame using ffmpeg
	\version 1.3
	\note Error description output in 'stderr' stream using 
	'ErrorFunctions' module

	A set of functions that allows you to encode frame using ffmpeg.
	@{
*/

//=============================================================================

/*!
	\brief Initialization the encode module using ffmpeg
	\param[in] bitrate Bit rate for encoder (influence to the encode speed)
	\param[in] framerate Frames Per Second (FPS)
	\param[in] width Width in pixels for image
	\param[in] height Height in pixels for image
	\param[in] gop Group Of Pictures (GOP)
	\param[in] b B frames number
	\return 0 - success, -1 - error
*/
static int 
Init_ffmpeg(int bitrate, int framerate,
				int width, int height,
				int gop, int b)
{
	int ret;
	//---------------------------------------------------------------------
	//codec = 
	// avcodec_find_encoder_by_name(CamStream_Encode_ffmpeg_CodecName);
	codec = avcodec_find_encoder(CamStream_Encode_ffmpeg_CodecID);
	if (codec == NULL) {
		errno = ENOSYS;
		errMsg(STR_PREF "avcodec_find_encoder for '%s'(%d)", 
			CamStream_Encode_ffmpeg_CodecName, 
			CamStream_Encode_ffmpeg_CodecID);
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
		"avcodec_find_encoder");
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
	pkt = av_packet_alloc();
	if (pkt == NULL) {
		errno = ENOSYS;
		errMsg(STR_PREF "av_packet_alloc");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "av_packet_alloc");
	//---------------------------------------------------------------------
	c->bit_rate 		= bitrate;

	// resolution must be a multiple of two
	c->width 		= width;
	c->height 		= height;
	
	// frames per second
	c->time_base 		= (AVRational){1, framerate};
	c->framerate 		= (AVRational){framerate, 1};

	// NOTE: bellow TODO
	// https://ffmpeg.org/doxygen/trunk/decoding__encoding_8c-source.html
	c->gop_size 		= gop; // intra frame only
	// TODO: if gop == 10 - emit one intra frame every ten frames
	
	c->max_b_frames 	= b; // intra frame only
	// TODO: probe b == 1

	c->pix_fmt 		= CamStream_Encode_ffmpeg_PixFmt;
	
	//c->color_range 	= CamStream_Encode_ffmpeg_ColRange;

	if (codec->id == AV_CODEC_ID_H264) {
        	av_opt_set(c->priv_data, "preset", "slow", 0);
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
	frame->format 	= c->pix_fmt;
	frame->width  	= c->width;
	frame->height 	= c->height;

	//frame->color_range 	= c->color_range;

	//---------------------------------------------------------------------
	ret = av_frame_get_buffer(frame, 0);
	if (ret < 0) {
		errno = ENOSYS;
		errMsg(STR_PREF "av_frame_get_buffer");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
		"av_frame_get_buffer");
	//---------------------------------------------------------------------
	return 0;
}

//=============================================================================

/*!
	\brief Delete the encode module using ffmpeg
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
	\brief Encode frame using ffmpeg
	\param[in] ptr Pointer to frame for encode
	\param[in] size Size of frame for encode
	\param[out] ptr_res Pointer to encode frame
	\param[out] size_res Size of encode frame
	\return 0 - success, -1 - error, -2 - EAGAIN,
	\note Some codec use several frame therefore can return EAGAIN
	\warning Memory for result buffers allocates here
*/
static int 
EncFrame_ffmpeg(
			unsigned char *ptr, 
			unsigned int size 	__attribute__((unused)), 
			unsigned char **ptr_res, 
			unsigned int *size_res
		)
{
	int 			ret, x, y, i;
	static int64_t 		num 			= 0;
	//---------------------------------------------------------------------
	av_packet_unref(pkt);
	//---------------------------------------------------------------------
	ret = av_frame_make_writable(frame);
	if (ret < 0) {
		errno = ENOSYS;
		errMsg(STR_PREF "av_frame_make_writable");
		return -1;
	}
	//---------------------------------------------------------------------
	// NOTE: only for YUV422 packed format
	for (y = 0, i = 0; y < c->height; ++y) {
		for (x = 0; x < c->width; ++x, i += 2) {
			frame->data[0][y * frame->linesize[0] + x] = 
				ptr[i];
		}
	}
	// NOTE: full height, but half width
	for (y = 0, i = 1; y < c->height; ++y) {
		for (x = 0; x < c->width/2; ++x, i += 4) {
			frame->data[1][y * frame->linesize[1] + x] = 
				ptr[i];
			frame->data[2][y * frame->linesize[2] + x] = 
				ptr[i + 2];
		}
	}
	//---------------------------------------------------------------------
	if (num == LONG_MAX) {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_YELLOW "pts limit");
		num = 0;
	}
	frame->pts 	= num++;
	//=====================================================================
	// encode
	ret = avcodec_send_frame(c, frame);
	if (ret < 0) {
		if (ret == AVERROR(EAGAIN)) {
			// NOTE: this means should be call 
			// avcodec_receive_packet,
			// but don't do it here
			errno = ENOSYS;
			errMsg(STR_PREF "avcodec_send_frame and "
			"EAGAIN: user must read output pkt");
			return -1;
		}
		if (ret == AVERROR(ENOMEM)) {
			errno = ENOSYS;
			errMsg(STR_PREF "avcodec_send_frame: ENOMEM");
			return -1;
		}
		errno = ENOSYS;
		errMsg(STR_PREF "avcodec_send_frame");
		return -1;
	}
	//---------------------------------------------------------------------
	// NOTE: pkt allocates by encoder and call av_frame_unref(frame)
	ret = avcodec_receive_packet(c, pkt);
	if (ret < 0) {
		if (ret == AVERROR(EAGAIN)) {
			return -2;
		}
		errno = ENOSYS;
		errMsg(STR_PREF "avcodec_receive_packet");
		return -1;
	}
	//---------------------------------------------------------------------
	*ptr_res 	= (unsigned char*)pkt->data;
	*size_res 	= (unsigned int)pkt->size;	
	//=====================================================================
	return 0;
}

//=============================================================================

/*!
	\brief This is last call after end of encode stream (flush encode)
	\param[out] ptr_res Pointer to encode frame (NULL if empty data)
	\param[out] size_res Size of encode frame (0 if empty data)
	\return 0 - success, -1 - error
*/
static int 
EndEnc_ffmpeg(unsigned char **ptr_res, unsigned int *size_res)
{
	int ret;
	
	// NOTE: return pkt after flush encoder
	// This signals the end of the stream. If the encoder still has 
	// packets buffered, it will return them after this call.
	ret = avcodec_send_frame(c, NULL);
	if (ret < 0) {
		errno = ENOSYS;
		errMsg(STR_PREF "flush: avcodec_send_frame");
		return -1;
	}
	// NOTE: must be avcodec_receive_packet after flush
	// but don't do it here
	av_packet_unref(pkt);
	
	*ptr_res 	= NULL;
	*size_res 	= 0u;
	
	return 0;
}

//=============================================================================

/*! @} */

#endif // FFMPEG_DISABLE

//=============================================================================
//=============================================================================

#ifndef DMAI_DISABLE

// dmai

/*!
	\defgroup dmai_encode Encode using dmai
	 \ingroup Encode
	\brief Encode frame using dmai
	\version 1.3
	\note Error description output in 'stderr' stream using 'error 
	functions' module

	A set of functions that allows you to encode frame using dmai.
	@{
*/

//=============================================================================

/*!
	\brief Initialization the encode module using dmai
	\param[in] bitrate Bit rate for encoder (influence to the encode speed)
	\param[in] framerate Frames Per Second (FPS)
	\param[in] width Width in pixels for image
	\param[in] height Height in pixels for image
	\param[in] gop Group Of Pictures (GOP)
	\param[in] b B frames number
	\return 0 - success, -1 - error
*/
static int 
Init_dmai(int bitrate, int framerate,
				int width, int height,
				int gop, int b)
{
	int getBuffer(Buffer_Handle *hBuf, Bool ref);
	int getVenc(int, int, int, int, int);
	//---------------------------------------------------------------------
	// Initialize Codec Engine runtime
	CERuntime_init();
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "CERuntime_init");
	//---------------------------------------------------------------------
	// Initialize Davinci Multimedia Application Interface
	Dmai_init();
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "Dmai_init");
	//---------------------------------------------------------------------
	// Get buffer for capture 
	if (getBuffer(&hCapBuf, TRUE) != 0) {
		hCapBuf = NULL;
		errno = ENOSYS;
		errMsg(STR_PREF "getBuffer(hCapBuf)");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "getBuffer(hCapBuf)");
	//---------------------------------------------------------------------
	// Get buffer for codec result
	if (getBuffer(&hDstBuf, FALSE) != 0) {
		hDstBuf = NULL;
		errno = ENOSYS;
		errMsg(STR_PREF "getBuffer(hDstBuf)");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "getBuffer(hDstBuf)");
	//---------------------------------------------------------------------
	//Buffer_print(hCapBuf);
	//Buffer_print(hDstBuf);
	//---------------------------------------------------------------------
	// Get video encoder
	if (getVenc(bitrate, framerate, width, height, b) != 0) {
		errno = ENOSYS;
		errMsg(STR_PREF "getVenc");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "getVenc");
	//---------------------------------------------------------------------
	return 0;
}

//-----------------------------------------------------------------------------

/*!
	\brief Get dmai buffer for capture and codec result
	\param[in] hBuf Pointer to buffer handle
	\param[in] ref if 'true' than don't allocate memory for buffer
	\return 0 - success, -1 - error
	\note Using in 'Init_dmai()'
*/
int 
getBuffer(Buffer_Handle *hBuf, Bool ref)
{
	BufferGfx_Attrs 	hBuf_attrs 	= BufferGfx_Attrs_DEFAULT;
	Int 			ret;

	bufSize = BufferGfx_calcSize(VideoStd_1080P_30, 
						ColorSpace_YUV420PSEMI);
	if (bufSize < 0) {
		CamStream_Print(TRUE, STR_PREF "BufferGfx_calcSize");
		return -1;
	}

	ret = BufferGfx_calcDimensions(VideoStd_1080P_30, 
				ColorSpace_YUV420PSEMI,	&hBuf_attrs.dim);
	if (ret < 0) {
		CamStream_Print(TRUE, STR_PREF "BufferGfx_calcDimensions");
		return -1;
	}
	hBuf_attrs.bAttrs.type = Buffer_Type_GRAPHICS;
	hBuf_attrs.colorSpace = ColorSpace_YUV420PSEMI;
        //hBuf_attrs.dim.lineLength = Dmai_roundUp(
	//			BufferGfx_calcLineLength(
	//				hBuf_attrs.dim.width,
        //                       	ColorSpace_YUV420PSEMI
	//				), 
	//			32
	//			);
	hBuf_attrs.dim.height = Dmai_roundUp( // This is '#define'
		hBuf_attrs.dim.height, CODECHEIGHTALIGN);

	hBuf_attrs.bAttrs.reference = ref;

	*hBuf = Buffer_create(bufSize, BufferGfx_getBufferAttrs(&hBuf_attrs));
	if (*hBuf == NULL) {
		CamStream_Print(TRUE, STR_PREF "Buffer_create");
		return -1;
	}

	return 0;
}

//-----------------------------------------------------------------------------

/*!
	\brief Get dmai video encoder
	\param[in] bitrate Bit rate for encoder (influence to the encode speed)
	\param[in] framerate Frames Per Second (FPS)
	\param[in] width Width in pixels for image
	\param[in] height Height in pixels for image
	\param[in] b B frames number
	\return 0 - success, -1 - error
	\note Using in 'Init_dmai()'
*/
int 
getVenc(int bitrate, int framerate, int width, int height, int b)
{
	VIDENC1_Params         	params 		= Venc1_Params_DEFAULT;
	VIDENC1_DynamicParams  	dynParams 	= Venc1_DynamicParams_DEFAULT;

	hEngine = Engine_open(engine->engineName, NULL, NULL);
	if (hEngine == NULL) {
		CamStream_Print(TRUE, STR_PREF "Engine_open");
		return -1;
	}

	params.maxWidth = width;
	params.maxHeight = Dmai_roundUp(height, CODECHEIGHTALIGN);
	params.encodingPreset = XDM_HIGH_SPEED; // !!! MOST SPEED !!!
						// ??? XDM_DEFAULT is bad ???
	params.inputChromaFormat = XDM_YUV_420SP;
	params.reconChromaFormat = XDM_YUV_420SP;
	
	// >>> SHIT DECODE
	//params.maxFrameRate      = framerate;
	// <<<

	// >>>
	//params.maxInterFrameInterval = 0;
	// <<<

	// grep "IVIDEO_NONE = 4"
	params.rateControlPreset = IVIDEO_NONE; // !!! MOST SPEED !!!
	params.maxBitRate = bitrate;//params.maxBitRate / 2 * 3; // ???
		//maxBitRate default is best

	dynParams.targetBitRate   = params.maxBitRate;
	dynParams.inputWidth      = width;
	dynParams.inputHeight     = height;
	dynParams.refFrameRate    = params.maxFrameRate;
	dynParams.targetFrameRate = params.maxFrameRate;

	// >>>SHIT DECODE
	//dynParams.intraFrameInterval = framerate / 1000;
	// <<<

	// !!! >>>
	//dynParams.interFrameInterval = b; 
			// ^--- Number of B frames between two reference !!!
	// !!! <<<

	// Create the video encoder
    	hVe1 = Venc1_create(hEngine, codecName, &params, &dynParams);
    	if (hVe1 == NULL) {
		CamStream_Print(TRUE, STR_PREF "Venc1_create");
		return -1;
	}

	return 0;
}

//=============================================================================

/*!
	\brief Delete the encode module using dmai
	\return 0 - success, -1 - error
*/
static int 
Delete_dmai(void)
{
	int er = 0;
	//---------------------------------------------------------------------
	if (hVe1 != NULL) {
		if (Venc1_delete(hVe1) < 0) {
			CamStream_Print(TRUE, STR_PREF "Venc1_delete");
			er = -1;
		} else {
			CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
				"Venc1_delete");
		}
		hVe1 = NULL;
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
							"hVe1 == NULL");
	}
	//---------------------------------------------------------------------
	if (hEngine != NULL) {
		Engine_close(hEngine); // don't return function
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
							"Engine_close");
		hEngine = NULL;
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
							"hEngine == NULL");
	}
	//---------------------------------------------------------------------
	if (hDstBuf != NULL) {
		if (Buffer_delete(hDstBuf) < 0) {
			CamStream_Print(TRUE, STR_PREF 
						"Buffer_delete(hDstBuf)");
			er = -1;
		} else {
			CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
				"Buffer_delete(hDstBuf)");
		}
		hDstBuf = NULL;
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
							"hDstBuf == NULL");
	}
	//---------------------------------------------------------------------
	if (hCapBuf != NULL) {
		if (Buffer_delete(hCapBuf) < 0) {
			CamStream_Print(TRUE, STR_PREF 
						"Buffer_delete(hCapBuf)");
			er = -1;
		} else {
			CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
				"Buffer_delete(hCapBuf)");
		}
		hCapBuf = NULL;
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
							"hCapBuf == NULL");
	}
	//---------------------------------------------------------------------
	return er;
}

//=============================================================================

/*!
	\brief Encode frame using dmai
	\param[in] ptr Pointer to frame for encode
	\param[in] size Size of frame for encode
	\param[out] ptr_res Pointer to encode frame
	\param[out] size_res Size of encode frame
	\return 0 - success, -1 - error
*/
static int 
EncFrame_dmai(
			unsigned char *ptr, 
			unsigned int size 	__attribute__((unused)), 
			unsigned char **ptr_res, 
			unsigned int *size_res
		)
{
	int ret;
	//---------------------------------------------------------------------
	capBufUserPtr = (Int8*)ptr;
	//---------------------------------------------------------------------
	// Set the pointer to DMAI buffer
	ret = Buffer_setUserPtr(hCapBuf, capBufUserPtr);
	if (ret < 0) {
		errno = ENOSYS;
		errMsg(STR_PREF "Buffer_setUserPtr");
		return -1;
	}
	ret = Buffer_setSize(hCapBuf, bufSize);
	if (ret < 0) {
		errno = ENOSYS;
		errMsg(STR_PREF "Buffer_setSize");
		return -1;
	}
	//---------------------------------------------------------------------
	// Set the number of bytes used
	Buffer_setNumBytesUsed(hCapBuf, bufSize);
	// Marking the buffer as busy
	Buffer_resetUseMask(hCapBuf);
	//---------------------------------------------------------------------
	// Encode the frame
	ret = Venc1_process(hVe1, hCapBuf, hDstBuf);
	if (ret < 0) {
		errno = ENOSYS;
		errMsg(STR_PREF "Venc1_process");
		return -1;
	}
	//---------------------------------------------------------------------
	// Get pointer to result buffer
	dstBufUserPtr = Buffer_getUserPtr(hDstBuf);
	//---------------------------------------------------------------------
	*ptr_res 	= (unsigned char*)dstBufUserPtr;
	*size_res 	= (int)Buffer_getNumBytesUsed(hDstBuf);
	//---------------------------------------------------------------------
	return 0;
}

//=============================================================================

/*!
	\brief Free dmai buffer after using result encode
	\param[out] ptr_res Pointer to encode frame (NULL if empty data)
	\param[out] size_res Size of encode frame (0 if empty data)
	\return 0 - success (always)
*/
static int 
EndEnc_dmai(unsigned char **ptr_res, unsigned int *size_res)
{
	// After DMAI considered the result buffer is free
	Buffer_freeUseMask(hDstBuf, 0);
	*ptr_res 	= NULL;
	*size_res 	= 0;
	return 0;
}

//=============================================================================

/*! @} */

#endif // DMAI_DISABLE

//=============================================================================
//=============================================================================

// stubs

static int 
Init_stub(
		int bitrate 	__attribute__((unused)), 
		int framerate 	__attribute__((unused)),
		int width 	__attribute__((unused)), 
		int height 	__attribute__((unused)),
		int gop 	__attribute__((unused)), 
		int b 		__attribute__((unused))
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
EncFrame_stub(unsigned char *ptr, unsigned int size, 
			unsigned char **ptr_res, unsigned int *size_res)
{
	*ptr_res = ptr;
	*size_res = size;
	return 0;
}

static int 
EndEnc_stub(unsigned char **ptr_res, unsigned int *size_res)
{
	ptr_res[0] = NULL;
	*size_res = 0;
	return 0;
}

//=============================================================================
//=============================================================================
//=============================================================================

#ifndef FFMPEG_DISABLE

struct enc_ops enc_ops_ffmpeg = {
	.CamStream_Encode_Init 		= Init_ffmpeg,
	.CamStream_Encode_Delete 	= Delete_ffmpeg,
	.CamStream_Encode_EncFrame	= EncFrame_ffmpeg,
	.CamStream_Encode_EndEnc	= EndEnc_ffmpeg,
};

#endif // FFMPEG_DISABLE

//-----------------------------------------------------------------------------

#ifndef DMAI_DISABLE

struct enc_ops enc_ops_dmai = {
	.CamStream_Encode_Init 		= Init_dmai,
	.CamStream_Encode_Delete 	= Delete_dmai,
	.CamStream_Encode_EncFrame	= EncFrame_dmai,
	.CamStream_Encode_EndEnc	= EndEnc_dmai,
};

#endif // DMAI_DISABLE

//-----------------------------------------------------------------------------

struct enc_ops enc_ops_stub = {
	.CamStream_Encode_Init 		= Init_stub,
	.CamStream_Encode_Delete 	= Delete_stub,
	.CamStream_Encode_EncFrame	= EncFrame_stub,
	.CamStream_Encode_EndEnc	= EndEnc_stub,
};

//=============================================================================
