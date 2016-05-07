
// ProcessAssistantDlg.h : ͷ�ļ�
//

#pragma once
#include <map>
#include <vector>
#include "afxwin.h"

// CProcessAssistantDlg �Ի���
class CProcessAssistantDlg : public CDialog
{
public:
    CProcessAssistantDlg(CWnd* pParent = NULL);	// ��׼���캯��

    // �Ի�������
    enum {
        IDD = IDD_PROCESSASSISTANT_DIALOG
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnPaint();
    afx_msg void OnBnClickedOk();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    DECLARE_MESSAGE_MAP()

protected:
    HICON m_hIcon;
    std::map<CString, CString> m_processListMap; //�����б�
    std::vector<CString> m_runList; //��Ҫ�����������Ľ����б�
    CListCtrl m_wndList;   //�����б�ؼ�����
    CImageList m_imgList;  //ͼ���б�
    int m_listCnt;         //�б������
    CString m_myPath;      //��ʱ�ļ����еĸ����ļ���

public:
    bool isProcessExist(CString name); //�ж�ָ�������Ƿ����(���Ƿ��Ѵ򿪸ó���)
    void showProcessList();            //��ʾ�����б�
    void loadTaskFileList();           //�������б��ļ���ȡ�����б�
    void loadTaskMgrList();            //�������������ȡ�����б�
};
