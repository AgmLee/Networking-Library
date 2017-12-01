#pragma once
#include <MessageIdentifiers.h>

#define NETWORKING_LOCALHOST "127.0.0.1"
#define SERVER_CONFIG_ADDRESS "config.cgf"

namespace NTWK
{
	struct ClientInfo
	{
		char* IP;
		char* Username;
	};

	enum NETWORKING_ID
	{
		ID_SERVER_PING = ID_USER_PACKET_ENUM + 1,
		ID_SERVER_CLIENT_OUTDATED,

		NETWORKING_ID_END
	};
} //Networking