#ifndef TEAM_MOD_H
#define TEAM_MOD_H

#include "BaseModule.h"

COM_MOD_CLASS


struct TeamPlayer
{
	int32_t gameid;
	int64_t pid;
	std::string name;
	int32_t level;
};

struct TeamInfo
{
	int32_t teamId;
	int64_t leader;
	std::map<int64_t, TeamPlayer> m_player;
};

class TeamModule:public BaseModule
{
public:
	TeamModule(BaseLayer*l ):BaseModule(l), m_team_id(0)
	{}
	~TeamModule()
	{}

private:
	// Í¨¹ý BaseModule ¼Ì³Ð
	virtual void Init() override;

	void onCreateTeam(NetMsg* msg);
	void onJoinTeam(NetMsg* msg);
	void onQuitTeam(NetMsg* msg);
	void onDisbandTeam(NetMsg* msg);

	SHARE<TeamInfo> createTeam(int32_t gameid, int64_t pid, const std::string& name, int32_t level);
	SHARE<TeamInfo> getTeam(int32_t teamid);
	void removeTeam(int32_t teamid);
	void broadTeamInfo(SHARE<TeamInfo>& team);
	void broadTeamMember(SHARE<TeamInfo>& team,int32_t mid,BuffBlock* pack);
private:
	COM_MOD_OBJ;

	int32_t m_team_id;
	std::map<int32_t, SHARE<TeamInfo>> m_teams;
	ServerPath m_game_path;
};

#endif // !TEAM_MOD_H

