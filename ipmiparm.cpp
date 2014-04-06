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

    int togglebit(int bit, int& mask) { return bit xor mask; }

    string kmodname;
    string parmname;
    int    value;

private:
    bool   m_isbitmask;
};

/**************************************************************
 * class parmapp
 *************************************************************/
class parmapp {
public:
    parmapp() {init();}
    parmapp(int test) {init(test);}
    void dump();
    int shell(stringstream& command, stringstream& outstr);
    void getmenu();
    void showmenu();
    template <typename T> void init_parms(T parm, stringstream vals);
    char getchar();
    int toxint(char c);

private:
    string topdir;
    vector<kmodparm> parms;

    void init(bool test = false);
    void init_kmod(string kmod);
};

/**************************************************************
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
 *
 */
template <typename T> void parmapp::init_parms(T parm, stringstream vals)
{

}

/**************************************************************
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
 *
 */
void parmapp::init(bool test)
{
    topdir = test ? "$HOME/" : "/sys/";
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

    while (fgets( buff, BUFSIZ, fp ) != NULL )
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

/**************************************************************
 * parmapp::toxint - convert single hex (base 16) chars to digits
 *
 */int parmapp::toxint(char c)
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
 *
 */
void parmapp::showmenu()
{
    string kmod = "";
    string cu = "";         // compilation unit of the kmod

    for (uint i = 0; i < parms.size(); ++i) {

        // A new line and separator line is printed when we start
        // showing parameters from a new kmod.
        //
        if (kmod != parms[i].kmodname) {
            kmod  = parms[i].kmodname;
            cout << "\n  kmod: " << kmod << endl
                 << "  -----------------------------\n";
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
        if (cu != parms[i].parmname.substr(0,2) && i > 0) {
            cu = parms[i].parmname.substr(0,2);
            cout << endl;
        }
        cout << "  " << setbase(16) << i << "  " << setbase(10)
             << std::left <<  setw(15) << parms[i].parmname
             << ": " << parms[i].value << endl;
    }
    cout << endl;
    cout << "  Select a parameter to edit: ";
    cout.flush();

}

/**************************************************************
 *
 */
void parmapp::getmenu()
{
    char ch;

    while (true) {
        showmenu();
        ch = getchar();

        if (ch == 'q')
            return;

        int i = toxint(ch);
        if (i == -1 || (uint)i >= parms.size())
            continue;

        kmodparm parm = parms[i];
        cout << "\ni: " << i << " kmmod: " << parm.kmodname << " "
             << "parm: " << parm.parmname << endl;

    }

}


/**************************************************************
** main - the main program
***************************************************************/
int main(int argc, char** argv)
{
    cout << argv[0] << ": ipmi kmod parameter manager\n";
    parmapp pa(argc);
    pa.dump();
    pa.getmenu();
     return 0;
}

