#include "ForwardScene.h"

void ForwardPlayer::init(FactorManager * fm)
{
	m_sock = -1;
	m_scene = NULL;
}

void ForwardPlayer::recycle(FactorManager * fm)
{
	m_scene = NULL;
}

void ForwardPlayer::CollectOpt(const LPMsg::CmdList & cmd)
{
	m_cmdList = cmd;
}

ForwardScene::ForwardScene()
{
}

void ForwardScene::init(FactorManager * fm)
{
	m_sceneId = 0;
	m_frame = 0;
	m_playerIndex = 0;
	m_sendMatch = false;
	m_ableAdd = true;
}

void ForwardScene::recycle(FactorManager * fm)
{
	for (size_t i = 0; i < MAX_PLAYER_INDEX; i++)
	{
		if (m_players[i])
			m_players[i] = NULL;
	}
}

void ForwardScene::AddPlayer(const SHARE<ForwardPlayer>& player)
{
	player->m_objectId = m_playerIndex;
	player->m_sceneId = m_sceneId;
	player->m_scene = this;
	player->m_cmdList.set_objid(m_playerIndex);

	m_players[m_playerIndex] = player;

	++m_playerIndex;
	if (m_playerIndex >= MAX_PLAYER_INDEX)
		m_ableAdd = false;
}
