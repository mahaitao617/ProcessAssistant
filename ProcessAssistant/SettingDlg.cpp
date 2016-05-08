// SettingDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ProcessAssistant.h"
#include "SettingDlg.h"
#include "afxdialogex.h"


// SettingDlg �Ի���

IMPLEMENT_DYNAMIC(SettingDlg, CDialogEx)

SettingDlg::SettingDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(SettingDlg::IDD, pParent)
{

}

SettingDlg::~SettingDlg()
{
}

void SettingDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(SettingDlg, CDialogEx)
    ON_BN_CLICKED(ID_SETOK, &SettingDlg::OnBnClickedSetOk)
    ON_BN_CLICKED(IDC_CHECK_OPEN_LAST_DIR, &SettingDlg::OnBnClickedCheckOpenLastDir)
END_MESSAGE_MAP()


// SettingDlg ��Ϣ�������


BOOL SettingDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    if (getStartup()){
        ((CButton *)GetDlgItem(IDC_CHECK_STARTUP))->SetCheck(TRUE);
    }
    return TRUE;
}

void SettingDlg::setStartup(bool enable)
{
    bool setOK = false;
    //�ж��Ƿ��п�����������û�������
    static LPCTSTR lpRun = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    HKEY hKey;
    long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE | KEY_READ, &hKey);
    if (lRet == ERROR_SUCCESS)
    {
        if (enable){ //������������
            static char myName[MAX_PATH] = { 0 };
            static DWORD dwRet = GetModuleFileName(NULL, myName, MAX_PATH); //�õ����������ȫ·��
            lRet = RegSetValueEx(hKey, "ProcessAssistant", 0, REG_SZ, (BYTE*)myName, dwRet);
            if (lRet == ERROR_SUCCESS)
                setOK = true;
        }
        else{ //�رտ�������
            lRet = RegDeleteValue(hKey, "ProcessAssistant");
            if (lRet == ERROR_SUCCESS)
                setOK = true;
        }
        RegCloseKey(hKey);
    }
    if (!setOK)
    {
        CString err;
        err.Format("lastError:%d(lRet:%d)", GetLastError(), lRet);
        MessageBox("�޸Ŀ���������ʧ��!" + err, "ʧ����ʾ");
    }
}

bool SettingDlg::getStartup()
{
    bool setOK = false; //��¼�Ƿ����п���������
    static LPCTSTR lpRun = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    HKEY hKey;
    long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE | KEY_READ, &hKey);
    if (lRet == ERROR_SUCCESS)
    {
        static char myName[MAX_PATH] = { 0 };
        static DWORD dwRet = GetModuleFileName(NULL, myName, MAX_PATH); //�õ����������ȫ·��
        char data[1000] = { 0 };
        DWORD dwType = REG_SZ, dwSize;
        RegQueryValueEx(hKey, "ProcessAssistant", 0, &dwType, (BYTE*)data, &dwSize);
        if (CString(data) == myName)
            setOK = true;
        RegCloseKey(hKey);
    }
    return setOK;
}

void SettingDlg::OnBnClickedSetOk()
{
    if ((BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_STARTUP)) != getStartup())
        setStartup(IsDlgButtonChecked(IDC_CHECK_STARTUP) != 0);
    CDialogEx::OnOK();
}

void SettingDlg::OnBnClickedCheckOpenLastDir()
{
    if (IsDlgButtonChecked(IDC_CHECK_OPEN_LAST_DIR))
        MessageBox("�밴�����²������ô�ѡ�\n1. ��һ���ļ��У�\n"
        "2. ���ļ���ѡ��(win7: ����->�ļ���ѡ�win8/win10: �鿴->ѡ��)��"
        "\n3. �л����鿴����ҳ��\n"
        "4. ��ѡ\"��¼ʱ��ԭ��һ���ļ��д���\"", "��ܰ��ʾ");
}
