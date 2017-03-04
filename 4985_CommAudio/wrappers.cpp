/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	wrappers.cpp
--
--	FUNCTIONS:      int startWinsock()
--
--	DATE:			March 3, 2017
--
--	DESIGNERS:		Robert Arendac
--
--	PROGRAMMERS:	Robert Arendac
--
--	NOTES:
--     A group of API calls wrapped up in functions.
---------------------------------------------------------------------------------------*/

#include "wrappers.h"
#include <stdio.h>

/*--------------------------------------------------------------------------------------
-- INTERFACE:     int startWinsock()
--                     VOID
--
-- RETURNS:       0 on failure, 1 on success
--
-- DATE:          March 3, 2017
--
-- DESIGNER:      Robert Arendac
--
-- PROGRAMMER:    Robert Arendac
--
-- NOTES:
-- Starts a winsock session to enable use of the Winsock DLL
---------------------------------------------------------------------------------------*/
int startWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        perror("WSAStartup failed");
        WSACleanup();
        return 0;
    }

    return 1;
}
