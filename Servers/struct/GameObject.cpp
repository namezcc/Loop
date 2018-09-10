#include "GameObject.h"

void GameObject::init(FactorManager * fm)
{
}

void GameObject::recycle(FactorManager * fm)
{
	m_event = NULL;
	m_change.clear();
}
