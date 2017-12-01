#include "ClientControl.h"
#include "FlyCamera.h"
#include "OtherPlayer.h"
#include <ntwk/ClientManager.h>
#include <Input.h>
#include <ntwk/ntwkTypes.h>
#include <iostream>
#include <ntwk/StaticFunctions.h>
#include <StringCompressor.h>
#include <imgui.h>

ClientControl* ClientControl::s_instance = nullptr;

ClientControl::ClientControl()
{
	m_messages.resize(m_maxMessageCount);
}

ClientControl::~ClientControl()
{
	m_cam = nullptr;
	m_clientInstance = nullptr;

	if (ntwk::ClientManager::Instance())
	{
		ntwk::ClientManager::Shutdown(true);
	}
}


ClientControl * ClientControl::Instance()
{
	if (!s_instance) s_instance = new ClientControl();
	return s_instance;
}

void ClientControl::Destroy()
{
	if (s_instance) 
	{
		delete s_instance;
		s_instance = nullptr;
	}
}

void ClientControl::initialize(unsigned int max, Camera * cam)
{
	m_maxMessageCount = max;

	if (m_otherPlayers.size() > 0)
	{
		for (unsigned int i = 0; i < m_otherPlayers.size(); i++)
		{
			delete m_otherPlayers[i];
			m_otherPlayers[i] = nullptr;
		}
	}
	m_otherPlayers.clear();
	m_otherPlayers.resize(0);

	m_hasSelf = false;

	m_messages.clear();
	m_messages.resize(m_maxMessageCount);

	m_cam = cam;

	m_clientInstance = ntwk::ClientManager::Instance();
	m_clientInstance->setReadFunction(ClientControl::ReadData);
}

void ClientControl::update(float dt, bool* con)
{
	aie::Input* input = aie::Input::getInstance();
	glm::vec3 direction;

	//Read packet data
	if (m_clientInstance)
	{
		if (m_clientInstance->beginProcess())
		{
			while (m_clientInstance->isProcessing())
			{
				ntwk::PacketReturn pr = m_clientInstance->handleNextPacket();

				if (pr.output != "")
					addMessage(pr.output);
				if (pr.packet)
					m_clientInstance->deallocate(pr.packet);
			}
		}
		else
		{
			m_cam->setPositionAndRotation(glm::vec3(0), 0, 0, 0);
			ntwk::ClientManager::Shutdown(true);
			*con = false;
			return;
		}
	}
	else
	{
		m_cam->setPositionAndRotation(glm::vec3(0), 0, 0, 0);
		*con = false;
		return;
	}
	
	//Spawn / update other players
	{
		const ntwk::ClientInfo* otherClients = m_clientInstance->getConnectedUsers();
		int OCSize = m_clientInstance->connectedUsersSize();
		unsigned int OPSize = m_otherPlayers.size();
		
		if (otherClients)
		{
			if (OCSize < 0) return;
			if (OPSize < (unsigned)OCSize)
			{
				m_startSending = true;
				unsigned int* newIDs = new unsigned int[OCSize];
				int ind = 0;
				for (unsigned int i = 0; i < (unsigned)OCSize; i++)
				{
					if (otherClients[i].username != "")
					{
						bool exists = false;
						for (unsigned int p = 0; p < OPSize; p++)
						{
							if (m_otherPlayers[p]->id == otherClients[i].id)
							{
								exists = true;
								break;
							}
						}
						if (!exists)
						{
							newIDs[ind++] = otherClients[i].id;
						}
					}
				}
				ind--;
				while (ind >= 0)
				{
					m_otherPlayers.push_back(new OtherPlayer());
					float r = (rand() / (float)RAND_MAX);
					float g = (rand() / (float)RAND_MAX);
					float b = (rand() / (float)RAND_MAX);
					m_otherPlayers.back()->initialize(glm::vec4(r, g, b, 1), glm::vec3(0), 4);
					m_otherPlayers.back()->id = newIDs[ind];
					m_otherPlayers.back()->setPos(glm::vec3(0));
					if (m_otherPlayers.back()->id == m_clientInstance->getOwnID() && !m_hasSelf)
					{
						m_otherPlayers.back()->isSelf = true;
						m_hasSelf = true;
					}
					ind--;
				}
				delete newIDs;
				newIDs = nullptr;
			}
			else if (OPSize > (unsigned)OCSize)
			{
				const unsigned int* removed = m_clientInstance->getRemovedUsers();
				int removedSize = m_clientInstance->removedUserSize();
				if (removedSize < 0) return;
				for (unsigned int i = 0; i < (unsigned)removedSize; i++)
				{
					for (unsigned int p = 0; p < OPSize; p++)
					{
						if (m_otherPlayers[p]->id == removed[i])
						{
							m_otherPlayers.erase(m_otherPlayers.begin() + p);
							OPSize = m_otherPlayers.size();
							break;
						}
					}
				}
			}

			ImGui::Begin("Players");
			for (auto& o : m_otherPlayers)
			{
				if (!o->isSelf)
				{
					o->update(dt);
					o->draw();
				}
				for (unsigned int i = 0; i < (unsigned)OCSize; i++)
				{
					if (otherClients[i].id == o->id)
					{
						ImGui::TextColored(ImVec4(o->colour.r, o->colour.g, o->colour.b, o->colour.a), otherClients[i].username.C_String());
						break;
					}
				}
			}
			ImGui::End();
		
		}
	}

	//ChatSystem
	{
		if (ImGui::Begin("Chat"))
		{
			std::string print;
			if (m_messages.size() > 0)
			{
				unsigned int pass = 0;
				for (std::vector<std::string>::iterator it = m_messages.begin(), end = m_messages.end(); 
					(it != end && pass < m_maxMessageCount - 1); ++it, ++pass)
				{
					print += it->c_str();
					print += "\n";
				}
			}
			ImGui::Text(print.c_str());
			
			ImGui::InputText("", m_buffer, c_buffersize);

			static bool once = false;
			if (input->isKeyDown(aie::INPUT_KEY_ENTER))
			{
				if (!once)
				{
					once = true;
					std::string completeString = m_buffer;
					std::vector<std::string> seperated;
					ntwk::SplitString(completeString, &seperated);
					if (seperated.size() > 0 && m_clientInstance)
					{
						if (seperated[0][0] == '/')
						{
							if (ntwk::Compare2String(seperated[0], "/say"))
							{
								if (seperated.size() > 2)
								{
									int i = m_clientInstance->getUserIndex(seperated[1]);
									if (i >= 0)
									{
										std::string message;
										size_t start = completeString.find(seperated[2]);
										if (start != std::string::npos)
										{
											message = "said to you: ";
											message += completeString.substr(start);

											/* Packet structure
											PacketID
											sendToID
											clientID
											dataType
											data*/
											RakNet::BitStream bs;
											bs.Write((RakNet::MessageID)ntwk::NTWK_ID_CNT_DIRECT);
											bs.Write(unsigned short(i));
											bs.Write(unsigned short(m_clientInstance->getOwnID()));
											bs.Write((ntwk::DataType)ntwk::NTWK_DATATYPE_STRING);
											RakNet::RakString input = message.c_str();
											RakNet::StringCompressor::Instance()->EncodeString(&input, m_clientInstance->getProperties()->bufferSize, &bs);

											uint32_t r = m_clientInstance->sendPacket(bs);
											if (r == 0) addMessage("Unable to send message.");
										}
									}
								}
							}
							else addMessage("Unknown command: " + seperated[0]);
						}
						else
						{
							/* Packet structure
							PacketID
							clientID
							data*/
							RakNet::BitStream bs;
							bs.Write((RakNet::MessageID)ntwk::NTWK_ID_CNT_BROADCAST);
							bs.Write((unsigned short)m_clientInstance->getOwnID());
							bs.Write((ntwk::DataType)ntwk::NTWK_DATATYPE_STRING);
							RakNet::RakString input = completeString.c_str();
							RakNet::StringCompressor::Instance()->EncodeString(&input, m_clientInstance->getProperties()->bufferSize, &bs);

							m_clientInstance->sendPacket(bs);
						}
					}

					std::string str = m_clientInstance->getOwnName() + ": ";
					str += completeString;

					addMessage(str);

					m_buffer[0] = '\0';
				}
			}
			else if (once) once = false;
		}
		ImGui::End();
	}

	//Disconnect
	{
		if (ImGui::Begin("Disconnect"))
		{
			if (ImGui::Button("Disconnect"))
			{
				ntwk::ClientManager::Shutdown(false);
			}
		}
		ImGui::End();

	}

	//Get movement data
	{
		glm::vec3 left = m_cam->getRow(0);
		glm::vec3 up = m_cam->getRow(1);
		glm::vec3 forward = m_cam->getRow(2);

		if (input->isKeyDown(aie::INPUT_KEY_W))
		{
			direction -= forward;
		}	
		if (input->isKeyDown(aie::INPUT_KEY_S))
		{
			direction += forward;
		}

		if (input->isKeyDown(aie::INPUT_KEY_A))
		{
			direction -= left;
		}
		if (input->isKeyDown(aie::INPUT_KEY_D))
		{
			direction += left;
		}

		if (input->isKeyDown(aie::INPUT_KEY_E))
		{
			direction += up;
		}
		if (input->isKeyDown(aie::INPUT_KEY_Q))
		{
			direction -= up;
		}

		if ((direction.x + direction.y + direction.z) > 0 ||
			(direction.x + direction.y + direction.z) < 0)
		{
			direction = glm::normalize(direction);
		}
	}
	
	glm::vec4 sendVel(direction, m_cam->getMovementSpeed());
	//Send movement data every 2nd frame
	if (m_pass > 0 && m_lastSentVel != sendVel && m_startSending)
	{
		if (m_clientInstance)
		{
			/*Packet Structure
			PacketID
			ClientID
			dataType
			data*/
			RakNet::BitStream bs;
			bs.Write((RakNet::MessageID)ntwk::NTWK_ID_CNT_BROADCAST);
			bs.Write((unsigned short)m_clientInstance->getOwnID());
			bs.Write((ntwk::DataType)ntwk::NTWK_DATATYPE_VECTOR4);
			bs.Write((char*)&sendVel, sizeof(glm::vec4));
			uint32_t i = m_clientInstance->sendPacket(bs);
			if (i != 0)
			{
				m_pass = 0;
				m_lastSentVel = sendVel;
			}
			else std::cerr << "Unable to send packet: velocity\n";
		}
	}

	//Send positional information occasionally
	glm::vec3 pos = m_cam->getPosition();
	if (m_timer > c_posInterval && m_lastSentPos != pos && m_startSending)
	{
		if (m_clientInstance)
		{
			/*Packet Structure
			PacketID
			ClientID
			dataType
			data*/
			RakNet::BitStream bs;
			bs.Write((RakNet::MessageID)ntwk::NTWK_ID_CNT_BROADCAST);
			bs.Write((unsigned short)m_clientInstance->getOwnID());
			bs.Write((ntwk::DataType)ntwk::NTWK_DATATYPE_VECTOR3);
			bs.Write((char*)&pos, sizeof(glm::vec3));
			uint32_t i = m_clientInstance->sendPacket(bs);
			if (i != 0)
			{
				m_timer = 0;
				m_lastSentPos = pos;
			}
			else std::cerr << "Unable to send packet: position\n";
		}
	}

	m_timer += dt;
	m_pass++;
}

bool ClientControl::ReadData(RakNet::BitStream &bs, ntwk::PacketReturn* pr)
{
	bs.IgnoreBytes(sizeof(RakNet::MessageID));

	unsigned short id;
	ntwk::DataType dataType;

	ntwk::ClientManager* instance = ntwk::ClientManager::Instance();
	ClientControl* client = ClientControl::Instance();

	if (bs.Read(id))
	{
		if (bs.Read(dataType))
		{
			switch (dataType)
			{
			case ntwk::NTWK_DATATYPE_VECTOR4:
			{
				int i = instance->getUserIndex(unsigned int(id));
				if (i < 0) pr->output = "Error at line 361!";
				else 
				{
					glm::vec4 setVel;
					if (bs.Read((char*)&setVel, sizeof(glm::vec4)))
					{
						for (auto& o : client->m_otherPlayers)
						{
							if (o->id == unsigned int(id))
							{
								o->setVel(setVel);
								return true;
							}
						}
						pr->output = "Unknown id!";
					}
					else pr->output = "Error at line 366!";
				}
				break;
			}
			case ntwk::NTWK_DATATYPE_STRING:
			{
				int i = instance->getUserIndex(unsigned int(id));
				if (i < 0) pr->output = "Unknown User at line 384!";
				else 
				{
					RakNet::RakString username = instance->getUsername(i);
					RakNet::RakString msg;
					if (RakNet::StringCompressor::Instance()->DecodeString(&msg, instance->getProperties()->bufferSize, &bs))
					{
						pr->output = username + ": " + msg;
						return true;
					}
					else pr->output = "Read Packet Error at line 390!";
				}
				break;
			}
			case ntwk::NTWK_DATATYPE_VECTOR3:
			{
				int i = instance->getUserIndex(unsigned int(id));
				if (i < 0)	pr->output = "Error at line 404!";
				else 
				{
					glm::vec3 newPos;
					if (bs.Read((char*)&newPos, sizeof(glm::vec3)))
					{
						for (auto& o : client->m_otherPlayers)
						{
							if (o->id == unsigned int(id))
							{
								o->setPos(newPos);
								return true;
							}
						}
						pr->output = "Error at line 411!";
					}
					else pr->output = "Error at line 409!";
				}
				break;
			}
			default:
				pr->output = "Unknown dataType: ";
				pr->output += dataType;
				break;
			}
		}
		else pr->output = "Packet Read Error at line 355!";
	}
	else pr->output = "Packet Read Error at line 353!";
	
	return false;
}

void ClientControl::addMessage(const std::string & str)
{
	m_messages.insert(m_messages.begin(), str);
}
