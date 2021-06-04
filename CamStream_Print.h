
/*!
	\defgroup print Application print
	\brief Simple print function for the application with the ability 
	to specify errors
	\version 1.3
	\note You can change output stream from stdout to your opened file 
	'FILE *'

	Simple print function for the application with the ability 
	to specify errors.
	@{

	\code
	CamStream_Print(FALSE, ANSI_COLOR_GREEN "operation success");
	CamStream_Print(TRUE, "operation fault");
	CamStream_PrintSetOut(my_out);
	\endcode
*/

//=============================================================================

#ifndef CamStream_Print_H
#define CamStream_Print_H

//=============================================================================

#define ANSI_COLOR_RED     	"\x1b[31m"	///< red color
#define ANSI_COLOR_GREEN   	"\x1b[32m"	///< green color
#define ANSI_COLOR_YELLOW  	"\x1b[33m"	///< yellow color
#define ANSI_COLOR_BLUE    	"\x1b[34m"	///< blue color
#define ANSI_COLOR_MAGENTA 	"\x1b[35m"	///< magenta color
#define ANSI_COLOR_CYAN    	"\x1b[36m"	///< cyan color
#define ANSI_COLOR_RESET   	"\x1b[0m"	///< reset color

/*
#define ANSI_COLOR_RED     	""		///< empty color
#define ANSI_COLOR_GREEN   	""		///< empty color
#define ANSI_COLOR_YELLOW  	""		///< empty color
#define ANSI_COLOR_BLUE    	""		///< empty color
#define ANSI_COLOR_MAGENTA 	""		///< empty color
#define ANSI_COLOR_CYAN    	""		///< empty color
#define ANSI_COLOR_RESET   	""		///< empty color
*/

//=============================================================================
/*!
	\brief Print with some adds. \n
	Out: \n
	err == FALSE: "[app]: " + printf(format, ...) + 
	"ANSI_COLOR_RESET" + "\n" \n
	err == TRUE: "[app]: [error]: " + "ANSI_COLOR_RED" + 
	printf(format, ...) + "ANSI_COLOR_RESET" + "\n"
	\param[in] err Boolean TRUE - for error msg, Boolean FALSE - for 
	simple print
	\param[in] format same printf()
	\param[in] ... same printf()
*/
void CamStream_Print(Boolean err, const char *format, ...);

//=============================================================================
/*!
	\brief Change out stream
	\param[in] new_out your opened file stream
*/
void CamStream_PrintSetOut(FILE *new_out);

//=============================================================================

#endif // CamStream_Print_H

//=============================================================================

/*! @} */
