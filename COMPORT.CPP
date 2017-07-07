// COMPORT.cpp: implementation of the CCOMPORT class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NEWFLASHBURN.h"
#include "COMPORT.h"
#include "Includes.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
HANDLE hCom; //串口
extern BOOL m_bReady;
BYTE	m_byteWriteFrame1[24];
BYTE	m_byteWriteFrame2[24];
BYTE	m_byteWriteFrame3[24];
BYTE	m_byteWriteFrame4[24];
UINT ThreadReadCOM(LPVOID lpParam);
UINT ThreadSendCOM(LPVOID lpParam);
CWinThread* pThreadRX;
CWinThread* pThreadTX;
BOOL	m_bThreadRXrunning;
BOOL	m_bThreadTXrunning;
BYTE	m_byteRXbuffer[BUFFERLENTH];
OVERLAPPED m_overlap;
OVERLAPPED m_osRead,m_osWrite; 
BYTE	m_bytecounter;
BOOL	m_bSendPackage;
extern BOOL	m_bWorking;
extern	BOOL	m_bHighSpeed;
extern	BOOL m_bConnection;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCOMPORT::CCOMPORT()
{

}

CCOMPORT::~CCOMPORT()
{
	
}
void CCOMPORT::InitPort()
{
	m_nMinPortNumber=-1;
	//串口初始化,先搜索本机所有串口号,默认打开串口号最小的串口
	if (SearchPort())
	{
		OpenPort(m_nMinPortNumber);
	}
	else
	{
		AfxMessageBox("没有找到串口!");
	}	
}
BOOL CCOMPORT::SearchPort()
{
	BOOL m_BHaveComPort=FALSE;
	//一般认为本机串口号不大于256
	CString m_strport;

	for (int i=1;i<=256;i++)
	{
		m_strport.Format("\\\\.\\COM%d",i);
		hCom=CreateFile(m_strport,//COM口
			GENERIC_READ|GENERIC_WRITE, //允许读和写
			0, //独占方式
			NULL,
			OPEN_EXISTING, //打开而不是创建
			FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, //重叠方式
			NULL);
		//打开成功
		if(hCom!=(HANDLE)-1)
		{
			if (m_BHaveComPort==FALSE)
			{
				m_BHaveComPort=TRUE;
				m_nMinPortNumber=i;
			}
			
			::PostMessage((HWND)AfxGetMainWnd()->m_hWnd, WM_FINDAVACOMPORT, 0, (LPARAM)i);
			CloseHandle(hCom);	//找到后先关闭串口以便下一个串口的查找
		}
	}
	return m_BHaveComPort;
}

BOOL CCOMPORT::OpenPort(int portnumber)
{
	BOOL m_bOpenSucceed=FALSE;
	CString m_strport;
	m_strport.Format("\\\\.\\COM%d",portnumber);
	if (portnumber!=-1)
	{
		hCom=CreateFile(m_strport,//COM口
			GENERIC_READ|GENERIC_WRITE, //允许读和写
			0, //独占方式
			NULL,
			OPEN_EXISTING, //打开而不是创建
			FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, //重叠方式
			NULL);
		//打开成功
		if(hCom==(HANDLE)-1)
		{
			AfxMessageBox("打开指定串口失败!");
			m_bConnection=FALSE;
			return FALSE;
		}
		m_bConnection=TRUE;
		SetupComm(hCom,BUFFERLENTH,BUFFERLENTH); //输入缓冲区和输出缓冲区的大小都是10000
		COMMTIMEOUTS TimeOuts;
		//设定读超时
		TimeOuts.ReadIntervalTimeout=0;
		TimeOuts.ReadTotalTimeoutMultiplier=0;
		TimeOuts.ReadTotalTimeoutConstant=0;
		//在读一次输入缓冲区的内容后读操作就立即返回，
		//而不管是否读入了要求的字符。
		
		//设定写超时
		TimeOuts.WriteTotalTimeoutMultiplier=0;
		TimeOuts.WriteTotalTimeoutConstant=0;
		SetCommTimeouts(hCom,&TimeOuts); //设置超时
		DCB dcb;
		GetCommState(hCom,&dcb);
		dcb.BaudRate=115200; //波特率为115200
		dcb.ByteSize=8; //每个字节有8位
		dcb.Parity=NOPARITY; //无奇偶校验位
		dcb.StopBits=ONESTOPBIT; //一个停止位
		dcb.fDtrControl=0;
		// set DCB to configure the serial port
		SetCommState(hCom,&dcb);
		PurgeComm(hCom,PURGE_TXCLEAR|PURGE_RXCLEAR);

		m_bThreadRXrunning=TRUE;
		m_bThreadTXrunning=TRUE;

		//	接收线程
		pThreadRX=AfxBeginThread(ThreadReadCOM,0);
		pThreadRX->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);
		//  发送线程
		pThreadTX=AfxBeginThread(ThreadSendCOM,0);
		pThreadTX->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);
	}
	return m_bOpenSucceed;
}

void CCOMPORT::CloseComPort()
{
	CloseHandle(hCom);
}
UINT ThreadReadCOM(LPVOID lpParam)
{	
	//中断方式串口收数
	static m_srxcounter=0;
	DWORD nBytesRead,dwEvent,dwError;  
	COMSTAT cs;

	cs.cbInQue=0;
	m_osRead.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);
	SetCommMask(hCom,EV_RXCHAR); 
	while(m_bThreadRXrunning==TRUE)
	{
		if (WaitCommEvent(hCom,&dwEvent,NULL))
		{
			//执行成功返回非零值
			ClearCommError(hCom,&dwError,&cs);
			if (dwEvent&EV_RXCHAR&&cs.cbInQue)
			{
				//读出的数据错误原因可能是缓冲区大小引起的
				if (ReadFile(hCom,m_byteRXbuffer,cs.cbInQue,&nBytesRead,&m_osRead))
				{
					::PostMessage(AfxGetMainWnd()->m_hWnd, WM_RX, cs.cbInQue, 0);
				}
			}
		}
		else
		{
			//执行失败
			int m_nError;
			m_nError=GetLastError();
			PurgeComm(hCom,PURGE_RXCLEAR);
		}
	}
	return 0;
}
BOOL WriteChar(BYTE* m_szWriteBuffer,DWORD m_nToSend)
{
	OVERLAPPED osWrite={0};
	DWORD dwWritten;
	DWORD dwRes;
	BOOL fRes;
	osWrite.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);
	if (osWrite.hEvent==NULL)
	{
		return FALSE;
	}
	if(!WriteFile(hCom,m_szWriteBuffer,m_nToSend,&dwWritten,&osWrite))
	{
		int m_nError=GetLastError();
		if (m_nError!=ERROR_IO_PENDING)
		{
			fRes=FALSE;
		}
		else
		{
			dwRes=WaitForSingleObject(osWrite.hEvent,INFINITE);
			switch (dwRes)
			{
			case WAIT_OBJECT_0:
				if (!GetOverlappedResult(hCom,&osWrite,&dwWritten,FALSE))
				{
					fRes=FALSE;
				} 
				else
				{
					if (dwWritten!=m_nToSend)
					{
						fRes=FALSE;
					} 
					else
					{
						fRes=TRUE;
					}
				}
				break;
			default:
				{
					fRes=FALSE;
					break;
				}
			}
		}
	}
	else
	{
		//写操作完成
		if (dwWritten!=m_nToSend)
		{
			fRes=FALSE;
		} 
		else
		{
			fRes=TRUE;
		}
	}
	//PurgeComm(hCom, PURGE_TXABORT|PURGE_TXCLEAR);
	CloseHandle(osWrite.hEvent);
	return fRes;
}

UINT ThreadSendCOM(LPVOID lpParam)
{
	while(m_bThreadTXrunning==TRUE)
	{
		//发送一包数据
		if (m_bSendPackage==TRUE)
		{
			WORD	m_ncrch16;
			//填入序列号和校验
			//第一帧
			m_byteWriteFrame1[0x04]=m_bytecounter++;
			m_ncrch16=crc16(m_byteWriteFrame1,22);
			m_byteWriteFrame1[0x16]=m_ncrch16&0x00ff;
			m_ncrch16=m_ncrch16&0xff00;
			m_ncrch16=m_ncrch16>>8;
			m_byteWriteFrame1[0x17]=(unsigned char)m_ncrch16;
			WriteChar(m_byteWriteFrame1,24);
			if (m_bHighSpeed==FALSE)
			{
				Sleep(SLEEPTIME);
			} 
			else
			{
				Delayms(DELAYTIMES);
			}
			
			//第二帧
			m_byteWriteFrame2[0x04]=m_bytecounter++;
			m_ncrch16=crc16(m_byteWriteFrame2,22);
			m_byteWriteFrame2[0x16]=m_ncrch16&0x00ff;
			m_ncrch16=m_ncrch16&0xff00;
			m_ncrch16=m_ncrch16>>8;
			m_byteWriteFrame2[0x17]=(unsigned char)m_ncrch16;
			WriteChar(m_byteWriteFrame2,24);
			if (m_bHighSpeed==FALSE)
			{
				Sleep(SLEEPTIME);
			} 
			else
			{
				Delayms(DELAYTIMES);
			}
			//第三帧
			m_byteWriteFrame3[0x04]=m_bytecounter++;
			m_ncrch16=crc16(m_byteWriteFrame3,22);
			m_byteWriteFrame3[0x16]=m_ncrch16&0x00ff;
			m_ncrch16=m_ncrch16&0xff00;
			m_ncrch16=m_ncrch16>>8;
			m_byteWriteFrame3[0x17]=(unsigned char)m_ncrch16;
			WriteChar(m_byteWriteFrame3,24);
			if (m_bHighSpeed==FALSE)
			{
				Sleep(SLEEPTIME);
			} 
			else
			{
				Delayms(DELAYTIMES);
			}
			//第四帧
			m_byteWriteFrame4[0x04]=m_bytecounter++;
			m_ncrch16=crc16(m_byteWriteFrame4,22);
			m_byteWriteFrame4[0x16]=m_ncrch16&0x00ff;
			m_ncrch16=m_ncrch16&0xff00;
			m_ncrch16=m_ncrch16>>8;
			m_byteWriteFrame4[0x17]=(unsigned char)m_ncrch16;
			WriteChar(m_byteWriteFrame4,24);
			if (m_bHighSpeed==FALSE)
			{
				Sleep(SLEEPTIME);
			} 
			else
			{
				Delayms(DELAYTIMES);
			}
			//发送消息给主对话框，显示发送信息
			::PostMessage(AfxGetMainWnd()->m_hWnd,WM_UCOM_WRITE,0,0);
			m_bSendPackage=FALSE;
		}
		if (m_bWorking==FALSE)
		{
			Sleep(SLEEPTIME);
		}
	}
	return 0;
}
int crc16(unsigned char *pchMsg,unsigned int wDataLen)
{
	unsigned  int wCRCTalbeAbs[] =
	{
		0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 
			0x3C00, 0x2800, 0xE401, 0xA001, 0x6C00, 
			0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 
			0x4400
	};
	unsigned  int wCRC = 0xFFFF;
	unsigned  int i = 0;
	unsigned char chChar = 0;
	
	for (i = 0; i < wDataLen; i++)
	{
		chChar = *pchMsg++;
		wCRC = wCRCTalbeAbs[(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
		wCRC = wCRCTalbeAbs[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
	}
	return wCRC;
}
void Delayms(int m_ns)
{
	LARGE_INTEGER litmp;
	LONGLONG QPart1,QPart2;
	double dfMinus, dfFreq, dfTim;
	QueryPerformanceFrequency(&litmp);
	dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率
	QueryPerformanceCounter(&litmp);
	QPart1 = litmp.QuadPart;// 获得初始值
	do
	{
		QueryPerformanceCounter(&litmp);
		QPart2 = litmp.QuadPart;//获得中止值
		dfMinus = (double)(QPart2-QPart1);
		dfTim = dfMinus / dfFreq;// 获得对应的时间值，单位为秒
	}while(dfTim <0.0000173575*m_ns);
}

