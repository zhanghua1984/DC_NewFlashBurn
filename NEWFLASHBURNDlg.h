// NEWFLASHBURNDlg.h : header file
//

#if !defined(AFX_NEWFLASHBURNDLG_H__EB26F18E_8261_458F_B34C_0A220D85007A__INCLUDED_)
#define AFX_NEWFLASHBURNDLG_H__EB26F18E_8261_458F_B34C_0A220D85007A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "comport.h"
UINT ThreadSendBin(LPVOID lpParam);
/////////////////////////////////////////////////////////////////////////////
// CNEWFLASHBURNDlg dialog
#define DATALENTH 960
class CNEWFLASHBURNDlg : public CDialog
{
// Construction
public:
	CString GetSystemTimeNumber();
	CString GetFilePath();
	void WriteRecoder(CString m_strtowrite);
	void CounterSecond();
	void ClearFlags();
	SendMSGtoSendThread();
	CString GetSystemTime();
	void Initialization();
	LRESULT OnThreadRXMessage(WPARAM wParam,LPARAM lParam);
	CNEWFLASHBURNDlg(CWnd* pParent = NULL);	// standard constructor
	CString m_strfilepath;
	long int m_lnfilelenth;
	CWinThread* pSendThread;
	CString m_strfilename;
	CString m_strshow;
	BOOL m_bGetFilePath;
// Dialog Data
	//{{AFX_DATA(CNEWFLASHBURNDlg)
	enum { IDD = IDD_NEWFLASHBURN_DIALOG };
	CRichEditCtrl	m_CTX;
	CRichEditCtrl	m_CRX;
	CButton	m_CCom;
	CRichEditCtrl	m_Cricheditshowmsg;
	CProgressCtrl	m_cprogress;
	CEdit	m_Cstatus;
	CEdit	m_CRXVALUE;
	CEdit	m_CRXLENTH;
	CEdit	m_CRXn;
	CComboBox	m_CTargetBoard;
	CComboBox	m_Ccomportnumber;
	CString	m_strstartaddr;
	CString	m_strconpath;
	CString	m_strgeopath;
	CString	m_strpersent;
	CString	m_strcountersecond;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNEWFLASHBURNDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CNEWFLASHBURNDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnReceiveAComPort(WPARAM wParam, LPARAM lParam); 
	afx_msg LRESULT OnWriteComPortMSG(WPARAM wParam, LPARAM lParam); 
	afx_msg LRESULT OnThreadMSG(WPARAM wParam, LPARAM lParam); 
	afx_msg void OnCloseoropen();
	afx_msg void OnButtonselectcon();
	afx_msg void OnButtonselectgeo();
	afx_msg void OnButtonsend();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnButtonspeed();
	afx_msg void OnSelchangeCombocomportnumber();
	afx_msg void OnButtoncleartx();
	afx_msg void OnButtonclearrx();
	afx_msg void OnButtonclearinfo();
	afx_msg void OnButtonclearall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWFLASHBURNDLG_H__EB26F18E_8261_458F_B34C_0A220D85007A__INCLUDED_)
