#include "ClientClass.h"

const char* RecvProp::GetName() const
{
	return m_pVarName;
}

int RecvProp::GetOffset() const
{
	return m_Offset;
}

RecvProp* RecvTable::EatShitPlease(int i)
{
	return &m_pProps[i];
}

SendPropType RecvProp::GetType() const
{
	return m_RecvType;
}