#include "TeamModule.h"

#include "Common_mod.h"

void TeamModule::Init()
{
	COM_MOD_INIT;


	m_msg_mod->AddMsgCall(N_TEAM_CREATE_TEAM, BIND_NETMSG(onCreateTeam));
	m_msg_mod->AddMsgCall(N_TEAM_JOIN_TEAM, BIND_NETMSG(onJoinTeam));
	m_msg_mod->AddMsgCall(N_TEAM_QUIT_TEAM, BIND_NETMSG(onQuitTeam));
	m_msg_mod->AddMsgCall(N_TEAM_DISBAND_TEAM, BIND_NETMSG(onDisbandTeam));


	m_game_path.push_back(*GetLayer()->GetServer());
	m_game_path.push_back(ServerNode{ LOOP_PROXY_TEAM,0 });
	m_game_path.push_back(ServerNode{LOOP_ROOM,0});
}

void TeamModule::onCreateTeam(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto gameid = pack->readInt32();
	auto pid = pack->readInt64();
	auto name = pack->readString();
	auto level = pack->readInt32();

	auto t = createTeam(gameid, pid, name, level);
	broadTeamInfo(t);
}

void TeamModule::onJoinTeam(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto gameid = pack->readInt32();
	auto teamid = pack->readInt32();
	auto pid = pack->readInt64();
	auto name = pack->readString();
	auto level = pack->readInt32();

	auto t = getTeam(teamid);
	if (t == NULL)
		return;

	auto it = t->m_player.find(pid);
	if (it != t->m_player.end())
		return;

	TeamPlayer p = {};
	p.gameid = gameid;
	p.level = level;
	p.name = name;
	p.pid = pid;

	t->m_player[p.pid] = p;
	broadTeamInfo(t);
}

void TeamModule::onQuitTeam(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto teamid = pack->readInt32();
	auto pid = pack->readInt64();

	auto t = getTeam(teamid);
	if (t == NULL)
		return;

	auto it = t->m_player.find(pid);
	if (it == t->m_player.end())
		return;

	LP_INFO << "quit team pid:" << pid;

	t->m_player.erase(it);
	if (t->m_player.size() > 0)
	{
		if (pid == t->leader)
			t->leader = t->m_player.begin()->second.pid;
		
		broadTeamInfo(t);
	}
	else
	{
		removeTeam(teamid);
	}
}

void TeamModule::onDisbandTeam(NetMsg * msg)
{
	auto pack = msg->m_buff;
	auto teamid = pack->readInt32();
	auto pid = pack->readInt64();

	auto team = getTeam(teamid);
	if (team == NULL || team->leader != pid)
		return;

	auto p = GET_LAYER_MSG(BuffBlock);
	p->writeInt64(0);
	p->writeInt32(teamid);
	broadTeamMember(team, N_TROM_TEAM_DISBAND, p);
	removeTeam(teamid);
}

SHARE<TeamInfo> TeamModule::createTeam(int32_t gameid, int64_t pid, const std::string& name, int32_t level)
{
	++m_team_id;
	auto team = GET_SHARE(TeamInfo);

	team->teamId = m_team_id;
	team->leader = pid;

	TeamPlayer pl = {};
	pl.gameid = gameid;
	pl.pid = pid;
	pl.name = name;
	pl.level = level;

	team->m_player[pid] = pl;

	m_teams[team->teamId] = team;
	return team;
}

SHARE<TeamInfo> TeamModule::getTeam(int32_t teamid)
{
	auto it = m_teams.find(teamid);
	if (it == m_teams.end())
		return NULL;
	return it->second;
}

void TeamModule::removeTeam(int32_t teamid)
{
	m_teams.erase(teamid);
	LP_INFO << "remove team id:" << teamid;
}

void TeamModule::broadTeamInfo(SHARE<TeamInfo>& team)
{
	auto pack = GET_LAYER_MSG(BuffBlock);
	pack->makeRoom(256);

	auto pidoffset = pack->getOffect();

	pack->writeInt64(0);
	pack->writeInt32(m_game_path[0].serid);
	pack->writeInt32(team->teamId);
	pack->writeInt64(team->leader);
	pack->writeInt32((int32_t)team->m_player.size());
	
	for (auto it:team->m_player)
	{
		pack->writeInt32(it.second.gameid);
		pack->writeInt64(it.second.pid);
		pack->writeString(it.second.name);
		pack->writeInt32(it.second.level);
	}

	broadTeamMember(team, N_TROM_TEAM_INFO, pack);
}

void TeamModule::broadTeamMember(SHARE<TeamInfo>& team, int32_t mid, BuffBlock * pack)
{
	auto lastid = team->m_player.rbegin()->second.pid;
	auto endoffset = pack->getOffect();
	for (auto it : team->m_player)
	{
		m_game_path[2].serid = it.second.gameid;
		pack->setOffect(0);
		pack->writeInt64(it.second.pid);
		pack->setOffect(endoffset);

		if (it.second.pid == lastid)
			m_trans_mod->SendToServer(m_game_path, mid, pack);
		else
		{
			auto npack = GET_LAYER_MSG(BuffBlock);
			npack->makeRoom(pack->getOffect());
			npack->writeBuff(pack->m_buff, pack->getOffect());
			m_trans_mod->SendToServer(m_game_path, mid, npack);
		}
	}
}
