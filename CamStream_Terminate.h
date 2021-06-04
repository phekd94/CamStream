
//=============================================================================

/*!
	\defgroup Terminate Terminate video stream
	\brief Terminate video stream
	\version 1.3

	Function that allows you terminate video stream.
	@{

	Usage example (no error checks):
	\code
	CamStream_Terminate_Check(TERMINATE_INIT);
	while (CamStream_Terminate_Check(TERMINATE_SDL2) == FALSE) {
		...
	}
	\endcode
*/

//=============================================================================

#ifndef CAM_STREAM_TERMINATE_H
#define CAM_STREAM_TERMINATE_H

//=============================================================================

#include "CamStream_TLPI.h"

#include <signal.h>
#include <setjmp.h>

//=============================================================================

/**
 *	Enumeration with operation names
 */
enum TermOp {
	TERMINATE_INIT = 0, 		/**< Init module */
	TERMINATE_SIG, 			/**< Check SIGINT */
	TERMINATE_SDL2, 		/**< Check SDL2 */
	TERMINATE_ITERATOR 		/**< Check inerator */
};

//=============================================================================

/*!
	\brief Check state to terminate a work
	\param[in] op Operation number (one of the enum 'TermOp')
	\return TRUE - terminate, FALSE - no terminate
	\warning TERMINATE_INIT also return state of terminate (if error 
	occur in init)
	\note TERMINATE_ITERATOR iterator number specify in 'define NUM_ITER'
*/
Boolean CamStream_Terminate_Check(int op);

//=============================================================================

extern sigjmp_buf *CamStream_Terminate_Env;

//=============================================================================

#endif // CAM_STREAM_TERMINATE_H

//=============================================================================

/*! @} */

//=============================================================================
