/******************************************************************************
**
**  ipmiparm - a menu-driven kmod parameter manager for ipmi kmods
**
**  Tony Camuso
**  April, 2014
**
**    ipmiparm is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**  GNU General Public License http://www.gnu.org/licenses/gpl.html
**
**  Copyright 2014 by Tony Camuso.
**
**  Very simple compile
**
**      $ make ipmiparm
**  or
**      $ g++ -o ipmiparm ipmiparm.cpp
**
******************************************************************************/

#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

/**************************************************************
 * class kmodparm
 *************************************************************/
class kmodparm {
public:

    kmodparm() {value = 0; m_isbitmask = false; }

    int togglebit(int bit, int& mask);

    string kmodname;
    string parmname;
    int    value;

private:
    bool   m_isbitmask;
};

/**************************************************************
 * kmodparm::togglebit - toggle a bit in a bitmask
 *
 */int kmodparm::togglebit(int bit, int &mask)
{
    bit = (1 << bit);
    mask = bit xor mask;
    return mask;
}

/**************************************************************
 * class parmapp
 *************************************************************/
class parmapp {
public:
    parmapp() {init();}
    parmapp(int test) {init(test);}

    void   dump();
    int    shell(stringstream& command, stringstream& outstr);
    void   getmenu();
    void   showmenu();
    char   getchar();
    int    toxint(char c);
    string tobin(int hex, int bits);
    void   editparm(kmodparm& parm);
    void   editparmbitmask(kmodparm& parm);
    int    getint(string prompt, int curval = 0);
    bool   toggleradix() {hexdec = !hexdec; return hexdec;}
    string getradixstr() {return hexdec ? "hex" : "dec";}

private:
    string topdir;
    bool hexdec;            // hex radix when true, dec when false
    bool binary;            // binary input enabled for bitmasks when true
    vector<kmodparm> parms;

    void init(bool test = false);
    void init_kmod(string kmod);
};

/**************************************************************
 * parmapp::dump - dump the contents of the vector of kemod parms
 *
 */
void parmapp::dump()
{
    for (uint i = 0; i < parms.size(); ++i) {
        cout << "kmod: " << parms[i].kmodname
             << "  parm: " << parms[i].parmname
             << "  val: " << parms[i].value << endl;
    }
}

/**************************************************************
 * parmapp::init_kmod - Initialize the vector of kmod parameters
 *                      by reading the contents of the parameter
 *                      files in sysfs.
 *
 * Makes a system() call to the bash shell.
 *
 */
void parmapp::init_kmod(string kmod)
{
    stringstream cmd;
    stringstream s1;
    stringstream s2;
    string str;
    string dir = topdir + "module/" + kmod + "/parameters/";
    int i = parms.size();

    cmd << "ls " << dir;
    shell(cmd, s1);

    while (getline(s1, str)) {
        parms.resize(parms.size() + 1);
        parms[i].kmodname = kmod;
        parms[i].parmname = str;

        cmd.str("");
        s2.str("");
        cmd << "cat " << dir << str;
        shell(cmd, s2);
        s2 >> parms[i].value;
        ++i;
    }
}

/**************************************************************
 * parmapp::init - top level init routine
 *
 */
void parmapp::init(bool test)
{
    topdir = test ? "$HOME/" : "/sys/";
    hexdec = true;
    binary = false;
    init_kmod("ipmi_si");
    init_kmod("ipmi_msghandler");
}

/**************************************************************
 * parmapp::shell - execute a shell command and capture its output
 *
 * command - stringstream containing the command string
 * outstr  - reference to a stringstream that will contain the
 *           output from the shell after running the command
 * BUFSIZ  - defined by the compiler. In g++ running on Linux
 *           64-bit kernel, it's 8192.
 */
int parmapp::shell(stringstream& command, stringstream& outstr)
{
    char buff[BUFSIZ];
    FILE *fp = popen(command.str().c_str(), "r");

    while (fgets(buff, BUFSIZ, fp ) != NULL)
        outstr << buff;

    pclose(fp);
    return 0;
}

/**************************************************************
 * parmapp::getchar() - read one character and return without
 *                      waiting for user to press RETURN key
 *
 * Makes a call to the bash shell
 *
 */
char parmapp::getchar()
{
    stringstream cmd;
    stringstream ans;

    cmd << "read -n1 val; echo $val 2>&1";
    shell(cmd, ans);
    return *ans.str().c_str();
}

string parmapp::tobin(int num, int bits)
{
    string temp;
    int bit;
    temp.resize(bits);
    for (int i = 0; i < bits; ++i) {
        bit = 1 << (bits - 1 - i);
        temp[i] = bit & num ? '1' : '0';
    }
    return temp;
}

/**************************************************************
 * parmapp::toxint - convert single hex (base 16) chars to digits
 *
 */
int parmapp::toxint(char c)
{
    if (c >= '0' && c <= '9')
        return c - 48;

    if (c >= 'a' && c <= 'f')
        return c - 87;

    if (c >= 'A' && c <= 'F')
        return c - 55;

    return -1;
}

/**************************************************************
 * parmapp::getint - obtain multi-char input from user accepting
 *                   only hex or decimal characters, depending
 *                   on the parmapp::radix variable.
 *
 */
int parmapp::getint(string prompt, int curval)
{
    int num;
    bool cnvgood;
    string str;

    while (true) {
        cout << prompt;
        //cout.flush();
        getline(cin, str);

        // If user simply pressed return key, return with the
        // current value.
        //
        if(str == "")
            return curval;

        // This code converts from string to number safely.
        //
        stringstream ss(str);
        if (hexdec)
            cnvgood = (ss >> hex >> num);
        else
            cnvgood = (ss >> num);

        if (cnvgood)
            break;

        cout << "Invalid number, please try again" << endl;
    }
    return num;
}

/**************************************************************
 * parmapp::editparm - get a new value for the kmod parameter
 *
 */
void parmapp::editparm(kmodparm& parm)
{
    printf("  %-15s: %3d  :  0x%02x\n",
           parm.parmname.c_str(),
           parm.value, parm.value);
    cout << "----------------------------------\n";
    parm.value = getint("  New value: ", parm.value);
}


/**************************************************************
 * parmapp::editparmbitmask - get a new value for the kmod
 *                            parameter as a bitmask.
 *
 * Provides ability to change individual bits in the value,
 * rather than changing the whole value with one input.
 *
 */
void parmapp::editparmbitmask(kmodparm& parm)
{
    char ch;

    while (true) {
        printf("  %-15s: %3d  :  0x%02x  :  %s\n",
               parm.parmname.c_str(), parm.value, parm.value,
               tobin(parm.value, 8).c_str());
        cout << "  --------------------------------------------\n";
        cout << "  Number from 0 to 7 to toggles the corresponding bit.\n";
        cout << "  v  prompts to change the value directly\n";
        cout << "  q  quit\n\n";
        cout << "  Your choice: ";
        cout.flush();
        ch = getchar();
        cout << endl;

        switch (ch) {
        case 'q' : return;
        case 'v' : parm.value = getint("  New Value: ", parm.value); break;
        case '0' :
        case '1' :
        case '2' :
        case '3' :
        case '4' :
        case '5' :
        case '6' :
        case '7' : parm.togglebit(toxint(ch), parm.value); break;
        }
    }
}


/**************************************************************
 * parmapp::showmenu - top level menu
 *
 */
void parmapp::showmenu()
{
    bool newkmod;
    string kmod = "";
    string cu = "";         // compilation unit of the kmod

    for (uint i = 0; i < parms.size(); ++i) {

        // A new line and separator line is printed when we start
        // showing parameters from a new kmod.
        //
        if (kmod != parms[i].kmodname) {
            kmod  = parms[i].kmodname;
            newkmod = true;
            cu = parms[i].parmname.substr(0,2);
            cout << "\n  kmod: " << kmod << endl
                 << "  -----------------------------------\n";
        }

        // kmod parameters are grouped by their compilation units
        // (source files) and are set apart by a leading newline.
        //
        // Compilation unit (source file) in the kmod should be
        // identifiable with the first three characters of the
        // parameter name, since they were named with this
        // convention. If a parameter belonging to a compilation
        // unit does not have the same first three characters as
        // the other parameters in that compilation unit, then it
        // will be separated by a space.
        //
        if (cu != parms[i].parmname.substr(0,2) && !newkmod) {
            cu = parms[i].parmname.substr(0,2);
            cout << endl;
        }
        newkmod = false;

        printf("  %0x  %-15s: %3d  :  0x%02x\n", i,
               parms[i].parmname.c_str(),
               parms[i].value, parms[i].value);
    }
    cout << "\n  r  switch radix. Current input radix: "
         << getradixstr() << endl;
    cout << "  q  quit\n\n";
    cout << "  Select a parameter to edit: ";
    cout.flush();
}

/**************************************************************
 * parmapp::getmenu - top level menu parser and main program loop
 *
 */
void parmapp::getmenu()
{
    char ch;
    string parmfile = topdir + "module/";
    stringstream cmd;
    stringstream ss;

    while (true) {
        showmenu();
        ch = getchar();

        switch (ch) {
        case 'q': return;
        case 'r': toggleradix(); break;
        }

        int i = toxint(ch);
        if (i == -1 || (uint)i >= parms.size())
            continue;

        cout << endl << endl;

        if (parms[i].parmname.find("debug") == string::npos)
            editparm(parms[i]);
        else
            editparmbitmask(parms[i]);

        cmd.str("");
        ss.str("");
        parmfile += parms[i].kmodname + "/parameters/" + parms[i].parmname;
        cmd << "echo " << parms[i].value << " > " << parmfile;
        shell(cmd, ss);
    }
}

/**************************************************************
** main - the main program
***************************************************************/
int main(int argc, char** argv)
{
    cout << argv[0] << ": ipmi kmod parameter manager\n";
    parmapp pa(argc);
    pa.getmenu();
     return 0;
}

