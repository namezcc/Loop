
#ifndef CONFIG_OBJECTS_H
#define CONFIG_OBJECTS_H

#include "GameObject.h"
#include "Reflection.h"


class TestObject:public GameObject
{
public:
    TestObject();
    virtual void init(FactorManager * fm);

    static constexpr std::array<const char*,2> FieldNames{"pro1","pro2"};
    virtual std::string GetProName(const int32_t& pid)
    {
        return TestObject::FieldNames[pid];
    }

    static constexpr std::array<PbFunc,2> PbFuncs{
        &PropertyToPB::ToInt32,
        &PropertyToPB::ToInt64
    };
    virtual PbFunc GetPbFunc(const int32_t& pid)
    {
        return TestObject::PbFuncs[pid];
    }

    virtual void CopySqlData()
    {
        SetValue<TestObject>(1,pro2);

        Clear();
    }

    void Set_pro1(const int32_t& v){ SetValue<TestObject>(0,v); }
    int32_t Get_pro1(){ return GetValue<int32_t>(0);}

    void Set_pro2(const int64_t& v);
    int64_t Get_pro2(){ return pro2;}

    int64_t pro2;
    std::shared_ptr<Reflect<TestObject>> m_sql;

};
REFLECT(TestObject,pro2)
TABLE_DESC_BEGAN(TestObject,pro2)
FIELD_DESC(pro2,SQL_FIELD_TYPE::SQL_INT,11,false,"",false,"")
TABLE_DESC_END

class Player:public GameObject
{
public:
    Player();
    virtual void init(FactorManager * fm);

    static constexpr std::array<const char*,5> FieldNames{"id","name","level","exp","glod"};
    virtual std::string GetProName(const int32_t& pid)
    {
        return Player::FieldNames[pid];
    }

    static constexpr std::array<PbFunc,5> PbFuncs{
        &PropertyToPB::ToInt64,
        &PropertyToPB::ToString,
        &PropertyToPB::ToInt32,
        &PropertyToPB::ToInt32,
        &PropertyToPB::ToInt32
    };
    virtual PbFunc GetPbFunc(const int32_t& pid)
    {
        return Player::PbFuncs[pid];
    }

    virtual void CopySqlData()
    {
        SetValue<Player>(0,id);
        SetValue<Player>(1,name);
        SetValue<Player>(2,level);
        SetValue<Player>(3,exp);
        SetValue<Player>(4,glod);

        Clear();
    }

    void Set_id(const int64_t& v);
    int64_t Get_id(){ return id;}

    void Set_name(const std::string& v);
    std::string Get_name(){ return name;}

    void Set_level(const int32_t& v);
    int32_t Get_level(){ return level;}

    void Set_exp(const int32_t& v);
    int32_t Get_exp(){ return exp;}

    void Set_glod(const int32_t& v);
    int32_t Get_glod(){ return glod;}

    int64_t id;
    std::string name;
    int32_t level;
    int32_t exp;
    int32_t glod;
    std::shared_ptr<Reflect<Player>> m_sql;

};
REFLECT(Player,id,name,level,exp,glod)
TABLE_DESC_BEGAN(Player,id)
FIELD_DESC(id,SQL_FIELD_TYPE::SQL_BIGINT,20,false,"",false,"'' AUTO_INCREMENT")
FIELD_DESC(name,SQL_FIELD_TYPE::SQL_VARCHAR,32,false,"",true,"'名字'")
FIELD_DESC(level,SQL_FIELD_TYPE::SQL_INT,11,false,"1",false,"'等级'")
FIELD_DESC(exp,SQL_FIELD_TYPE::SQL_INT,11,false,"0",false,"'exp'")
FIELD_DESC(glod,SQL_FIELD_TYPE::SQL_INT,11,false,"0",false,"")
TABLE_DESC_END


#endif

