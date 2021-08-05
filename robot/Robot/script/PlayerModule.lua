local Player = require "Player"
local PlayerModule = class()

function PlayerModule:ctor()
	
end

function PlayerModule:init(mods)
	self._Net = mods.NetModule
	self._Map = mods.MapModule
	self._Login = mods.LoginModule

	self._player = Player.new(0,0,0)
	self._wave = 0
	self._print = false
	self._storage = {
		[1] = {},
		[2] = {},
	}
end

function PlayerModule:afterInit()
	-- self._Net:AddMsgCallBack(SMCODE.SM_ENTER_MAP,self.OnEnterMap,self,"ServerMsgEnterMap")
	-- self._Net:AddMsgCallBack(SMCODE.SM_SIMPLE_ATTR,self.OnAttrChange,self,"AckSimpleAttr")
	-- self._Net:AddMsgCallBack(SMCODE.SM_ENTITY_DO_SKILL,self.OnEntityDoSkill,self,"AckEntityDoSkill")
	-- self._Net:AddMsgCallBack(SMCODE.SM_PLATE_GEM_INFO,self.OnSkillPlateInfo,self,"ProtoInt32Array")
	-- self._Net:AddMsgCallBack(SMCODE.SM_ENTITY_STATE,self.OnEntityState,self,"SmEntityState")
	-- self._Net:AddMsgCallBack(SMCODE.SM_DAMAGE_INFO,self.OnDamageInfo,self,"AckDamageList")
	-- self._Net:AddMsgCallBack(SMCODE.SM_PASSIVE_DO_SKILL,self.OnBaojiDoSkill,self,"SmPassiveDoSkill")
	-- self._Net:AddMsgCallBack(SMCODE.SM_PLAYER_ATTR,self.OnPlayerAttr,self,"SmPlayerAttr")
	-- self._Net:AddMsgCallBack(SMCODE.SM_STORAGE_INFO,self.OnStorageInfo,self,"StorageInfo")
	-- self._Net:AddMsgCallBack(SMCODE.SM_TALENT_INFO,self.OnTalenInfo,self,"SmTalentInfo")
	-- self._Net:AddMsgCallBack(SMCODE.SM_DROP_INFO,self.OnDropInfo,self,"SmDropInfo")
	-- self._Net:AddMsgCallBack(SMCODE.SM_TASK_INFO,self.OnTaskInfo,self,"SmTaskInfo")
	-- self._Net:AddMsgCallBack(SMCODE.SM_BUFF_INFO,self.OnBuffInfo,self,"SmBuffInfo")
	-- self._Net:AddMsgCallBack(SMCODE.SM_SOUL_INFO,self.OnShowMsgInfo,self,"SmSoulInfo","soul info ----------------")
	-- self._Net:AddMsgCallBack(SMCODE.SM_ATK_SPEED,self.OnShowMsgInfo,self,"SmAtkSpeed","atk info ----------------")
	-- self._Net:AddMsgCallBack(SMCODE.SM_SKILL_ENTITY,self.OnShowSkillInfo,self,"SmSkillEntityList","skill info ----------------")
	
	-----------------
	CMD:AddCmdCall(self,self.DoMove,"move","move x z :move player")
	CMD:AddCmdCall(self,self.DoSkill,"skill","pam: skillid  posx  posz  dirx  dirz   etype  entId : use skill")
	CMD:AddCmdCall(self,self.DoCalcSkill,"cskill","pam: skillid  wave index missId  posx  posz  dirx  dirz   etype  entId... : calc skill")
	CMD:AddCmdCall(self,self.DoStopSkill,"stop","pam: skillid: stop skill")
	CMD:AddCmdCall(self,self.DoSetPlate,"gem","pam: isputon  bagslot  stype  slot: set gem or putdown")
	CMD:AddCmdCall(self,self.DoSetPrint,"p","pam: : set print")
	CMD:AddCmdCall(self,self.DoGm,"gm","pam: cmd -pam1 -pam2 -pam3 -pam4")
	CMD:AddCmdCall(self,self.DoShowInfo,"show","pam: bag")
	CMD:AddCmdCall(self,self.DoUpdateTalent,"talent","pam: ison id...")
	CMD:AddCmdCall(self,self.DoPickItem,"pick","pam: id...")
	CMD:AddCmdCall(self,self.DoRevive,"revive","pam: : 复活")
	CMD:AddCmdCall(self,self.DoTask,"task","pam: -a:接受 id -c:检查 t,gid,pam -s:提交 gid")
	CMD:AddCmdCall(self,self.DoTest,"test","test...")
	CMD:AddCmdCall(self,self.ShowMid,"pmid","show msg id")
	CMD:AddCmdCall(self,self.Relogin,"relogin","reconnect...")
	CMD:AddCmdCall(self,self.SetYaoji,"syj","set yaoji id")
	CMD:AddCmdCall(self,self.UseYaoji,"uyj","use yaoji skill")
	CMD:AddCmdCall(self,self.SetSoulCard,"soul","set soulcard:p1.slot p2.cardId")

end

---------------------- msg ---------------------------------

function PlayerModule:OnEnterMap( data )
	local map = self._Map:ChangeMap(data.map)
	if map == nil then
		print("error map = nil mapId =",data.map)
		return
	end

	self._player:init(data.entity_id,data.pos.x,data.pos.z)

	map:AddEntity(self._player)

	print("Enter Map success ")
	print(persent.block(data))
end

function PlayerModule:OnAttrChange(data)

	print("attr change")
	printTable(data)
end

function PlayerModule:OnEntityDoSkill( data )
	if data.doEntity == self._player:GetEntityId() then
		-- self do skill
	else
		-- other do skill
		if not self._print then
			return
		end
		print("entity ",data.doEntity," use skill ",data.info.skillId)
	end
end

function PlayerModule:OnEnterView( data )
	for i,v in ipairs(data.players) do
		if v.entity_id ~= self._player:GetEntityId() then
			local ply = Player.new(v.entity_id,v.pos.x,v.pos.z)
			self._Map:AddPlayer(ply)
		end
	end
end

function PlayerModule:OnSkillPlateInfo( data )
	print("skill Player --------------- ",getMillisecond())
	printTable(data)
end

function PlayerModule:OnEntityState( data )
	if self._player:GetEntityId() == data.entityId then
		print("player state --------->  ",data.state)
	end
end

local DamageType = {
	[0] = "[普通]",
	[1] = "[闪避]",
	[2] = "[格挡]",
	[3] = "[暴击]",
}

function PlayerModule:OnDamageInfo( data )
	if not self._print then
		return
	end
	print("OnDamageInfo ------ ")
	for i,v in ipairs(data.info) do
		print("entity:",v.entityID,DamageType[v.type],":",v.value)
	end
end

function PlayerModule:OnBaojiDoSkill( data )
	print("baoji do skill ",data.skillId)
	local pam = {
		data.skillId,
		0,0,0,0,0,0,
	}
	self:DoPassiveSkill(pam)
end

local showAttr = {
	ATTR_TYPE.AT_HP,
	ATTR_TYPE.AT_MAXHP,
	ATTR_TYPE.AT_SPEED,
}

function PlayerModule:OnPlayerAttr( data )
	local attr = {}
	local len = #data.types
	for i=1,len do
		attr[data.types[i]] = data.values[i]
	end
	print(" attr ----------- ")
	for i,v in ipairs(showAttr) do
		local cfg = CFG.GetAttr(v)
		print("\t"..cfg.name.." : "..attr[v])
	end
	print(" ")
end

function PlayerModule:OnStorageInfo( data )
	for i,v in ipairs(data.storage_slot) do
		if self._storage[v.storage] then
			self._storage[v.storage][v.slot] = v.data
		end
	end
end

function PlayerModule:OnTalenInfo( data )
	print("ison ",data.ison)
	printTable(data.list)
end

function PlayerModule:OnDropInfo( data )
	print("drop -------------- ")
	local str = "entid:%d itemid:%d count:%d"
	for i,v in ipairs(data.items) do
		print(string.format( str, v.entity_id,v.item_id,v.count ))
	end
end

function PlayerModule:OnTaskInfo( data )
	print("task info ------------")
	printTable(data.task)
end

function PlayerModule:OnBuffInfo( data )
	print("buff -------------------------")
	printTable(data)
end

function PlayerModule:OnShowMsgInfo( data,msg )
	print(msg)
	printTable(data)
end

function PlayerModule:OnShowSkillInfo( data,msg )
	if data.list[1].test > 0 then
		if self.test == nil then
			print("test ------ ")
			self.test = 1
		end
		return
	end
	print(msg)
	printTable(data)
	self.test = nil
end

----------------  CMD ---------------------

local TN = tonumber

function PlayerModule:DoMove(pam)
	if #pam ~= 2 then
		print(" error pams")
		return
	end

	local x = TN(pam[1])
	local z = TN(pam[2])

	self._player:SetPos(x,z)
	
	local pb = {
		pos_vec = {
			{
				x = x,
				z = z,
			}
		}
	}
	self._Net:SendGameData("ClientMsgMove",CMCODE.CM_MOVE,pb)
end

function PlayerModule:DoSkill( pam )
	if #pam < 7 then
		return false
	end

	self._wave = self._wave + 1

	local pb = {
		skillId = TN(pam[1]),
		posx = TN(pam[2]),
		posz = TN(pam[3]),
		dirx = TN(pam[4]),
		dirz = TN(pam[5]),
		entity_id = {
			type = TN(pam[6]),
			entity_id = TN(pam[7]),
		},
		wave = self._wave,
	}
	print("do skill time ",NOW_TICK)
	self._Net:SendGameData("ReqDoSkill",CMCODE.CM_DO_SKILL,pb)
end

function PlayerModule:DoPassiveSkill( pam )
	if #pam < 7 then
		return false
	end

	self._wave = self._wave + 1

	local pb = {
		skillId = TN(pam[1]),
		posx = TN(pam[2]),
		posz = TN(pam[3]),
		dirx = TN(pam[4]),
		dirz = TN(pam[5]),
		entity_id = {
			type = TN(pam[6]),
			entity_id = TN(pam[7]),
		},
		wave = self._wave,
	}
	self._Net:SendGameData("ReqDoSkill",CMCODE.CM_PASSIVE_DO_SKILL,pb)
end

function PlayerModule:DoCalcSkill(pam)
	if #pam < 10 then
		return false
	end
	
	local ents = {}
	for i=10,#pam do
		table.insert( ents, {
			type = TN(pam[9]),
			entity_id = TN(pam[i]),
		})
	end

	local pb = {
		skillId = TN(pam[1]),
		wave = TN(pam[2]),
		index = TN(pam[3]),
		missileId = TN(pam[4]),
		posx = TN(pam[5]),
		posz = TN(pam[6]),
		dirx = TN(pam[7]),
		dirz = TN(pam[8]),
		entitys = ents,
	}
	self._Net:SendGameData("ReqCalcSkill",CMCODE.CM_DO_CALC_SKILL,pb)
end

function PlayerModule:DoStopSkill( pam )
	if #pam ~= 1 then
		return false
	end

	local pb = {
		skillId = TN(pam[1]),
	}
	self._Net:SendGameData("CmStopSkill",CMCODE.CM_STOP_SKILL,pb)
end

local SLOT_LV1_BEGIN = 4
local SLOT_LV2_BEGIN = 12

function PlayerModule:DoSetPlate( pam )
	if #pam ~= 4 then
		return false
	end

	local gemslot = TN(pam[4])
	if TN(pam[3]) == 1 then
		gemslot = gemslot + SLOT_LV1_BEGIN
	elseif TN(pam[3]) == 2 then
		gemslot = gemslot + SLOT_LV2_BEGIN
	end

	local pb = {}

	if TN(pam[1]) > 0 then
		pb.from_type = STORAGE_T.BAG
		pb.from_slot = TN(pam[2])
		pb.to_type = STORAGE_T.GEM
		pb.to_slot = gemslot
	else
		pb.from_type = STORAGE_T.GEM
		pb.from_slot = gemslot
		pb.to_type = STORAGE_T.BAG
		pb.to_slot = TN(pam[2])
	end

	print("gem set time",getMillisecond())
	self._Net:SendGameData("CmStorageMove",CMCODE.CM_STORAGE_MOVE,pb)
end

function PlayerModule:DoSetPrint()
	if self._print == true then
		self._print = false
		print("close print dmg skill use ----")
	else
		self._print = true
		print("open print dmg skill use ----")
	end
end

local GM_TYPE = {
	hot = 0,
	attr = 1,
	lv = 2,
	exp = 3,
	item = 4,		--加道具
	equip = 5,		--加装备
	buff = 6,
	clearbag = 7,
	delbag = 8,
}

function PlayerModule:DoGm( pam )
	local gtype = GM_TYPE[pam[1]]
	if gtype == nil then
		print("error gm")
		return
	end

	local pb = {
		type = gtype,
		pam1 = TN(pam[2] or 0),
		pam2 = TN(pam[3] or 0),
		pam3 = TN(pam[4] or 0),
		pam4 = TN(pam[5] or 0),
	}
	
	self._Net:SendGameData("SmDebugGm",CMCODE.CM_DEBUG_GM,pb)
end

function PlayerModule:DoShowInfo( pam )
	if #pam < 1 then
		return false
	end

	if pam[1] == "bag" then
		printTable(self._storage[STORAGE_T.BAG])
	end
end

function PlayerModule:DoUpdateTalent( pam )
	if #pam < 2 then
		return false
	end

	local ids = {}

	for i=2,#pam do
		table.insert( ids,TN(pam[i]))
	end
	local pb = {
		ison = TN(pam[1]),
		list = {
			list = ids
		}
	}
	self._Net:SendGameData("CmChangeTalent",CMCODE.CM_CHANGE_TALENT,pb)
end

function PlayerModule:DoPickItem( pam )
	if #pam < 1 then
		return false
	end
	local list = {}

	for i,v in ipairs(pam) do
		table.insert( list, TN(v))
	end

	local pb = {
		i32 = list,
	}
	self._Net:SendGameData("ProtoInt32Array",CMCODE.CM_PICK_DROP_ITEM,pb)
end

function PlayerModule:DoRevive( )
	self._Net:SendGameData("ProtoInt32",CMCODE.CM_REVIVE,{i32=0})
end

function PlayerModule:DoTask( pam )
	if #pam < 1 then
		return false
	end

	local cmd = pam[1]
	local pb = {}
	if cmd == "a" then
		if #pam < 2 then
			return false
		end
		pb.i32 = TN(pam[2])
		self._Net:SendGameData("ProtoInt32",CMCODE.CM_ACCEPT_TASK,pb)
	elseif cmd == "c" then
		if #pam < 4 then
			return false
		end
		pb.type = TN(pam[2])
		pb.groupid = TN(pam[3])
		pb.param1 = TN(pam[4])
		if pam[5] then
			pb.param2 = TN(pam[5])
		end
		self._Net:SendGameData("CmCheckTask",CMCODE.CM_CHECK_TASK_CONDITION,pb)
	elseif cmd == "s" then
		if #pam < 2 then
			return false
		end
		pb.groupid = TN(pam[2])
		self._Net:SendGameData("CmSubmitTask",CMCODE.CM_SUBMIT_TASK,pb)
	end
end

function PlayerModule:DoTest( pam )
	local pbmsg = {
		entity_id = 1,
		monster_id = 1,
		pos = {0,0,0},
		dest = {0,0,0},
		maxhp = 5,
		hp = 1,
		atkSpeed = 1.0,
		move_speed = 5,
		quality = 1,
		camp = 1,
		cizhui = {1,1,1,1},
		buffs = {
			{
				buffid = 1,
				time = 1,
				eledmg = 1,
			}
		},
	}

	self._Net:SendGameData("MapMonsterInfo",CMCODE.CM_PLATER_LUA_TEST,pbmsg)
end

function PlayerModule:ShowMid( pam )
	local mid = TN(pam[1])
	if mid == 0 then
		if SHOW_MID == true then
			SHOW_MID = false
		else
			SHOW_MID = true
		end
	else
		print("get msg id ",mid,"=",MID_MAP[mid])
	end
end

function PlayerModule:Relogin( pam )
	self._Login:DoConnectServer()
end

function PlayerModule:SetYaoji( pam )
	local id = TN(pam[1])
	local pbmsg = {
		i32 = id,
	}
	self._Net:SendGameData("ProtoInt32",CMCODE.CM_SET_YAOJI,pbmsg)
end

function PlayerModule:UseYaoji( Pam )
	local pbmsg = {}
	self._Net:SendGameData("ProtoInt32",CMCODE.CM_USE_YAOJI,pbmsg)
end

function PlayerModule:SetSoulCard( pam )
	local pbmsg = {
		data1 = TN(pam[1]),
		data2 = TN(pam[2]),
	}
	self._Net:SendGameData("ProtoPairInt32",CMCODE.CM_SET_SOUL_CARD,pbmsg)
end

------------------------------------------




return PlayerModule