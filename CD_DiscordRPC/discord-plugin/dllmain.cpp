#include "dllmain.h"
#include <string> 
#include <stdlib.h>
#include <filesystem>
#include "ProcessMem.cpp"

ProcessMem PM;

string FullPath = PM.GetProcessPathByPID(GetCurrentProcessId());
filesystem::path p(FullPath);
string filename = p.filename().string();
uintptr_t moduleBase = PM.GetModuleBaseAddress(GetCurrentProcessId(), filename.c_str());
HANDLE hProcess = GetCurrentProcess();

int game_state;

int event_type;

int event_type2_ptr;
int event_type2;

int event_val1;
float event_val2;

int menu_id;

char trkname[64] = {};
char trkeditorname[64] = {};
string trkname_str;

int cur_players_count;
int max_players_count;
int max_netplayers_count;

int replay_loaded;

int race_network;

int currcardefspecs_ptr;
int currcardefspecs_ptr2;

int carname_len;
char carname[64] = {};
string carname_str;

string eventinfo;

string formatTime(int seconds) {
	int minutes = seconds / 60;
	int remainingSeconds = seconds % 60;

	if ((minutes > 0) && (remainingSeconds > 0))
	{
		return to_string(minutes) + " Minutes " + to_string(remainingSeconds) + " Seconds";
	}
	else if ((minutes > 0) && (remainingSeconds == 0))
	{
		return to_string(minutes) + " Minutes";
	}
	else
	{
		return to_string(seconds) + " Seconds";
	}
}

string formatPts(int number) {
	string numStr = to_string(number);
	string formattedNum = "";
	int count = 0;

	bool isNegative = (number < 0);
	if (isNegative) {
		numStr = numStr.substr(1); 
	}

	for (int i = numStr.length() - 1; i >= 0; i--) {
		formattedNum = numStr[i] + formattedNum;
		count++;

		if (count == 3 && i != 0) {
			formattedNum = "," + formattedNum;
			count = 0;
		}
	}

	if (isNegative) {
		formattedNum = "-" + formattedNum;
	}

	return formattedNum + " Points";
}

void MainThread() {
	DiscordRichPresence drp;

	drp.startTimestamp = time(0);

	Discord_Initialize(APPLICATION_ID, 0, 0, 0);

	ReadProcessMemory(hProcess, (void*)(moduleBase + 0x2492FF), &max_netplayers_count, 1, 0);
	ReadProcessMemory(hProcess, (void*)(moduleBase + 0x248AC9), &max_players_count, 1, 0);
	max_players_count++;

	while (true)
	{
		ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x3DDEE0, { 0x14, 0xA0, 0x104, 0x4, 0x20, 0x4, 0x10 })), &game_state, 4, 0);
		if (game_state == 0)
		{
			drp.partySize = 0;
			drp.partyMax = 0;

			drp.largeImageKey = "icon_mainlarge";
			drp.largeImageText = "Crashday";

			ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x0 })), &menu_id, 4, 0);

			if (menu_id == 5340816)
			{
				drp.details = "In Tony's Shop";
				drp.state = "Selecting an Event";
				drp.smallImageKey = "";
				drp.smallImageText = "";
			}
			else if (menu_id == 5471472)
			{
				ReadProcessMemory(hProcess, (void*)(moduleBase + 0x395D20), &currcardefspecs_ptr, 4, 0);
				ReadProcessMemory(hProcess, (void*)(currcardefspecs_ptr), &currcardefspecs_ptr2, 4, 0);
				ReadProcessMemory(hProcess, (void*)(currcardefspecs_ptr2 + 0x8), &carname_len, 4, 0);
				ReadProcessMemory(hProcess, (void*)(currcardefspecs_ptr2 + 0xC), &carname, sizeof(carname), 0);

				carname_str = "Current Car: " + string(carname).substr(0, carname_len);

				drp.state = "Choosing a Car";
				drp.smallImageKey = "icon_car";
				drp.smallImageText = carname_str.c_str();
			}
			else if (menu_id == 5584080)
			{
				drp.state = "Browsing a Lobbies";
				drp.smallImageKey = "";
				drp.smallImageText = "";
			}
			else if (menu_id == 5656816)
			{
				drp.details = "";
				drp.state = "Multiplayer Lobby";
				drp.smallImageKey = "";
				drp.smallImageText = "";
			}
			else if (menu_id == 5321328)
			{
				drp.details = "In Tony's Shop";
				drp.state = "Main Menu";
				drp.smallImageKey = "";
				drp.smallImageText = "";
			}
			else if ((menu_id == 5838864) || (menu_id == 5858560) || (menu_id == 5882256) || (menu_id == 5871856))
			{
				drp.state = "Track Editor";
				if (menu_id == 5838864)
				{
					ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x3A9160, { 0x118, 0x13 })), &trkeditorname, sizeof(trkeditorname), 0);
					drp.details = trkeditorname;
				}
			}
			else if (menu_id != 0)
			{
				drp.details = "In Garage";
				drp.state = "Main Menu";
				drp.smallImageKey = "";
				drp.smallImageText = "";
			}
		}
		else
		{
			ReadProcessMemory(hProcess, (void*)(moduleBase + 0x3A87FC), &event_type, 4, 0);
			ReadProcessMemory(hProcess, (void*)(moduleBase + 0x3CD0E0), &race_network, 4, 0);

			if ((event_type == 4) || (event_type == 7))
			{
				drp.partySize = 0;
				drp.partyMax = 0;
			}
			else
			{
				ReadProcessMemory(hProcess, (void*)(moduleBase + 0x38CEDC), &cur_players_count, 4, 0);
				drp.partySize = cur_players_count;

				if (race_network == 0)
				{
					drp.partyMax = max_players_count;
				}
				else
				{
					drp.partyMax = max_netplayers_count;
				}
			}

			if (race_network == 0)
			{
				drp.state = "Single Event";
			}
			else
			{
				drp.state = "Multiplayer Event";
			}

			drp.smallImageKey = "";
			drp.smallImageText = "";

			ReadProcessMemory(hProcess, (void*)(moduleBase + 0x3A7CB0), &replay_loaded, 1, 0);
			if (replay_loaded == 0)
			{
				ReadProcessMemory(hProcess, (void*)(moduleBase + 0x38D5F0), &event_type2_ptr, 4, 0);
				ReadProcessMemory(hProcess, (void*)(event_type2_ptr + 0x60), &event_type2, 4, 0);
				ReadProcessMemory(hProcess, (void*)(event_type2_ptr + 0x60 + 0x4), &event_val1, 4, 0);
				ReadProcessMemory(hProcess, (void*)(event_type2_ptr + 0x60 + 0x8), &event_val2, 4, 0);

				if (event_type == 0)
				{
					drp.largeImageKey = "icon_modewreck";
					drp.largeImageText = "Wrecking Match";
					if (event_type2 == 7)
					{
						drp.smallImageKey = "icon_event";
						eventinfo = "Last Man Standing";
					}
					else if (event_type2 == 8)
					{
						drp.smallImageKey = "icon_event";
						eventinfo = "Deathmatch - " + to_string(event_val1) + " Wrecks";
					}
					else if (event_type2 == 9)
					{
						drp.smallImageKey = "icon_eventteams";
						eventinfo = "Deathmatch - " + to_string(event_val1) + " Wrecks";
					}
					else if (event_type2 == 10)
					{
						drp.smallImageKey = "icon_event";
						eventinfo = "In Advance - " + to_string(event_val1) + " Wrecks";
					}
				}
				else if (event_type == 1)
				{
					drp.largeImageKey = "icon_modestunt";
					drp.largeImageText = "Stunt Mode";
					if (event_type2 == 0)
					{
						drp.smallImageKey = "icon_event";
						eventinfo = "Time Limit - " + formatTime(event_val2);
					}
					if (event_type2 == 1)
					{
						drp.smallImageKey = "icon_eventteams";
						eventinfo = "Time Limit - " + formatTime(event_val2);
					}
					if (event_type2 == 2)
					{
						drp.smallImageKey = "icon_event";
						eventinfo = "Target Score - " + formatPts(event_val1);
					}
					if (event_type2 == 3)
					{
						drp.smallImageKey = "icon_eventteams";
						eventinfo = "Target Score - " + formatPts(event_val1);
					}
				}
				else if (event_type == 2)
				{
					drp.largeImageKey = "icon_modespeed";
					drp.largeImageText = "Bomb Run";
					drp.smallImageKey = "icon_event";
					drp.smallImageText = "Bomb Run";
				}
				else if (event_type == 3)
				{
					drp.largeImageKey = "icon_moderace";
					drp.largeImageText = "Race";
					if (event_type2 == 4)
					{
						drp.smallImageKey = "icon_event";
						eventinfo = to_string(event_val1) + " Laps";
					}
					else if (event_type2 == 5)
					{
						drp.smallImageKey = "icon_event";
						eventinfo = "Point-To-Point";
					}
					else if (event_type2 == 6)
					{
						drp.smallImageKey = "icon_event";
						eventinfo = "Knock Out";
					}
				}
				else if (event_type == 4)
				{
					drp.largeImageKey = "icon_modetestdrive";
					drp.largeImageText = "Test Drive";
					drp.smallImageKey = "icon_event";
					drp.smallImageText = "Test Drive";
				}
				else if (event_type == 5)
				{
					drp.largeImageKey = "icon_modectf";
					drp.largeImageText = "Hold The Flag";
					if (event_type2 == 11)
					{
						drp.smallImageKey = "icon_event";
						eventinfo = to_string(event_val1) + " Flags";
					}
					else if (event_type2 == 12)
					{
						drp.smallImageKey = "icon_eventteams";
						eventinfo = to_string(event_val1) + " Flags";
					}
				}
				else if (event_type == 6)
				{
					drp.largeImageKey = "icon_modeptb";
					drp.largeImageText = "Pass The Bomb";
					if (event_type2 == 13)
					{
						drp.smallImageKey = "icon_event";
						eventinfo = to_string(event_val1) + " Rounds | " + formatTime(event_val2);
					}
				}
				else if (event_type == 7)
				{
					drp.largeImageKey = "icon_modeminigames";
					drp.largeImageText = "Mini Games";
					if (event_type2 == 16)
					{
						drp.smallImageKey = "icon_event";
						eventinfo = "Long Jump";
					}
					else if (event_type2 == 17)
					{
						drp.smallImageKey = "icon_event";
						eventinfo = "Vehicle Blast";
					}
					else if (event_type2 == 18)
					{
						drp.smallImageKey = "icon_event";
						eventinfo = "Checkpoint Chase";
					}
					else if (event_type2 == 19)
					{
						drp.smallImageKey = "icon_event";
						eventinfo = "Mass Destruction";
					}
				}
				drp.smallImageText = eventinfo.c_str();

				ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x3A8814, { 0xC })), &trkname, sizeof(trkname), 0);
				trkname_str = string(trkname).substr(0, string(trkname).find(".trk"));

				if (trkname_str == "_!testdrive!_")
				{
					drp.details = trkeditorname;
				}
				else
				{
					drp.details = trkname_str.c_str();
				}
			}
			else
			{
				drp.largeImageKey = "icon_mainlarge";
				drp.largeImageText = "Crashday";
				drp.details = "Watching a Replay";
			}
		}
		Sleep(100);
		Discord_UpdatePresence(&drp);
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
	if (reason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hModule);
		CreateThread(0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(MainThread), 0, 0, 0);
	}
	else if (reason == DLL_PROCESS_DETACH)
		Discord_Shutdown();

	return TRUE;
}