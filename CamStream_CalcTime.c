
//=============================================================================

// TODO: if pause_time pass; late; slow client or server

//=============================================================================

// for CLOCK_MONOTONIC time type
//#define _GNU_SOURCE
#define _POSIX_C_SOURCE		199309L

//=============================================================================

#include "CamStream_TLPI.h"
#include <sys/time.h>
#include <time.h>

//=============================================================================

static struct timespec		T_start;
static struct timespec		T_finish;
static struct timespec		T_delta;
static struct timespec		T_pause;
static struct timespec		T_remain;
static int			clt_errno;
static long			pause_time_ns	= 41666667; // 24 FPS
						// = 40000000; // 25 FPS

//=============================================================================

#define STR_PREF		"[clt]: "

// NOTE: CLOCK_MONOTONIC because should be T_finish.* > T_start.*
#define CLOCK_TYPE		CLOCK_MONOTONIC

//=============================================================================

int 
CamStream_CalcTime_SaveStart(void)
{
	// reset old error state
	clt_errno = 0;

	if (clock_gettime(CLOCK_TYPE, &T_start) == -1) {
		errMsg(STR_PREF "clock_gettime(&T_start)");
		clt_errno = 1;
		return -1;
	}
	return 0;
}

//-----------------------------------------------------------------------------

int 
CamStream_CalcTime_SaveFinish(void)
{
	if (clock_gettime(CLOCK_TYPE, &T_finish) == -1) {
		errMsg(STR_PREF "clock_gettime(&T_finish)");
		clt_errno = 1;
		return -1;
	}
	return 0;
}

//=============================================================================

// NOTE: Assume that always T_finish.* > T_start.*
struct timespec *
CamStream_CalcTime_GetDelta(void)
{
	if (clt_errno == 1) {
		return NULL;
	}
	
	if (T_finish.tv_nsec < T_start.tv_nsec) {
		T_finish.tv_sec -= (time_t)1;
		T_finish.tv_nsec += 1000000000L;
	}
	T_delta.tv_sec = T_finish.tv_sec - T_start.tv_sec;
	T_delta.tv_nsec = T_finish.tv_nsec - T_start.tv_nsec;

	return &T_delta;
}

//=============================================================================

void 
CamStream_CalcTime_SetPauseTime(int p)
{
	if (p > 0 && p < 121) {
		pause_time_ns = 1000000000L / (long)p;
	}
}

//=============================================================================

static int 
nanosleep_(struct timespec *T_p, struct timespec *T_r)
{
	if (nanosleep(T_p, T_r) == -1) {
		if (errno == EINTR) {
			errMsg(STR_PREF "nanosleep is interrupted");
			clt_errno = 4;
			return -4; 	
		}
		errMsg(STR_PREF "nanosleep");
		return -1;
	}
	return 0;
}

//-----------------------------------------------------------------------------

int 
CamStream_CalcTime_SetPause(void)
{
	if (clt_errno == 4) {
		// NOTE: don't accurate
		// NOTE: see nanosleep(2) -> NOTES (absolute time)
		return nanosleep_(&T_remain, &T_remain);
	}

	if (CamStream_CalcTime_GetDelta() == NULL) {
		return -2;
	}
	
	if (T_delta.tv_nsec >= pause_time_ns) {
		return -3;
	}
	
	T_pause.tv_sec = 0;
	T_pause.tv_nsec = pause_time_ns - T_delta.tv_nsec;
	
	return nanosleep_(&T_pause, &T_remain);
}

//=============================================================================

long 
CamStream_CalcTime_GetAccuracy(void)
{
	struct timespec res;
	if (clock_getres(CLOCK_TYPE, &res) == -1) {
		errMsg(STR_PREF "clock_getres");
		return -1;
	}
	return res.tv_nsec;
}

//=============================================================================

double 
CamStream_CalcTime_CalcAvFps(void)
{
	struct timespec *t;
	static long i = 1;
	static double av_old = 0;
	static double av_new = 0;
	static double av_fps = 0;

	// Get diff between start and stop stamps
	t = CamStream_CalcTime_GetDelta();
	if (t != NULL) {
		if (av_old == 0) {
			av_old = 1000000000.0 / t->tv_nsec;
			av_fps = av_old;	
		} else {
			av_old = av_fps;
			av_new = 1000000000.0 / t->tv_nsec;
			av_fps = (av_old + av_new) / 2;
		}
		
		// WARNING: don't accurate auto-tuning method 
		if ((i % (int)av_fps) == 0) {
			i = 1;
			return av_fps;
		}
		++i;
	} else {
		return -1;
	}
	
	return 0;
}

//=============================================================================

// TODO: write the test func
void
CamStream_CalcTime_Test(void)
{

}

//=============================================================================

