GAME_TYPE = {
    SINGLE = 1,
	MUILT = 2,
	SEND_PACK = 3,
}

SERVER_IP = "127.0.0.1"
SERVER_PORT = 22001

SHOW_MID = true
MID_MAP = {}

ACCOUNT_SID = {
	zcc1 = 6,
	zcc2 = 6,
	zcc4 = 6,
	zilong22 = 6,
}

GAME_ADDR = {
	[0] = {
		ip = "127.0.0.1",
		port = 22001,
		uid = "zcc1",
	},
	[2] = {
		ip = "127.0.0.1",
		port = 18004,
		uid = "zilong22",
	},
	[3] = {
		ip = "192.168.5.189",
		port = 8004,
	},
	[4] = {
		ip = "10.40.2.82",
		port = 8004,
	}
}

ENTITY_TYPE = {
    ET_PLAYER = 1,
}

ATTR_TYPE = {
    AT_HP = 3,					--//当前生命
	AT_SPEED = 5,				--//移动速度
	AT_ATK_SPEED = 9,			--//每秒多少次 1500 1.5次每秒
	AT_MAXHP = 10,				--//生命
	AT_HP_RECOVER = 11,			--//生命恢复
	AT_HP_HIT_RECOVER = 12,		--//击中生命恢复
	AT_HP_KILL_RECOVER = 13,	--//击杀生命恢复
	AT_ATK_MIN = 14,			--//伤害下限
	AT_ATK_MAX = 15,			--//伤害上限
	AT_ELEMENT_ATK_MIN = 16,	--//元素伤害上限
	AT_ELEMENT_ATK_MAX = 17,	--//元素伤害下限
	AT_LEI_ATK_MIN = 18,		--//雷伤害下限
	AT_LEI_ATK_MAX = 19,		--//雷伤害上限
	AT_DU_ATK_MIN = 20,			--//毒伤害下限
	AT_DU_ATK_MAX = 21,			--//毒伤害上限
	AT_HUO_ATK_MIN = 22,		--//火伤害下限
	AT_HUO_ATK_MAX = 23,		--//火伤害上限
	AT_BING_ATK_MIN = 24,		--//冰伤害下限
	AT_BING_ATK_MAX = 25,		--//冰伤害上限
	AT_SHENG_ATK_MIN = 26,		--//冰伤害下限
	AT_SHENG_ATK_MAX = 27,		--//冰伤害上限
	AT_AN_ATK_MIN = 28,			--//暗伤害下限
	AT_AN_ATK_MAX = 29,			--//暗伤害上限
	AT_ELEMENT_DEF = 30,		--//元素抗性
	AT_LEI_DEF = 31,			--//雷抗性
	AT_DU_DEF = 32,				--//毒抗性
	AT_HUO_DEF = 33,			--//火抗性
	AT_BING_DEF = 34,			--//冰抗性
	AT_SHENG_DEF = 35,			--//圣抗性
	AT_AN_DEF = 36,				--//暗抗性
	AT_DEF_VAL = 37,			--//护甲值
	AT_AVOID = 38,				--//闪避值
	AT_MAGIC = 39,				--//魔能值
	AT_BAOJI_VAL = 40,			--//暴击值
	AT_PER_HP = 1001,			--//生命值加成
	AT_PER_FINAL_HP = 1002,		--//最终生命值加成
	AT_PER_HP_RECOVER = 1003,	--//生命恢复加成
	AT_PER_ATK = 1011,			--//攻击加成
	AT_PER_ELEMENT_ATK = 1012,	--//元素攻击加成
	AT_PER_LEI_ATK = 1013,		--//雷元素攻击加成
	AT_PER_DU_ATK = 1014,		--//毒元素攻击加成
	AT_PER_HUO_ATK = 1015,		--//火元素攻击加成
	AT_PER_BING_ATK = 1016,		--//冰元素攻击加成
	AT_PER_SHENG_ATK = 1017,	--//圣元素攻击加成
	AT_PER_AN_ATK = 1018,		--//暗元素攻击加成
	AT_PER_ELEMENT_DEF = 1052,	--//元素抗性加成
	AT_PER_LEI_DEF = 1053,		--//雷抗性加成
	AT_PER_DU_DEF = 1054,		--//毒抗性加成
	AT_PER_HUO_DEF = 1055,		--//火抗性加成
	AT_PER_BING_DEF = 1056,		--//冰抗性加成
	AT_PER_SHENG_DEF = 1057,	--//圣抗性加成
	AT_PER_AN_DEF = 1058,		--//暗抗性加成
	AT_PER_DEF_VAL = 1087,		--//护甲加成
	AT_PER_AVOID = 1088,		--//闪避加成
	AT_PER_MAGIC = 1089,		--//魔能加成
	AT_PER_BAOJI_VAL = 1090,	--//暴击值加成
	AT_PER_BAOJI_DMG = 1091,	--//暴击伤害加成
	AT_PER_ATK_SPEED  = 1104,	--//攻击速度加成
	AT_PER_SPEED = 1107,		--//移动速度加成
}

STORAGE_T = {
    PUTON = 1,
    BAG = 2,
    GEM = 3,
}