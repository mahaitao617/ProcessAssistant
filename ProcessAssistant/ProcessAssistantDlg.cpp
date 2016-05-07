
// ProcessAssistantDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ProcessAssistant.h"
#include "ProcessAssistantDlg.h"
#include "afxdialogex.h"
#include "tlhelp32.h"
#include <fstream>
#include <string>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CProcessAssistantDlg �Ի���



CProcessAssistantDlg::CProcessAssistantDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CProcessAssistantDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    char path[MAX_PATH] = "";
    GetTempPath(MAX_PATH, path);
    m_myPath = path + CString("ProcessAssistant\\");
    CreateDirectory(m_myPath, 0);
}

void CProcessAssistantDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_PROCESS, m_wndList);
}

BEGIN_MESSAGE_MAP(CProcessAssistantDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDOK, &CProcessAssistantDlg::OnBnClickedOk)
    ON_WM_TIMER()
END_MESSAGE_MAP()


// CProcessAssistantDlg ��Ϣ�������

BOOL CProcessAssistantDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
    //  ִ�д˲���
    SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
    SetIcon(m_hIcon, FALSE);		// ����Сͼ��

    // TODO:  �ڴ���Ӷ���ĳ�ʼ������
    //����ѡ�� + ��ʾ����� + ��ѡ�� + ��ƽ������
    m_wndList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES | LVS_EX_FLATSB);
    //���ñ�ͷ
    m_wndList.InsertColumn(0, "(����)   ������", 0, 124);
    m_wndList.InsertColumn(1, "�ļ�λ��", 0, 415);
    m_wndList.InsertColumn(2, "��ע", 0, 50);
    //�����ı���ʾ��ɫ
    m_wndList.SetTextColor(RGB(0, 255, 0));

    SetTimer(0, 100, NULL);  //��鵱ǰ����������еĽ���,�����������
    SetTimer(1, 1, NULL);    //���ϴιر�ʱ���õĽ���
    SetTimer(2, 2000, NULL); //�鿴���Ľ����Ƿ�������

    return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CProcessAssistantDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // ���ڻ��Ƶ��豸������

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // ʹͼ���ڹ����������о���
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // ����ͼ��
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CProcessAssistantDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


#include <psapi.h>
#pragma comment(lib,"Psapi.lib")
void CProcessAssistantDlg::showProcessList()
{
    HANDLE hp = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
    if (hp == INVALID_HANDLE_VALUE){
        CString err;
        err.Format("%d", GetLastError());
        MessageBox("�������̿���ʧ�ܣ�" + err, "��ܰ��ʾ");
        return;
    }
    m_wndList.DeleteAllItems();
    int cnt = 0;
    static CImageList *imgList = NULL;
    delete imgList;
    imgList = new CImageList;
    imgList->Create(32, 32, ILC_COLOR32, 0, 100);
    m_wndList.SetImageList(imgList, LVSIL_SMALL);

    m_processListMap.clear();
    m_runList.clear();
    ifstream fin(m_myPath + "AutoRunProcessList.txt");
    if (fin.is_open()){
        string process;
        while (getline(fin, process)){
            HICON hIcon = ExtractIcon(AfxGetInstanceHandle(), process.c_str(), 0);
            int indexIcon = imgList->Add(hIcon);
            CString exeName = process.substr(process.rfind('\\') + 1).c_str();
            int item = m_wndList.InsertItem(cnt++, exeName.Left(exeName.GetLength() - 4), indexIcon); //����һ��
            m_wndList.SetItemText(item, 1, process.c_str());
            m_wndList.SetCheck(item);
            if (isProcessExist(process.c_str()))
                m_wndList.SetItemText(item, 2, "������");
            m_processListMap[process.c_str()] = exeName; //����map
            m_runList.push_back(process.c_str());
        }
    }
    fin.close();

    PROCESSENTRY32 pe32 = { sizeof(pe32) };
    for (BOOL find = Process32First(hp, &pe32); find != 0; find = Process32Next(hp, &pe32)) {
        HANDLE hd = OpenProcess(PROCESS_ALL_ACCESS, 0, pe32.th32ProcessID);
        if (hd != NULL) {
            char exePath[255];
            GetModuleFileNameEx(hd, NULL, exePath, 255);
            //��ó���ͼ��
            HICON hIcon = ExtractIcon(AfxGetInstanceHandle(), exePath, 0);
            if (hIcon != NULL && m_processListMap.find(exePath) == m_processListMap.end()){
                CString path = exePath, exeName = path.Right(path.GetLength() - path.ReverseFind('\\') - 1);
                static char myName[MAX_PATH] = { 0 }; //������Ľ�����
                static int unused1 = GetModuleFileName(NULL, myName, MAX_PATH);
                if (path == myName || path.Find("system32") != -1 || path.Find("SysWOW64") != -1)
                    continue;
                int indexIcon = imgList->Add(hIcon);
                int item = m_wndList.InsertItem(cnt++, exeName.Left(exeName.GetLength() - 4), indexIcon); //����һ��
                m_wndList.SetItemText(item, 1, exePath);
                if (isProcessExist(exeName))
                    m_wndList.SetItemText(item, 2, "������");
                m_processListMap[exePath] = exeName; //����map
            }
            DestroyIcon(hIcon); //����ͼ��
            CloseHandle(hd);
        }
    }
    CloseHandle(hp);
}


bool CProcessAssistantDlg::isProcessExist(CString name)
{
    HANDLE hp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hp == INVALID_HANDLE_VALUE)
        return false;
    int i = name.ReverseFind('\\');
    if (i != -1)
        name = name.Right(name.GetLength() - i - 1);
    PROCESSENTRY32 pe32 = { sizeof(pe32) };
    for (BOOL find = Process32First(hp, &pe32); find != 0; find = Process32Next(hp, &pe32)) {
        if (name == pe32.szExeFile){
            CloseHandle(hp);
            return true;
        }
    }
    CloseHandle(hp);
    return false;
}

void CProcessAssistantDlg::OnBnClickedOk()
{
    m_runList.clear();
    ofstream fout(m_myPath + "AutoRunProcessList.txt");
    for (int i = 0; i < m_wndList.GetItemCount(); ++i)
        if (m_wndList.GetCheck(i)) {
            fout << m_wndList.GetItemText(i, 1) << endl;
            m_runList.push_back(m_wndList.GetItemText(i, 1));
        }
    fout.close();

    CString selected;
    for (auto& elem : m_runList)
        selected += elem + "\n";
    if (selected.IsEmpty()){
        MessageBox("��ȡ�����п���������", "�ر���ʾ", MB_ICONINFORMATION);
    }
    else{
        MessageBox("�ѿ������п���������: \r\n" + selected + "�ػ�ʱ��������"
                   "���δ�رգ��´ο��������д˳����Զ����������̣�",
                   "������ʾ", MB_ICONINFORMATION);
    }
}


void CProcessAssistantDlg::OnTimer(UINT_PTR nIDEvent)
{
    static CString autoRunFile = m_myPath + "autorun.txt";
    switch (nIDEvent)
    {
        case 0: //�ڱ���������ʱִ��һ��, ��鵱ǰ����������еĽ���,�����������
        {
            KillTimer(0);
            showProcessList();
            //�ж��Ƿ��п�����������û�������
            HKEY hKey;
            LPCTSTR lpRun = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
            long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE | KEY_READ, &hKey);
            if (lRet == ERROR_SUCCESS)
            {
                char myName[MAX_PATH] = { 0 };
                DWORD dwRet = GetModuleFileName(NULL, myName, MAX_PATH); //�õ����������ȫ·��
                char data[1000] = { 0 };
                DWORD dwType = REG_SZ, dwSize;
                RegQueryValueEx(hKey, "ProcessAssistant", 0, &dwType, (BYTE*)data, &dwSize); //���Ƿ����и�������
                if (CString(data) == myName){
                    MessageBox("�Ѵ��ڸó��򿪻��������", "��ܰ��ʾ");
                    break;
                }
                lRet = RegSetValueEx(hKey, "ProcessAssistant", 0, REG_SZ, (BYTE *)myName, dwRet);
                RegCloseKey(hKey);
                if (lRet != ERROR_SUCCESS)
                {
                    MessageBox("ϵͳ��������,������ɿ�����������", "��ܰ��ʾ");
                }
                else
                {
                    MessageBox("�ѳɹ����øó��򿪻���������", "��ܰ��ʾ", MB_ICONINFORMATION);
                }
            }
            else
            {
                CString err;
                err.Format("lastError:%d(lRet:%d)", GetLastError(), lRet);
                MessageBox("��ע�����������Ŀ¼ʧ��!" + err, "ʧ����ʾ");
            }
            break;
        }
        case 1: //�ڱ���������ʱִ��һ��, �Զ����ϴ�δ�رյĽ���
        {
            KillTimer(1);
            ifstream fin(autoRunFile);
            if (fin.is_open()){
                string process;
                while (getline(fin, process)){
                    if (!isProcessExist(process.c_str()))
                        ShellExecute(0, "open", process.c_str(), 0, 0, SW_SHOW);
                }
            }
            fin.close();
            DeleteFile(autoRunFile);
            break;
        }
        case 2: //ÿ��һ��ʱ���ִ��һ��, �鿴���Ľ����Ƿ�������
        {
            CString lists;
            for (auto& elem : m_runList){
                if (isProcessExist(elem)){
                    lists += elem + "\n";
                }
            }
            if (lists != ""){
                ofstream fout(autoRunFile);
                fout << lists;
                fout.close();
            }
            else{
                DeleteFile(autoRunFile);
            }
            break;
        }
        default:
            break;
    }
    CDialog::OnTimer(nIDEvent);
}


//��һ�ַ�ʽ��ȡ�����б�
#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")
void enumProcesses()
{
    int cnt = 0;
    DWORD PID[1024];
    DWORD needed;
    EnumProcesses(PID, sizeof(PID), &needed);
    DWORD NumProcess = needed / sizeof(DWORD);
    char exePath[MAX_PATH];

    for (DWORD i = 0, cnt = 0; i < NumProcess; i++, cnt++)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, PID[i]);
        if (hProcess)
        {
            GetModuleFileNameEx(hProcess, NULL, exePath, sizeof(exePath));
            HICON hIcon = ExtractIcon(AfxGetInstanceHandle(), exePath, 0); //��ó���ͼ��
            CString path = exePath, exeName = path.Right(path.GetLength() - path.ReverseFind('/'));
            DestroyIcon(hIcon); //����ͼ��
        }
        CloseHandle(hProcess);
    }
}
