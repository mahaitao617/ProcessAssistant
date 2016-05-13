
// ProcessAssistantDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ProcessAssistant.h"
#include "ProcessAssistantDlg.h"
#include <afxdialogex.h>

#include <tlhelp32.h>
#include <psapi.h>
#pragma comment(lib,"psapi.lib")

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
    m_listCnt = 0;

    char path[MAX_PATH] = "";
    GetTempPath(MAX_PATH, path);
    CString myPath = path + CString("ProcessAssistant\\");
    CreateDirectory(myPath, 0); //����ʱ�ļ����д�����Ӧ�õ��ļ���

    m_autoRunListFile = myPath + "autorun.dat";
    m_checkedListFile = myPath + "checkedList.dat";

    pMapRunning = NULL;
}

CProcessAssistantDlg::~CProcessAssistantDlg()
{
    Shell_NotifyIcon(NIM_DELETE, &m_nid);
    ReleaseMutex(m_hmutex);
}

void CProcessAssistantDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_PROCESS, m_wndList);
}


#define WM_NOTIFYICONMSG WM_USER + 1 //������Ϣ
#define ID_HK_OPEN_MAIN_DLG 0 //ȫ�ֿ�ݼ�-�������ڵ�ID

BEGIN_MESSAGE_MAP(CProcessAssistantDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_TIMER()
    ON_WM_CLOSE()
    ON_WM_HOTKEY()
    ON_MESSAGE(WM_NOTIFYICONMSG, OnNotifyIconMsg)
    ON_BN_CLICKED(IDOK, &CProcessAssistantDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDC_BT_SET, &CProcessAssistantDlg::OnBnClickedBtSet)
    ON_COMMAND(ID_OPEN_MAIN_DLG, &CProcessAssistantDlg::OnOpenMainDlg)
    ON_COMMAND(ID_EXIT_ME, &CProcessAssistantDlg::OnExitMe)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST_PROCESS, &CProcessAssistantDlg::OnDblclkListProcess)
    ON_NOTIFY(NM_RCLICK, IDC_LIST_PROCESS, &CProcessAssistantDlg::OnNM_RClickListProcess)
    ON_COMMAND(ID_OPEN_DIR, &CProcessAssistantDlg::OnRClick_OpenDir)
    ON_COMMAND(ID_START_PROCESS, &CProcessAssistantDlg::OnRClick_StartProcess)
    ON_COMMAND(ID_END_PROCESS, &CProcessAssistantDlg::OnRClick_EndProcess)
    ON_COMMAND(ID_CHECK_OR_NOT_PROCESS, &CProcessAssistantDlg::OnRClick_CheckOrNotProcess)
END_MESSAGE_MAP()

///���ù����ڴ淽ʽʵ�ֽ���ͨ�š������ڴ�ʵ�ʾ����ļ�ӳ���һ���������

//�����ļ�ӳ�䣬�൱�ڴ���ͨ���ŵ�.�����ɹ�����0
int createMyFileMap(void* &lp, size_t size, const char* str)
{
    HANDLE h = CreateFileMapping((HANDLE)0xFFFFFFFF, 0, PAGE_READWRITE, 0, size, str);
    if (h == NULL)
    {
        MessageBox(0, "Create File Mapping Faild", str, 0);
        return -1;
    }
    lp = MapViewOfFile(h, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, size);
    if (lp == NULL)
    {
        MessageBox(0, "View MapFile Faild-c", str, 0);
        return -2;
    }
    return 0;
}

//���ļ�ӳ�䣬�൱�ڷ����ŵ����򿪳ɹ�����0
int openMyFileMap(void* &lp, size_t size, const char* str, bool showBox = 1)
{
    HANDLE h = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, 0, str);
    if (h == NULL)
    {
        if (showBox) MessageBox(0, "Open File Mapping Faild", str, 0);
        return -1;
    }
    lp = MapViewOfFile(h, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, size);
    if (lp == NULL)
    {
        if (showBox) MessageBox(0, "View MapFile Faild-o", str, 0);
        return -2;
    }
    return 0;
}

//�����ļ�ӳ�䣬�൱�����ŵ�������Ϣ
template<typename T_Set>
int setMyFileMap(void* &lp, size_t size, const T_Set* pValue)
{
    if (lp == 0 || pValue == 0 || size == 0)
        return -1;
    memcpy(lp, pValue, size);
    return 0;
}

//���½����б��̵߳ĺ���
void updateListThread(void * _pDlg)
{
    CProcessAssistantDlg* pDlg = static_cast<CProcessAssistantDlg*>(_pDlg);
    while (pDlg){
        Sleep(3000);
        pDlg->updateProcessList();
    }
}

// CProcessAssistantDlg ��Ϣ�������

BOOL CProcessAssistantDlg::OnInitDialog()
{
    //���û��������Ʊ�֤���ֻ��һ����ʵ����������
    m_hmutex = CreateMutex(NULL, true, "ProcessAssistant");
    if (ERROR_ALREADY_EXISTS == GetLastError()) { //���������Ѵ�����ֱ�ӹر�
        if (0 == openMyFileMap(pMapRunning, 4, "NewInstance", 0)){
            int val = 1;
            setMyFileMap(pMapRunning, 4, &val);
        }
        SetTimer(2, 3000, NULL); //3����Զ��ر�
        MessageBox("�벻Ҫ�ظ��򿪸ó���Ŷ���ҵ�ǰ����֪ͨ����������^ _ ^",
                   0, MB_ICONWARNING);
        OnCancel();
        return FALSE;
    }
    createMyFileMap(pMapRunning, 4, "NewInstance"); //���������ڴ�

    CDialog::OnInitDialog();
    // ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�ִ�д˲���
    SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
    SetIcon(m_hIcon, FALSE);		// ����Сͼ��

    //����ѡ�� + ��ʾ����� + ��ѡ�� + ��ƽ������
    m_wndList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_FLATSB);
    //���ñ�ͷ
    m_wndList.InsertColumn(0, "(����)   ������", 0, 124);
    m_wndList.InsertColumn(1, "�ļ�λ��", 0, 415);
    m_wndList.InsertColumn(2, "��ע", 0, 50);
    //�����ı���ʾ��ɫ
    m_wndList.SetTextColor(RGB(0, 0, 0));
    //����ͼ���б�
    m_iconList.Create(32, 32, ILC_COLOR32, 0, 100);
    m_wndList.SetImageList(&m_iconList, LVSIL_SMALL);

    SetTimer(0, 100, NULL); //��ʾ�����б�,���ϴ�δ�رյĽ���,�����������
    SetTimer(1, 500, NULL);

    _beginthread(updateListThread, 0, this); //������̨�̶߳��ڼ����Ӧ�����Ƿ�������

    //����������Ϣ - ���������︳ֵ,����ڹ��캯����ֵ,���ָ������ͼ�����ʧ
    m_nid.cbSize = sizeof(NOTIFYICONDATA);
    m_nid.hWnd = m_hWnd;
    m_nid.uFlags = NIF_INFO | NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.dwInfoFlags = NIIF_INFO;
    m_nid.hIcon = m_hIcon;
    m_nid.uCallbackMessage = WM_NOTIFYICONMSG;
    strcpy(m_nid.szTip, "��������");
    strcpy(m_nid.szInfoTitle, "����������פ����֪ͨ��");//���ݱ���
    strcpy(m_nid.szInfo, "�ҽ����������������õĳ����Ƿ������У�������ػ���"
           "ʱ��û�йر����ǣ��ҽ����´ο���ʱΪ���˴�����Ŷ�������Ե��"
           "�����ڵĹرգ����һ���һֱ������������");//��������
    Shell_NotifyIcon(NIM_ADD, &m_nid);

    //����ȫ�ֿ�ݼ�: Ctrl+Shift+D ��������
    RegisterHotKey(GetSafeHwnd(), ID_HK_OPEN_MAIN_DLG, MOD_CONTROL | MOD_SHIFT, 'D');

    return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

BOOL CProcessAssistantDlg::PreTranslateMessage(MSG* pMsg)
{
    static HACCEL hAccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR1));
    if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
    {
        if (hAccel && TranslateAccelerator(m_hWnd, hAccel, pMsg))
            return TRUE;
    }
    return CDialog::PreTranslateMessage(pMsg);
}

HCURSOR CProcessAssistantDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
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

void CProcessAssistantDlg::OnTimer(UINT_PTR nIDEvent)
{
    switch (nIDEvent)
    {
        case 0: //�ڱ���������ʱִ��һ��,��鵱ǰ����������еĽ���,�����������
        {
            KillTimer(0);
            showProcessList();
            //�õ����ϵ�һ�����иó���ʱĬ������Ϊ������
            bool firstRun = false;
            ifstream fin(m_checkedListFile);
            if (!fin.is_open())
                firstRun = true;
            fin.close();
            if (firstRun){
                m_setDlg.setStartup(1);
                //��֤���ļ���֮��һֱ����
                ofstream fout(m_checkedListFile, ios::out | ios::app);
                fout.close();
                MessageBox("��ѡ��Ӧ������ߵ�ѡ�򼴿ɿ���������������ػ�ʱ"
                           "��Щ�������δ�رգ��´ο��������д˳����Զ�����"
                           "��Ӧ���̣�", "������ʾ", MB_ICONINFORMATION);
            }
            openUnclosedProcess(); //���ϴιػ�ʱδ�رյĽ���
            break;
        }
        case 1:
        {
            if (pMapRunning && *(int*)pMapRunning == 1){
                int val = 0;
                setMyFileMap(pMapRunning, 4, &val);
                strcpy(m_nid.szInfoTitle, "��ܰ��ʾ");//���ݱ���
                strcpy(m_nid.szInfo, "���ˣ��һ��������������أ��������Ұ�");
                Shell_NotifyIcon(NIM_MODIFY, &m_nid);
            }
            break;
        }
        case 2:
        {
            exit(-1);
            break;
        }
        default:
            break;
    }
    CDialog::OnTimer(nIDEvent);
}

void CProcessAssistantDlg::OnBnClickedOk()
{
    m_checkedList.clear();
    ofstream fout(m_checkedListFile);
    for (int i = 0; i < m_wndList.GetItemCount(); ++i)
        if (m_wndList.GetCheck(i)){
            fout << m_wndList.GetItemText(i, 1) << endl;
            m_checkedList.insert(m_wndList.GetItemText(i, 1));
        }
    fout.close();

    CString selected;
    for (auto& elem : m_checkedList)
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
    m_setDlg.DoModal();
}

void CProcessAssistantDlg::OnOpenMainDlg()
{
    ShowWindow(SW_SHOW);
    SetFocus();
}

void CProcessAssistantDlg::OnExitMe()
{
    OnOpenMainDlg();
    if (MessageBox("�˳��󽫲��ܼ��ָ�����������״̬�����ܾ��޷�����´ο���"
        "����ϴ�δ�رյĽ�����Ŷ������ȷ��Ҫ���̵Ĺر���ô��", "�˳�����",
        MB_YESNO | MB_ICONQUESTION) == IDYES)
        PostQuitMessage(0);
}

void CProcessAssistantDlg::OnClose()
{
    ShowWindow(SW_HIDE);
    strcpy(m_nid.szInfoTitle, "�������������ص�֪ͨ��");//���ݱ���
    strcpy(m_nid.szInfo, "�ҽ����������������õĳ����Ƿ������У�������"
           "��ر��ң������Ҽ������ң�Ȼ��ѡ���˳�");//��������
    Shell_NotifyIcon(NIM_MODIFY, &m_nid);
}

LRESULT CProcessAssistantDlg::OnNotifyIconMsg(WPARAM wParam, LPARAM lParam)
{
    switch (lParam) {
        case WM_RBUTTONDOWN: //�����������ҽ�
        {
            CMenu pMenu;//���ز˵�
            if (pMenu.LoadMenu(IDR_MENU1)) {
                CPoint pt;
                GetCursorPos(&pt);
                SetForegroundWindow();
                CMenu* pPopup = pMenu.GetSubMenu(0);
                pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
            }
            break;
        }
        case WM_LBUTTONDBLCLK:
        {
            OnOpenMainDlg();
            break;
        }
        default:
            break;
    }
    return 0;
}

void CProcessAssistantDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
    if (nHotKeyId == ID_HK_OPEN_MAIN_DLG)
        OnOpenMainDlg();
    CDialog::OnHotKey(nHotKeyId, nKey1, nKey2);
}

void CProcessAssistantDlg::OnDblclkListProcess(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    int item = pNMItemActivate->iItem;
    if (item >= 0 && item <= m_wndList.GetItemCount())
        m_wndList.SetCheck(item, !m_wndList.GetCheck(item));
    *pResult = 0;
}

void CProcessAssistantDlg::OnNM_RClickListProcess(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    m_selected = pNMItemActivate->iItem;
    if (m_selected >= 0 && m_selected <= m_wndList.GetItemCount()){
        int subIndex = (m_wndList.GetItemText(m_selected, 2) == "") ? 1 : 2;
        CMenu pMenu; //���ز˵�
        if (pMenu.LoadMenu(IDR_MENU1)) {
            CPoint pt;
            GetCursorPos(&pt);
            CMenu* pPopup = pMenu.GetSubMenu(subIndex);
            pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
        }
    }
    *pResult = 0;
}

void CProcessAssistantDlg::OnRClick_OpenDir()
{
    CString path = m_wndList.GetItemText(m_selected, 1);
    path = "/e,/select, " + path; //ͨ���������ʵ��ѡ����Ӧ�ļ�
    ShellExecute(m_hWnd, "open", "explorer", path, 0, SW_SHOW); //���ļ���
}

void CProcessAssistantDlg::OnRClick_StartProcess()
{
    CString path = m_wndList.GetItemText(m_selected, 1);
    ShellExecute(m_hWnd, "open", path, 0, 0, SW_SHOW);
    updateProcessList(); //����ˢ���б�
}

void CProcessAssistantDlg::OnRClick_EndProcess()
{
    CString exeName = m_wndList.GetItemText(m_selected, 0) + ".exe";
    HANDLE hd_ths = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hd_ths == INVALID_HANDLE_VALUE)
        return;
    PROCESSENTRY32 pe32 = { sizeof(pe32) };
    //�������������еĽ��̵���Ϣ��������ǰ����Ϣ���浽����pe32��
    for (BOOL find = Process32First(hd_ths, &pe32); find != 0; find = Process32Next(hd_ths, &pe32)) {
        if (pe32.szExeFile == exeName){
            HANDLE hd_op = OpenProcess(PROCESS_ALL_ACCESS, 0, pe32.th32ProcessID);
            TerminateProcess(hd_op, 0); //��������
            CloseHandle(hd_op);
            updateProcessList(); //����ˢ���б�
            break;
        }
    }
    CloseHandle(hd_ths);
}

void CProcessAssistantDlg::OnRClick_CheckOrNotProcess()
{
    if (m_selected >= 0 && m_selected <= m_wndList.GetItemCount())
        m_wndList.SetCheck(m_selected, !m_wndList.GetCheck(m_selected));
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
    loadListFromTaskFile();
    loadListFromTaskMgr();
}

void CProcessAssistantDlg::loadListFromTaskFile()
{
    m_listCnt = 0;
    ifstream fin(m_checkedListFile);
    if (fin.is_open()){
        string process;
        while (getline(fin, process)){
            if (process == "" || !PathFileExists(process.c_str()))
                break;
            HICON hIcon = ExtractIcon(AfxGetInstanceHandle(), process.c_str(), 0);
            int indexIcon = m_iconList.Add(hIcon);
            CString exeName = process.substr(process.rfind('\\') + 1).c_str(); //���õ��ļ���
            int item = m_wndList.InsertItem(m_listCnt++, exeName.Left(exeName.GetLength() - 4), indexIcon);
            m_wndList.SetItemText(item, 1, process.c_str());
            m_wndList.SetCheck(item);
            if (isProcessExist(process.c_str()))
                m_wndList.SetItemText(item, 2, "������");
            m_processList.insert(process.c_str()); //����map
            m_processIndexMap[process.c_str()] = m_listCnt - 1;
            m_checkedList.insert(process.c_str());
        }
    }
    fin.close();
}

void CProcessAssistantDlg::loadListFromTaskMgr()
{
    m_listCnt = m_wndList.GetItemCount();
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
    //�������������еĽ��̵���Ϣ��������ǰ����Ϣ���浽����pe32��
    for (BOOL find = Process32First(hp, &pe32); find != 0; find = Process32Next(hp, &pe32)) {
        HANDLE hd = OpenProcess(PROCESS_ALL_ACCESS, 0, pe32.th32ProcessID);
        if (hd != NULL) {
            char exePath[255];
            GetModuleFileNameEx(hd, NULL, exePath, 255);
            exePath[0] = toupper(exePath[0]); //�����̷���Сд��ʶ������
            if (m_processList.count(exePath) == 0){
                //��ó���ͼ��
                HICON hIcon = ExtractIcon(AfxGetInstanceHandle(), exePath, 0);
                if (hIcon != NULL){ //·����Ч������ͼ��
                    CString path = exePath, exeName = pe32.szExeFile;
                    if (path == myName || path.Find("system32") != -1 || path.Find("SysWOW64") != -1){
                        DestroyIcon(hIcon); //����ͼ��
                        CloseHandle(hd);
                        continue;
                    }
                    int indexIcon = m_iconList.Add(hIcon);
                    int item = m_wndList.InsertItem(m_listCnt++, exeName.Left(exeName.GetLength() - 4), indexIcon); //����һ��
                    m_wndList.SetItemText(item, 1, exePath);
                    m_wndList.SetItemText(item, 2, "������");
                    m_processList.insert(exePath); //����ü�¼
                    m_processIndexMap[exePath] = m_listCnt - 1;
                    DestroyIcon(hIcon); //����ͼ��
                }
            }
            CloseHandle(hd);
        }
    }
    CloseHandle(hp);
}

void CProcessAssistantDlg::openUnclosedProcess()
{
    ifstream fin(m_autoRunListFile);
    if (fin.is_open()){
        string process;
        while (getline(fin, process)){
            if (!isProcessExist(process.c_str())){
                if ((int)ShellExecute(0, "open", process.c_str(), 0, 0, SW_SHOW) < 32){
                    CString err;
                    err.Format("��%sʧ��! lastError:%d", process.c_str(),
                               GetLastError());
                    MessageBox(err, "ʧ����ʾ", MB_ICONERROR);
                }
            }
        }
    }
    fin.close();
    DeleteFile(m_autoRunListFile);
}

void CProcessAssistantDlg::updateProcessList()
{
    //ɾ�����˳��Ľ��̵��кͼ�¼
    for (auto it = m_processList.begin(); it != m_processList.end();){
        auto itTmp = it++;
        int idx = m_processIndexMap[*itTmp]; //��ǰ��¼�����е�����
        if (!isProcessExist(*itTmp)){ //�ý����Ѳ�����
            if (m_checkedList.count(*itTmp) == 0){ //�����������б���,����Ҫɾ��
                for (auto& elem : m_processIndexMap) //���ý�������Ľ�����ż�һ
                    if (elem.second > idx)
                        --elem.second;
                m_wndList.DeleteItem(idx); //ɾ����
                m_processIndexMap.erase(m_processIndexMap.find(*itTmp));
                m_processList.erase(itTmp); //�ӽ����б�ӳ�����ɾ��
            } //���̲����ڵ����������б���,ֻ��Ҫ��״̬�޸�
            else if (m_wndList.GetItemText(idx, 2) != "")
                m_wndList.SetItemText(idx, 2, "");
        }
        else if (m_checkedList.count(*itTmp) != 0){ //���������б���
            if (m_wndList.GetItemText(idx, 2) != "������")
                m_wndList.SetItemText(idx, 2, "������");
        }
    }
    //��������������е������б�
    loadListFromTaskMgr();
    //���������е���Ҫ�������Ľ��̱��浽�ļ���,�Թ��´ο������Զ���
    CString listsToRun;
    static CString lastLists;
    for (auto& elem : m_checkedList)
        if (isProcessExist(elem))
            listsToRun += elem + "\n";
    if (listsToRun != lastLists){
        if (listsToRun.IsEmpty())
            DeleteFile(m_autoRunListFile);
        else{
            ofstream fout(m_autoRunListFile);
            fout << listsToRun;
            fout.close();
        }
        lastLists = listsToRun;
    }
}


///��һ�ֻ�ȡ�����б�ķ�ʽ
#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")
void enumProcesses()
{
    int cnt = 0;
    DWORD pid[1024], needed;
    EnumProcesses(pid, sizeof(pid), &needed);
    DWORD numProcess = needed / sizeof(DWORD);
    char exePath[MAX_PATH];
    for (DWORD i = 0; i < numProcess; i++)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid[i]);
        if (hProcess != NULL)
        {
            GetModuleFileNameEx(hProcess, NULL, exePath, sizeof(exePath));
            HICON hIcon = ExtractIcon(AfxGetInstanceHandle(), exePath, 0); //��ó���ͼ��
            CString path = exePath, exeName = path.Right(path.GetLength() - path.ReverseFind('/'));
            DestroyIcon(hIcon); //����ͼ��
        }
        CloseHandle(hProcess);
    }
}
