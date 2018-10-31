#ifndef FORWARD_SCENE_H
#define FORWARD_SCENE_H

#include "BaseModule.h"

#include "protoPB/client/battle.pb.h"
#include "protoPB/server/sbattle.pb.h"

#define MAX_PLAYER_INDEX	40

class ForwardScene;

struct ForwardPlayer:public LoopObject
{
	int32_t m_sock;
	int64_t m_playerId;
	int32_t m_sceneId;
	int32_t m_proxyId;
	int32_t m_roomSerId;
	int32_t m_objectId;
	ForwardScene* m_scene;

	LPMsg::CmdList	m_cmdList;

	// 通过 LoopObject 继承
	virtual void init(FactorManager * fm) override;
	virtual void recycle(FactorManager * fm) override;

	inline bool OnLine() { return m_sock != -1; }

	void CollectOpt(const LPMsg::CmdList & cmd);
};

class ForwardScene:public LoopObject
{
public:
	ForwardScene();
	~ForwardScene()
	{};

	void AddPlayer(const SHARE<ForwardPlayer>& player);

	inline void SetSceneId(const int32_t& scid) { m_sceneId = scid; }
	inline int32_t GetSceneId() { return m_sceneId; }
	inline bool AbleAddPlayer() { return m_ableAdd; }
	inline SHARE<ForwardPlayer> GetPlayer(const int32_t& index) { return m_players[index]; }
	inline int32_t GetPlayerNum() { return m_playerIndex; }
	inline void AddFrame() { ++m_frame; }
	inline int32_t GetFrame() { return m_frame; }

	// 通过 LoopObject 继承
	virtual void init(FactorManager * fm) override;
	virtual void recycle(FactorManager * fm) override;

private:
	
	int32_t m_sceneId;
	int32_t m_frame;
	int32_t m_playerIndex;
	bool	m_ableAdd;
	SHARE<ForwardPlayer> m_players[MAX_PLAYER_INDEX];

public:
	bool m_sendMatch;
};

#endif
