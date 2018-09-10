
#include "ConfigObjects.h"

TestObject::TestObject()
{
   m_sql = std::shared_ptr<Reflect<TestObject>>(new Reflect<TestObject>(this));
   AddPropery<int32_t>(14);
   AddPropery<int64_t>(1);
}

void TestObject::init(FactorManager * fm)
{
    GameObject::init(fm);
    Set_pro2(0);

    m_sql->flag = 0;

    Clear();
}

void TestObject::Set_pro2(const int64_t& v)
{
    SetValue<TestObject>(1,v);
    m_sql->Set_pro2(v);
}

REFLECT_CPP_DEFINE(TestObject)
TABLE_CPP_DEFINE(TestObject)
constexpr std::array<const char*,2> TestObject::FieldNames;
constexpr std::array<GameObject::PbFunc, 2> TestObject::PbFuncs;

Player::Player()
{
   m_sql = std::shared_ptr<Reflect<Player>>(new Reflect<Player>(this));
   AddPropery<int64_t>(3);
   AddPropery<std::string>(7);
   AddPropery<int32_t>(15);
   AddPropery<int32_t>(3);
   AddPropery<int32_t>(3);
}

void Player::init(FactorManager * fm)
{
    GameObject::init(fm);
    Set_id(0);

    m_sql->flag = 0;

    Clear();
}

void Player::Set_id(const int64_t& v)
{
    SetValue<Player>(0,v);
    m_sql->Set_id(v);
}

void Player::Set_name(const std::string& v)
{
    SetValue<Player>(1,v);
    m_sql->Set_name(v);
}

void Player::Set_level(const int32_t& v)
{
    SetValue<Player>(2,v);
    m_sql->Set_level(v);
}

void Player::Set_exp(const int32_t& v)
{
    SetValue<Player>(3,v);
    m_sql->Set_exp(v);
}

void Player::Set_glod(const int32_t& v)
{
    SetValue<Player>(4,v);
    m_sql->Set_glod(v);
}

REFLECT_CPP_DEFINE(Player)
TABLE_CPP_DEFINE(Player)
constexpr std::array<const char*,5> Player::FieldNames;
constexpr std::array<GameObject::PbFunc, 5> Player::PbFuncs;
