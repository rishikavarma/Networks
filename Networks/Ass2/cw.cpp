#include <bits/stdc++.h>
#include<string>

using namespace std;

int main(int argc,char* argv[]){
	double ki=1,kn=1,km=1,kf=1,ps;
	int tot;
	double mss=1;
	string output;
	bool ci=false,cn=false,cm=false,cf=false,cp=false,ct=false,co=false;
	ofstream file;
	if(argc<9){
		cout<<"Input Error.\n";
		exit(0);
	}
	for(int i=1;i<argc;){
		string temp(argv[i]);
		if(!temp.compare("-i")&&!ci){
			i++;
			ki=stod(argv[i]);
			if(ki<1||ki>4){
				cout<<"Ki out of range.\n";
				exit(0);
			}
			i++; 
			ci=true;
		}
		else if(!temp.compare("-m")&&!cm){
			i++;
			km=stod(argv[i]);
			if(km<0.5||km>2){
				cout<<"Km out of range.\n";
				exit(0);
			}
			i++; 
			cm=true;
		}
		else if(!temp.compare("-n")&&!cn){
			i++;
			kn=stod(argv[i]);
			if(kn<0.5||kn>2){
				cout<<"Kn out of range.\n";
				exit(0);
			}
			i++; 
			cn=true;
		}
		else if(!temp.compare("-f")&&!cf){
			i++;
			kf=stod(argv[i]);
			if(kf<0.1||kf>0.5){
				cout<<"Kf out of range.\n";
				exit(0);
			}
			i++; 
			cf=true;
		}
		else if(!temp.compare("-s")&&!cp){
			i++;
			ps=stod(argv[i]);
			if(kf<0||kf>1){
				cout<<"Ps out of range.\n";
				exit(0);
			}
			i++; 
			cp=true;
		}
		else if(!temp.compare("-T")&&!ct){
			i++;
			tot=stoi(argv[i]);
			i++; 
			ct=true;
		}
		else if(!temp.compare("-o")&&!co){
			i++;
			string t1(argv[i]);
			output=t1;
			i++; 
			co=true;
		}
		else{
			cout<<"Input Error.\n";
			exit(0);
		}
	}
	if(!cf||!cp||!ct||!co){
		cout<<"Input Error.\n";
		exit(0);
	}
	file.open(output);
	int n=0;
	double cw=ki;
	double thresh=FLT_MAX;
	double rws=1024;
	default_random_engine generator;
    bernoulli_distribution distribution(ps);
	while(n<tot){
		int packs=ceil(cw);
		for(int j=0;j<packs&&n<tot;j++){
			if(!distribution(generator)){
				if(cw<thresh){
					cw = min(float(rws),float(cw + km));
				}
				else{
					cw = min(float(rws),float(cw + kn/cw));
				}
				file<<cw<<endl;
				n++;
			}
			else{
				thresh=cw/2;
				cw=max(float(1.0),float(kf*cw));
				file<<cw<<endl;
				n++;
				break;
			}

		}


	}

}