#include "Windows.h"
#include "ProcessMem.cpp"
#include "CDLocalization.cpp"
#include "iniparser.h"

using namespace std;

using IniParser = ini::IniParser;

IniParser ipa;
ini::inimap ini_data;

CDLocalization loc;

filesystem::path ini_file_path = filesystem::current_path() / "CD_GearBox.ini";

string text_speedo;

float spacewidth;
float widthscale;

string text_space()
{
    ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x3CF700, { 0x50B0 + 0x19E4 })), &widthscale, 4, 0);
    ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x3CF700, { 0x50B0 + 0x1664 })), &spacewidth, 4, 0);
    string result;
    for (int i = 0; i < round(-0.00679 * widthscale - 3.06333 * spacewidth + 32.2048); i++)
    {
        result += " ";
    }
    //cout << "Width scale: " << widthscale << " Space width: " << spacewidth << " Result length: " << result.length() << " (" << -0.00679 * widthscale - 3.06333 * spacewidth + 32.2048 << ")" << endl;
    return result;
}

vector<string> elements_toshow = { "btnGarageContinue", "btnQuitTransmission", "lblTransmissionSelSubCap", "frmTransmissionWindow", "lblTransmissionAutomatic", "lblTransmissionManual", "picTransmissionAutomatic", "picTransmissionManual" };
vector<string> elements_tohide = { "lblGarageMainSubCap", "picCarLogo", "btnLastCar", "btnNextCar", "btnSelectTransmission", "btnOpenCustomize", "btnOpenColorChoice", "btnGarageBack" };

vector<string> elements_noracing = { "btnSelectTransmission", "btnQuitTransmission", "lblTransmissionSelSubCap", "frmTransmissionWindow", "lblTransmissionAutomatic", "lblTransmissionManual", "picTransmissionAutomatic", "picTransmissionManual" };

int keyevent_shiftup = 0;
int keyevent_shiftdown = 0;
bool hud_enablegearindicator = false;
bool engine_enableneutralgear = false;

int pause_state;
int race_state;

int carplayer_gear_ptr;
int carplayer_gear1;
int carplayer_gear2;
int carplayer_gearcount;

int gamemodeflag;

int gear_change = 1;

int players_count;

int addr_speedo;
int addr2_speedo;

float carplayer_rpmcur1;
float carplayer_rpmcur2;

float carplayer_rpmshiftup;
float carplayer_rpmshiftdown;

float game_speed;

float carplayer_speed;

string curgearcolor;
string curgear;
string geartype;

int carplayer_physics_off = 1;
int carplayer_physics_on = 0;

bool window_init = false;
bool is_hovered = false;
bool arrowl = false;
bool arrowr = false;
bool blinds = false;

int menu_id;

int garcam;

int elements_count;
int element_name_len;
char element_name[256] = {};
string element_name_str;
int element_appearance = 0;

int race_info_len;
char race_info[1024] = {};
string race_info_str;

float element_animdur = 0.f;
float menutitledur;

int hovered_element;
int hovered_element_mouse;

int key_enter;
int key_joystick_x;

int key_escape;
int key_joystick_y;

int key_lmb;
int key_rmb;

int event_type;

string gamevar;

int spectator_id;

DWORD WINAPI MainTHREAD(LPVOID)
{
    text_speedo.reserve(128);

    if (!filesystem::exists(ini_file_path))
    {
        ipa.setAllowComments(true);
        ipa.setCommentSign('#');

        //ini_data["CD_GearBox"]["#0"] = "# For key value use decimal Virtual Key codes (https://cherrytree.at/misc/vk.htm).";
        ini_data["CD_GearBox"]["KeyEvent.ShiftUp"] = "160";
        ini_data["CD_GearBox"]["KeyEvent.ShiftDown"] = "162";
        //ini_data["CD_GearBox"]["#1"] = "# This option displays a gear indicator near the odometer on the speedometer.";
        ini_data["CD_GearBox"]["HUD.EnableGearIndicator"] = "true";
        //ini_data["CD_GearBox"]["#2"] = "# This option adds the inclusion of neutral gear for cars. With this option, performing a \"speedbug\" is impossible.";
        ini_data["CD_GearBox"]["Engine.EnableNeutralGear"] = "true";

        ipa.writeFile(ini_file_path.string(), ini_data);
    }

    if (ipa.readFile(ini_file_path.string()))
    {
        auto ini_data2 = ipa.parse();

        keyevent_shiftup = stoi(ini_data2["CD_GearBox"]["KeyEvent.ShiftUp"]);
        keyevent_shiftdown = stoi(ini_data2["CD_GearBox"]["KeyEvent.ShiftDown"]);

        if (ini_data2["CD_GearBox"]["HUD.EnableGearIndicator"] == "true")
        {
            hud_enablegearindicator = true;
        }
        else
        {
            hud_enablegearindicator = false;
        }

        if (ini_data2["CD_GearBox"]["Engine.EnableNeutralGear"] == "true")
        {
            engine_enableneutralgear = true;
        }
        else
        {
            engine_enableneutralgear = false;
        }
    }

    if (hud_enablegearindicator)
    {
        addr_speedo = (DWORD)&text_speedo;

        ReadProcessMemory(hProcess, (void*)(addr_speedo), &addr2_speedo, 4, 0);

        WriteProcessMemory(hProcess, (void*)(moduleBase + 0x1ABEA3), &addr2_speedo, 4, 0);
        WriteProcessMemory(hProcess, (void*)(moduleBase + 0x1ABBE2), &addr2_speedo, 4, 0);
    }

    while (true)
    {
        ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x0 })), &menu_id, 4, 0);

        ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x3DEA20, { 0x10, 0x4, 0x734 })), &pause_state, 1, 0);
        ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x38D5F0, { 0x80 })), &race_state, 1, 0);
        ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x3DC550, { 0x8 })), &game_speed, 4, 0);

        ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x38D5F0, { 0x6C })), &gamemodeflag, 4, 0);

        if (race_state == 0)
        {
            ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x244 })), &elements_count, 1, 0);
            ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x3BC998, { 0x24 })), &garcam, 1, 0);

            if (menu_id == 5471472)
            {
                if (!window_init)
                {
                    is_hovered = false;
                }
                ReadProcessMemory(hProcess, (void*)(moduleBase + 0x3A87FC), &event_type, 4, 0);
                if (event_type == 3)
                {
                    struct ElementData {
                        LPVOID name_len_addr;
                        LPVOID name_addr;
                        LPVOID appearance_addr;
                        LPVOID animdur_addr;
                        LPVOID posx_addr;
                        LPVOID scalex_addr;
                    };

                    std::vector<ElementData> element_data(elements_count);

                    for (uint32_t i = 0; i < elements_count; i++)
                    {
                        element_data[i].name_len_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x44 + (i * 4), 0x10, 0x8 });
                        element_data[i].name_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x44 + (i * 4), 0x10, 0xC });
                        element_data[i].appearance_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x44 + (i * 4), 0x55 });
                        element_data[i].animdur_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x44 + (i * 4), 0x13C });
                        element_data[i].posx_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x44 + (i * 4), 0x58 });
                        element_data[i].scalex_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x44 + (i * 4), 0x60 });
                    }

                    LPVOID menutitledur_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x280 });

                    LPVOID key_enter_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D5F4, { 0x1A94 });
                    LPVOID key_joystick_x_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D5F4, { 0x1BB9 });
                    LPVOID key_escape_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D5F4, { 0x1A79 });
                    LPVOID key_joystick_y_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D5F4, { 0x1BBB });
                    LPVOID key_lmb_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D5F4, { 0x1B84 });
                    LPVOID key_rmb_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D5F4, { 0x1B85 });
                    LPVOID hovered_element_mouse_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D65C, { 0x248, 0x2A });
                    LPVOID hovered_element_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D65C, { 0x248, 0x28 });
                    LPVOID blinds_type_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x25E });
                    LPVOID blinds_dur_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x2B8 });
                    LPVOID gamemodeflag_addr;

                    uint32_t gamemodeflag_index = race_info_str.find("gamemodeflag=") + 14;
                    gamemodeflag_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x3A8800, { 0xC + gamemodeflag_index });

                    // ------------------ Inner Loop ------------------
                    for (uint32_t i = 0; i < elements_count; i++)
                    {
                        uint32_t element_name_len;
                        char element_name[256]; // Adjust size as needed
                        if (!ReadProcessMemory(hProcess, element_data[i].name_len_addr, &element_name_len, 4, 0)) {
                            // Handle error
                            continue;
                        }
                        if (!ReadProcessMemory(hProcess, element_data[i].name_addr, &element_name, element_name_len, 0)) {
                            // Handle error
                            continue;
                        }
                        string element_name_str = string(element_name, element_name_len);

                        // --- garcam != 1 ---
                        if (garcam != 1)
                        {
                            element_appearance = 0;

                            if (element_name_str == elements_tohide[4])
                            {
                                WriteProcessMemory(hProcess, element_data[i].appearance_addr, &element_appearance, 1, 0);
                            }
                            if (element_name_str == elements_tohide[5])
                            {
                                WriteProcessMemory(hProcess, element_data[i].appearance_addr, &element_appearance, 1, 0);
                            }
                            if (element_name_str == elements_tohide[6])
                            {
                                WriteProcessMemory(hProcess, element_data[i].appearance_addr, &element_appearance, 1, 0);
                            }
                        }
                        // --- garcam == 1 ---
                        else
                        {
                            if (element_name_str == elements_toshow[5])
                            {
                                float pos_x = 424.f;
                                WriteProcessMemory(hProcess, element_data[i].posx_addr, &pos_x, 4, 0);
                            }

                            if (!ReadProcessMemory(hProcess, element_data[i].appearance_addr, &element_appearance, 1, 0)) {
                                // Handle error
                            }
                            if (element_appearance == 0)
                            {
                                WriteProcessMemory(hProcess, element_data[i].animdur_addr, &element_animdur, 4, 0);
                            }

                            // --- Arrow Logic ---  (Consolidated for readability)
                            if (element_name_str == elements_tohide[2] || element_name_str == elements_tohide[3])
                            {
                                float scale_x;
                                bool& arrow_flag = (element_name_str == elements_tohide[2]) ? arrowl : arrowr; // Reference to the correct flag
                                float default_scale_x = (element_name_str == elements_tohide[2]) ? -100.f : 100.f;

                                if (!window_init)
                                {
                                    if (!arrow_flag)
                                    {
                                        ReadProcessMemory(hProcess, element_data[i].scalex_addr, &scale_x, 4, 0);
                                        arrow_flag = true;
                                    }
                                    else
                                    {
                                        WriteProcessMemory(hProcess, element_data[i].scalex_addr, &scale_x, 4, 0);
                                    }
                                }
                                else
                                {
                                    //scale_x = default_scale_x;
                                    WriteProcessMemory(hProcess, element_data[i].scalex_addr, &default_scale_x, 4, 0);
                                }
                            }

                            float menutitledur;
                            if (!ReadProcessMemory(hProcess, menutitledur_addr, &menutitledur, 4, 0)) {
                                // Handle Error
                            }

                            // --- Keypress Handling ---
                            if (menutitledur > 0.4f && PM.IsCurrentProcessActive())
                            {
                                BYTE key_enter, key_joystick_x, key_escape, key_joystick_y, key_lmb, key_rmb;
                                WORD hovered_element_mouse;

                                if (!ReadProcessMemory(hProcess, key_enter_addr, &key_enter, 1, 0)) { /* Handle Error */ }
                                if (!ReadProcessMemory(hProcess, key_joystick_x_addr, &key_joystick_x, 1, 0)) { /* Handle Error */ }
                                if (!ReadProcessMemory(hProcess, key_escape_addr, &key_escape, 1, 0)) { /* Handle Error */ }
                                if (!ReadProcessMemory(hProcess, key_joystick_y_addr, &key_joystick_y, 1, 0)) { /* Handle Error */ }
                                if (!ReadProcessMemory(hProcess, key_lmb_addr, &key_lmb, 1, 0)) { /* Handle Error */ }
                                if (!ReadProcessMemory(hProcess, key_rmb_addr, &key_rmb, 1, 0)) { /* Handle Error */ }
                                if (!ReadProcessMemory(hProcess, hovered_element_mouse_addr, &hovered_element_mouse, 2, 0)) { /* Handle Error */ }

                                if (!window_init)
                                {
                                    if ((key_enter == 128) || (key_joystick_x == 128))
                                    {
                                        //Sleep(10);
                                        window_init = true;
                                    }
                                    if (element_name_str == elements_tohide[4])
                                    {
                                        if (((key_lmb == 128) || (key_rmb == 128)) && (i == hovered_element_mouse))
                                        {
                                            //Sleep(10);
                                            window_init = true;
                                        }
                                    }
                                }
                                else
                                {
                                    if ((key_escape == 128) || (key_joystick_y == 128))
                                    {
                                        //Sleep(10);
                                        window_init = false;
                                    }
                                    if (element_name_str == elements_toshow[1])
                                    {
                                        if (((key_lmb == 128) || (key_rmb == 128)) && (i == hovered_element_mouse))
                                        {
                                            //Sleep(10);
                                            window_init = false;
                                        }
                                    }
                                }
                            }

                            // --- elements_toshow/tohide loops --- (Consolidated)
                            for (const auto& element : elements_toshow)
                            {
                                if (element_name_str == element)
                                {
                                    element_appearance = (window_init) ? 1 : 0;
                                    WriteProcessMemory(hProcess, element_data[i].appearance_addr, &element_appearance, 1, 0);
                                    break; // Stop after finding the first match
                                }
                            }

                            for (const auto& element : elements_tohide)
                            {
                                if (element_name_str == element)
                                {
                                    element_appearance = (window_init) ? 0 : 1;
                                    WriteProcessMemory(hProcess, element_data[i].appearance_addr, &element_appearance, 1, 0);
                                    break; // Stop after finding the first match
                                }
                            }

                            // --- Hover Logic ---
                            if ((element_name_str == elements_toshow[6]) && (window_init) && (!is_hovered))
                            {
                                WriteProcessMemory(hProcess, hovered_element_addr, &i, 2, 0);
                                is_hovered = true;
                            }

                            // --- Gamemode Flag Logic ---
                            if (window_init)
                            {
                                WORD hovered_element;
                                if (!ReadProcessMemory(hProcess, hovered_element_addr, &hovered_element, 2, 0)) { /* Handle Error */ }

                                int gamemodeflag_val = -1; // Initialize with a default value.
                                if (element_name_str == elements_toshow[6] && i == hovered_element)
                                {
                                    gamemodeflag_val = 48;
                                }
                                else if (element_name_str == elements_toshow[7] && i == hovered_element)
                                {
                                    gamemodeflag_val = 49;
                                }

                                // Only write if gamemodeflag_val was actually set.
                                if (gamemodeflag_val != -1) {
                                    WriteProcessMemory(hProcess, gamemodeflag_addr, &gamemodeflag_val, 1, 0);
                                }
                            }

                            // --- Blinds Logic ---
                            if (window_init)
                            {
                                if (!blinds)
                                {
                                    int blinds_type = 1;
                                    float blinds_dur = 0.f;
                                    WriteProcessMemory(hProcess, blinds_type_addr, &blinds_type, 1, 0);
                                    WriteProcessMemory(hProcess, blinds_dur_addr, &blinds_dur, 4, 0);
                                    blinds = true;
                                }
                            }
                            else
                            {
                                if (blinds)
                                {
                                    int blinds_type = 3;
                                    float blinds_dur = 0.f;
                                    WriteProcessMemory(hProcess, blinds_type_addr, &blinds_type, 1, 0);
                                    WriteProcessMemory(hProcess, blinds_dur_addr, &blinds_dur, 4, 0);
                                    blinds = false;
                                }
                            }
                        }
                    }
                }
                // ================== Second Major Conditional ==================
                else
                {
                    // ------------------ Pre-Inner Loop Setup ------------------
                    struct ElementData {
                        LPVOID name_len_addr;
                        LPVOID name_addr;
                        LPVOID appearance_addr;
                        LPVOID animdur_addr;
                        LPVOID posx_addr;
                        LPVOID scalex_addr;
                    };

                    std::vector<ElementData> element_data(elements_count);

                    for (uint32_t i = 0; i < elements_count; i++)
                    {
                        element_data[i].name_len_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x44 + (i * 4), 0x10, 0x8 });
                        element_data[i].name_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x44 + (i * 4), 0x10, 0xC });
                        element_data[i].appearance_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x44 + (i * 4), 0x55 });
                        element_data[i].animdur_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x44 + (i * 4), 0x13C });
                        element_data[i].posx_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x44 + (i * 4), 0x58 });
                        element_data[i].scalex_addr = (LPVOID)PM.FindDMAAddy(hProcess, moduleBase + 0x38D658, { 0x44 + (i * 4), 0x60 });
                    }

                    for (uint32_t i = 0; i < elements_count; i++)
                    {
                        uint32_t element_name_len;
                        char element_name[256]; // Adjust size as needed
                        if (!ReadProcessMemory(hProcess, element_data[i].name_len_addr, &element_name_len, 4, 0)) {
                            // Handle error
                            continue;
                        }
                        if (!ReadProcessMemory(hProcess, element_data[i].name_addr, &element_name, element_name_len, 0)) {
                            // Handle error
                            continue;
                        }
                        string element_name_str = string(element_name, element_name_len);

                        for (string element : elements_noracing)
                        {
                            if (element_name_str == element)
                            {
                                element_appearance = 0;
                                WriteProcessMemory(hProcess, element_data[i].appearance_addr, &element_appearance, 1, 0);
                            }
                        }
                    }
                }
            }
            else
            {
                window_init = false;
            }
        }

        if ((pause_state == 0) && (game_speed != 0.f))
        {
            ReadProcessMemory(hProcess, (void*)(moduleBase + 0x38CEDC), &players_count, 4, 0);
            for (uint32_t i = 1; i <= players_count; i++)
            {
                ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x38CEE0, { 4 + (8 * (i - 1)), 0x6EB8 })), &carplayer_speed, 4, 0);
                ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x38CEE0, { 4 + (8 * (i - 1)), 0x6E78 })), &carplayer_gear_ptr, 4, 0);
                ReadProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x2FC), &carplayer_gear1, 1, 0);
                ReadProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x2FC + 1), &carplayer_gear2, 1, 0);

                if (engine_enableneutralgear)
                {
                    if (((i != 1) && (gamemodeflag == 1)) || (gamemodeflag == 0))
                    {
                        if ((carplayer_speed == 0.f) || ((carplayer_gear1) == 255 && (carplayer_speed > 0.f)))
                        {
                            carplayer_gear1 = 0;
                            carplayer_gear2 = 0;

                            WriteProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x2FC), &carplayer_gear1, 1, 0);
                            WriteProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x2FC + 1), &carplayer_gear2, 1, 0);
                        }
                    }
                }
                if (i == 1)
                {
                    if ((gamemodeflag == 1) && (race_state == 3))
                    {
                        ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x38CEE0, { 0x4, 0x490C })), &carplayer_gearcount, 1, 0);

                        if (PM.IsCurrentProcessActive())
                        {
                            if (GetAsyncKeyState(keyevent_shiftup) & 1)
                            {
                                if (carplayer_gear1 < carplayer_gearcount)
                                {
                                    int carplayer_gear = carplayer_gear1 + 1;
                                    WriteProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x2FC), &carplayer_gear, 1, 0);
                                    WriteProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x2FC + 1), &carplayer_gear, 1, 0);
                                    WriteProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x2FC + 3), &gear_change, 1, 0);
                                }
                                else if (carplayer_gear1 == 255)
                                {
                                    int carplayer_gear = 0;
                                    WriteProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x2FC), &carplayer_gear, 1, 0);
                                    WriteProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x2FC + 1), &carplayer_gear, 1, 0);
                                    WriteProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x2FC + 3), &gear_change, 1, 0);
                                }
                            }
                            if (GetAsyncKeyState(keyevent_shiftdown) & 1)
                            {
                                if ((carplayer_gear1 > 0) && (carplayer_gear1 != 255))
                                {
                                    int carplayer_gear = carplayer_gear1 - 1;
                                    WriteProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x2FC), &carplayer_gear, 1, 0);
                                    WriteProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x2FC + 1), &carplayer_gear, 1, 0);
                                    WriteProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x2FC + 3), &gear_change, 1, 0);
                                }
                                else if (carplayer_gear1 == 0)
                                {
                                    int carplayer_gear = 255;
                                    WriteProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x2FC), &carplayer_gear, 1, 0);
                                    WriteProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x2FC + 1), &carplayer_gear, 1, 0);
                                    WriteProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x2FC + 3), &gear_change, 1, 0);
                                }
                            }
                        }
                    }
                }
                ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x3DEA20, { 0x10, 0x4, 0x198 })), &spectator_id, 1, 0);
                if (i == spectator_id + 1)
                {
                    if (hud_enablegearindicator)
                    {
                        //loc.Init(hProcess, moduleBase);

                        if (gamemodeflag == 0)
                        {
                            //geartype = "AT";
                            geartype = loc.LocString("GearBox", "SHIFTAUTOMATIC");
                        }
                        else
                        {
                            //geartype = "MT";
                            geartype = loc.LocString("GearBox", "SHIFTMANUAL");
                        }

                        if (carplayer_gear1 == 0)
                        {
                            curgear = "N";

                            //WriteProcessMemory(hProcess, (void*)(addr2_speedo + text_offset), &text_gearn, strlen(text_gearn), 0);
                        }
                        else if (carplayer_gear1 == 255)
                        {
                            curgear = "R";

                            //WriteProcessMemory(hProcess, (void*)(addr2_speedo + text_offset), &text_gearr, strlen(text_gearr), 0);
                        }
                        else
                        {
                            curgear = to_string(carplayer_gear1);

                            //WriteProcessMemory(hProcess, (void*)(addr2_speedo + text_offset), &carplayer_gear_str, carplayer_gear_str.length(), 0);
                        }

                        ReadProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x30C), &carplayer_rpmcur1, 4, 0);

                        ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x38CEE0, { 0x4, 0x4958 })), &carplayer_rpmshiftup, 4, 0);
                        ReadProcessMemory(hProcess, (LPVOID)(PM.FindDMAAddy(hProcess, moduleBase + 0x38CEE0, { 0x4, 0x4954 })), &carplayer_rpmshiftdown, 4, 0);

                        ReadProcessMemory(hProcess, (void*)(carplayer_gear_ptr + 0x30C), &carplayer_rpmcur2, 4, 0);

                        if ((((carplayer_rpmcur1 <= carplayer_rpmcur2) && (carplayer_rpmcur1 >= carplayer_rpmshiftup)) || (((carplayer_rpmcur1 >= carplayer_rpmcur2) && (carplayer_rpmcur1 <= carplayer_rpmshiftdown) && (carplayer_gear1 != 0) && (carplayer_gear2 != 0) && (carplayer_gear1 != 1) && (carplayer_gear2 != 1)))) && ((carplayer_gear1 != 255) && (carplayer_gear2 != 255)) || (carplayer_gear1 != carplayer_gear2))
                        {
                            curgearcolor = "1"; // Red
                        }
                        else
                        {
                            curgearcolor = "7"; // White
                        }

                        string text_format = format("\\+{}\\-\\-\\-  \\9{}\\d\\+\\+|\\{}{} \\-\\-\\9{}", "%d", "%s", curgearcolor, curgear, geartype) + text_space();
                        text_speedo = text_format;
                    }
                }
            }
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, MainTHREAD, 0, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

