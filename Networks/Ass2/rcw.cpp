#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <climits>
#include <math.h>
using namespace std;
int rws = 1024;
int ssthresh = INT_MAX;
ofstream file;
int recvr(bool flag,int cwnd,double s,int seq_no){
	int ack_no = seq_no;
	if(flag){
		double r = ((double) rand() / (RAND_MAX));
		cout << r <<endl;
		if(r > (1-s)){ // s or (1-s) need to get clarity
			cout << "successful"<<endl;
			return ack_no;
		}
		else{
			cout << "timeout"<<endl;
			return ack_no - 1;
		}
	}
	return ack_no - 1;
}

int grow_exp(int cwnd,double m){
	int a = ceil(cwnd + m);
	return min(a,rws);
}

int grow_lin(int cwnd, double n){
	int a = ceil(cwnd + (n/cwnd));
	return min(a,rws);
}

int slow_start(int cwnd,double f){
	int a = ceil(f*cwnd);
	return max(1,a);
}
int sender(int t,int cwnd,double s,double m,double n,double f){
	int ack_no = 0;
	int seq_no = 0;
	while(seq_no < t){
		bool flag = true;
		for(int ii = 0; ii<cwnd && ii<t; ii++){
			ack_no = recvr(flag,cwnd,s,seq_no);
			if(ack_no < seq_no) flag = false;
			seq_no ++;
			if (flag){
				if(cwnd < ssthresh){
					cout << "exponential growth ... from "<< cwnd;
					cwnd = grow_exp(cwnd,m);
					cout << "to "<< cwnd << endl;
				}
				else{
					cout << "linear growth ... from "<<cwnd;
					cwnd = grow_lin(cwnd,n);
					cout << "to "<< cwnd << endl;
				}
			}
			else break;
		}
		if(!flag){
			ssthresh = int(cwnd/2);
			cout << "slow start ... from "<<cwnd;
			cwnd = slow_start(cwnd,f);
			cout << "to "<< cwnd << endl; 
			seq_no = ack_no + 1;
		}
	}
	return 0;
}

int main(int argc,char* argv[]){
	double i = 1.00,m = 1.00,n = 1.00,f,s;
	int t;
	char* filename;
	for(int j = 1; j < argc;){
		switch(argv[j++][1]){
			case('i'): i = stod(argv[j]);break;
			case('m'): m = stod(argv[j]);break;
			case('n'): n = stod(argv[j]);break;
			case('f'): f = stod(argv[j]);break;
			case('s'): s = stod(argv[j]);break;
			case('T'): t = stod(argv[j]);break;
			case('o'): filename = argv[j];break;
		}
	}
	file.open(filename);
	int cwnd = int(i);
	sender(t,cwnd,s,m,n,f);
}