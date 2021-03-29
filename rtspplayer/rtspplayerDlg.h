
// rtspplayerDlg.h : ͷ�ļ�
//

#pragma once

#include "gmf_player.h"

// CrtspplayerDlg �Ի���
class CrtspplayerDlg : public CDialogEx
{
// ����
public:
	CrtspplayerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RTSPPLAYER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonStop();
private:
	gmf_player_t* m_pPlayer1;
	gmf_player_t* m_pPlayer2;
};
