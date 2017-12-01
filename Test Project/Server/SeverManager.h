#pragma once
#include "types.h"

namespace NTWK 
{

	class ServerManager
	{
	public:
		ServerManager();
		virtual ~ServerManager();

		bool startUp(const char* configPath);
		void packetHandle();
		void shutDown();

	private:

		bool loadConfig(const char* filePath);
		void createConfig(const char* filePath);

		unsigned short m_port = 0;
		const char* m_IP;
		unsigned int m_maxConnection = 0;
		NTWK::ClientInfo m_connectedClients[];

	};

} //NTWK