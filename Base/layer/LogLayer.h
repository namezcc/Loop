#ifndef LOG_LATER_H
#define LOG_LATER_H
#include "BaseLayer.h"

class LogLayer:public BaseLayer
{
public:
	LogLayer();
	~LogLayer();

private:


	// ͨ�� BaseLayer �̳�
	virtual void init() override;

	virtual void loop() override;

	virtual void close() override;

	virtual void GetDefaultTrans(int & ltype, int & lid) override;

};

#endif