#pragma once
#include <BitStream.h>
#include <vector>
#define GLM_SWIZZLE
#include <glm/glm.hpp>

class Camera;
class OtherPlayer;
namespace ntwk
{
	struct PacketReturn;
	class ClientManager;
}

class ClientControl
{
public:
	static ClientControl* Instance();
	static void Destroy();

	void update(float dt, bool* con);
	void initialize(unsigned int max, Camera* cam);
	void addMessage(const std::string&);

	static bool ReadData(RakNet::BitStream&, ntwk::PacketReturn*);

private:
	ClientControl();
	~ClientControl();

	Camera* m_cam = nullptr;
	unsigned int m_maxMessageCount = 10;
	unsigned int m_oldestMessage = 10;
	std::vector<OtherPlayer*> m_otherPlayers;
	std::vector<std::string> m_messages;

	bool m_startSending = false;
	bool m_hasSelf = false;

	glm::vec4 m_lastSentVel;
	glm::vec3 m_lastSentPos;

	ntwk::ClientManager* m_clientInstance = nullptr;
	static ClientControl* s_instance;

	float m_timer = 0;
	const float c_posInterval = 5.0f;
	int m_pass = 0;
	const int c_buffersize = 255;
	char* m_buffer = new char[c_buffersize]
	{
		'\0'
	};
};

