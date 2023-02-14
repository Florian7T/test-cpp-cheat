#include <windows.h>
#include <iostream>
#include <tlhelp32.h>
#include <vector>
#include "psapi.h"

using namespace std;

// g++ cheat.cpp -lpsapi -static -static-libgcc -o cheat.o

DWORD64 FindDMAAddy(HANDLE hProc, DWORD64 ptr, std::vector<unsigned int> offsets)
{
    DWORD64 addr = ptr;
    for (unsigned int i = 0; i < offsets.size(); ++i)
    {
        ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
        addr += offsets[i];
    }
    return addr;
}

DWORD64 GetModuleBase(const wchar_t* ModuleName, DWORD procID)
{
    MODULEENTRY32W ModuleEntry = { 0 };
    HANDLE SnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procID);
    if (!SnapShot) return NULL;
    ModuleEntry.dwSize = sizeof(ModuleEntry);
    if (!Module32FirstW(SnapShot, &ModuleEntry)) return NULL;
    do
    {
        if (!wcscmp(ModuleEntry.szModule, ModuleName))
        {
            CloseHandle(SnapShot);
            return (DWORD64)ModuleEntry.modBaseAddr;
        }
    } while (Module32NextW(SnapShot, &ModuleEntry));
    CloseHandle(SnapShot);
    return NULL;
}


// =================== PTR ===================
DWORD64 BASE_ADDRESS = 0;

DWORD64 ptr_Health = 0x31FAFB0;
vector<unsigned int> off_Health = {0x58,0x92C};
DWORD64 final_Health = 0;

DWORD64 ptr_Ammo = 0x31FAF90;
vector<unsigned int> off_Ammo = {0x58,0x8D8};
DWORD64 final_Ammo = 0;

// =================== VAL ===================

int maxHealth = 2120403456; //1120403456
int maxAmmo = 9999;

// =================== BOOL ===================
bool healthMod = false;
bool ammoMod = false;
//=================== --- ===================

int main (){

    HWND hwnd = FindWindow(NULL,"Sword With Sauce ");

    if(hwnd == NULL){
        cout << "couldnt find game window" << endl;
        Sleep(1000);
        exit(-1);
    }

    DWORD procID;//GetModule
    GetWindowThreadProcessId(hwnd,&procID);
    DWORD64 address = GetModuleBase(L"SwordWithSauce-Win64-Shipping.exe", procID);
    cout << "virtual base address: " << address <<endl;
    cout << "prc: " << procID << "  |  modbase: "<<std::hex << address <<endl;

    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS,FALSE,procID);
    cout << "handle: " << handle << endl;

    ReadProcessMemory(handle, (void*)address, &BASE_ADDRESS, sizeof(BASE_ADDRESS), NULL);
    cout << "final base address: " << BASE_ADDRESS << endl;
    if (procID == NULL){
        cout << "couldnt obtain game process" << endl;
        Sleep(1000);
        exit(-1);
    }

    unsigned  int tries = 0;
    while (true) {
        final_Health = FindDMAAddy(handle, address + ptr_Health,off_Health);
        final_Ammo = FindDMAAddy(handle, address + ptr_Ammo,off_Ammo);

        int val1 = 0;
        BOOL succes1 = ReadProcessMemory(handle, (void*)(final_Health), &val1, sizeof(val1), 0);
        BOOL succes2 = ReadProcessMemory(handle, (void*)(final_Ammo), &val1, sizeof(val1), 0);
        cout << final_Health << " " << final_Ammo << " " << succes1 << " " << succes2 << endl;
        if(final_Health == 0 || final_Ammo == 0 || !succes1 || !succes2){
            tries++;
            cout << "failed to obtain new memory pointers sleeping 2s.. | attempt: " << tries << endl;
            Sleep(2000);
            if(tries > 9){
                break;
            }
            continue;
        } else tries = 0;

        cout << "health addr: " << hex << final_Health << endl<< endl;
        int count = 0;
        unsigned short ammoSucces = 2;
        bool holdF1 = false, holdF2 = false;

        BOOL succes = true;
        while (true) {
            //=================== KEY LOOP ===================
            if (GetKeyState(VK_F1) & 0x8000){
                if(!holdF1){
                    holdF1 = true;
                    if(healthMod) {
                        cout << "god mode off" << endl;
                        healthMod = false;
                    }else {
                        final_Health = FindDMAAddy(handle, address + ptr_Health,off_Health);
                        cout << "god mode on | new address: " << hex << final_Health << endl;
                        healthMod = true;
                    }
                }
            } else holdF1 = false;
            if (GetKeyState(VK_F2) & 0x8000){
                if(!holdF2){
                    holdF2 = true;
                    if(ammoMod) {
                        cout << "unlimited ammo off" << endl;
                        ammoMod = false;
                    }else {
                        final_Ammo = FindDMAAddy(handle, address + ptr_Ammo,off_Ammo);
                        cout << "unlimited ammo on | new address: " << hex << final_Health << endl;
                        ammoMod = true;
                    }
                }
            } else holdF2 = false;
            if (GetKeyState(VK_F4)) exit(0);

            //=================== MEM WRITE ===================
            if(healthMod) succes = WriteProcessMemory(handle,(void *) (final_Health),&maxHealth,sizeof(maxHealth), 0);
            if (ammoMod && count % 5 == 0){
                /*ammoSucces =*/ WriteProcessMemory(handle,(void *) (final_Ammo),&maxAmmo,sizeof(maxAmmo), 0);}
            /*cout << "write >> health: " << succes << ", ammo: "
                 << ammoSucces << " " << hex << final_Health << " "
                 << final_Ammo << endl; ammoSucces = 2;*/

            if (!succes) break;
            Sleep(50);
        }
    }


}
