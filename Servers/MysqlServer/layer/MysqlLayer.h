#ifndef MYSQL_LAYER_H
#define MYSQL_LAYER_H
#include "BaseLayer.h"

class MysqlLayer:public BaseLayer
{
public:
	MysqlLayer();
	~MysqlLayer();

private:


	// Í¨¹ý BaseLayer ¼Ì³Ð
	virtual void init() override;

	virtual void loop() override;

	virtual void close() override;

	virtual void GetDefaultTrans(int32_t & ltype, int32_t & lid) override;

};

#endif