
// ProcessAssistantDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ProcessAssistant.h"
#include "ProcessAssistantDlg.h"
#include <afxdialogex.h>

#include <tlhelp32.h>
#include <psapi.h>
#pragma comment(lib,"Psapi.lib")

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

    m_autoRunFile = m_myPath + "autorun.txt";
}

CProcessAssistantDlg::~CProcessAssistantDlg()
{
    Shell_NotifyIcon(NIM_DELETE, &nid);
    ReleaseMutex(hmutex);
}

void CProcessAssistantDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_PROCESS, m_wndList);
}


#define WM_NOTIFYICONMSG WM_USER + 1 //������Ϣ

BEGIN_MESSAGE_MAP(CProcessAssistantDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_TIMER()
    ON_WM_CLOSE()
    ON_MESSAGE(WM_NOTIFYICONMSG, OnNotifyIconMsg)
    ON_BN_CLICKED(IDOK, &CProcessAssistantDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDC_BT_SET, &CProcessAssistantDlg::OnBnClickedBtSet)
    ON_COMMAND(ID_OPEN_MAIN_DLG, &CProcessAssistantDlg::OnOpenMainDlg)
    ON_COMMAND(ID_EXIT_ME, &CProcessAssistantDlg::OnExitMe)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST_PROCESS, &CProcessAssistantDlg::OnDblclkListProcess)
END_MESSAGE_MAP()


// CProcessAssistantDlg ��Ϣ�������

BOOL CProcessAssistantDlg::OnInitDialog()
{
    //���û��������Ʊ�֤���ֻ��һ����ʵ����������
    hmutex = ::CreateMutex(NULL, true, "ProcessAssistant");
    if (ERROR_ALREADY_EXISTS == GetLastError()) { //���������Ѵ�����ֱ�ӹر�
        MessageBox("�벻Ҫ�ظ��򿪸ó���Ŷ,�ҵ�ǰ����֪ͨ����������^ _ ^", 0, MB_ICONWARNING);
        OnCancel();
    }
    CDialog::OnInitDialog();
    // ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�ִ�д˲���
    SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
    SetIcon(m_hIcon, FALSE);		// ����Сͼ��

    //����ѡ�� + ��ʾ����� + ��ѡ�� + ��ƽ������
    m_wndList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES | LVS_EX_FLATSB);
    //���ñ�ͷ
    m_wndList.InsertColumn(0, "(����)   ������", 0, 124);
    m_wndList.InsertColumn(1, "�ļ�λ��", 0, 415);
    m_wndList.InsertColumn(2, "��ע", 0, 50);
    //�����ı���ʾ��ɫ
    m_wndList.SetTextColor(RGB(0, 255, 0));

    m_listCnt = 0;
    m_imgList.Create(32, 32, ILC_COLOR32, 0, 100);
    m_wndList.SetImageList(&m_imgList, LVSIL_SMALL);

    SetTimer(0, 100, NULL);  //��鵱ǰ����������еĽ���,�����������
    SetTimer(1, 1000, NULL); //�鿴���Ľ����Ƿ�������
    openUnclosedProcess();   //���ϴιػ�ʱδ�رյĽ���

    //����������Ϣ - ���������︳ֵ������ڹ��캯����ֵ�����ָ������ͼ�����ʧ
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = m_hWnd;
    nid.uFlags = NIF_INFO | NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.dwInfoFlags = NIIF_INFO;
    nid.hIcon = m_hIcon;
    nid.uCallbackMessage = WM_NOTIFYICONMSG;
    strcpy(nid.szTip, "��������");
    strcpy(nid.szInfoTitle, "����������פ����֪ͨ��");//���ݱ���
    strcpy(nid.szInfo, "�ҽ����������������õĳ����Ƿ������У������Ե��"
           "�����ڵĹرգ��һ���һֱ�����������ţ���������ر��ң�"
           "�����Ҽ������ң�Ȼ��ѡ���˳�");//��������
    Shell_NotifyIcon(NIM_ADD, &nid);

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

HCURSOR CProcessAssistantDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CProcessAssistantDlg::OnTimer(UINT_PTR nIDEvent)
{
    switch (nIDEvent)
    {
        case 0: //�ڱ���������ʱִ��һ��, ��鵱ǰ����������еĽ���,�����������
        {
            KillTimer(0);
            showProcessList();
            //�õ����ϵ�һ�����иó���ʱĬ������Ϊ������
            bool firstRun = false;
            ifstream fin(m_myPath + "AutoRunProcessList.txt");
            if (!fin.is_open())
                firstRun = true;
            fin.close();
            if (firstRun){
                setDlg.setStartup(1);
                //��֤���ļ���֮��һֱ����
                ofstream fout(m_myPath + "AutoRunProcessList.txt", ios::out | ios::app);
                fout.close();
                MessageBox("��ѡ��Ӧ������ߵ�ѡ�򼴿ɿ���������������ػ�ʱ"
                           "��Щ�������δ�رգ��´ο��������д˳����Զ�����"
                           "��Ӧ���̣�", "������ʾ", MB_ICONINFORMATION);
            }
            break;
        }
        case 1: //ÿ��һ��ʱ���ִ��һ��, �鿴���Ľ����Ƿ�������
        {
            updateProcessList();
            break;
        }
        default:
            break;
    }
    CDialog::OnTimer(nIDEvent);
}

void CProcessAssistantDlg::OnBnClickedOk()
{
    m_runList.clear();
    ofstream fout(m_myPath + "AutoRunProcessList.txt");
    for (int i = 0; i < m_listCnt; ++i)
        if (m_wndList.GetCheck(i)) {
            fout << m_wndList.GetItemText(i, 1) << endl;
            m_runList.insert(m_wndList.GetItemText(i, 1));
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

void CProcessAssistantDlg::OnBnClickedBtSet()
{
    setDlg.DoModal();
}

void CProcessAssistantDlg::OnOpenMainDlg()
{
    ShowWindow(SW_SHOW);
}

void CProcessAssistantDlg::OnExitMe()
{
    PostQuitMessage(0);
}

void CProcessAssistantDlg::OnClose()
{
    ShowWindow(SW_HIDE);
    strcpy(nid.szInfoTitle, "�������������ص�֪ͨ��");//���ݱ���
    strcpy(nid.szInfo, "�ҽ����������������õĳ����Ƿ������У�������ػ���"
           "ʱ��û�йر����ǣ��ҽ����´ο���ʱΪ���˴�����Ŷ");//��������
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

LRESULT CProcessAssistantDlg::OnNotifyIconMsg(WPARAM wParam, LPARAM lParam)
{
    CPoint Point;
    CMenu pMenu;//���ز˵�
    switch (lParam) {
        case WM_RBUTTONDOWN: //�����������ҽ�
            if (pMenu.LoadMenu(IDR_MENU1)) {
                CMenu* pPopup = pMenu.GetSubMenu(0);
                GetCursorPos(&Point);
                SetForegroundWindow();
                pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, Point.x, Point.y, this);
            }
            break;
        case WM_LBUTTONDBLCLK:
            this->ShowWindow(SW_SHOW);
            break;
        default:
            break;
    }
    return 0;
}

void CProcessAssistantDlg::OnDblclkListProcess(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    int item = pNMItemActivate->iItem;
    if (item >= 0 && item <= m_wndList.GetItemCount())
        m_wndList.SetCheck(item, !m_wndList.GetCheck(item));
    *pResult = 0;
}

// CProcessAssistantDlg �Զ��庯��

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

void CProcessAssistantDlg::showProcessList()
{
    loadTaskFileList();
    loadTaskMgrList();
}

void CProcessAssistantDlg::loadTaskFileList()
{
    ifstream fin(m_myPath + "AutoRunProcessList.txt");
    if (fin.is_open()){
        string process;
        while (getline(fin, process)){
            HICON hIcon = ExtractIcon(AfxGetInstanceHandle(), process.c_str(), 0);
            int indexIcon = m_imgList.Add(hIcon);
            CString exeName = process.substr(process.rfind('\\') + 1).c_str();
            int item = m_wndList.InsertItem(m_listCnt++, exeName.Left(exeName.GetLength() - 4), indexIcon); //����һ��
            m_wndList.SetItemText(item, 1, process.c_str());
            m_wndList.SetCheck(item);
            if (isProcessExist(process.c_str()))
                m_wndList.SetItemText(item, 2, "������");
            m_processListMap[process.c_str()] = exeName; //����map
            m_processIndexMap[process.c_str()] = m_listCnt - 1;
            m_runList.insert(process.c_str());
        }
    }
    fin.close();
}

void CProcessAssistantDlg::loadTaskMgrList()
{
    static char myName[MAX_PATH] = { 0 }; //������Ľ�����
    static int unused1 = GetModuleFileName(NULL, myName, MAX_PATH);
    static char unused2 = myName[0] = toupper(myName[0]); //�����̷���Сд��ʶ������
    HANDLE hp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hp == INVALID_HANDLE_VALUE){
        CString err;
        err.Format("%d", GetLastError());
        MessageBox("�������̿���ʧ�ܣ�" + err, "��ܰ��ʾ");
        return;
    }
    PROCESSENTRY32 pe32 = { sizeof(pe32) };
    for (BOOL find = Process32First(hp, &pe32); find != 0; find = Process32Next(hp, &pe32)) {
        HANDLE hd = OpenProcess(PROCESS_ALL_ACCESS, 0, pe32.th32ProcessID);
        if (hd != NULL) {
            char exePath[255];
            GetModuleFileNameEx(hd, NULL, exePath, 255);
            exePath[0] = toupper(exePath[0]); //�����̷���Сд��ʶ������
            //��ó���ͼ��
            HICON hIcon = ExtractIcon(AfxGetInstanceHandle(), exePath, 0);
            if (hIcon != NULL && m_processListMap.find(exePath) == m_processListMap.end()){
                CString path = exePath, exeName = path.Right(path.GetLength() - path.ReverseFind('\\') - 1);
                if (path == myName || path.Find("system32") != -1 || path.Find("SysWOW64") != -1)
                    continue;
                int indexIcon = m_imgList.Add(hIcon);
                int item = m_wndList.InsertItem(m_listCnt++, exeName.Left(exeName.GetLength() - 4), indexIcon); //����һ��
                m_wndList.SetItemText(item, 1, exePath);
                m_wndList.SetItemText(item, 2, "������");
                m_processListMap[exePath] = exeName; //����map
                m_processIndexMap[exePath] = m_listCnt - 1;
            }
            DestroyIcon(hIcon); //����ͼ��
            CloseHandle(hd);
        }
    }
    CloseHandle(hp);
}

void CProcessAssistantDlg::openUnclosedProcess()
{
    ifstream fin(m_autoRunFile);
    if (fin.is_open()){
        string process;
        while (getline(fin, process)){
            if (!isProcessExist(process.c_str())){
                if ((int)ShellExecute(0, "open", process.c_str(), 0, 0, SW_SHOW) < 32){
                    CString err;
                    err.Format("��%sʧ��!lastError:%d", process.c_str(),
                               GetLastError());
                    MessageBox(err, "ʧ����ʾ");
                }
            }
        }
    }
    fin.close();
    DeleteFile(m_autoRunFile);
}

void CProcessAssistantDlg::updateProcessList()
{
    //ɾ�����˳��Ľ��̵��кͼ�¼
    for (auto it = m_processListMap.begin(); it != m_processListMap.end();){
        auto itTmp = it++;
        CString path = itTmp->first;
        if (!isProcessExist(itTmp->second)){ //�ý����Ѳ�����
            if (m_runList.count(itTmp->first) == 0){ //�����������б���,����Ҫɾ��
                for (auto& elem : m_processIndexMap)  //���ý�������Ľ�����ż�һ
                    if (elem.second > m_processIndexMap[path])
                        --elem.second;
                m_wndList.DeleteItem(m_processIndexMap[path]); //ɾ����
                m_processIndexMap.erase(m_processIndexMap.find(path));
                m_processListMap.erase(itTmp); //�ӽ����б�ӳ�����ɾ��
            }
            else{ //���̲����ڵ����������б���,ֻ��Ҫ��״̬�޸�
                if (m_wndList.GetItemText(m_processIndexMap[path], 2) != "")
                    m_wndList.SetItemText(m_processIndexMap[path], 2, "");
            }
        }
    }
    //����ÿ�����̵�״̬
    m_listCnt = m_wndList.GetItemCount();
    for (int i = 0; i < m_listCnt; ++i){
        if (isProcessExist(m_wndList.GetItemText(i, 1))){
            if (m_wndList.GetItemText(i, 2) != "������")
                m_wndList.SetItemText(i, 2, "������");
        }
        else if (m_wndList.GetItemText(i, 2) != "")
            m_wndList.SetItemText(i, 2, "");
    }
    //��������������е������б�
    loadTaskMgrList();
    //���������е���Ҫ�������Ľ��̱��浽�ļ���,�Թ��´ο������Զ���
    CString lists;
    for (auto& elem : m_runList)
        if (isProcessExist(elem))
            lists += elem + "\n";
    if (lists != ""){
        ofstream fout(m_autoRunFile);
        fout << lists;
        fout.close();
    }
    else
        DeleteFile(m_autoRunFile);
}



///��һ�ֻ�ȡ�����б�ķ�ʽ
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
