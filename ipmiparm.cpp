#include <iostream>
//#include <cstdio>
#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>

using namespace std;

/**************************************************************
 *
 */
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
 *
 */
class parmapp {
public:
    parmapp() {init();}
    parmapp(int test) {init(test);}
    void dump();
    int shell(stringstream& command, stringstream& outstr);

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
    for (unsigned i = 0; i < parms.size(); ++i) {
        cout << "kmod: " << parms[i].kmodname
             << "  parm: " << parms[i].parmname << endl;
    }
}

/**************************************************************
 *
 */
void parmapp::init_kmod(string kmod)
{
    stringstream cmd;
    stringstream ss;
    string str;
    int i = parms.size();

    cmd << "ls " << topdir << "/module/" << kmod << "/parameters/";
    shell(cmd, ss);

    while (getline(ss, str)) {
        parms.resize(parms.size() + 1);
        parms[i].kmodname = kmod;
        parms[i].parmname = str;
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
 *
 */
int parmapp::shell(stringstream& command, stringstream& outstr)
{
    string fname = "cmdout.txt";
    fstream file;
    string line;

    file.open(fname.c_str(), ios::out | ios::in | ios::trunc);

    if (!file.is_open()) {
        cout << "parmapp::shell(): Error occurred creating file filename: "
             << fname << endl;
        return 1;
    }

    command << " > " << (const char*)fname.c_str() << endl;
    system(command.str().c_str());

    while (getline(file, line))
        outstr << line << endl;

    file.close();
    ::remove(fname.c_str());
    return 0;
}

/**************************************************************
**
***************************************************************/
int main(int argc, char** argv)
{
    parmapp pa(argc);

    cout << argv[0] << ": ipmi kmod parameter manager\n\n";

    pa.dump();

     return 0;
}

