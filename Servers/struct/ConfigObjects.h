
#ifndef CONFIG_OBJECTS_H
#define CONFIG_OBJECTS_H

#include "GameObject.h"
#include "Reflection.h"


class TestObject:public GameObject
{
public:
    TestObject();
    virtual void init(FactorManager * fm);

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


#endif

