#include <iostream>
#include <cmath>
#include <bits/stdc++.h>

using namespace std;

int main(int argc, char* argv[])
{
    float ki = 1;
    float km = 1;
    float kn = 1;
    float kf = 0.5;
    float ps = 0.001;
    int out = 1000;
    float RWS = 1024;
    string ofile = "output.txt";
    for(int i=1; i<argc; i+=2)
    {  
        if(argv[i][1] == 'i')
            ki = stof(argv[i+1]);
        else if(argv[i][1] == 'm')
            km = stof(argv[i+1]);
        else if(argv[i][1] == 'n')
            kn = stof(argv[i+1]);
        else if(argv[i][1] == 'f')
            kf = stof(argv[i+1]);
        else if(argv[i][1] == 's')
            ps = stof(argv[i+1]);
        else if(argv[i][1] == 'T')
            out = stoi(argv[i+1]);
        else if(argv[i][1] == 'o')
            ofile = string(argv[i+1]);
        else //invalid syntax
            {
                cout << "Invalid Arguments\nExiting .. " << endl;
                exit(-1);
            }
    }
    //input done
    float threshold = FLT_MAX;
    float cwnd = ki;
    int ii = 0;
    int N =0;
    ofstream fout(ofile);
    default_random_engine generator;
    bernoulli_distribution distribution(ps);
    bool timeout;
    while(ii < out){
        N = ceil((cwnd));
        timeout = false;
        for(int jj = 0; jj < N && ii < out ;jj++){
            timeout = distribution(generator);
            if(timeout) break;

            if(threshold > cwnd)
                cwnd = min(float(RWS), float(cwnd + km));

            else cwnd = min(float(RWS),float(cwnd + kn/cwnd));
            fout << cwnd << endl ;
            ii++;
        }
        if(timeout){
            threshold = cwnd/2;
            cwnd = max((float)1.0, float(kf*cwnd));
            fout << cwnd << endl ;
            ii++;
        }

    }
    fout.close();
    return 0;
}
