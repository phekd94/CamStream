
//=============================================================================

// NOTE:


//=============================================================================

#include "CamStream_TLPI.h"

//-----------------------------------------------------------------------------

#include "CamStream_Print.h"
#include "CamStream_IO.h"
#include "CamStream_Commands.h"

//-----------------------------------------------------------------------------

#include <fcntl.h> // for fcntl(2)

//=============================================================================

#define STR_PREF "[cmd]: "

//-----------------------------------------------------------------------------

#define CMD_NUM          10
#define CMD_LENGTH       12

#define CMD_INDEX_ID     2

#define CMD_ID_RESERVE   0x60

//#define CELL_WIDTH       (160 / 8) // (320 / 8)
//#define CELL_HEIGHT      120 // 256
#define CELL_WIDTH       (320 / 8)
#define CELL_HEIGHT      256

#define BUF_PARAMS_LEN   64

#define SD_DEV_NAME      "/dev/mmcblk0"

//=============================================================================

// Commands id(code) and length
struct cmd_id_len {
	unsigned char id;
	unsigned int length;
} cil[] = {
	{0x21, 8},
	{0x20, 7},
	{0x30, 4},
	{0x31, 6},
	{0x32, 4},
	{0x33, 5},
	{0x40, 4},
	{0x41, 4},
	{0x50, 4},
	{0x60, 7},
	{0x00, 0},
};

//=============================================================================

// Buffer for commands
static unsigned char cmd_buf[CMD_NUM * CMD_LENGTH + sizeof(unsigned int)];

// Buffer for cells
static unsigned char cell_buf[CELL_WIDTH * CELL_HEIGHT];

// CamV parameters
static unsigned char camVparams_buf[BUF_PARAMS_LEN];

// MMC fd
static int fd_mmc = -1;

//=============================================================================

// TODO: add doxygen description
int 
CamStream_Commands_SendParams(int fd)
{
	camVparams_buf[3] = 6; // camera fault + auto mode
	camVparams_buf[4] = 1; // image type (self-control)
	camVparams_buf[5] = 0x01; // current mode (x4)
	camVparams_buf[7] = 2; // x = 2
	camVparams_buf[9] = 2; // y = 2
	camVparams_buf[15] = 20; // temperature (20)
	camVparams_buf[17] = 30; // accumulation time (30 << 8 = 7680)
	camVparams_buf[19] = 0x18; // analog amplifier (x1.26)
	camVparams_buf[20] = 0x0F; // 3904: digital amplifier (30.5)
	camVparams_buf[21] = 0x40; // 3904
	camVparams_buf[22] = 73; // threshold (2.2...)
	
	//camVparams_buf[19] = 0x01; // analog amplifier (x1)
	
	return WriteToFd(fd, camVparams_buf, BUF_PARAMS_LEN, 
		1/*add size*/, TRANSMIT_TYPE_PARAMS);
}

//=============================================================================

// TODO: add doxygen description
static int 
ResetNonBlock(int fd, int flags)
{
	// Reset O_NONBLOCK flag
	flags &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) == -1) {
		errMsg("fcntl(fd, F_SETFL, - O_NONBLOCK)");
		return -1;
	}
	return 0;
}

//-----------------------------------------------------------------------------

int 
CamStream_Commands_ReadCmd(int fd)
{
	static int 		flags 		= -1;
	ssize_t 		num_read;
	unsigned int 		u_size;
	ssize_t			pos;
	int i;
	
	// First call
	if (flags == -1) {
		flags = fcntl(fd, F_GETFL);
		if (flags == -1) {
			errMsg(STR_PREF "fcntl(F_GETFL)");
			return -1;
		}
	}
	
	// Set O_NONBLOCK flag
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) == -1) {
		errMsg(STR_PREF "fcntl(fd, F_SETFL, + O_NONBLOCK)");
		return -1;
	}
	
	// Try to read in non block mode
	num_read = read(fd, &u_size, sizeof(u_size));
	if (num_read == 0) {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_YELLOW 
							"EOF cmd size");
		ResetNonBlock(fd, flags);
		return -1;
	} else if (num_read == -1 && 
				(errno == EAGAIN || errno == EWOULDBLOCK)) {
		if (ResetNonBlock(fd, flags) == -1) {
			return -1;
		}
		return 0;
	} else if (num_read == -1) {
		errMsg(STR_PREF "read cmd size");
		ResetNonBlock(fd, flags);
		return -1;
	} else if (num_read != sizeof(u_size)) {
		CamStream_Print(TRUE, STR_PREF "num_read != sizeof(u_size)");
		ResetNonBlock(fd, flags);
		return -1;
	}
	
	// Success read in non block mode
	// CamStream_Print(FALSE, STR_PREF "size has read = %u", u_size);
	
	// Clear O_NONBLOCK flag
	// WARNING: This can be program hang cause at bellow function
	if (ResetNonBlock(fd, flags) == -1) {
		return -1;
	}
	
	num_read = readn(fd, cmd_buf, u_size + sizeof(unsigned int));
	if (num_read == 0) {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_YELLOW 
							"EOF cmd data");
		return -1;
	} else if (num_read == -1) {
		errMsg(STR_PREF "read cmd data");
		return -1;
	} else if (num_read != (ssize_t)(u_size + sizeof(unsigned int))) { 
 // not need, but compilator warning (ISO: 6.5.8) --^
		CamStream_Print(TRUE, STR_PREF "num_read != u_size + cmd");
		return -1;
	}
	
	if (cmd_buf[0] != CAPTURE_TYPE_COMMAND) {
		CamStream_Print(TRUE, STR_PREF "Read msg is not command");
		return -1;
	}
	
	for (pos = sizeof(unsigned int); pos < num_read; 
							pos += cil[i].length) {
		for (i = 0; cil[i].id != 0x00; ++i) {
			if (cmd_buf[pos + CMD_INDEX_ID] == cil[i].id)
				break;
		}
		
		if (cil[i].id != 0x00) {
			CamStream_Print(FALSE, STR_PREF "0x%02X", cil[i].id);
			if (cil[i].id == CMD_ID_RESERVE) {
				if (CamStream_Commands_SendParams(fd) == -1) {
					CamStream_Print(TRUE, STR_PREF 
					"CamStream_Commands_SendParams()");
					return -1;
				}
			} else {
				/*
				ssize_t tot_out;
				tot_out = pwrite(
					fd_mmc, 
					&cmd_buf[pos], 
					cil[i].length, 
					0);
				if (tot_out == -1) {
					errMsg(STR_PREF "pwrite MMC");
					break;
				}
				CamStream_Print(FALSE, STR_PREF 
					ANSI_COLOR_GREEN "cmd writes success");
				*/
			}
		} else {
			CamStream_Print(TRUE, STR_PREF "Unknow command");
			break;
		}
	}
	
	return 0;
}

//=============================================================================

int 
CamStream_Commands_SendCells(int fd)
{
	/*static int i = 1;
	cell_buf[i-1] = 0;
	cell_buf[i++] = 1;
	if (i > CELL_WIDTH * CELL_HEIGHT - 1)
		i = 1;*/
	memset(cell_buf, 0, CELL_WIDTH * CELL_HEIGHT);
	cell_buf[0] = 1;
	cell_buf[4*80] = 1;
	//cell_buf[CELL_WIDTH * CELL_HEIGHT - 1] = 1;
	
	return WriteToFd(fd, cell_buf, CELL_WIDTH * CELL_HEIGHT, 
		1/*add size*/, TRANSMIT_TYPE_CELLS);
}

//=============================================================================

int 
CamStream_Commands_InitMMC(void)
{
	fd_mmc = open(SD_DEV_NAME, O_RDWR);
	if (fd_mmc == -1) {
		errMsg(STR_PREF "open %s", SD_DEV_NAME);
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
						"open %s", SD_DEV_NAME);
	return 0;
}

//=============================================================================

int 
CamStream_Commands_DeleteMMC(void)
{
	if (fd_mmc == -1) {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
						"fd_mmc == -1");
		return 0;
	}
	if (close(fd_mmc) == -1) {
		errMsg(STR_PREF "close %s", SD_DEV_NAME);
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
						"close %s", SD_DEV_NAME);
	return 0;
}

//=============================================================================
