// NEWFLASHBURNDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NEWFLASHBURN.h"
#include "NEWFLASHBURNDlg.h"
#include "COMPORT.h"
#include "Includes.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//ȫ�ֱ���
//HANDLE hCom; //����
extern HANDLE hCom; //����
extern CWinThread* pThreadRX;
extern CWinThread* pThreadTX;
extern BOOL	m_bThreadRXrunning;
extern BOOL	m_bThreadTXrunning;
CString m_strgfilepath;
extern BYTE	m_byteWriteFrame1[24];
extern BYTE	m_byteWriteFrame2[24];
extern BYTE	m_byteWriteFrame3[24];
extern BYTE	m_byteWriteFrame4[24];

extern BOOL	m_bSendPackage;

UINT	m_uintBaseAddr;
BOOL	m_bReady;
extern  BYTE	m_byteRXbuffer[BUFFERLENTH];
BYTE	m_byteFrame[24];
UINT	m_unReSendAddr;
int		m_nBinFileLenth;
int		m_nBinFileCur;
BOOL	m_bReSend;
BOOL	m_bRecoder;

BOOL	m_bWorking;

BOOL	m_bHighSpeed;
int		m_ncountersecond;

BOOL m_bConnection;
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
#define MAXQSIZE BUFFERLENTH

/*����ѭ������ʵ��ѭ������*/

typedef BYTE QElemType ; //������������ 

typedef struct
{
	QElemType *base;  //ѭ�����еĴ洢�ռ� �� ������ָ��ָʾ�洢ʵ�ʳ���10�ֽڵ��ַ���) 
	int front;  //��ͷԪ���±꣨ȡԪ�أ� 
	int rear;  //��βԪ���±꣨����Ԫ�أ� 
}SqQueue;            

//����ѭ�����У�ѭ����������
int InitQueue(SqQueue &Q)
{
    Q.base=(QElemType *)malloc(MAXQSIZE*sizeof(QElemType));
    if(!Q.base) 
		return -1;  //����ʧ�ܣ�����-1 
    Q.front=0;
    Q.rear=0;
    return 0;    //�����ɹ�������0 
}    

//����Ԫ�أ�����У�
int EnQueue(SqQueue &Q,QElemType e)
{
    if((Q.rear+1)%MAXQSIZE==Q.front)	//��һ���Ƿ�Ͷ�ȡָ���ػ�
		return -1;  //����
    //memcpy(Q.base[Q.rear],e);  //��ֵ
	Q.base[Q.rear]=e;
    Q.rear=(Q.rear+1)%MAXQSIZE;  //�±��һ 
    return 0;  //��ӳɹ�
}

//ȡԪ��(����ָ�룬ָ��ȡ����ֵ�� 
int DeQueue(SqQueue &Q,QElemType &e)
{
    if(Q.rear==Q.front)
        return -1;   //�ӿ�
    //memcpy(e,Q.base[Q.front]);
	e=Q.base[Q.front];
    Q.front=(Q.front+1)%MAXQSIZE; //�±��һ 
    return 0;  //�ɹ�
}  
int ClearQueue(SqQueue &Q)
{
	Q.front=0;
    Q.rear=0;
	return 0;
}
int QueueLenth(SqQueue Q)
{
	return (Q.rear-Q.front+MAXQSIZE)%MAXQSIZE;
}

SqQueue q;  //���ݽṹ ���λ���������
int flag; 
QElemType strIn,strOut;
 
class CAboutDlg : public CDialog
{

public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNEWFLASHBURNDlg dialog

CNEWFLASHBURNDlg::CNEWFLASHBURNDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNEWFLASHBURNDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNEWFLASHBURNDlg)
	m_strstartaddr = _T("");
	m_strconpath = _T("");
	m_strgeopath = _T("");
	m_strpersent = _T("");
	m_strcountersecond = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNEWFLASHBURNDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNEWFLASHBURNDlg)
	DDX_Control(pDX, IDC_RICHEDITTX, m_CTX);
	DDX_Control(pDX, IDC_RICHEDITRX, m_CRX);
	DDX_Control(pDX, IDC_CLOSEOROPEN, m_CCom);
	DDX_Control(pDX, IDC_RICHEDITSHOWMSG, m_Cricheditshowmsg);
	DDX_Control(pDX, IDC_PROGRESSPERSENT, m_cprogress);
	DDX_Control(pDX, IDC_EDITRXVALUE, m_CRXVALUE);
	DDX_Control(pDX, IDC_EDITRXLENTH, m_CRXLENTH);
	DDX_Control(pDX, IDC_RXCOUNTER, m_CRXn);
	DDX_Control(pDX, IDC_COMBOTARGETBOARD, m_CTargetBoard);
	DDX_Control(pDX, IDC_COMBOCOMPORTNUMBER, m_Ccomportnumber);
	DDX_Text(pDX, IDC_STARTADDR, m_strstartaddr);
	DDX_Text(pDX, IDC_CONFIGPATH, m_strconpath);
	DDX_Text(pDX, IDC_GEOPATH, m_strgeopath);
	DDX_Text(pDX, IDC_STATICPERSENT, m_strpersent);
	DDX_Text(pDX, IDC_EDITCOUNTERSECOND, m_strcountersecond);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNEWFLASHBURNDlg, CDialog)
	//{{AFX_MSG_MAP(CNEWFLASHBURNDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_FINDAVACOMPORT,OnReceiveAComPort)
	ON_MESSAGE(WM_UCOM_WRITE,OnWriteComPortMSG)
	ON_MESSAGE(WM_UDISPLAY_MSG, OnThreadMSG)
	ON_BN_CLICKED(IDC_CLOSEOROPEN, OnCloseoropen)
	ON_BN_CLICKED(IDC_BUTTONSELECTCON, OnButtonselectcon)
	ON_BN_CLICKED(IDC_BUTTONSELECTGEO, OnButtonselectgeo)
	ON_BN_CLICKED(IDC_BUTTONSEND, OnButtonsend)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTONSPEED, OnButtonspeed)
	ON_CBN_SELCHANGE(IDC_COMBOCOMPORTNUMBER, OnSelchangeCombocomportnumber)
	ON_BN_CLICKED(IDC_BUTTONCLEARTX, OnButtoncleartx)
	ON_BN_CLICKED(IDC_BUTTONCLEARRX, OnButtonclearrx)
	ON_BN_CLICKED(IDC_BUTTONCLEARINFO, OnButtonclearinfo)
	ON_BN_CLICKED(IDC_BUTTONCLEARALL, OnButtonclearall)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_RX,OnThreadRXMessage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNEWFLASHBURNDlg message handlers

BOOL CNEWFLASHBURNDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	Initialization();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CNEWFLASHBURNDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CNEWFLASHBURNDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNEWFLASHBURNDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CNEWFLASHBURNDlg::Initialization()
{
	CCOMPORT m_comport;
	m_comport.InitPort();	//���ڳ�ʼ��

	m_CTargetBoard.InsertString(0,"ATP_MPM_A");
	m_CTargetBoard.InsertString(1,"ATP_MPM_B");
	m_CTargetBoard.InsertString(2,"ATP_MPM_C");
	m_CTargetBoard.InsertString(3,"ATP_MPM_D");
	m_CTargetBoard.InsertString(4,"ATO_MPM");
	m_CTargetBoard.SetCurSel(0);
	//������׼��֡
	m_bReady=FALSE;
	m_bWorking=FALSE;	//���ڷ����߳̿�����Ϣ.
	m_bHighSpeed=FALSE;
	m_ncountersecond=0;
	m_bGetFilePath=TRUE;
}
LRESULT CNEWFLASHBURNDlg::OnReceiveAComPort(WPARAM wParam, LPARAM lParam)
{
	static int m_nindex=0;
	CString m_strport;
	m_strport.Format("COM%d",lParam);
	m_Ccomportnumber.InsertString(m_nindex,m_strport);
	m_nindex++;
	m_Ccomportnumber.SetCurSel(0);
	return 0;
}

void CNEWFLASHBURNDlg::OnCloseoropen() 
{
	// TODO: Add your control notification handler code here
	if (m_bConnection==FALSE)
	{
		char m_charsel[10];
		CString m_strsel;
		m_Ccomportnumber.GetWindowText(m_charsel,7);
		m_strsel=m_charsel;
		m_strsel.Delete(0,3);
		CCOMPORT m_comport;
		m_comport.OpenPort(atoi(m_strsel));
		m_bConnection=TRUE;
		m_CCom.SetWindowText("�رմ���");
	} 
	else
	{
		OnSelchangeCombocomportnumber() ;	//�رյ�ǰ���ں��߳�
		m_bConnection=FALSE;
		m_CCom.SetWindowText("�򿪴���");
	}

}

void CNEWFLASHBURNDlg::OnButtonselectcon() 
{
	// TODO: Add your control notification handler code here
	CFileDialog mFileDlg(TRUE, NULL,NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT , "CON�ļ�(*.con)|*.con|All Files (*.*)|*.*|", AfxGetMainWnd());
	
    if(mFileDlg.DoModal()==IDOK)
	{
		m_strstartaddr="0x00000000";
		m_strfilepath=mFileDlg.GetPathName();//������ʾ·������
		CString m_strxie="\\";
		CString m_strxiexie="\\\\";
		m_strfilepath.Replace(m_strxie,m_strxiexie);	//��һ����б���滻Ϊ˫б��
		m_strconpath=m_strfilepath;
		m_strgfilepath=m_strconpath;
		m_strgeopath="";
		UpdateData(FALSE);
		if (m_strfilepath=="")
		{
			AfxMessageBox("�ļ�·��Ϊ�գ���ȡ�ļ�·��ʧ��!");
			return;
		}
		FILE * m_bfilep=fopen(m_strfilepath,"ab+");
		if (m_bfilep==NULL)
		{
			AfxMessageBox("���ļ�ʧ��!");
		}
		long int m_lnfilepos = ftell(m_bfilep);
		fseek(m_bfilep,0,SEEK_END);
		m_lnfilepos = ftell(m_bfilep);
		m_lnfilelenth=m_lnfilepos;
		fseek(m_bfilep,0,SEEK_SET);
		if ((m_lnfilepos%DATALENTH)!=0)
		{
			AfxMessageBox("�ļ���С�����Ϸ����ļ���ʽ,�ļ��ֽ�������Ϊ960��������!");
		}
		fclose(m_bfilep);
	}
	ClearFlags();
}

void CNEWFLASHBURNDlg::OnButtonselectgeo() 
{
	// TODO: Add your control notification handler code here
	CFileDialog mFileDlg(TRUE, NULL,NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT , "GEO�ļ�(*.geo)|*.geo|All Files (*.*)|*.*|", AfxGetMainWnd());
	
    if(mFileDlg.DoModal()==IDOK)
	{
		m_strstartaddr="0x00800000";
		m_strfilepath=mFileDlg.GetPathName();//������ʾ·������
		CString m_strxie="\\";
		CString m_strxiexie="\\\\";
		m_strfilepath.Replace(m_strxie,m_strxiexie);	//��һ����б���滻Ϊ˫б��
		m_strgeopath=m_strfilepath;
		m_strgfilepath=m_strgeopath;
		m_strconpath="";
		UpdateData(FALSE);
		FILE * m_bfilep=fopen(m_strgeopath,"ab+");
		if (m_bfilep==NULL)
		{
			AfxMessageBox("���ļ�ʧ��!");
		}
		long int m_lnfilepos = ftell(m_bfilep);
		fseek(m_bfilep,0,SEEK_END);
		m_lnfilepos = ftell(m_bfilep);
		m_lnfilelenth=m_lnfilepos;
		fseek(m_bfilep,0,SEEK_SET);
		fclose(m_bfilep);
	}
	ClearFlags();
}

void CNEWFLASHBURNDlg::OnButtonsend() 
{
	// TODO: Add your control notification handler code here
	ClearFlags();
	if (m_strstartaddr=="")
	{
		AfxMessageBox("δѡ�����ļ�!");
		return;
	}
	//AllFuctionDisable();
	//m_snsecond=0;

	m_byteWriteFrame1[0x00]=0xFC;
	m_byteWriteFrame2[0x00]=0xFC;
	m_byteWriteFrame3[0x00]=0xFC;
	m_byteWriteFrame4[0x00]=0xFC;

	m_byteWriteFrame1[0x01]=0x0C;
	m_byteWriteFrame2[0x01]=0x0C;
	m_byteWriteFrame3[0x01]=0x0C;
	m_byteWriteFrame4[0x01]=0x0C;

	m_byteWriteFrame1[0x02]=0x02;
	m_byteWriteFrame2[0x02]=0x02;
	m_byteWriteFrame3[0x02]=0x02;
	m_byteWriteFrame4[0x02]=0x02;

	m_byteWriteFrame1[0x05]=0x70;
	m_byteWriteFrame2[0x05]=0xE0;
	m_byteWriteFrame3[0x05]=0xE1;
	m_byteWriteFrame4[0x05]=0xE2;

	//����Ŀ���ַ
	int m_nsel=0;
	m_nsel=m_CTargetBoard.GetCurSel();
	// 		ATP_MPM_A��ַ��0x93;
	//		ATP_MPM_B��ַ��0xA3
	// 		ATP_MPM_C��ַ��0xB3
	// 		ATP_MPM_D��ַ��0xC3
	// 		ATO_MPM��ַ��  0xD3
	switch (m_nsel)
	{
	case 0:
		{
			m_byteWriteFrame1[0x03]=0x93;
			m_byteWriteFrame2[0x03]=0x93;
			m_byteWriteFrame3[0x03]=0x93;
			m_byteWriteFrame4[0x03]=0x93;
			break;
		}
	case 1:
		{
			m_byteWriteFrame1[0x03]=0xa3;
			m_byteWriteFrame2[0x03]=0xa3;
			m_byteWriteFrame3[0x03]=0xa3;
			m_byteWriteFrame4[0x03]=0xa3;
			break;
		}
	case 2:
		{
			m_byteWriteFrame1[0x03]=0xb3;
			m_byteWriteFrame2[0x03]=0xb3;
			m_byteWriteFrame3[0x03]=0xb3;
			m_byteWriteFrame4[0x03]=0xb3;
			break;
		}
	case 3:
		{
			m_byteWriteFrame1[0x03]=0xc3;
			m_byteWriteFrame2[0x03]=0xc3;
			m_byteWriteFrame3[0x03]=0xc3;
			m_byteWriteFrame4[0x03]=0xc3;
			break;
		}
	case 4:
		{
			m_byteWriteFrame1[0x03]=0xd3;
			m_byteWriteFrame2[0x03]=0xd3;
			m_byteWriteFrame3[0x03]=0xd3;
			m_byteWriteFrame4[0x03]=0xd3;
			break;
		}
	}

	//������ʱ����ʼ����׼��֡���ȴ������

	//׼��֡��ֵ
	//70
	m_byteWriteFrame1[0x06]=0xff;
	m_byteWriteFrame1[0x07]=0xff;
	m_byteWriteFrame1[0x08]=0xff;
	m_byteWriteFrame1[0x09]=0xee;

	m_byteWriteFrame2[0x06]=0x88;
	m_byteWriteFrame2[0x07]=0xaa;


	m_byteWriteFrame3[0x06]=0x88;
	m_byteWriteFrame3[0x07]=0xaa;


	m_byteWriteFrame4[0x06]=0x88;
	m_byteWriteFrame4[0x07]=0xaa;


	//��������
	if (m_strstartaddr=="0x00000000")
	{
		m_byteWriteFrame1[0x0a]=0x88;
		m_byteWriteFrame1[0x0b]=0x77;
		m_byteWriteFrame1[0x0c]=0x88;
		m_byteWriteFrame1[0x0d]=0x77;

		m_byteWriteFrame2[0x08]=0x88;
		m_byteWriteFrame2[0x09]=0x77;
		m_byteWriteFrame2[0x0a]=0x88;
		m_byteWriteFrame2[0x0b]=0x77;
		m_byteWriteFrame2[0x0c]=0x88;
		m_byteWriteFrame2[0x0d]=0x77;

		m_byteWriteFrame3[0x08]=0x88;
		m_byteWriteFrame3[0x09]=0x77;
		m_byteWriteFrame3[0x0a]=0x88;
		m_byteWriteFrame3[0x0b]=0x77;
		m_byteWriteFrame3[0x0c]=0x88;
		m_byteWriteFrame3[0x0d]=0x77;

		m_byteWriteFrame4[0x08]=0x88;
		m_byteWriteFrame4[0x09]=0x77;
		m_byteWriteFrame4[0x0a]=0x88;
		m_byteWriteFrame4[0x0b]=0x77;
		m_byteWriteFrame4[0x0c]=0x88;
		m_byteWriteFrame4[0x0d]=0x77;
	} 
	//��������
	else
	{
		m_byteWriteFrame1[0x0a]=0x88;
		m_byteWriteFrame1[0x0b]=0x99;
		m_byteWriteFrame1[0x0c]=0x88;
		m_byteWriteFrame1[0x0d]=0x99;

		m_byteWriteFrame2[0x08]=0x88;
		m_byteWriteFrame2[0x09]=0x99;
		m_byteWriteFrame2[0x0a]=0x88;
		m_byteWriteFrame2[0x0b]=0x99;
		m_byteWriteFrame2[0x0c]=0x88;
		m_byteWriteFrame2[0x0d]=0x99;

		m_byteWriteFrame3[0x08]=0x88;
		m_byteWriteFrame3[0x09]=0x99;
		m_byteWriteFrame3[0x0a]=0x88;
		m_byteWriteFrame3[0x0b]=0x99;
		m_byteWriteFrame3[0x0c]=0x88;
		m_byteWriteFrame3[0x0d]=0x99;

		m_byteWriteFrame4[0x08]=0x88;
		m_byteWriteFrame4[0x09]=0x99;
		m_byteWriteFrame4[0x0a]=0x88;
		m_byteWriteFrame4[0x0b]=0x99;
		m_byteWriteFrame4[0x0c]=0x88;
		m_byteWriteFrame4[0x0d]=0x99;
	}
	m_uintBaseAddr=strtoul(m_strstartaddr,NULL,16);
	SetTimer(1,50,NULL);
	m_bWorking=TRUE;
	m_ncountersecond=0;
	m_strcountersecond.Format("%d ��",m_ncountersecond);
	UpdateData(FALSE);

	//��ջ�����нṹ
	ClearQueue(q);
}

LRESULT CNEWFLASHBURNDlg::OnThreadRXMessage(WPARAM wParam,LPARAM lParam)
{
	static int m_snc=0;
	m_snc+=wParam;
	CString m_temp;
	m_temp.Format("%d",m_snc);
	m_CRXn.SetWindowText(m_temp);

	static bool m_binitsq=TRUE;
	if (m_binitsq==TRUE)
	{
		m_binitsq=FALSE;
		
		if (InitQueue(q)==-1)
		{
			AfxMessageBox("����������ʧ��!");
		}
	}
	//�����յĴ�������ȫ�����
	for (int i=0;i<(int)wParam;i++)
	{
		EnQueue(q,m_byteRXbuffer[i]);
	}
	//�жϻ��������Ƿ�һ֡����
	for (int m_nc=QueueLenth(q);m_nc>=24;m_nc--)
	{
		//���� ����Ƿ��յ�0xfc
		if(DeQueue(q,m_byteFrame[0])!=0)
		{
			AfxMessageBox("��������ʧ��");
		}
		if (m_byteFrame[0]==0xfc)
		{
			m_strshow="FC";
			for (int j=1;j<24;j++)
			{
				DeQueue(q,m_byteFrame[j]);
				m_temp.Format("%02X",m_byteFrame[j]);
				m_strshow+=m_temp;
			}
			//�������
			m_strshow+="\r\n";
			m_nc-=23;
			//�����������
			WriteRecoder(m_strshow);
			m_CRX.ReplaceSel(m_strshow);
			m_CRX.PostMessage(WM_VSCROLL, SB_BOTTOM,0);
			CString m_str;
			//�����
			if ((m_byteFrame[0]==0xfc)&&(m_byteFrame[0x01]==0x0c)&&(m_byteFrame[0x05]==0x70))
			{
				//׼��
				if ((m_byteFrame[0x06]==0xee)&&(m_byteFrame[0x07]==0xff))
				{
					//׼����ɿ�ʼ��������
					KillTimer(1);
					//��ʼ��ʱ
					SetTimer(2,1000,NULL);
					m_ncountersecond=0;
					m_strcountersecond.Format("%d ��",m_ncountersecond);
					UpdateData(FALSE);
					Sleep(5);
					pSendThread=AfxBeginThread(ThreadSendBin,(LPVOID)1);
					m_str="׼��!";
					m_str.Insert(0,GetSystemTime());
					m_str+="\r\n";
					m_Cricheditshowmsg.ReplaceSel(m_str);
					m_Cricheditshowmsg.PostMessage(WM_VSCROLL, SB_BOTTOM,0);
					//SendMSGtoSendThread();
				}
				//У��ɹ�
				if ((m_byteFrame[0x06]==0xdd)&&(m_byteFrame[0x07]==0xdd))
				{
					m_str="����У��ɹ�!";
					m_str+="\r\n";
					m_str.Insert(0,GetSystemTime());
					m_Cricheditshowmsg.ReplaceSel(m_str); 
					m_Cricheditshowmsg.PostMessage(WM_VSCROLL, SB_BOTTOM,0);


				}	
				//У��ʧ��
				if ((m_byteFrame[0x06]==0x77)&&(m_byteFrame[0x07]==0x77))
				{
					m_str="����У��ʧ��!";
					m_str+="\r\n";
					m_str.Insert(0,GetSystemTime());
					m_Cricheditshowmsg.ReplaceSel(m_str); 
					m_Cricheditshowmsg.PostMessage(WM_VSCROLL, SB_BOTTOM,0);
				}
				//���ݵ�ַ��Ӳ������ַ��Χ ���
				if ((m_byteFrame[0x06]==0xbb)&&(m_byteFrame[0x07]==0xbb)&(m_byteFrame[0x08]==0xbb)&&(m_byteFrame[0x09]==0xbb))
				{
					m_str="�������!";
					m_str+="\r\n";
					m_str.Insert(0,GetSystemTime());
					m_Cricheditshowmsg.ReplaceSel(m_str); 
					m_Cricheditshowmsg.PostMessage(WM_VSCROLL, SB_BOTTOM,0);
				}
				//���·���
				if ((m_byteFrame[0x06]==0xee)&&(m_byteFrame[0x07]==0xee)&(m_byteFrame[0x08]==0xee)&&(m_byteFrame[0x09]==0xee))
				{
					//�������̷߳�����Ϣ������֪�ط���ַ
					SendMSGtoSendThread();
					m_str="�ط�����!";
					m_str+="\r\n";
					m_str.Insert(0,GetSystemTime());
					m_Cricheditshowmsg.ReplaceSel(m_str); 
					m_Cricheditshowmsg.PostMessage(WM_VSCROLL, SB_BOTTOM,0);
					CString m_strresend;
					m_str.Format("%02X",m_byteFrame[0x0a]);
					m_strresend=m_str;
					m_str.Format("%02X",m_byteFrame[0x0b]);
					m_strresend+=m_str;
					m_str.Format("%02X",m_byteFrame[0x0c]);
					m_strresend+=m_str;
					m_str.Format("%02X",m_byteFrame[0x0d]);
					m_strresend+=m_str;
					m_strresend+="\r\n";
					m_strresend.Insert(0,"�ط���ַ:");
					m_Cricheditshowmsg.ReplaceSel(m_strresend); 
					m_Cricheditshowmsg.PostMessage(WM_VSCROLL, SB_BOTTOM,0);
				}
			}

		}
	}
	if (m_strshow.GetLength()>1200)
	{
		m_strshow.Empty();
	}
	//m_CRX.SetWindowText(m_strshow);
	//�����ܳ��Ⱥ���Ч��ʼλ
	m_temp.Format("%d",wParam);
	m_CRXLENTH.SetWindowText(m_temp);
	//m_temp.Format("%d",lParam);
	//m_CRXVALUE.SetWindowText(m_temp);
	return 0;
}

void CNEWFLASHBURNDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	switch (nIDEvent)
	{
	case 1:
		{
			if (m_bSendPackage==FALSE)
			{
				m_bSendPackage=TRUE;
			}
			break;
		}
	case 2:
		{
			CounterSecond();
			break;
		}
	}
	CDialog::OnTimer(nIDEvent);
}
LRESULT CNEWFLASHBURNDlg::OnWriteComPortMSG(WPARAM wParam, LPARAM lParam)
{
	//��ʾ������Ϣ
	static int m_nTX=0;
	CString m_str;
	CString m_strf1,m_strf2,m_strf3,m_strf4;
	for (int i=0;i<24;i++)
	{
		m_str.Format("%02X",m_byteWriteFrame1[i]);
		m_strf1+=m_str;
		m_str.Format("%02X",m_byteWriteFrame2[i]);
		m_strf2+=m_str;
		m_str.Format("%02X",m_byteWriteFrame3[i]);
		m_strf3+=m_str;
		m_str.Format("%02X",m_byteWriteFrame4[i]);
		m_strf4+=m_str;
	}
	m_strf1+="\r\n";
	m_strf2+="\r\n";
	m_strf3+="\r\n";
	m_strf4+="\r\n";
	
	m_str=m_strf1+m_strf2+m_strf3+m_strf4;
	//�ļ�д���¼
	m_CTX.ReplaceSel(m_str);
	m_CTX.PostMessage(WM_VSCROLL, SB_BOTTOM,0);
	m_nTX+=96;
	m_str.Format("%d",m_nTX);
	m_CRXVALUE.SetWindowText(m_str);
	return 0;
}

UINT ThreadSendBin(LPVOID lpParam)
{
	if (m_strgfilepath!="")
	{
		FILE * m_bfilep=fopen(m_strgfilepath,"ab+");
		if (m_bfilep==NULL)
		{
			//������Ϣ�����̣߳���ʾ������Ϣ
			::PostMessage(AfxGetMainWnd()->m_hWnd,WM_UDISPLAY_MSG,0,0);
		}
		//�õ��ļ��ܳ���
		fseek(m_bfilep,SEEK_SET,SEEK_END);
		m_nBinFileLenth=ftell(m_bfilep);
		//��ʼ����
		//ָ�븴λ
		fseek(m_bfilep,SEEK_SET,SEEK_SET);
		MSG m_msgresend;
		PeekMessage(&m_msgresend, NULL, WM_USER, WM_USER, PM_NOREMOVE);	//ǿ�ƴ�����Ϣ����
		for (m_nBinFileCur=0;m_nBinFileCur<m_nBinFileLenth;m_nBinFileCur+=60)
		{
			Sleep(10);
			while(m_bSendPackage==TRUE)
			{
				//�ȴ������̷߳������
				Sleep(1);
			}
			if (m_bReSend==TRUE)
			{
				m_nBinFileCur=m_unReSendAddr;
				m_bReSend=FALSE;
			}
			//��һ֡
			//�����ַ��д��
			UINT m_uintadd;
			unsigned char m_ucReadBin[16];
			m_uintadd=m_uintBaseAddr+m_nBinFileCur;
			m_byteWriteFrame1[0x09]=m_uintadd;
			m_uintadd=m_uintadd>>8;
			m_byteWriteFrame1[0x08]=m_uintadd;
			m_uintadd=m_uintadd>>8;
			m_byteWriteFrame1[0x07]=m_uintadd;
			m_uintadd=m_uintadd>>8;
			m_byteWriteFrame1[0x06]=m_uintadd;

			fseek(m_bfilep,m_nBinFileCur,SEEK_SET);
			fread( m_ucReadBin, sizeof(char), 12, m_bfilep);
			for (int i=0;i<12;i++)
			{
				m_byteWriteFrame1[0x0a+i]=m_ucReadBin[i];
			}

			fseek(m_bfilep,m_nBinFileCur+12,SEEK_SET);
			fread( m_ucReadBin, sizeof(char), 16, m_bfilep);
			for (int j=0;j<16;j++)
			{
				m_byteWriteFrame2[0x06+j]=m_ucReadBin[j];
			}

			fseek(m_bfilep,m_nBinFileCur+12+16,SEEK_SET);
			fread( m_ucReadBin, sizeof(char), 16, m_bfilep);
			for (int k=0;k<16;k++)
			{
				m_byteWriteFrame3[0x06+k]=m_ucReadBin[k];
			}

			fseek(m_bfilep,m_nBinFileCur+12+16+16,SEEK_SET);
			fread( m_ucReadBin, sizeof(char), 16, m_bfilep);
			for (int l=0;l<16;l++)
			{
				m_byteWriteFrame4[0x06+l]=m_ucReadBin[l];
			}
			m_bSendPackage=TRUE;
			//������Ϣ�����̣߳���ʾ������Ϣ
			::PostMessage(AfxGetMainWnd()->m_hWnd,WM_UDISPLAY_MSG,1,m_nBinFileCur);
		}
		if (m_nBinFileCur+60>=m_nBinFileLenth)
		{
			::PostMessage(AfxGetMainWnd()->m_hWnd,WM_UDISPLAY_MSG,1,m_nBinFileLenth);
			m_bRecoder=FALSE;
		}
		else
		{
			::PostMessage(AfxGetMainWnd()->m_hWnd,WM_UDISPLAY_MSG,1,m_nBinFileCur+60);
			m_bRecoder=FALSE;
		}
		//������ɱ�־
		while(m_bSendPackage==TRUE)
		{
			//�ȴ������̷߳������
			Sleep(1);
		}
		m_byteWriteFrame1[0x09]=0xff;
		m_byteWriteFrame1[0x08]=0xff;
		m_byteWriteFrame1[0x07]=0xff;
		m_byteWriteFrame1[0x06]=0xff;
		m_bSendPackage=TRUE;
		::PostMessage(AfxGetMainWnd()->m_hWnd,WM_UDISPLAY_MSG,2,0);
	}
	return 0;
}

CString CNEWFLASHBURNDlg::GetSystemTime()
{
	CString m_strsystime;
	SYSTEMTIME  m_systime;
	GetLocalTime(&m_systime);
	m_strsystime.Format("%4d-%02d-%02d %02d:%02d:%02d",m_systime.wYear,m_systime.wMonth,m_systime.wDay, m_systime.wHour,m_systime.wMinute,m_systime.wSecond);
	return m_strsystime;
}

CNEWFLASHBURNDlg::SendMSGtoSendThread()
{
	//�����ط���ַ��������ƫ����
	UINT m_unresendaddr=0;
	m_unresendaddr=m_byteFrame[0x0a];
	m_unresendaddr=m_unresendaddr<<8;
	m_unresendaddr+=m_byteFrame[0x0b];
	m_unresendaddr=m_unresendaddr<<8;
	m_unresendaddr+=m_byteFrame[0x0c];
	m_unresendaddr=m_unresendaddr<<8;
	m_unresendaddr+=m_byteFrame[0x0d];
	
	m_unReSendAddr=m_unresendaddr-m_uintBaseAddr;
	m_bReSend=TRUE;
}
LRESULT CNEWFLASHBURNDlg::OnThreadMSG(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case 0:
		{
			AfxMessageBox("���ļ�ʧ��!");
			break;
		}
	case 1:
		{
			//������ʾ
			float m_fcur=0,m_ffilelenth=0,m_fpersent=0;
			
			int	m_npersent=0;
			m_ffilelenth=(float)m_nBinFileLenth;
			m_fcur=(float)lParam;
			m_fpersent=m_fcur/m_ffilelenth;
			m_npersent=(int)(m_fpersent*100);
			m_cprogress.SetPos(m_npersent);
			m_strpersent.Format("%d",m_npersent);
			m_strpersent.Insert(0,"%");
			UpdateData(FALSE);
			break;
		}
	case 2:
		{
			CString m_str;
			m_str="���ݷ������";
			m_str+="\r\n";
			m_str.Insert(0,GetSystemTime());
			m_strfilename="";
			m_bRecoder=FALSE;
			//AllFuctionable();
			KillTimer(2);
			m_Cricheditshowmsg.ReplaceSel(m_str);
			m_Cricheditshowmsg.PostMessage(WM_VSCROLL, SB_BOTTOM,0);
		}
	}
	return 0;
}

void CNEWFLASHBURNDlg::ClearFlags()
{
	m_cprogress.SetPos(0);
	m_strpersent.Format("%d",0);
	m_strpersent.Insert(0,"%");
	UpdateData(FALSE);
}

void CNEWFLASHBURNDlg::CounterSecond()
{
	m_ncountersecond++;
	m_strcountersecond.Format("%d ��",m_ncountersecond);
	UpdateData(FALSE);
}

void CNEWFLASHBURNDlg::OnButtonspeed() 
{
	// TODO: Add your control notification handler code here
	if (m_bHighSpeed==FALSE)
	{
		m_bHighSpeed=TRUE;
	}
}

void CNEWFLASHBURNDlg::OnSelchangeCombocomportnumber() 
{
	// TODO: Add your control notification handler code here
	//�����߳�
	m_bThreadRXrunning=FALSE;
	m_bThreadTXrunning=FALSE;
	CloseHandle(hCom);
}

void CNEWFLASHBURNDlg::WriteRecoder(CString m_strtowrite)
{
	if (m_bGetFilePath==TRUE)
	{
		m_strfilename=GetFilePath();
		//m_strrecoderpos=m_strfilename;
		m_bGetFilePath=FALSE;
	}
	if (m_strfilename!="")
	{
		CFile m_file;
		//Ĭ��Ϊ��ǰ����Ŀ¼�����ǳ�������Ŀ¼
		m_file.Open(m_strfilename,CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate);
		//m_strtowrite+="\r\n";
		m_file.SeekToEnd();
		m_file.Write(m_strtowrite,m_strtowrite.GetLength());
		m_file.Close();
	}
}

CString CNEWFLASHBURNDlg::GetFilePath()
{
	CString m_strfilepath;
	int		m_nfilepathlenth=0;
	HMODULE module = GetModuleHandle(0); 
	char pFileName[MAX_PATH]; 
	GetModuleFileName(module, pFileName, MAX_PATH);
	m_strfilepath=pFileName;
	//ɾ���ļ�����
	m_nfilepathlenth=m_strfilepath.Find("FLASHBURN.exe");
	m_strfilepath.Delete(m_nfilepathlenth,13);
	m_strfilepath+=GetSystemTimeNumber();
	m_strfilepath+="FlashingRecoder.txt";
	return m_strfilepath;
}

CString CNEWFLASHBURNDlg::GetSystemTimeNumber()
{
	CString m_strsystime;
	SYSTEMTIME  m_systime;
	GetLocalTime(&m_systime);
	m_strsystime.Format("%4d%02d%02d%02d%02d%02d",m_systime.wYear,m_systime.wMonth,m_systime.wDay, m_systime.wHour,m_systime.wMinute,m_systime.wSecond);
	return m_strsystime;
}

void CNEWFLASHBURNDlg::OnButtoncleartx() 
{
	// TODO: Add your control notification handler code here
	long m_longlenth=m_CTX.GetTextLength();
	m_CTX.SetSel(0,m_longlenth);
	m_CTX.Clear();
}

void CNEWFLASHBURNDlg::OnButtonclearrx() 
{
	// TODO: Add your control notification handler code here
	long m_longlenth=m_CRX.GetTextLength();
	m_CRX.SetSel(0,m_longlenth);
	m_CRX.Clear();
}

void CNEWFLASHBURNDlg::OnButtonclearinfo() 
{
	// TODO: Add your control notification handler code here
	long m_longlenth=m_Cricheditshowmsg.GetTextLength();
	m_Cricheditshowmsg.SetSel(0,m_longlenth);
	m_Cricheditshowmsg.Clear();
}

void CNEWFLASHBURNDlg::OnButtonclearall() 
{
	// TODO: Add your control notification handler code here
	long m_longlenth=m_CTX.GetTextLength();
	m_CTX.SetSel(0,m_longlenth);
	m_CTX.Clear();

	m_longlenth=m_CRX.GetTextLength();
	m_CRX.SetSel(0,m_longlenth);
	m_CRX.Clear();

	m_longlenth=m_Cricheditshowmsg.GetTextLength();
	m_Cricheditshowmsg.SetSel(0,m_longlenth);
	m_Cricheditshowmsg.Clear();

}

BOOL CNEWFLASHBURNDlg::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CDialog::DestroyWindow();
}
