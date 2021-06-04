
//=============================================================================

/*!
	\defgroup Commands Send/get commands
	\brief Send/get commands
	\version 1.3
	\note Error description output in 'stderr' stream using 
	'ErrorFunctions' module
	\warning Should be function where memory allocates for result buffer 
	for commands

	A set of functions that allows you to send/get commands.
	@{

	Usage example (no error checks):
	\code
	
	\endcode
*/

//=============================================================================

#ifndef CAM_STREAM_COMMANDS_H
#define CAM_STREAM_COMMANDS_H

//=============================================================================

#define TRANSMIT_TYPE_NONE	0	/**< No transmit type */
#define TRANSMIT_TYPE_FRAME	1	/**< Transmit frame */
#define TRANSMIT_TYPE_COMMAND	2	/**< Transmit command */
#define TRANSMIT_TYPE_CELLS	3	/**< Transmit cells */
#define TRANSMIT_TYPE_PARAMS	4	/**< Transmit CamV parameters */

#define CAPTURE_TYPE_NONE	TRANSMIT_TYPE_NONE      /**< No capture type */
#define CAPTURE_TYPE_FRAME	TRANSMIT_TYPE_FRAME     /**< Capture frame */
#define CAPTURE_TYPE_COMMAND	TRANSMIT_TYPE_COMMAND   /**< Capture command */
#define CAPTURE_TYPE_CELLS	TRANSMIT_TYPE_CELLS     /**< Capture cells */
#define CAPTURE_TYPE_PARAMS	TRANSMIT_TYPE_PARAMS	/**< Transmit CamV 
							parameters */


//=============================================================================

/*!
	\brief Send camera parameters
	\param[in] fd Server socket file descriptor
	\return 0 - success, -1 - error
*/
int CamStream_Commands_SendParams(int fd);

//-----------------------------------------------------------------------------

/*!
	\brief Read commands array
	\param[in] fd Server socket file descriptor
	\return 0 - success, -1 - error
	\note Using 'readn' function for read all data from server socket
	\warning Call this function can be program hang cause
*/
int CamStream_Commands_ReadCmd(int fd);

//-----------------------------------------------------------------------------

/*!
	\brief Send cells array
	\param[in] fd Server socket file descriptor
	\return 0 - success, -1 - error
*/
int CamStream_Commands_SendCells(int fd);

//-----------------------------------------------------------------------------
/*!
	\brief Open SD/MMC device
	\return 0 - success, -1 - error
*/
int CamStream_Commands_InitMMC(void);

//-----------------------------------------------------------------------------
/*!
	\brief Close SD/MMC device
	\return 0 - success, -1 - error
*/
int CamStream_Commands_DeleteMMC(void);

//=============================================================================

#endif // CAM_STREAM_COMMANDS_H

//=============================================================================

/*! @} */

//=============================================================================