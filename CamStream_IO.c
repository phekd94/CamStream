
// TODO: buffer allocate memory or static buffer with frame max size

//=============================================================================

#include "CamStream_TLPI.h"

#include "CamStream_Print.h"
#include "CamStream_Commands.h"

//=============================================================================

#define STR_PREF   "[i/o]: "

//-----------------------------------------------------------------------------

// TODO: another way for this const and BUFFER
#define BUF_DATA_SIZE 		100000

// TODO: another way for this const
//#define BUF_DATA_SIZE 		10000 // malloc + 4096 for decode !!!! 
					// + delete and delete in module_delete

//=============================================================================

// TODO: add doxygen description
ssize_t 
writen(int fd, void *buffer, size_t count)
{
	ssize_t 	numWritten;
	size_t 		totWritten;
	const char 	*buf;

	buf = (char*)buffer;
	for (totWritten = 0; totWritten < count; ) {
		numWritten = write(fd, buf, count - totWritten);
		if (numWritten <= 0) {
			// NOTE: EINTR: call was interrupted by a signal
			// (see man 2 write)
			if (numWritten == -1 && errno == EINTR)
				continue;
			else
				return -1;
		}
		totWritten	+= numWritten;
		buf		+= numWritten;
	}
	return totWritten;
}

//-----------------------------------------------------------------------------

// TODO: add doxygen description
// TODO: write - signal SIGPIPE and error EPIPE (server close)
int 
WriteToFd(int fd, unsigned char *ptr, unsigned int size, 
	  int c, unsigned int msg_type)
{
	if (c == 1) {
		if (writen(fd, &size, sizeof(unsigned int)) != 
						sizeof(unsigned int)) {
			errMsg(STR_PREF "write(size)");
			return -1;
		}
		//CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
		//	"write(size)");
	}
	
	if (msg_type != TRANSMIT_TYPE_NONE) {
		if (writen(fd, &msg_type, sizeof(unsigned int)) !=
						sizeof(unsigned int)) {
			errMsg(STR_PREF "write(trn type)");
			return -1;
		}
		//CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
		//	"write(trn type)");
	}

	if (writen(fd, ptr, size) != size) {
		errMsg(STR_PREF "write(data)");
		return -1;
	}
	//CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "write(data)");

	return 0;
}

//=============================================================================

// TODO: add doxygen description
ssize_t 
readn(int fd, void *buf_tmp, size_t count)
{
	ssize_t numRead;
	size_t totRead;
	char *buf;

	buf = buf_tmp;
	for (totRead = 0; totRead < count; ) {
		numRead = read(fd, buf, count - totRead);
		
		if (numRead == 0) {
			return totRead;
		}
		if (numRead == -1) {
			if (errno == EINTR) {
				CamStream_Print(FALSE, STR_PREF "EINTR");
				continue;
			} else {
				return -1;
			}
		}

		totRead += numRead;
		buf += numRead; 
	}
	return totRead;
}

//-----------------------------------------------------------------------------

// TODO: add doxygen description
// TODO: read - signal ??? and error ??? (client close) (+ signal() for the 
// signal)
unsigned char * 
ReadFromFd(int fd, unsigned int *bytesused, unsigned int *msg_type)
{
	//---------------------------------------------------------------------
	static unsigned char 		data_buf[BUF_DATA_SIZE];
	ssize_t 			num_read;
	unsigned int 			u_size;
	//---------------------------------------------------------------------
	// read size
	num_read = readn(fd, &u_size, sizeof(unsigned int));
	if (num_read == 0) {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_YELLOW "EOF size");
		*bytesused = 0;
		return NULL;
	} else if (num_read == -1) {
		errMsg(STR_PREF "read size");
		*bytesused = 0;
		return NULL;
	} else if (num_read == sizeof(unsigned int)) {
		//CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN
		//	"read size: %u", u_size);
	} else {
		errno = ENOSYS;
		errMsg(STR_PREF "size: num_read < sizeof(unsigned int)");
		*bytesused = 0;
		return NULL;
	}
	//---------------------------------------------------------------------
	// read message type
	if (*msg_type != CAPTURE_TYPE_NONE) {
		num_read = readn(fd, msg_type, sizeof(unsigned int));
		if (num_read == 0) {
			CamStream_Print(FALSE, STR_PREF ANSI_COLOR_YELLOW 
							"EOF msg type");
			*bytesused = 0;
			return NULL;
		} else if (num_read == -1) {
			errMsg(STR_PREF "read msg type");
			*bytesused = 0;
			return NULL;
		} else if (num_read == sizeof(unsigned int)) {
			//CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN
			//	"read msg_type");
			switch (*msg_type) {
				case CAPTURE_TYPE_FRAME:
					//printf("CAPTURE_TYPE_FRAME\n");
					break;
				case CAPTURE_TYPE_COMMAND:
					//printf("CAPTURE_TYPE_COMMAND\n");
					break;
				case CAPTURE_TYPE_CELLS:
					//printf("CAPTURE_TYPE_CELLS\n");
					break;
				case CAPTURE_TYPE_PARAMS:
					//printf("CAPTURE_TYPE_PARAMS\n");
					break; 
				default:
					CamStream_Print(FALSE, STR_PREF 
						ANSI_COLOR_YELLOW
						"bad type of message");
					*bytesused = 0;
					return NULL;
			}
		} else {
			errno = ENOSYS;
			errMsg(STR_PREF 
				"msg type: num_read < sizeof(unsigned int)");
			*bytesused = 0;
			return NULL;
		}
	} else {
		*msg_type = CAPTURE_TYPE_FRAME;
	}
	//---------------------------------------------------------------------
	num_read = readn(fd, data_buf, (size_t)u_size);
	if (num_read == 0) {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_YELLOW "EOF data");
		*bytesused = 0;
		return NULL;
	} else if (num_read == -1) {
		errMsg(STR_PREF "read data");
		*bytesused = 0;
		return NULL;
	} else if (num_read == u_size) {
		//CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
		//	"data has read");
		*bytesused = u_size;
		return &data_buf[0];
	} else {
		errno = ENOSYS;
		errMsg(STR_PREF "num_read < u_size");
		*bytesused = 0;
		return NULL;
	}
	//---------------------------------------------------------------------
	*bytesused = 0;
	return NULL;
}

//=============================================================================
