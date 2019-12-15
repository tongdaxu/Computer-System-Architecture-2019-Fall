#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
#include <math.h>

using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.
#define DEBUG false

struct Branch{
    string address;
    bool result;
};

class SatCounter{
public:
    bool lhs;
    bool rhs;
    SatCounter(){
        lhs = true;
        rhs = true;
    }
    void nextState(bool now){
        if (lhs and rhs){
            /* Strong confidence in Taken
             * Taken -> Strong confidence in Taken
             * Not taken -> weak confidence in Taken
             * */
            if (now){
            } else {
                rhs = false;
            }
            return;
        }

        if (lhs and (not rhs)){
            /* Weak confidence in taken
             * Taken -> strong confidence in Taken
             * Not taken -> strong confindnce in not Taken
             * */
            if (now) {
                rhs = true;
            } else {
                lhs = false;
            }
            return;
        }

        if ((not lhs) and rhs){
            /* weak confidence in not taken
             * Taken -> strong confidence in taken
             * Not taken -> strong confidence in not taken
             * */
            if (now) {
                lhs = true;
            } else {
                rhs = false;
            }
            return;
        }

        if ((not lhs) and (not rhs)){
            /* Strong confidence in not taken
             * Taken -> weak confidence in not taken
             * Not taken -> strong confidence in not taken
             * */
            if (now){
                rhs = true;
            } else {}
            return;
        }
    }
    bool decide(){
        if (lhs){
            return true;
        } else {
            return false;
        }
    }
    void printState(){
        if (lhs and rhs){
            cout<<"strong confidence in taken"<<endl;
        }
        if ((not lhs) and rhs) {
            cout<<"weak confidence in not taken"<<endl;
        }
        if (lhs and (not rhs)){
            cout<<"weak confidence in taken"<<endl;
        }
        if ((not lhs) and (not rhs)){
            cout<<"strong confidence in not taken"<<endl;
        }
    }
};

class BPB{
public:
    /* 2^{m} * 2^{k} saturation bits
     * k bits history for each entry
     * */
    vector<vector<SatCounter>> SCs;
    string GH;

    BPB(int m, int k){
        int size = (int) pow(2, m);
        int length = (int) pow(2, k);

        vector<SatCounter> initializer;
        initializer.resize(length);
        SCs.assign(size, initializer);

        GH = string(k, '1');
    }

    void printSatet(){
        for(vector<vector<SatCounter>>::iterator it = SCs.begin(); it != SCs.end(); ++it) {
            for (vector<SatCounter>::iterator itt = it->begin(); itt != it->end(); ++itt){
                itt->printState();
            }
        }
    }
};

class Trace{
public:
    vector<Branch> trace_history;
    Trace(string trace_file_name){
        ifstream con_file;
        string line;
        int i=0;
        con_file.open(trace_file_name);
        if (con_file.is_open())
        {
            while (getline(con_file, line)){
                Branch temp_branch = {bitset<32>(stoul(line.substr(0, 8), 0, 16)).to_string(), line.substr(9, 1) == "1"};
                if (DEBUG) {cout<<"read trace address: "<<temp_branch.address<<" with value: "<<temp_branch.result<<endl;}
                trace_history.push_back(temp_branch);
            }
        } else {
            cout<<"Unable to open trace history file";
        }
    }
};

class Config{
public:
    int m;
    int k;
    Config(string con_file_name){
        ifstream con_file;
        string line;
        int i=0;
        con_file.open(con_file_name);
        if (con_file.is_open())
        {
            if (getline(con_file, line)){
                m = stoi(line);
                if (DEBUG) {cout<<"read m: "<<m<<endl;}
            } else {
                cout<<"read m failed"<<endl;
            }

            if (getline(con_file, line)){
                k = stoi(line);
                if (DEBUG) {cout<<"read k: "<<k<<endl;}
            } else {
                cout<<"read k failed"<<endl;
            }

        } else {
            cout<<"Unable to open configuration file";
        }
        con_file.close();
    }
};

int main(int argc, char **argv)
{
    /* 9: function name
     * */
    if (argc != 3){
        cout<<argc<<" arguments, are you sure?"<<endl;
        return 0;
    }

    string config_path = string(argv[1]);
    string trace_path = string(argv[2]);

    Config my_config(config_path);
    Trace my_trace(trace_path);

    BPB myBPB(my_config.m, my_config.k);
    vector<bool> trace_result;
    int counter = 1;
    int fault = 0;

    for (vector<Branch>::iterator current = my_trace.trace_history.begin(); current != my_trace.trace_history.end(); ++ current){
        int position = stoul (current->address.substr(32 - my_config.m, my_config.m), 0, 2);
        string history = myBPB.GH;
        int history_position = stoul(history, 0, 2);

        bool result = current->result;
        bool predict = myBPB.SCs[position][history_position].decide();

        trace_result.push_back(predict);

        if ((predict != result)){
            cout<<"branch["<<counter<<"] predicted as: "<<predict<<", is actually: "<<result<<endl;
            ++ fault;
        }

        /* update the state machine
         * update the history
         * */

        myBPB.SCs[position][history_position].nextState(result);

        if (result){
            myBPB.GH = '1' + history.substr(1, my_config.k - 1);
        } else {
            myBPB.GH = '0' + history.substr(1, my_config.k - 1);
        }
        ++ counter;
    }

    cout<<"in total out of "<<counter - 1<<" instructions, "<<fault<<" mistakes are made"<<endl;

    ofstream trace_out;
    trace_out.open("trace.out");
    if (trace_out.is_open())
    {
        for (vector<bool>::iterator it = trace_result.begin(); it != trace_result.end(); ++ it)
        {
            trace_out << *it <<endl;
        }

    }
    else cout<<"Unable to open file";
    trace_out.close();



    return 0;
}