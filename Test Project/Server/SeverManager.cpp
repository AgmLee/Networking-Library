#include "SeverManager.h"
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

NTWK::ServerManager::ServerManager()
{
}

NTWK::ServerManager::~ServerManager()
{
}

bool NTWK::ServerManager::startUp(const char * configPath)
{
	return loadConfig(configPath);
}

void NTWK::ServerManager::packetHandle()
{
}

void NTWK::ServerManager::shutDown()
{
}

bool NTWK::ServerManager::loadConfig(const char * filePath)
{
	std::fstream file;
	file.open(filePath, std::ios_base::in);

	if (file.is_open())
	{
		char current = ' ';
		char buff[30];
		while (file.getline(buff, 30))
		{
			std::string line = "";
			std::string lower = "";
			line = lower = buff;
			std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
			std::string substring = line.substr(line.find('=') + 1, line.length());
			if (lower.find("port") != std::string::npos)
			{
				m_port = (unsigned short)std::strtoul(substring.c_str(), NULL, 0);
			}
			else if (lower.find("maxcount") != std::string::npos)
			{
				m_maxConnection = (unsigned int)std::strtoul(substring.c_str(), NULL, 0);
			}
		}
		return true;
	}
	return false;
}

void NTWK::ServerManager::createConfig(const char * filePath)
{
}
