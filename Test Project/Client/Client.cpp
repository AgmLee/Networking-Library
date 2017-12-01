#include "Client.h"
#include "Gizmos.h"
#include "Input.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>

#include "OtherPlayer.h"
#include "FlyCamera.h"
#include "ClientControl.h"

#include <ntwk/ClientManager.h>
#include <ntwk/StaticFunctions.h>
#include <ntwk/ntwkTypes.h>

using glm::vec3;
using glm::vec4;
using glm::mat4;
using aie::Gizmos;

Client::Client() {
}

Client::~Client() {
}

bool Client::startup() {
	
	setBackgroundColour(0.25f, 0.25f, 0.25f);

	// initialise gizmo primitive counts
	Gizmos::create(10000, 10000, 10000, 10000);

	// create simple camera transforms
	m_cam = new FlyCamera();
	m_cam->setLookAt(vec3(10), vec3(0), vec3(0, 1, 0));
	m_cam->setPerspective(glm::pi<float>() * 0.25f, getWindowWidth() / (float)getWindowHeight(), 0.1f, 1000.f);
	
	m_client = ClientControl::Instance();

	return true;
}

static char* buffer = new char[255]{ '\0' };
static char* name = new char[255] { '\0' };
void Client::shutdown() {

	if (buffer)
	{
		delete buffer;
		buffer = nullptr;
	}
	if (name)
	{
		delete name;
		name = nullptr;
	}
	Gizmos::destroy();
	if (m_client)
	{
		ClientControl::Destroy();
		m_client = nullptr;
	}
	if (m_cam)
	{
		delete m_cam;
		m_cam = nullptr;
	}
}

void Client::update(float deltaTime) {

	// query time since application started
	float time = getTime();

	// wipe the gizmos clean for this frame
	Gizmos::clear();

	m_cam->update(deltaTime, m_window);

	static bool searching = false;
	static bool connected = false;
	if (!connected)
	{
		//Manager has started
		if (ntwk::ClientManager::Instance())
		{
			if (ImGui::Begin("Connect to Server"))
			{
				ImGui::InputText("IP Address", buffer, 255);
				if (ImGui::Button("Connect"))
				{
					std::string address = buffer;
					
					//Localhost
					if (address.find("localhost") != std::string::npos)
					{
						//Specified Port
						if (address.find('|') != std::string::npos)
						{
							unsigned short port;
							ntwk::GetPortFromFullAddress(address, &port);
							uint32_t r = ntwk::ClientManager::SetConnection("127.0.0.1", port);
							if (r == RakNet::RAKNET_STARTED)
							{
								m_client->initialize(10, m_cam);
								connected = true;
							}
							else std::cout << "Couldn't Connect.\n";
						}
						//Default Port
						else
						{
							uint32_t r = ntwk::ClientManager::SetConnection("127.0.0.1", 41014);
							if (r == RakNet::RAKNET_STARTED)
							{
								m_client->initialize(10, m_cam);
								connected = true;
							}
							else std::cout << "Couldn't Connect.\n";
						}
					}
					//Specified Address
					else if (address.find('.') != std::string::npos)
					{
						//Specified Port
						if (address.find('|') != std::string::npos)
						{
							uint32_t r = ntwk::ClientManager::SetConnection(address);
							if (r == RakNet::RAKNET_STARTED)
							{
								m_client->initialize(10, m_cam);
								connected = true;
							}
							else std::cout << "Couldn't Connect.\n";
						}
						else
						{
							uint32_t r = ntwk::ClientManager::SetConnection(address, 41014);
							if (r == RakNet::RAKNET_STARTED)
							{
								m_client->initialize(10, m_cam);
								connected = true;
							}
							else std::cout << "Couldn't Connect.\n";
						}
					}
					//Unknown input
					else
					{
						std::cout << "Unknown address.\n";
					}
				}
			}
			ImGui::End();
			if (ImGui::Begin("Server Browser"))
			{
				static std::vector<std::string> found;
				if (!searching)
				{
					if (ImGui::Button("Search"))
					{
						found.clear();
						searching = ntwk::ClientManager::Search(41014);
					}
				}
				else
				{
					ImGui::Text("Searching...");
					ntwk::ClientManager* instance = ntwk::ClientManager::Instance();
					if (instance->beginProcess())
					{
						while (instance->isProcessing())
						{
							ntwk::PacketReturn pr;
							instance->handleNextPacket(&pr);

							if (pr.output != "")
								found.push_back(pr.output);

							static float timer = 0;
							if (timer > 5)
							{
								if (found.size() == 0) found.push_back("No servers found.");
								searching = false;
								timer = 0;
							}
							else timer += deltaTime;

							if (pr.packet)
								instance->deallocate(pr.packet);
						}
					}
				}
				for (std::string& str : found)
				{
					ImGui::Text(str.c_str());
				}
			}
			ImGui::End();
		}
		//Manager hasn't started yet
		else
		{
			if (ImGui::Begin("Welcome!"))
			{
				ImGui::InputText("Username", name, 255);
				if (ImGui::Button("Start"))
				{
					if (name[0] != '\0')
					{
						ntwk::ClientManager::Startup(name);
						ntwk::ClientManager::Instance()->setWritePort(true);
					}
				}
			}
			ImGui::End();
		}
	}
	else
	{
		m_client->update(deltaTime, &connected);
	}

	// quit if we press escape
	aie::Input* input = aie::Input::getInstance();

	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();
}

void Client::draw() {

	// wipe the screen to the background colour
	clearScreen();

	// update perspective in case window resized
	m_cam->setPerspective(glm::pi<float>() * 0.25f,
										  getWindowWidth() / (float)getWindowHeight(),
										  0.1f, 1000.f);

	Gizmos::draw(m_cam->getProjectionView());
}

