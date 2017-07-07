// COMPORT.h: interface for the CCOMPORT class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPORT_H__38BD4218_502E_4DB1_A508_D1347CB389B2__INCLUDED_)
#define AFX_COMPORT_H__38BD4218_502E_4DB1_A508_D1347CB389B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define  DELAYTIMES 50
#define  SLEEPTIME 5
int		crc16(unsigned char *pchMsg,unsigned int wDataLen);
void	Delayms(int m_ns);

class CCOMPORT  
{
public:
	void CloseComPort();
	BOOL OpenPort(int portnumber);
	BOOL SearchPort();
	BOOL WriteChar(BYTE* m_szWriteBuffer,DWORD m_nToSend);
	void InitPort();
	CCOMPORT();
	virtual ~CCOMPORT();
	int m_nMinPortNumber;
};

#endif // !defined(AFX_COMPORT_H__38BD4218_502E_4DB1_A508_D1347CB389B2__INCLUDED_)
