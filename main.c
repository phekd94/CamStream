
//=============================================================================

// NOTE: check cpu and mem perfomance
// ps -aux | grep cams
// top -p 5668

// NOTE: check net perfomance
// sudo iftop -PB -i lo

// NOTE: mtrace() at start main();
// #include <mcheck.h> // for mtrace(3)

// FIXME: for gray decode: AV_CODEC_FLAG_GRAY !!!
// AV_CODEC_FLAG_GLOBAL_HEADER
// AV_CODEC_FLAG2_LOCAL_HEADER

/*
// TODO: check buffer size (see tcp(7) -> ioctl)
#include <sys/ioctl.h>
extern int fd_client;
int count;
ioctl(fd_client, FIONREAD, &count)
*/

// TODO: CamStream_Transmit: try again connect to server when it disconnect

// TODO: cmd line parameters + struct parameters at each module + default 
//                                                           struct parameters

// TODO: trnToFile: open(O_NOATTIME) (kerrisk: chapter 4 and 14)

// TODO: WriteToFd: defragmitation transmit (see kerrisk)

// TODO: Check capture module whin it interrupting due to inet + need get mask 
//                                                                      reset?

//=============================================================================

#include "CamStream_TLPI.h"

#include "CamStream_Capture.h"
#include "CamStream_Transmit.h"
#include "CamStream_Print.h"
#include "CamStream_CalcTime.h"
#include "CamStream_Encode.h"
#include "CamStream_Decode.h"
#include "CamStream_Terminate.h"
#include "CamStream_Commands.h"
#include "CamStream_IO.h"

#ifndef DMAI_DISABLE

#include <xdc/std.h> 		// Void, Char, Int and etc.
#include "../demo.h" 		// GlobalData

// Global variable declarations for this application (need for ../demo.h)
GlobalData 	gbl 	= GBL_DATA_INIT; 

#endif // DMAI_DISABLE

//=============================================================================

static struct trn_ops 		trn;
static struct enc_ops		enc;
static struct dec_ops		dec;
static struct cap_ops		cap;

//=============================================================================

static int 			width			= 640; // 1920;
static int 			height			= 480; // 1080;
static int			framerate		= 24; // 25000
static int			bitrate			= 4000000; // 6000000
static int			gop			= 0;
static int			b			= 0;
static int			with_size		= 1;

//=============================================================================

static void __attribute__((unused))
PrintAvFPS(void)
{
	static Boolean 		start 		= TRUE;
	static double 		av_fps;
	
	if (start == TRUE) {
		start = FALSE;
		printf("[app]: average FPS = %5.2f", av_fps);
		fflush(stdout);
	}
	av_fps = CamStream_CalcTime_CalcAvFps();
	if (av_fps > 0) {
		printf("\b\b\b\b\b%5.2f", av_fps);
		fflush(stdout);
	}
}

//=============================================================================

static int 
CamStream_Delete(void)
{	
 #define	STR_PREF	"[delete]: "

	int ret = 0;
	
	/*if (CamStream_Commands_DeleteMMC() == -1) {
		ret = -1;
		CamStream_Print(TRUE, STR_PREF "CamStream_Commands_DeleteMMC");
	}*/

	if (dec.CamStream_Decode_Delete() == -1) {
		ret = -1;
		CamStream_Print(TRUE, STR_PREF "dec_Delete");
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "dec_Delete");

	if (enc.CamStream_Encode_Delete() == -1) {
		ret = -1;
		CamStream_Print(TRUE, STR_PREF "enc_Delete");
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "enc_Delete");

	if (trn.CamStream_Transmit_Delete() == -1) {
		ret = -1;
		CamStream_Print(TRUE, STR_PREF "trn_Delete");
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "trn_Delete");

	if (cap.CamStream_Capture_Delete() == -1) {
		ret = -1;
		CamStream_Print(TRUE, STR_PREF "cap_Delete");
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "cap_Delete");
	
	return ret;

 #undef		STR_PREF	
}

//=============================================================================

static void 
CamStream_ErrExit(char *s)
{
	CamStream_Print(TRUE, "%s", s);
	if (CamStream_Delete() == -1) {
		CamStream_Print(TRUE, "CamStream_Delete");
	}
	exit(EXIT_FAILURE);
}

//=============================================================================

static void 
CamStream_Init(void)
{
 #define	STR_PREF	"[init]: "

	//trn = trn_ops_gui;
	//trn = trn_ops_file;
	//trn = trn_ops_inet;
	trn = trn_ops_stub;

	//enc = enc_ops_ffmpeg;
	//enc = enc_ops_dmai;
	enc = enc_ops_stub;

	//dec = dec_ops_ffmpeg_deprecated;
	//dec = dec_ops_ffmpeg;
	dec = dec_ops_stub;

	//cap = cap_ops_v4l2;
	//cap = cap_ops_file;
	//cap = cap_ops_inet;
	cap = cap_ops_stub;

	if (CamStream_Terminate_Check(TERMINATE_INIT) == TRUE) {
		CamStream_ErrExit(STR_PREF "handle for signal SIGINT");
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
		"handle for signal SIGINT");

	if (cap.CamStream_Capture_Init() == -1) {
		CamStream_ErrExit(STR_PREF "cap_Init");
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "cap_Init");
	
	if (trn.CamStream_Transmit_Init(width, height) == -1) {
		CamStream_ErrExit(STR_PREF "trn_Init");
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "trn_Init");

	if (
		enc.CamStream_Encode_Init(bitrate, framerate, 
			width, height, gop, b) == -1
	) {
		CamStream_ErrExit(STR_PREF "enc_Init");
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "enc_Init");

	if (dec.CamStream_Decode_Init() == -1) {
		CamStream_ErrExit(STR_PREF "dec_Init");
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "dec_Init");
	
	/*if (CamStream_Commands_InitMMC() == -1) {
		CamStream_ErrExit(STR_PREF "CamStream_Commands_InitMMC");
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
						"CamStream_Commands_InitMMC");
	*/
 #undef		STR_PREF
}

//=============================================================================

// TODO: parameters
int 
main(
	int argc 		__attribute__((unused)),
	char const *argv[] 	__attribute__((unused))
)
{
	unsigned char*  frame_ptr;
	unsigned char*  frame_colors_ptrs[3];
	unsigned int    bytesused, msg_type;
	
	int             ret;
	
	sigset_t        blockMask;
	
	// Set mask for processor
	sigemptyset(&blockMask);
	sigaddset(&blockMask, SIGINT);
	sigprocmask(SIG_BLOCK, &blockMask, NULL);
	// Set point for jump from handler
	if (sigsetjmp(*CamStream_Terminate_Env, 1) == 0) {
		CamStream_Print(FALSE, ANSI_COLOR_GREEN 
			"sigsetjmp(): set point for jump");
	} else {
		printf("\b\b");
		CamStream_Print(FALSE, ANSI_COLOR_GREEN 
			"sigsetjmp(): jump from signal handler");
		
		// TODO: stop, DQBUF and other
		
		if (CamStream_Delete() == -1) {
			CamStream_Print(TRUE, "CamStream_Delete");
			exit(EXIT_FAILURE);
		} else {
			CamStream_Print(FALSE, ANSI_COLOR_GREEN 
				"CamStream_Delete");
			exit(EXIT_SUCCESS);
		}
	}
	
	CamStream_Capture_v4l2_CamType = CAM_TYPE_LOGITECH_USB_YUV;
	//CamStream_Transmit_inet_Address = "10.1.0.95";
	//CamStream_Transmit_inet_Address = "10.2.0.113";
	CamStream_Transmit_inet_Address = "127.0.0.1";
	//CamStream_CalcTime_SetPauseTime(30);
	//CamStream_Capture_file_FileName = "output.h264";
	//CamStream_Transmit_file_OutName = "output.h264";
	
	CamStream_Init();
	CamStream_Print(FALSE, ANSI_COLOR_GREEN "CamStream_Init");
	
	
	sigprocmask(SIG_UNBLOCK, &blockMask, NULL);
	if (cap.CamStream_Capture_Start() == -1) {
		CamStream_ErrExit("cap_Start");
	} else {
		CamStream_Print(FALSE, ANSI_COLOR_GREEN "cap_Start");
	}
	sigprocmask(SIG_BLOCK, &blockMask, NULL);
	
	if (CamStream_Transmit_inet_fd != -1) {
		if (CamStream_Commands_SendParams(
					CamStream_Transmit_inet_fd) == -1) {
			CamStream_ErrExit("CamStream_Commands_SendParams");
		} else {
			CamStream_Print(FALSE, ANSI_COLOR_GREEN 
				"CamStream_Commands_SendParams");
		}
	}
	
	while (CamStream_Terminate_Check(TERMINATE_SIG) == FALSE) {
		
		
		// TEST
		CamStream_Print(FALSE, ANSI_COLOR_YELLOW "[test]: sleep(2)");
		sleep(2);
		
		
		//CamStream_CalcTime_SaveStart();
		
		//-------------------------------------------------------------
		/* Client:
		 * 
		 * set non block fd from Transmit;
		 * if (nread > 0)
		 * 	set block fd from Transmit;
		 * 	set or get all cmd;
		 * 	if (get)
		 * 		send to server;
		 * 	continue;
		 * set block fd from Transmit;
		 * 
		 */
		
		if (CamStream_Transmit_inet_fd != -1) {
			if (CamStream_Commands_ReadCmd(
					CamStream_Transmit_inet_fd) == -1) {
				CamStream_Print(TRUE, 
						"CamStream_Commands_ReadCmd");
				break;
			}
		}
		
		// continue;
		
		//-------------------------------------------------------------
		/* Server:
		 * 
		 * if (need send cmd)
		 * 	send cmd using fd from Capture
		 * 	continue;
		 * 
		 */
		/*
		if (cmd_msg_size > 0 && 
				CamStream_Capture_inet_fd_client != -1) {
			
			if (WriteToFd(
				CamStream_Capture_inet_fd_client, 
				cmd_msg, cmd_msg_size, 
				with_size, TRANSMIT_TYPE_COMMAND) 
			== -1) {
				CamStream_Print(TRUE, "WriteToFd(command)");
				break;
			}
			cmd_msg = 0;
			
			continue;
		}
		*/
		// continue;
		
		//-------------------------------------------------------------
		
		sigprocmask(SIG_UNBLOCK, &blockMask, NULL);
		frame_ptr = cap.CamStream_Capture_Get(&bytesused, &msg_type);
		if (frame_ptr == NULL) {
			CamStream_Print(TRUE, "cap_GetFrame");
			break;
		}
		sigprocmask(SIG_BLOCK, &blockMask, NULL);
		
		if (msg_type == CAPTURE_TYPE_FRAME) {
			//-----------------------------------------------------

			ret = enc.CamStream_Encode_EncFrame(
					frame_ptr, bytesused, 
					&frame_ptr, &bytesused);
			if (ret == -1) {
				CamStream_Print(TRUE, "enc_EncFrame");
				break;
			} else if (ret == -2) {
				if (cap.CamStream_Capture_ThrowFrame() == -1) {
					CamStream_Print(TRUE, 
						"cap_ThrowFrame");
					break;
				}
				continue;
			}

			//-----------------------------------------------------
			
			ret = dec.CamStream_Decode_DecFrame(
					frame_ptr, bytesused, 
					frame_colors_ptrs, &bytesused);
			if (ret == -1) {
				CamStream_Print(TRUE, "dec_DecFrame");
				break;
			}else if (ret == -2) {
				if (cap.CamStream_Capture_ThrowFrame() == -1) {
					CamStream_Print(TRUE, 
						"cap_ThrowFrame");
					break;
				}
				continue;
			}

			//-----------------------------------------------------
			
			if (bytesused > 0) {
				ret = trn.CamStream_Transmit_SendFrame(
					frame_colors_ptrs, 
					bytesused, 
					width, height, with_size, 
					//TRANSMIT_COL_COLOR_YUV_FFMPEG);
					TRANSMIT_COL_COLOR_YUV_PACKED);
				if (ret == -1) {
					CamStream_Print(TRUE, "trn_SendFrame");
					break;
				}
			}
			
			//-----------------------------------------------------
			
			// FIXME: closer to capture frame (for actual info)
			if (CamStream_Transmit_inet_fd != -1) {
				ret = CamStream_Commands_SendCells(
					CamStream_Transmit_inet_fd);
				if (ret == -1) {
					CamStream_Print(TRUE, 
					"CamStream_Commands_SendCells");
					break;
				}
			}
			
			//-----------------------------------------------------
		
		} else if (msg_type == CAPTURE_TYPE_COMMAND) {
			/*
			 * Server:
			 *
			 * emit to Front;
			 * 
			 */
		} else if (msg_type == CAPTURE_TYPE_CELLS) {
			/*
			 * Server:
			 *
			 * emit to Front;
			 * 
			 */
		}

		if (cap.CamStream_Capture_ThrowFrame() == -1) {
			CamStream_Print(TRUE, "cap_ThrowFrame");
			break;
		}

		//CamStream_CalcTime_SaveFinish();
		//PrintAvFPS();
		//CamStream_CalcTime_SetPause();
	}
	//---------------------------------------------------------------------
	//printf("\b\b\b\b\b\b\b       \n");
	printf("\b\b\b");
	//---------------------------------------------------------------------
	if (enc.CamStream_Encode_EndEnc(&frame_ptr, &bytesused) == -1) {
		CamStream_Print(TRUE, "enc_EndCode");
	} else {
		CamStream_Print(FALSE, ANSI_COLOR_GREEN "enc_EndCode");
	}

	if (dec.CamStream_Decode_EndDec(&frame_ptr, &bytesused) == -1) {
		CamStream_Print(TRUE, "dec_EndDec");
	} else {
		CamStream_Print(FALSE, ANSI_COLOR_GREEN "dec_EndDec");
	}
	//---------------------------------------------------------------------
	if (cap.CamStream_Capture_Stop() == -1) {
		CamStream_ErrExit("cap_Stop");
	} else {
		CamStream_Print(FALSE, ANSI_COLOR_GREEN "cap_Stop");
	}
	//---------------------------------------------------------------------
	if (CamStream_Delete() == -1) {
		CamStream_Print(TRUE, "CamStream_Delete");
		exit(EXIT_FAILURE);
	} else {
		CamStream_Print(FALSE, ANSI_COLOR_GREEN "CamStream_Delete");
		exit(EXIT_SUCCESS);
	}
}

//=============================================================================
