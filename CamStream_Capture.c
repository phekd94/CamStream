
//=============================================================================

// TODO: v4l2 doc

//=============================================================================

#include "CamStream_TLPI.h"

#include "CamStream_Print.h"
#include "CamStream_Capture.h"
#include "CamStream_Commands.h"
#include "CamStream_IO.h"

//-----------------------------------------------------------------------------

// file
#include <fcntl.h>		// flags for open
#include <sys/stat.h>		// permission for open

//-----------------------------------------------------------------------------

// v4l2
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

//-----------------------------------------------------------------------------

// inet
#include "CamStream_InetSockets.h"

//=============================================================================

#define STR_PREF		"[cap]: "

//=============================================================================

// v4l2
char* 		CamStream_Capture_v4l2_DevName = "/dev/video0";
int 		CamStream_Capture_v4l2_BufCount = 3;
enum CamType	CamStream_Capture_v4l2_CamType = CAM_TYPE_NONE;

//-----------------------------------------------------------------------------

// file
char* 		CamStream_Capture_file_FileName = "input.h264";

//-----------------------------------------------------------------------------

// inet
char* 		CamStream_Capture_inet_Service = "50000";
int 		CamStream_Capture_inet_Backlog = 5;

//=============================================================================

// v4l2
struct buffer {
	void *start;
	size_t length;
};

//=============================================================================

// v4l2
static int				fd_dev		= -1;
static struct v4l2_requestbuffers	req;
static struct buffer			*buffers	= NULL;
static struct v4l2_buffer 		buf;
static enum v4l2_buf_type		type;

//-----------------------------------------------------------------------------

// file
static int				fd_file		= -1;

//-----------------------------------------------------------------------------

// inet
static int				fd_socket	= -1;

// FIXME: another way
// static int				fd_client	= -1;
int		CamStream_Capture_inet_fd_client	= -1;

static struct sockaddr_storage 		claddr;
static socklen_t 			addrlen;
static char				claddr_s[IN_ADDR_STR_LEN];

//=============================================================================
//=============================================================================

// v4l2

/*!
	\defgroup v4l2 Capture a video using v4l2
	 \ingroup Capture
	\brief Capture a video using v4l2
	\version 1.3
	\note Error description output in 'stderr' stream using 
	'ErrorFunctions' module

	A set of functions that allows you to capture a video using v4l2.
	@{
*/

//=============================================================================

static int 
logitech_usb(int params)
{
	unsigned int			i; // unsigned due to comprassion with 
						// unsigned req.count in for()
	struct v4l2_capability		capbl;
	struct v4l2_fmtdesc		fmtdesc;
	struct v4l2_format		fmt;
	//---------------------------------------------------------------------
 // Open capture device
	fd_dev = open(CamStream_Capture_v4l2_DevName, O_RDWR);
	if (fd_dev == -1) {
		errMsg(STR_PREF "open %s", CamStream_Capture_v4l2_DevName);
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
		"open %s", CamStream_Capture_v4l2_DevName);
	//---------------------------------------------------------------------
 // Get capability
	memset(&capbl, 0, sizeof(struct v4l2_capability));

	if (ioctl(fd_dev, VIDIOC_QUERYCAP, &capbl) == -1) {
		errMsg(STR_PREF "ioctl VIDIOC_QUERYCAP");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
		"ioctl VIDIOC_QUERYCAP");

	CamStream_Print(FALSE, STR_PREF "\t\tdriver: %s", capbl.driver);
	CamStream_Print(FALSE, STR_PREF "\t\tcard: %s", capbl.card);
	CamStream_Print(FALSE, STR_PREF "\t\tbus_info: %s", capbl.bus_info);
	CamStream_Print(FALSE, STR_PREF "\t\tversion: %d.%d.%d",
			((capbl.version >> 16) & 0xFF),
			((capbl.version >> 8) & 0xFF),
			(capbl.version & 0xFF));
	CamStream_Print(FALSE, STR_PREF "\t\tcapabilities: 0x%08x", 
			capbl.capabilities);
	CamStream_Print(FALSE, STR_PREF "\t\tdevice_caps: 0x%08x", 
			capbl.device_caps);
	//---------------------------------------------------------------------
 // Enumeration formats
	memset(&fmtdesc, 0, sizeof(struct v4l2_fmtdesc));
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	while (ioctl(fd_dev, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
		if (fmtdesc.index == 0) 
			CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN
				"ioctl VIDIOC_ENUM_FMT");
		CamStream_Print(FALSE, STR_PREF "\t\t----------");
		CamStream_Print(FALSE, STR_PREF "\t\tindex = %d", 
			fmtdesc.index);
		CamStream_Print(FALSE, STR_PREF "\t\ttype = %d", 
			fmtdesc.type);
		CamStream_Print(FALSE, STR_PREF "\t\tdescription = %s", 
			fmtdesc.description);
		CamStream_Print(FALSE, STR_PREF "\t\tpixelformat = 0x%08x", 
			fmtdesc.pixelformat);
		fmtdesc.index++;
	}
	CamStream_Print(FALSE, STR_PREF "\t\t----------");
	if (fmtdesc.index == 0) {
		errMsg(STR_PREF "ioctl VIDIOC_ENUM_FMT");
		return -1;
	}
	//---------------------------------------------------------------------
 // Set format
	if (params == 1) {
		memset (&fmt, 0, sizeof(struct v4l2_format));
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		//fmt.fmt.pix.width = WIDTH;
		//fmt.fmt.pix.height = HEIGHT;
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG; // 0x47504a4d
		//fmt.fmt.pix.field = V4L2_FIELD_ANY;

		if (ioctl(fd_dev, VIDIOC_S_FMT, &fmt) == -1) {
			errMsg(STR_PREF "ioctl VIDIOC_S_FMT");
			return -1;
		}
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
						"VIDIOC_S_FMT");
	}
	//---------------------------------------------------------------------
 // Get format
	memset(&fmt, 0, sizeof(struct v4l2_format));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl(fd_dev, VIDIOC_G_FMT, &fmt) == -1) {
		errMsg(STR_PREF "ioctl VIDIOC_G_FMT");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "VIDIOC_G_FMT");

	CamStream_Print(FALSE, STR_PREF 
		"\t\tGET_FMT values after SET_FMT ioctl");
	CamStream_Print(FALSE, STR_PREF "\t\tfmt.pix.pixelformat = 0x%08x", 
		fmt.fmt.pix.pixelformat);
	CamStream_Print(FALSE, STR_PREF "\t\tfmt.pix.width = %d", 
		fmt.fmt.pix.width);
	CamStream_Print(FALSE, STR_PREF "\t\tfmt.pix.height = %d", 
		fmt.fmt.pix.height);
	CamStream_Print(FALSE, STR_PREF "\t\tfmt.pix.bytesperline = %d", 
		fmt.fmt.pix.bytesperline);
	CamStream_Print(FALSE, STR_PREF "\t\tfmt.pix.sizeimage = %d", 
		fmt.fmt.pix.sizeimage);
	CamStream_Print(FALSE, STR_PREF "\t\tfmt.pix.colorspace = %d", 
		fmt.fmt.pix.colorspace);
	//---------------------------------------------------------------------
 // Require buffers
	memset(&req, 0, sizeof(struct v4l2_requestbuffers));
	req.count = CamStream_Capture_v4l2_BufCount;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (ioctl(fd_dev, VIDIOC_REQBUFS, &req) == -1) {
		errMsg(STR_PREF "ioctl VIDIOC_REQBUFS");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "VIDIOC_REQBUFS");

	CamStream_Print(FALSE, STR_PREF "\t\tcount = %d", req.count);
	//---------------------------------------------------------------------
 // Allocate memory for 'struct buffer'
	buffers = (struct buffer*)calloc(req.count, sizeof(struct buffer));
	if (buffers == NULL) {
		errMsg(STR_PREF "calloc 'struct buffer'");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"calloc %d 'struct buffer'", req.count);

	for (i = 0; i < req.count; ++i) {
		buffers[i].start = NULL;
	}
	//---------------------------------------------------------------------
 // Query buffers
	for (i = 0; i < req.count; ++i) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		
		if (ioctl(fd_dev, VIDIOC_QUERYBUF, &buf) == -1) {
			errMsg(STR_PREF "ioctl VIDIOC_QUERYBUF %d", i);
			return -1;
		}
		
		buffers[i].length = buf.length;
		buffers[i].start = 
			mmap(NULL, buf.length, PROT_READ , // | PROT_WRITE,
				MAP_SHARED, fd_dev, buf.m.offset);
		
		if (buffers[i].start == MAP_FAILED) {
			errMsg(STR_PREF "mmap %d", i);
			buffers[i].start = NULL;
			return -1;
		} 
		
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN
				"VIDIOC_QUERYBUF %d", i);
		
		CamStream_Print(FALSE, STR_PREF 
			"\t\tbuffer:%d phy:%x mmap:%p length:%d", 
			buf.index, buf.m.offset, 
			buffers[i].start, buf.length);
	}
	//---------------------------------------------------------------------
 // QBUF
	for (i = 0; i < req.count; ++i) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		
		if (ioctl(fd_dev, VIDIOC_QBUF, &buf) == -1) {
			errMsg(STR_PREF "ioctl VIDIOC_QBUF %d", i);
			return -1;
		}
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
				"VIDIOC_QBUF %d", i);
	}
	//---------------------------------------------------------------------
	return 0;
}

//-----------------------------------------------------------------------------

static int 
camV(void)
{
	unsigned int			i; // unsigned due to comprassion with 
						// unsigned req.count in for()
	int width = 1920;
	int height = 1080;
	int height_raw = 1620; // height * 1.5
	struct v4l2_capability		capbl;				//  2
	struct v4l2_input 		input;				//  3
	struct v4l2_standard		standard;			//  6
	struct v4l2_cropcap		crop;				//  7
	struct v4l2_fmtdesc		fmtdesc;			//  8
	struct v4l2_format		fmt;				//  9
	struct v4l2_requestbuffers	req;				//  12
	//---------------------------------------------------------------------
 // 1. Open capture device
  // NOTE: some hack
  // NOTE: call mt9p031 driver
	fd_dev = open(CamStream_Capture_v4l2_DevName, O_RDWR);
	if (fd_dev == -1) {
		errMsg(STR_PREF "open %s", CamStream_Capture_v4l2_DevName);
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "open %s", 
					CamStream_Capture_v4l2_DevName);
	//---------------------------------------------------------------------
 // 2. Get capability
	memset(&capbl, 0, sizeof(struct v4l2_capability));

	if (ioctl(fd_dev, VIDIOC_QUERYCAP, &capbl) == -1) {
		errMsg(STR_PREF "ioctl VIDIOC_QUERYCAP");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
						"ioctl VIDIOC_QUERYCAP");

	//CamStream_Print(FALSE, STR_PREF "\t\tdriver: %s", capbl.driver);
	//CamStream_Print(FALSE, STR_PREF "\t\tcard: %s", capbl.card);
	//CamStream_Print(FALSE, STR_PREF "\t\tbus_info: %s", capbl.bus_info);
	//CamStream_Print(FALSE, STR_PREF "\t\tversion: %d.%d.%d",
			//((capbl.version >> 16) & 0xFF),
			//((capbl.version >> 8) & 0xFF),
			//(capbl.version & 0xFF));
	//CamStream_Print(FALSE, STR_PREF "\t\tcapabilities: 0x%08x", 
			//capbl.capabilities);
	//CamStream_Print(FALSE, STR_PREF "\t\tdevice_caps: 0x%08x", 
			//capbl.device_caps);
	//---------------------------------------------------------------------
 // 3. Enumeration input
	memset(&input, 0, sizeof(struct v4l2_input));
	input.type = V4L2_INPUT_TYPE_CAMERA;
	input.index = 0;
  // NOTE: while() instead if()
	if (ioctl(fd_dev, VIDIOC_ENUMINPUT, &input) == 0) {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"ioctl VIDIOC_ENUMINPUT");
		//CamStream_Print(FALSE, STR_PREF 
			// "\t\tindex: %d", input.index);
		//CamStream_Print(FALSE, STR_PREF "\t\tname: %s", input.name);
  	} else {
		errMsg(STR_PREF "ioctl VIDIOC_ENUMINPUT");
		return -1;
	}
	//---------------------------------------------------------------------
 // 4. Set input
  // NOTE: call mt9p031 driver
	if (ioctl(fd_dev, VIDIOC_S_INPUT, &input.index) == -1) {
		errMsg(STR_PREF "ioctl VIDIOC_S_INPUT");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
						"ioctl VIDIOC_S_INPUT");
	//---------------------------------------------------------------------
 // 5. Get input
	if (ioctl(fd_dev, VIDIOC_G_INPUT, &input.index) == -1) {
		errMsg(STR_PREF "ioctl VIDIOC_G_INPUT");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
						"ioctl VIDIOC_G_INPUT");

	//CamStream_Print(FALSE, STR_PREF "\t\tindex: %d", input.index);
	//---------------------------------------------------------------------
 // 6. Enumeration standards
	memset(&standard, 0, sizeof(struct v4l2_standard));
	if (ioctl(fd_dev, VIDIOC_ENUMSTD, &standard) == -1) {
		errMsg(STR_PREF "ioctl VIDIOC_ENUMSTD");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
						"ioctl VIDIOC_ENUMSTD");

  // NOTE: don't app_Print with params bellow. See simple my_encode
	//---------------------------------------------------------------------
 // 7. Select cropping
	memset(&crop, 0, sizeof(struct v4l2_cropcap));
	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(fd_dev, VIDIOC_CROPCAP, &crop) == -1) {
		errMsg(STR_PREF "ioctl VIDIOC_CROPCAP");
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"ioctl VIDIOC_CROPCAP");
	}
	//---------------------------------------------------------------------
 // 8. Enumeration formats
	memset(&fmtdesc, 0, sizeof(struct v4l2_fmtdesc));
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	while (ioctl(fd_dev, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
		if (fmtdesc.index == 0) 
			CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN
				"ioctl VIDIOC_ENUM_FMT");
		CamStream_Print(FALSE, STR_PREF "\t\t----------");
		CamStream_Print(FALSE, STR_PREF "\t\tindex = %d", 
			fmtdesc.index);
		CamStream_Print(FALSE, STR_PREF "\t\ttype = %d", 
			fmtdesc.type);
		CamStream_Print(FALSE, STR_PREF "\t\tdescription = %s", 
			fmtdesc.description);
		CamStream_Print(FALSE, STR_PREF "\t\tpixelformat = 0x%08x", 
			fmtdesc.pixelformat);
		fmtdesc.index++;
	}
	CamStream_Print(FALSE, STR_PREF "\t\t----------");
	if (fmtdesc.index == 0) {
		errMsg(STR_PREF "ioctl VIDIOC_ENUM_FMT");
		return -1;
	}
	//---------------------------------------------------------------------
 // 9. Try format
	//---------------------------------------------------------------------
 // 10. Set format
  // NOTE: call mt9p031 driver
	memset(&fmt, 0, sizeof(struct v4l2_format));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height_raw;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SBGGR8;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;

	if (ioctl(fd_dev, VIDIOC_S_FMT, &fmt) == -1) {
		errMsg(STR_PREF "ioctl VIDIOC_S_FMT");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "VIDIOC_S_FMT");
	//---------------------------------------------------------------------
 // 11. Get format
	memset(&fmt, 0, sizeof(struct v4l2_format));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl(fd_dev, VIDIOC_G_FMT, &fmt) == -1) {
		errMsg(STR_PREF "ioctl VIDIOC_G_FMT");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "VIDIOC_G_FMT");

	CamStream_Print(FALSE, STR_PREF 
			"\t\tGET_FMT values after SET_FMT ioctl");
	CamStream_Print(FALSE, STR_PREF "\t\tfmt.pix.pixelformat = 0x%08x", 
		fmt.fmt.pix.pixelformat);
	CamStream_Print(FALSE, STR_PREF "\t\tfmt.pix.width = %d", 
		fmt.fmt.pix.width);
	CamStream_Print(FALSE, STR_PREF "\t\tfmt.pix.height = %d", 
		fmt.fmt.pix.height);
	CamStream_Print(FALSE, STR_PREF "\t\tfmt.pix.bytesperline = %d", 
		fmt.fmt.pix.bytesperline);
	CamStream_Print(FALSE, STR_PREF "\t\tfmt.pix.sizeimage = %d", 
		fmt.fmt.pix.sizeimage);
	CamStream_Print(FALSE, STR_PREF "\t\tfmt.pix.colorspace = %d", 
		fmt.fmt.pix.colorspace);
	//---------------------------------------------------------------------
 // 12. Require buffers
	memset(&req, 0, sizeof(struct v4l2_requestbuffers));
	req.count = CamStream_Capture_v4l2_BufCount;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (ioctl(fd_dev, VIDIOC_REQBUFS, &req) == -1) {
		errMsg(STR_PREF "ioctl VIDIOC_REQBUFS");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "VIDIOC_REQBUFS");

	CamStream_Print(FALSE, STR_PREF "\t\tcount = %d", req.count);
	//---------------------------------------------------------------------
 // 13. Allocate memory for 'struct buffer'
	buffers = (struct buffer*)calloc(req.count, sizeof(struct buffer));
	if (buffers == NULL) {
		errMsg(STR_PREF "calloc 'struct buffer'");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"calloc %d 'struct buffer'", req.count);

	for (i = 0; i < req.count; ++i)
		buffers[i].start = NULL;
	//---------------------------------------------------------------------
 // 14. Query buffers
	for (i = 0; i < req.count; ++i) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		
		if (ioctl(fd_dev, VIDIOC_QUERYBUF, &buf) == -1) {
			errMsg(STR_PREF "ioctl VIDIOC_QUERYBUF %d", i);
			return -1;
		}
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN
				"VIDIOC_QUERYBUF %d", i);
		
		buffers[i].length = buf.length;
		buffers[i].start = 
			mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
				MAP_SHARED, fd_dev, buf.m.offset);
		
		if (buffers[i].start == MAP_FAILED) {
			errMsg(STR_PREF "mmap %d", i);
			buffers[i].start = NULL;
			return -1;
		} 

		CamStream_Print(FALSE, STR_PREF 
			"\t\tbuffer:%d phy:%x mmap:%p length:%d", 
			buf.index, buf.m.offset, 
			buffers[i].start, buf.length);
	}
	//---------------------------------------------------------------------
 // 15. QBUF
	for (i = 0; i < req.count; ++i) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		
		if (ioctl(fd_dev, VIDIOC_QBUF, &buf) == -1) {
			errMsg(STR_PREF "ioctl VIDIOC_QBUF %d", i);
			return -1;
		}
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
				"VIDIOC_QBUF %d", i);
	}
	//---------------------------------------------------------------------
 // Set U and V to const values
	for (i = 0; i < req.count; ++i) {
		char *framePtr = buffers[i].start;
		memset(&framePtr[width*height], 128, 
			width*height_raw - width*height);
	}
	//---------------------------------------------------------------------
	return 0;
}

//-----------------------------------------------------------------------------

/*!
	\brief Initialization the capture module using v4l2
	\return 0 - success, -1 - error
	\warning Memory for result buffers allocates here
*/
static int 
Init_v4l2(void)
{
	switch (CamStream_Capture_v4l2_CamType) {
	case CAM_TYPE_NONE:
		CamStream_Print(TRUE, STR_PREF 
				"Not the type of camera (CAM_TYPE_NONE)");
		return -1;
	case CAM_TYPE_LOGITECH_USB_YUV:
		return logitech_usb(2);
	case CAM_TYPE_LOGITECH_USB_MJPEG:
		return logitech_usb(1);
	case CAM_TYPE_CAM_V:
		return camV();
	default:
		CamStream_Print(TRUE, STR_PREF "Unknown camera type");
		return -1;
	}
}

//=============================================================================

/*!
	\brief Start the capture module using v4l2
	\return 0 - success, -1 - error
*/
static int 
Start_v4l2(void)
{
 // Start stream
  // NOTE: call mt9p031 driver
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	if (ioctl(fd_dev, VIDIOC_STREAMON, &type) == -1) {
		errMsg(STR_PREF "ioctl VIDIOC_STREAMON");
		return -1;
	}	
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "VIDIOC_STREAMON");
	return 0;
}

//=============================================================================

/*!
	\brief Get pointer to buffer containing frame from input video 
	stream
	\param[out] bytesused Pointer to variable containing bytes used number
	\param[out] msg_type Pointer to message type flag (FRAME, COMMAND or 
	NONE) (here always FRAME)
	\return Pointer to 'unsigned char' - success, NULL - error (also 
	bytesused -> 0)
*/
static unsigned char *
Get_v4l2(unsigned int *bytesused, unsigned int *msg_type)
{
 // DQBUF
	memset(&buf, 0, sizeof(struct v4l2_buffer));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	
	*msg_type = CAPTURE_TYPE_FRAME;
	
	if (ioctl(fd_dev, VIDIOC_DQBUF, &buf) == -1) {
		errMsg(STR_PREF "ioctl VIDIOC_DQBUF%s",
			(errno == EAGAIN) ? " and errno == EAGAIN" : "");
		*bytesused = 0u;
		return NULL;
	}	
	// CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
	//		"VIDIOC_DQBUF %d", buf.index);

	// CamStream_Print(FALSE, STR_PREF "\t\tindex = %d address = 0x%p", 
	//		buf.index, buffers[buf.index].start);
	// CamStream_Print(FALSE, STR_PREF "\t\tbytesused = %d length = %d", 
	//		buf.bytesused, buf.length);

	*bytesused = buf.bytesused;
	
	return (unsigned char*)(buffers[buf.index].start);
}

//=============================================================================

/*!
	\brief Put pointer to buffer containing the frame back into the buffer 
	array
	(pointer got from get frame func is not valid after call the func)
	\return 0 - success, -1 - error
*/
static int 
ThrowFrame_v4l2(void)
{
 // QBUF
	if (ioctl(fd_dev, VIDIOC_QBUF, &buf) == -1) {
		errMsg(STR_PREF "ioctl VIDIOC_QBUF %d", buf.index);
		return -1;
	}
	// CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
	//		"VIDIOC_QBUF %d", buf.index);
	return 0;
}

//=============================================================================

/*!
	\brief Stop the capture module using v4l2
	\return 0 - success, -1 - error
*/
static int 
Stop_v4l2(void)
{
 // Stop stream
  // NOTE: call mt9p031 driver
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	if (ioctl(fd_dev, VIDIOC_STREAMOFF, &type) == -1) {
		errMsg(STR_PREF "ioctl VIDIOC_STREAMOFF");
		return -1;
	}	
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "VIDIOC_STREAMOFF");
	return 0;
}

//=============================================================================

/*!
	\brief Delete the capture module using v4l2
	\return 0 - success, -1 - error
*/
static int 
Delete_v4l2(void)
{
	int ret, er = 0;
	unsigned int i; // unsigned due to comprassion with 
			// unsigned req.count in for()

	if (buffers != NULL) {
		for (i = 0; i < req.count; ++i) {
			if (buffers[i].start != NULL) {
				ret = munmap(buffers[i].start, 
							buffers[i].length);
				buffers[i].start = NULL;
				if (ret == -1) {
					errMsg(STR_PREF "munmap %d", i);
					er = -1;
				} else {
					CamStream_Print(FALSE, 
						STR_PREF ANSI_COLOR_GREEN 
						"munmap %d", i);
				}
			}
		}
		free(buffers);
		buffers = NULL;
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"free 'buffers'");
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"buffers == NULL");
	}

	if (fd_dev != -1) {
  // NOTE: call vpfe-capture: vpfe_release
		ret = close(fd_dev);
		fd_dev = -1;
		if (ret == -1) {
			errMsg(STR_PREF "close %s", 
			       CamStream_Capture_v4l2_DevName);
			er = -1;
		} else {
			CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
					"close %s", 
					CamStream_Capture_v4l2_DevName);
		}
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"fd_dev == -1");
	}
	
	return er;
}

//=============================================================================

/*! @} */

//=============================================================================
//=============================================================================

// file

/*!
	\defgroup file Capture a video from file
	 \ingroup Capture
	\brief Capture a video from file
	\version 1.3
	\note Error description output in 'stderr' stream using 
	'ErrorFunctions' module
	\warning Memory for result buffers allocates here 
	(global static memory)

	A set of functions that allows you to capture a video from file.
	@{
*/

//=============================================================================

/*!
	\brief Initialization capture from file module
	\return 0 - success, -1 - error
*/
static int 
Init_file(void)
{
	// NOTE: see todo in head this file
	// set end of buffer to 0 (this ensures that no overreading happens)
	// memset(file_buf, 0, FILE_BUF_SIZE);
	//---------------------------------------------------------------------
	fd_file = open(CamStream_Capture_file_FileName, O_RDONLY);
	if (fd_file == -1) {
		errMsg(STR_PREF "open %s", CamStream_Capture_file_FileName);
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "open %s", 
			CamStream_Capture_file_FileName);
	//---------------------------------------------------------------------
	return 0;
}

//=============================================================================

/*!
	\brief Delete the capture from file module
	\return 0 - success, -1 - error
*/
static int 
Delete_file(void)
{
	int ret, er = 0;
	if (fd_file != -1) {
		ret = close(fd_file);
		fd_file = -1;
		if (ret == -1) {
			errMsg(STR_PREF "close %s", 
					CamStream_Capture_file_FileName);
			er = -1;
		} else {
			CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
					"close %s", 
					CamStream_Capture_file_FileName
       				);
		}
	}
	else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"fd_file == -1");
	}
	return er;
}

//=============================================================================

/*!
	\brief Start the capture from file module (stub)
	\return 0 - success (always)
*/
static int 
Start_file(void)
{
	return 0;
}

//=============================================================================

/*!
	\brief Stop the capture from file module (stub)
	\return 0 - success (always)
*/
static int 
Stop_file(void)
{
	return 0;
}

//=============================================================================

/*!
	\brief Get pointer to buffer containing frame from file
	\param[out] bytesused Pointer to variable containing bytes used number
	\param[out] msg_type Pointer to message type flag (FRAME, COMMAND or 
	NONE) (here always NONE; ReadFromFd set FRAME)
	\return Pointer to 'unsigned char' - success, NULL - error (also 
	bytesused -> 0)
*/
static unsigned char * 
Get_file(unsigned int *bytesused, unsigned int *msg_type)
{
	*msg_type = CAPTURE_TYPE_NONE;
	return ReadFromFd(fd_file, bytesused, msg_type);
}

//=============================================================================

/*!
	\brief Put pointer to buffer containing the frame back into the buffer 
	array (stub)
	\return 0 - success (always)
*/
static int 
ThrowFrame_file(void)
{
	return 0;
}

//=============================================================================

/*! @} */

//=============================================================================
//=============================================================================

// inet

/*!
	\defgroup inet Capture a video from inet
	 \ingroup Capture
	\brief Capture a video from inet
	\version 1.3
	\note Error description output in 'stderr' stream using 
	'ErrorFunctions' module
	\warning Only one client
	\warning Memory for result buffers allocates here 
	(global static memory)

	A set of functions that allows you to capture a video from inet.
	@{
*/

//=============================================================================

/*!
	\brief Initialization the capture from inet module
	\return 0 - success, -1 - error
*/
static int 
Init_inet(void)
{
	fd_socket = inetListen(CamStream_Capture_inet_Service, 
				CamStream_Capture_inet_Backlog, 
				&addrlen);
	if (fd_socket == -1) {
		errno = ENOSYS;
		errMsg(STR_PREF "inetListen");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "inetListen");
	//---------------------------------------------------------------------
	return 0;
}

//=============================================================================

/*!
	\brief Delete the capture from inet module
	\return 0 - success, -1 - error
*/
static int 
Delete_inet(void)
{
	if (fd_socket != -1) {
		if (close(fd_socket) == -1) {
			fd_socket = -1;
			errMsg(STR_PREF "close server");
			return -1;
		}
		fd_socket = -1;
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"close server");
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"fd_socket == -1");
	}
	//---------------------------------------------------------------------
	return 0;
}

//=============================================================================

/*!
	\brief Start the capture from inet module
	\return 0 - success, -1 - error
*/
static int 
Start_inet(void)
{
	CamStream_Capture_inet_fd_client = accept(
		fd_socket, (struct sockaddr *) &claddr, &addrlen);
	if (CamStream_Capture_inet_fd_client == -1) {
		errMsg(STR_PREF "accept");
		return -1;
	}
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN "accept");

	inetAddressStr((struct sockaddr*)&claddr, addrlen, 
		claddr_s, IN_ADDR_STR_LEN);
	CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"connect client %s", claddr_s);
	//---------------------------------------------------------------------
	return 0;
}

//=============================================================================

/*!
	\brief Stop the capture from inet module
	\return 0 - success, -1 - error
*/
static int 
Stop_inet(void)
{
	if (CamStream_Capture_inet_fd_client != -1) {
		// TODO: maybe close calls signal ???
		if (close(CamStream_Capture_inet_fd_client) == -1) {
			CamStream_Capture_inet_fd_client = -1;
			errMsg(STR_PREF "close client");
			return -1;
		}
		CamStream_Capture_inet_fd_client = -1;
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"close client");
	} else {
		CamStream_Print(FALSE, STR_PREF ANSI_COLOR_GREEN 
			"CamStream_Capture_inet_fd_client == -1");
	}
	//---------------------------------------------------------------------
	return 0;
}

//=============================================================================

/*!
	\brief Get pointer to buffer containing frame from inet
	\param[out] bytesused Pointer to variable containing bytes used number
	\param[out] msg_type Pointer to message type flag (FRAME, COMMAND or 
	NONE) (here always FRAME or COMMAND; ReadFromFd set coresponding value)
	\return Pointer to 'unsigned char' - success, NULL - error (also 
	bytesused -> 0)
	\note Using 'readn' function for read all data from client socket
*/
static unsigned char * 
Get_inet(unsigned int *bytesused, unsigned int *msg_type)
{
	*msg_type = CAPTURE_TYPE_FRAME; // or CAPTURE_TYPE_COMAND;
					// or CAPTURE_TYPE_CELLS;
	return ReadFromFd(
		CamStream_Capture_inet_fd_client, bytesused, msg_type);
}

//=============================================================================

/*!
	\brief Put pointer to buffer containing the frame back into the buffer 
	array (stub)
	\return 0 - success (always)
*/
static int 
ThrowFrame_inet(void)
{
	return 0;
}

//=============================================================================

/*! @} */

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
Start_stub(void)
{
	return 0;
}

static int 
Stop_stub(void)
{
	return 0;
}

static unsigned char * 
Get_stub(unsigned int *bytesused, unsigned int *msg_type)
{
	static unsigned char buf_stub[32];
	*msg_type = CAPTURE_TYPE_FRAME;
	*bytesused = 32u;
	return &buf_stub[0];
}

static int 
ThrowFrame_stub(void)
{
	return 0;
}

//=============================================================================
//=============================================================================
//=============================================================================

struct cap_ops cap_ops_v4l2 = {
	.CamStream_Capture_Init		= Init_v4l2,
	.CamStream_Capture_Delete	= Delete_v4l2,
	.CamStream_Capture_Start	= Start_v4l2,
	.CamStream_Capture_Stop		= Stop_v4l2,
	.CamStream_Capture_Get		= Get_v4l2,
	.CamStream_Capture_ThrowFrame 	= ThrowFrame_v4l2,
};

//-----------------------------------------------------------------------------

struct cap_ops cap_ops_file = {
	.CamStream_Capture_Init 	= Init_file,
	.CamStream_Capture_Delete 	= Delete_file,
	.CamStream_Capture_Start 	= Start_file,
	.CamStream_Capture_Stop 	= Stop_file,
	.CamStream_Capture_Get		= Get_file,
	.CamStream_Capture_ThrowFrame 	= ThrowFrame_file,
};

//-----------------------------------------------------------------------------

struct cap_ops cap_ops_inet = {
	.CamStream_Capture_Init 	= Init_inet,
	.CamStream_Capture_Delete 	= Delete_inet,
	.CamStream_Capture_Start 	= Start_inet,
	.CamStream_Capture_Stop 	= Stop_inet,
	.CamStream_Capture_Get	 	= Get_inet,
	.CamStream_Capture_ThrowFrame 	= ThrowFrame_inet,
};

//-----------------------------------------------------------------------------

struct cap_ops cap_ops_stub = {
	.CamStream_Capture_Init 	= Init_stub,
	.CamStream_Capture_Delete 	= Delete_stub,
	.CamStream_Capture_Start 	= Start_stub,
	.CamStream_Capture_Stop 	= Stop_stub,
	.CamStream_Capture_Get	 	= Get_stub,
	.CamStream_Capture_ThrowFrame 	= ThrowFrame_stub,
};

//=============================================================================
