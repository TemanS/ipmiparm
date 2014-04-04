#include <iostream>
//#include <cstdio>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>


using namespace std;

class kmodParm {
public:
    int togglebit(int bit, int& mask) { return bit xor mask; }

private:
    string m_kmodname;
    string m_parmname;
    int    m_value;
    bool   m_isbitmask;
};

int shell(string& cmdstr, string& outstr)
{
    string fname = "cmdout.txt";
    fstream file;
    string line;
    stringstream ss;

    file.open(fname.c_str());

    if (!file.is_open()) {
        cout << "Error occurred creating file filename: " << fname << endl;
        return(1);
    }

    stringstream command;
    command << cmdstr << " > " << (const char*)fname.c_str();
    system(command.str().c_str());

    while (getline(file, line))
        ss << line << endl;

    outstr = ss.str();
    file.close();
    cout << outstr << endl;

    //::remove(fname.c_str());
    return 0;
}

int main()
{
    string ipmi_si = "ipmi_si";
    string ipmi__msg = "ipmi_msghandler";
    string cmd = "ls $HOME/module/ipmi_si/parameters/";
    string rst;

    shell(cmd, rst);
    cout << rst << endl;
    return 0;
}

