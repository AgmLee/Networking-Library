#include "ServerController.h"
#include "ServerManager.h"
#include <ntwk/ntwkTypes.h>
#include <ntwk/StaticFunctions.h>
#include <BitStream.h>
#include <StringCompressor.h>

ntwk::ServerController * ntwk::ServerController::s_instance = nullptr;

//Public
ntwk::ServerController * ntwk::ServerController::Instance()
{
	if (!s_instance)
		s_instance = new ServerController();
	return s_instance;
}
void ntwk::ServerController::Destroy()
{
	if (s_instance)
	{
		delete s_instance;
		s_instance = nullptr;
	}
}
void ntwk::ServerController::EndProcessing(void)
{
	if (s_instance)
		s_instance->m_processing = false;
}

std::string ntwk::ServerController::checkInput(const std::string & input)
{
	std::vector<std::string> split(0);
	SplitString(input, &split);
	std::string rstr = compareToCommands(split, input);
	return rstr;
}
bool ntwk::ServerController::isProcessing(void)
{
	return m_processing;
}

void ntwk::ServerController::setProcessing(bool bo)
{
	m_processing = bo;
}

//Protected
ntwk::ServerController::ServerController()
{
}
ntwk::ServerController::~ServerController()
{
}

std::string ntwk::ServerController::compareToCommands(const std::vector<std::string>& vector, const std::string& input)
{
	std::string rstr = "\n";
	size_t vectorSize = vector.size();

	//If there is a command, change based on the command.
	if (vectorSize > 0)
	{
		std::string cmd = StringToLower(vector[0]);

		//Shutdown
		if (cmd == "shutdown")
		{
			rstr += "Shutting down the server.\n";
			ntwk::ServerManager::Shutdown(false);
			m_processing = false;
		}
		//Help
		else if (cmd == "help")
		{
			if (vectorSize > 1)
			{
				cmd = StringToLower(vector[1]);
				//Shutdown
				if (cmd == "shutdown")
				{
					rstr += "shutdown: Closes the server and saves the config file.\n";
				}
				//Save config
				else if(cmd == "save")
				{
					rstr += "save: Saves the config file.\n";
				}
				//Message user
				else if (cmd == "whisper")
				{
					rstr += "whisper [username] [message]: Sends the message to the given user only.\n";
				}
				//Message All
				else if(cmd == "say")
				{
					rstr += "say [message]: Sends a message to all users connected to the server.\n";
				}
				//Help
				else if (cmd == "help")
				{
					rstr += "help: Shows a list of commands.\nhelp [command]: Explains what the command does and how to use it.\n";
				}
				//Kick user
				else if (cmd == "kick")
				{
					rstr += "kick [username] [reason]: Kick the given user from the server, reason can be blank.\n";
				}
				//Force command
				else if (cmd == "force")
				{
					rstr += "force [command]: Forces the command to execute, compatible commands include:\nshutdown\n";
				}
				//Ping single
				else if (cmd == "ping")
				{
					rstr += "ping [username]: Simply pings the user, use for debugging or testing connections.\n";
				}
				//Ping all
				else if (cmd == "pingall")
				{
					rstr += "pingall: Simply pings all the connected users, use for debugging or testing connections.\n";
				}
				//Get users
				else if (cmd == "users")
				{
					rstr += "users: Returns a list of connected users.\n";
				}
				//Unknown command
				else rstr +="Unknown Command: " + vector[1] + "\n";
			}
			//List Commands
			else
			{
				rstr += "shutdown\nsave\nhelp\nusers\nwhisper\nsay\nkick\nforce\nping\npingall\n";
			}
		}
		//Save config
		else if (cmd == "save")
		{
			ntwk::ServerManager* instance = ntwk::ServerManager::Instance();
			if (instance) 
			{
				rstr += "Saving.\n";
				instance->saveConfig();
			}
			else
			{
				m_processing = false;
				rstr += "ERROR, instance is null, ending process.\n";
			}
		}
		//List users
		else if (cmd == "users")
		{
			ntwk::ServerManager* instance = ntwk::ServerManager::Instance();
			if (instance)
			{
				//List the users connected to the server
				const ntwk::ClientInfo* clients = instance->getConnectedUsers();
				unsigned int maxCount = instance->getMaxUserCount();
				for (unsigned int i = 0; i < maxCount; i++)
				{
					if (clients[i].username != "")
						rstr += clients[i].username + "\n";
				}
			}
			else
			{
				m_processing = false;
				rstr += "ERROR, instance is null, ending process.\n";
			}
		}
		//Message all users
		else if (cmd == "say")
		{
			if (vectorSize > 1)
			{
				ntwk::ServerManager* instance = ntwk::ServerManager::Instance();
				if (instance)
				{
					/*Packet Structure
					PacketID
					Message*/
					//Get the message
					std::string message = "";
					size_t start = input.find(vector[1]);
					message = input.substr(start);

					//Write to BitStream
					RakNet::BitStream bs;
					bs.Write((RakNet::MessageID)NTWK_ID_SVR_MESSAGE);
					RakNet::RakString input = message.c_str();
					RakNet::StringCompressor::Instance()->EncodeString(&input, instance->getProperties()->bufferSize, &bs);

					uint32_t i = instance->broadcastPacket(bs);
					if (i != 0) rstr += "Server: " + message;
					else rstr += "Unable to send message.\n";
				}
				else
				{
					m_processing = false;
					rstr += "ERROR, instance is null, ending process.\n";
				}
			}
			else rstr += "No Message, include a message to send.\n";
		}		
		//Message user
		else if (cmd == "whisper")
		{
			if (vectorSize > 1)
			{
				if (vectorSize > 2)
				{
					ntwk::ServerManager* instance = ntwk::ServerManager::Instance();
					if (instance)
					{
						/*Packet Structure
						PacketID
						Message*/
						//Get the message
						std::string message = "Whispered to you: ";
						size_t start = input.find(vector[2]);
						message += input.substr(start);

						//Write to BitStream
						RakNet::BitStream bs;
						bs.Write((RakNet::MessageID)NTWK_ID_SVR_MESSAGE);
						RakNet::RakString input = message.c_str();
						RakNet::StringCompressor::Instance()->EncodeString(&input, instance->getProperties()->bufferSize, &bs);

						uint32_t i = instance->sendPacket(bs, vector[1]);

						if (i != 0) rstr += "Server to " + vector[1] + ": " + message + "\n";
						else rstr += "Unable to send message.\n";
					}
					else
					{
						m_processing = false;
						rstr += "ERROR, instance is null, ending process.\n";
					}

				}
				else rstr += "No Message, include a message to send.\n";
			}
			else rstr += "No username, include the username of the person you are trying to message.\n";
		}
		//Kick user
		else if (cmd == "kick")
		{
			if (vectorSize > 1)
			{
				ntwk::ServerManager* instance = ntwk::ServerManager::Instance();
				if (instance)
				{
					unsigned int test = instance->getIndexOfUser(vector[1]);
					if (test == instance->getMaxUserCount()) rstr += vector[1] + " doesn't exist\n";
					else 
					{
						std::string reason = "";
						if (vectorSize > 2)
						{
							size_t start = input.find(vector[2]);
							reason = input.substr(start);
						}
						else reason = "Kicked by host.";

						uint32_t i = instance->closeConnection(vector[1], reason);
						if (i != 0)
						{
							rstr = "Kicked: " + vector[1] + "\n";
							if (instance->getConnectedCount() > 1)
							{
								/*Packet Structure
								PacketID
								DisconnectedClientID
								Message*/
								RakNet::BitStream bs;
								bs.Write((RakNet::MessageID)NTWK_ID_DISCONNECTED_INFO);
								bs.Write((unsigned short)test);

								RakNet::RakString message = vector[1].c_str();
								message += " was ";
								message += reason.c_str();

								RakNet::StringCompressor::Instance()->EncodeString(&message, instance->getProperties()->bufferSize, &bs);
								i = instance->sendPacket(bs, test, true);
								if (i == 0) rstr += "Error sending to other clients.\n";
							}
						}
						else rstr = "Unable to kick: " + vector[1] + "\n";
					}
				}
				else
				{
					m_processing = false;
					rstr += "ERROR, instance is null, ending process.\n";
				}
			}
			else rstr += "No username, include the username of the person you are trying to message.\n";
		}
		//Force
		else if (cmd == "force")
		{
			if (vectorSize > 1)
			{
				cmd = StringToLower(vector[1]);
				//Shutdown
				if (cmd == "shutdown")
				{
					rstr +="Forcing Shutdown.\n";
					ntwk::ServerManager::Shutdown(true, true);
					m_processing = false;
				}
				//Incompatible
				else rstr +="Incompatible: " + vector[1] + ".\n";
			}
			else rstr +="Incomplete Command.\n";
		}
		//Ping all clients
		else if (cmd == "pingall")
		{
			ntwk::ServerManager* instance = ntwk::ServerManager::Instance();
			if (instance)
			{
				instance->ping(RakNet::UNASSIGNED_SYSTEM_ADDRESS);
			}
			else
			{
				m_processing = false;
				rstr += "ERROR, instance is null, ending process.\n";
			}
		}
		//Ping single client
		else if (cmd == "ping")
		{
			if (vectorSize > 1)
			{
				ntwk::ServerManager* instance = ntwk::ServerManager::Instance();
				if (instance)
				{
					instance->ping(vector[1]);
				}
				else
				{
					m_processing = false;
					rstr += "ERROR, instance is null, ending process.\n";
				}
			}
			else rstr += "No username, include the username of the person you are trying to message.\n";
		}
		//Unknown command
		else rstr += "Unknown Command, use \'help\' for a list of commands.\n";
		
		rstr += "\n";
	}
	
	return rstr;
}
