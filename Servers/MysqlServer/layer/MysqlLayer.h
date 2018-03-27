#ifndef MYSQL_LAYER_H
#define MYSQL_LAYER_H
#include "BaseLayer.h"

class MysqlLayer:public BaseLayer
{
public:
	MysqlLayer();
	~MysqlLayer();

private:


	// ͨ�� BaseLayer �̳�
	virtual void init() override;

	virtual void loop() override;

	virtual void close() override;

	virtual void GetDefaultTrans(int & ltype, int & lid) override;

};

#endif