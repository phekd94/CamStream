
//=============================================================================

/*!
	\defgroup CalcTime Calculate time
	\brief Work with time
	\version 1.3
	\note Error description output in 'stderr' stream using 
	'ErrorFunctions' module

	A set of functions that allows you to put time stamps, calculate the 
	time for passing stamps, set a pause and calculate FPS.
	@{
	
	Usage example (no error checks):
	\code
	struct timespec *t;

	CamStream_CalcTime_SaveStart();
	func();
	CamStream_CalcTime_SaveFinish();

	t = CamStream_CalcTime_GetDelta();
	if (t != NULL)
		printf("T_delta: %ld.%ld\n", t->tv_sec, t->tv_nsec);

	while (CamStream_CalcTime_SetPauseTime() == -4);
	\endcode
	or
	\code
	CamStream_CalcTime_SaveStart();
	func();
	CamStream_CalcTime_SaveFinish();

	printf("fps = %f\n", CamStream_CalcTime_CalcAvFps());
	\endcode
	and test:
	\code
	CamStream_CalcTime_Test();
	\endcode
*/

//=============================================================================

#ifndef CAM_STREAM_CALC_TIME_H
#define CAM_STREAM_CALC_TIME_H

//=============================================================================

#include <time.h>

//=============================================================================

/*! 
	\brief Set start stamp
	\return 0 - success, -1 - error
*/
int CamStream_CalcTime_SaveStart(void);
/*! 
	\brief Set finish stamp
	\return 0 - success, -1 - error
*/
int CamStream_CalcTime_SaveFinish(void);
/*! 
	\brief Calculate a delta between start and finish stamps
	\return Pointer to 'struct timespec' - success, NULL - error
*/
struct timespec *CamStream_CalcTime_GetDelta(void);
/*! 
	\brief Set frame per second
	\param[in] p Frame Per Second (FPS)
	\warning 0 < FPS < 121
*/
void CamStream_CalcTime_SetPauseTime(int p);
/*! 
	\brief Set pause. Stop execution for the time specified by the pause 
	value
	\return 0 - success, -1 - nanosleep error, -2 - GetDelta error, 
	-3 - Delta more then pause time, -4 - errno == EINTR (see nanosleep(2))
*/
int CamStream_CalcTime_SetPause(void);
/*! 
	\brief Get accuracy
	\return Accuracy in nanoseconds - success, -1 - error
*/
long CamStream_CalcTime_GetAccuracy(void);
/*! 
	\brief Get avarage Frames Per Second (FPS)
	\return Avarage FPS in 'double', 0 - success, but should not return 
	value, -1 - error 
	\note Auto-tuning for 1 second
	\warning Don't accurate auto-tuning method 
*/
double CamStream_CalcTime_CalcAvFps(void);
/*! 
	\brief Module test
*/
void CamStream_CalcTime_Test(void);

//=============================================================================

#endif // CAM_STREAM_CALC_TIME_H

//=============================================================================

/*! @} */

//=============================================================================

