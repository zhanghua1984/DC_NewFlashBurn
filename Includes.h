// Includes.h: interface for the Includes class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INCLUDES_H__0A319C65_3AFD_49AD_8C84_09E646AE9D35__INCLUDED_)
#define AFX_INCLUDES_H__0A319C65_3AFD_49AD_8C84_09E646AE9D35__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define WM_FINDAVACOMPORT (WM_USER+1)		//找到一个有效串口号
#define WM_RX	WM_USER+2
#define BUFFERLENTH	10000
#define WM_UCOM_WRITE	(WM_USER+102) 
#define WM_UDISPLAY_MSG (WM_USER+103)
class Includes  
{
public:
	Includes();
	virtual ~Includes();

};

#endif // !defined(AFX_INCLUDES_H__0A319C65_3AFD_49AD_8C84_09E646AE9D35__INCLUDED_)
