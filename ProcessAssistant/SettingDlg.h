#pragma once


// SettingDlg �Ի���

class SettingDlg : public CDialogEx
{
    DECLARE_DYNAMIC(SettingDlg)

public:
    SettingDlg(CWnd* pParent = NULL);   // ��׼���캯��
    virtual ~SettingDlg();

    // �Ի�������
    enum {
        IDD = IDD_SET
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedSetOk();
    void setStartup(bool enable);   //���ÿ���������״̬
    bool getStartup();              //�Ƿ�������������
    afx_msg void OnBnClickedCheckOpenLastDir();
};
