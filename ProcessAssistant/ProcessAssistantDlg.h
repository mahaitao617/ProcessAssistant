
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
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnPaint();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedBtSet();
    afx_msg void OnOpenMainDlg();  //�������˵�-��������
    afx_msg void OnExitMe();       //�������˵�-�˳�
    afx_msg void OnClose();        //�Ի���Ĺر���Ϣ
    afx_msg LRESULT OnNotifyIconMsg(WPARAM wParam, LPARAM lParam);//����֪ͨ����Ϣ
    afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2); //��Ӧ��ݼ���Ϣ
    afx_msg void OnDblclkListProcess(NMHDR *pNMHDR, LRESULT *pResult); //˫����Ϣ
    afx_msg void OnNM_RClickListProcess(NMHDR *pNMHDR, LRESULT *pResult); //�Ҽ�������Ϣ
    afx_msg void OnRClick_OpenDir();           //�Ҽ��˵�-������λ��
    afx_msg void OnRClick_StartProcess();      //�Ҽ��˵�-�򿪸ý���
    afx_msg void OnRClick_EndProcess();        //�Ҽ��˵�-�����ý���
    afx_msg void OnRClick_CheckOrNotProcess(); //�Ҽ��˵�-��ѡ/����ѡ�ý���
    DECLARE_MESSAGE_MAP()

protected:
    HICON m_hIcon;
    std::set<CString> m_processList; //�ڱ������ʾ�����н��̼���(����m_checkedList)
    std::set<CString> m_checkedList; //��Ҫ�����������Ľ��̼���
    std::map<CString, int> m_processIndexMap; //���̶�Ӧ������ŵ�ӳ���
    CListCtrl m_wndList;        //�����б�ؼ�����
    CImageList m_iconList;      //ͼ���б�
    int m_listCnt;              //�б������
    SettingDlg m_setDlg;        //���öԻ���
    CString m_checkedListFile;  //�����ѹ�ѡ�Ľ����б���ļ�(����m_autoRunListFile)
    CString m_autoRunListFile;  //������Ҫ�Զ����еĽ����б���ļ�
    NOTIFYICONDATA m_nid;       //֪ͨ����Ϣ�Ľṹ�����
    HANDLE m_hmutex;            //��֤���ֻ��һ��ʵ�������еĻ�����
    void *pMapRunning;          //ָ�����ڴ�����ָ��
    int m_selected;             //��ǰ��ѡ�еı���������

public:
    bool isProcessExist(CString name); //�ж�ָ�������Ƿ����(���Ƿ��Ѵ򿪸ó���)
    void showProcessList();            //��ʾ�����б�
    void loadListFromTaskFile();       //�������б��ļ���ȡ�����б�
    void loadListFromTaskMgr();        //�������������ȡ�����б�
    void openUnclosedProcess();        //���ϴ�δ�رյĽ���
    void updateProcessList();          //���½����б�(ͨ����ʱ���Զ�����)
    CString getFileDescription(const CString& filePathName); //��ȡexe�ļ����ļ�����
};
