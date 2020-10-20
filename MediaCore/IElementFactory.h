#ifndef _I_ELEMENT_FACTORY_H_
#define _I_ELEMENT_FACTORY_H_

class CMediaElement;

class IElementFactory
{
public:
	virtual CMediaElement* CreateElement() = 0;
	virtual CMediaElement* CreateElement(const std::string &name) = 0;
	//virtual const std::string GetElementDescription() = 0;
};

#endif  //_I_ELEMENT_FACTORY_H_