
//=============================================================================

#ifndef CAM_STREAM_IO_H
#define CAM_STREAM_IO_H

//=============================================================================

int WriteToFd(int fd, unsigned char *ptr, unsigned int size, 
		int c, unsigned int msg_type);

unsigned char* ReadFromFd(int fd, unsigned int *bytesused, 
				unsigned int *msg_type);

ssize_t readn(int fd, void *buf_tmp, size_t count);
ssize_t writen(int fd, void *buffer, size_t count);

//=============================================================================

#endif // CAM_STREAM_IO_H

//=============================================================================