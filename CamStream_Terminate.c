
//=============================================================================

#ifndef SDL2_DISABLE

#include <SDL2/SDL.h> 

#endif // SDL2_DISABLE

#include "CamStream_Terminate.h"

//=============================================================================

#define STR_PREF  "[ter]: "

//=============================================================================

#define NUM_ITER  1

//=============================================================================

static Boolean  terminate  = FALSE;

sigjmp_buf      env;
sigjmp_buf      *CamStream_Terminate_Env  = &env;

//=============================================================================

void 
sigHandler(int sig __attribute__((unused)))
{
	siglongjmp(env, 1);
}

//-----------------------------------------------------------------------------
/*
void 
sigHandlerExt(
	int sig __attribute__((unused)), 
	siginfo_t *si, 
	void *ucontext __attribute__((unused))
)
{
	
}
*/
//=============================================================================

Boolean 
CamStream_Terminate_Check(int op)
{
	#ifndef SDL2_DISABLE
	static SDL_Event  event;
	#endif // SDL2_DISABLE
	
	static int        i;
	struct sigaction  sa;
	sigset_t          blockMask;

	switch (op)
	{
		case TERMINATE_INIT:
			i = 0;
			
			// Init mask for processor
			sigemptyset(&blockMask);
			sigaddset(&blockMask, SIGINT);
			
			// Set handler for SIGINT
			sigemptyset(&sa.sa_mask);
			sa.sa_flags = 0; // SA_SIGINFO;
			sa.sa_handler = sigHandler;
			// sa.sa_handler = sigHandlerExt;
			sigaction(SIGINT, &sa, NULL);
			break;
		case TERMINATE_SIG:
			// Set mask for processor
			// NOTE: see kerrisk: 20.txt about delivery unblocked 
			// signal (or SUSv4)
			//sigprocmask(SIG_UNBLOCK, &blockMask, NULL);
			//sigprocmask(SIG_BLOCK, &blockMask, NULL);
			// TODO: replace to sigpending()
			break;
		case TERMINATE_SDL2:
			
			#ifndef SDL2_DISABLE
			
			if (SDL_PollEvent(&event) == 1) {
				if (event.window.event == 
						SDL_WINDOWEVENT_CLOSE) {
					terminate = TRUE;
				}
			}
			
			#endif // SDL2_DISABLE
			
			break;
		case TERMINATE_ITERATOR:
			if (i++ == NUM_ITER) {
				terminate = TRUE;
			}
			break;
		default:
			break;
	}

	return terminate;
}

//=============================================================================
