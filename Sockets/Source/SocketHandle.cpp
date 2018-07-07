/**
 * Created by TekuConcept on June 15, 2018
 */

#include "SocketHandle.h"

using namespace Impact;

#if defined(__WINDOWS__)
	#include <WinSock2.h>
#else
 	#define INVALID_SOCKET -1
#endif


SocketHandle::SocketHandle() : descriptor(INVALID_SOCKET) {}


SocketHandle::~SocketHandle() {}