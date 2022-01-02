#include <windows.h>
#include <iostream>
#include<TCHAR.H>
using namespace std;
HANDLE hSerial;
HANDLE hSerial2;
int _tmain(int argc, _TCHAR* argv[])
{
    //��� ��� ��������� ����� � ������ ����, ��� �� ����� �� �������, ������� �� ��������� ��� ������� ��� �����-������ �������
    LPCTSTR sPortName = L"\\\\.\\COM1";
    hSerial = ::CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            cout << "serial port does not exist.\n";
        }
        cout << "some other error occurred.\n";
    }
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        cout << "getting state error\n";
    }
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        cout << "error setting serial port state\n";
    }
    hSerial2 = ::CreateFile(L"\\\\.\\COM2", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hSerial2 == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            cout << "serial port does not exist.\n";
        }
        cout << "some other error occurred.\n";
    }
    //������ ����� ���������� ����������
    char data[] = "D";  //���� ��������
    DWORD dwSize = sizeof(data);   // ������ ���� ������
    DWORD dwBytesWritten;    // ��� ����� ���������� ���������� ���������� ����
    BOOL iRet = WriteFile(hSerial, data, dwSize, &dwBytesWritten, NULL);
    cout << dwSize << " Bytes in string. " << dwBytesWritten << " Bytes sended. " << endl;
    DWORD iSize;
    char sReceivedChar;
    ReadFile(hSerial2, &sReceivedChar, 1, &iSize, 0);  // �������� 1 ����
    if (iSize > 0)   cout << sReceivedChar;
    return 0;
}