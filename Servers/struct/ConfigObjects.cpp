
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
