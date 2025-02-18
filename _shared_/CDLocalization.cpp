#include <regex>
#include <fstream>
#include "filesystem"
#include "Windows.h"

using namespace std;

char curlanguange[32] = {};

bool unit_init = false;

string unit_output;
string unitstring;

HANDLE hProcess2;
uintptr_t moduleBase2;

ProcessMem PM2;

std::vector<std::string> splitString(const std::string& str) {
	std::vector<std::string> result;
	std::stringstream ss(str);
	std::string item;

	while (std::getline(ss, item, ' ')) {
		result.push_back(item);
	}

	return result;
}

std::string removeControlChars(const std::string& str) {
	std::string result = "";
	for (char c : str) {
		if (!std::iscntrl(c)) {
			result += c;
		}
	}
	return result;
}

std::string trimLeadingSpaces(const std::string& str) {
	size_t first = str.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) {
		return "";
	}
	return str.substr(first);
}

class CDLocalization
{
public:
	char* Init(HANDLE hProcessInput, uintptr_t moduleBaseInput)
	{
		hProcess2 = hProcessInput;
		moduleBase2 = moduleBaseInput;

		ReadProcessMemory(hProcess2, (void*)(moduleBase2 + 0x3E3118), &curlanguange, sizeof(curlanguange), 0);
		
		return curlanguange;
	}

	string LocString(string strinput, filesystem::path gamedirectory, string plainfilename = "")
	{
		bool section = false;
		bool id = false;
		bool text = false;
		if (strinput.starts_with("$ID"))
		{
			string locplain;

			if (plainfilename.empty())
			{
				locplain = string(curlanguange) + ".plain";
			}
			else
			{
				locplain = plainfilename + ".plain";
			}

			filesystem::path locpath(gamedirectory / "loc" / curlanguange / locplain);

			if (!filesystem::exists(locpath))
			{
				locpath.assign(gamedirectory / "loc" / "eng" / locplain);
			}

			vector<string> db_entry = splitString(strinput);

			ifstream file(locpath);
			string line;
			//printf("Section: %s\tID: %s\n", db_entry[1].c_str(), db_entry[2].c_str());
			while (getline(file, line))
			{
				if ((line == ("SECTION: " + db_entry[1])) && (section == false))
				{
					section = true;
				}

				if (section == true)
				{
					if (line.starts_with("ID")) //((line.starts_with("ID")) && (line.substr(strlen(line.c_str()) - strlen(db_entry[2].c_str()), strlen(line.c_str()))) == db_entry[2])
					{
						try
						{
							string line2 = regex_replace(line.substr(0, line.find("#")).substr(line.substr(0, line.find("#")).length() - db_entry[2].length(), line.substr(0, line.find("#")).length()), regex("^ +| +$|( ) +"), "$1");

							if ((line2 == removeControlChars(db_entry[2])) && (id == false))
							{
								id = true;
							}
						}
						catch (...) {}
					}
				}

				if ((section == true) && (id == true) && (text == false))
				{
					if (line.starts_with("TEXT"))
					{
						string line2 = trimLeadingSpaces(line.substr(4, line.length()));
						text = true;
						return line2;
					}
				}
			}
		}
		else
		{
			return strinput;
		}
	}

	string LocUnit(float value, string unit, filesystem::path gamedirectory)
	{
		string unitspath;
		int metricsystem;
		float unitmultiplier;

		string unitsfile = "units.ini";

		ReadProcessMemory(hProcess2, (LPVOID)(PM2.FindDMAAddy(hProcess2, moduleBase2 + 0x3E3110, { 0x60 })), &metricsystem, 4, 0);

		if (removeControlChars(curlanguange) == "eng")
		{
			unitspath = (gamedirectory).string() + "\\" + unitsfile;
		}
		else
		{
			unitspath = (gamedirectory).string() + "\\loc\\" + curlanguange + "\\" + unitsfile;
		}

		ifstream file(unitspath);
		string line;
		int lineCounter = 0;
		int lineUnitIndex;
		while (getline(file, line))
		{
			lineCounter++;
			if (removeControlChars(line.substr(0, line.find("#"))) == unit)
			{
				lineUnitIndex = lineCounter;
			}
			if (metricsystem == 1)
			{
				if (lineCounter == lineUnitIndex + 1)
				{
					unitstring = removeControlChars(line.substr(0, line.find("#")));
				}
				unit_output = (to_string((int)value) + " " + unitstring);
			}
			else
			{
				
				if (lineCounter == lineUnitIndex + 2)
				{
					unitstring = removeControlChars(line.substr(0, line.find("#")));
				}
				if (lineCounter == lineUnitIndex + 3)
				{
					unitmultiplier = stof(line.substr(0, line.find("#")));
				}
				unit_output = (to_string((int)(value * unitmultiplier)) + " " + unitstring);
			}
		}
		cout << unit_output << endl;
		return unit_output;
	}
};