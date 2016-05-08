
// ProcessAssistantDlg.h : ͷ�ļ�
//

#pragma once
#include <map>
#include <set>
#include "afxwin.h"
#include "SettingDlg.h"

// CProcessAssistantDlg �Ի���
class CProcessAssistantDlg : public CDialog
{
public:
    CProcessAssistantDlg(CWnd* pParent = NULL);	// ��׼���캯��
    ~CProcessAssistantDlg();
    // �Ի�������
    enum {
        IDD = IDD_PROCESSASSISTANT_DIALOG
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnPaint();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedBtSet();
    afx_msg void OnOpenMainDlg();  //�������˵�-��������
    afx_msg void OnExitMe();       //�������˵�-�˳�
    afx_msg void OnClose();        //�Ի���Ĺر���Ϣ
    afx_msg LRESULT OnNotifyIconMsg(WPARAM wParam, LPARAM lParam);//����֪ͨ����Ϣ
    afx_msg void OnDblclkListProcess(NMHDR *pNMHDR, LRESULT *pResult); //˫����Ϣ
    DECLARE_MESSAGE_MAP()

protected:
    HICON m_hIcon;
    std::map<CString, CString> m_processListMap; //�����б�
    std::map<CString, int> m_processIndexMap;    //���̶�Ӧ�������
    std::set<CString> m_runList; //��Ҫ�����������Ľ��̼���
    CListCtrl m_wndList;   //�����б�ؼ�����
    CImageList m_imgList;  //ͼ���б�
    int m_listCnt;         //�б������
    CString m_myPath;      //��ʱ�ļ����еĸ����ļ���
    SettingDlg setDlg;     //���öԻ���
    CString m_autoRunFile; //������Ҫ�Զ����еĽ��̵��ļ�
    NOTIFYICONDATA nid;    //֪ͨ��Ϣ�Ľṹ�����
    HANDLE hmutex;         //����������,��ֻ֤��һ��ʵ��������

public:
    bool isProcessExist(CString name); //�ж�ָ�������Ƿ����(���Ƿ��Ѵ򿪸ó���)
    void showProcessList();            //��ʾ�����б�
    void loadTaskFileList();           //�������б��ļ���ȡ�����б�
    void loadTaskMgrList();            //�������������ȡ�����б�
    void openUnclosedProcess();        //���ϴ�δ�رյĽ���
    void updateProcessList();          //���½����б�
};
