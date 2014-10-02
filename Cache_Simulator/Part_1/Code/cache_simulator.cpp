#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<string.h>

using namespace std;

typedef int word_t;				//Defining Typedefs
typedef int SRC_T;

int dirty_blk=0;				//Defining constants
int tot_rmiss=0;
int tot_miss=0;
int tot_rins=0;
int tot_ins=0;

long long int lat=0;

struct myblock					//structure for cache line
{
	int myage;
	int mytag;
	char myvalid;
	char mydirty;
	int myset;
	word_t *myaddr;
};

struct mycache					//structure for cache 
{
	int mycapac;
	int myblksz;
	int myassoc;
	int mypol;
	int mytotblock;
	struct myblock *myblk;
};


void display(struct mycache *mycp){		//function to display cache contents
	int x,y,z;
	printf(" Set Valid Dirty  Tag Bits  \t Word(s)\n");
	for(x=0;x<mycp->mytotblock;x++)
	{
		z=x/mycp->myassoc;
		printf("%4d   T     %d   0x%08x\t",z,mycp->myblk[x].mydirty,rand());
		for(y=0;y<mycp->myblksz/4;++y)
		{
			printf(" 0x%08x ", mycp->myblk[x].myaddr[y]);
		}
		cout<<endl;
		
	}
}

void mycache_init(struct mycache *mycp)		//initializing the cache
{
	int x;
	mycp->mytotblock=(mycp->mycapac/mycp->myblksz);
	mycp->myblk=(struct myblock*) calloc(mycp->mytotblock,sizeof(struct myblock));
	for(x=0;x<mycp->mytotblock;++x)
	mycp->myblk[x].myaddr=(word_t *) calloc(mycp->myblksz/4,sizeof(int));
}

void mycache_access(struct mycache *mycp, SRC_T SRC, int rw)		//function for access(read/write) calls to cache
{
	int mytemp=mycp->mytotblock/mycp->myassoc;
	struct myblock *myrepl;
	word_t myaddr=SRC/(mycp->myblksz/4);
	myaddr%=mytemp;
	int flag=0,x,y;
	for(x=myaddr*mycp->myassoc;x<myaddr*mycp->myassoc+mycp->myassoc;x++)
	{
		if(mycp->myblk[x].myage){
		for(y=0;y<mycp->myblksz/4;++y)
		{
			if(SRC==mycp->myblk[x].myaddr[y]){
			flag=1;
			lat+=5;
			
			break;
			}
			
		}
		}
		if(flag) break;
	}
	if(flag) {
	mycp->myblk[x].myage=lat;
	if(rw) mycp->myblk[x].mydirty=1;
	}
	else {
		tot_miss++;
		if(!rw) tot_rmiss++;
		lat+=60;
		if(mycp->mypol){
				myrepl=&(mycp->myblk[myaddr*mycp->myassoc]);
				for(x=myaddr*mycp->myassoc+1;x<myaddr*mycp->myassoc+mycp->myassoc;x++)
				{
					if(myrepl->myage>mycp->myblk[x].myage)
					{
						myrepl=&(mycp->myblk[x]);
					}
				}
		}
		else{
					myrepl=&(mycp->myblk[rand()%(mycp->myassoc) + myaddr*mycp->myassoc]);
		}
	if(myrepl->mydirty) 		//evicting dirty block
	{
	lat+=60;
	dirty_blk++;
	}

	for(x=0;x<(mycp->myblksz/4);++x)			//replacing old cache line
	myrepl->myaddr[x]=SRC - (SRC%(mycp->myblksz/4))+x;
	if(rw) myrepl->mydirty=1;
	else myrepl->mydirty=0;
	myrepl->myage=lat;
	
	lat+=5;
	}
}
int main(int argc,char *argv[])			//main function
{
	struct mycache a;
	word_t x,y;
	int i;
	int j;
	a.mycapac=32*1024;
	a.myblksz=8;
	a.myassoc=2;
	a.mypol=1;
	for(j=1;j<argc;j+=2){				//reading arguments for cache paramters
		if(!strcmp(argv[j],"-cap")){
			a.mycapac=atoi(argv[j+1])*1024;
		}
		else if(!strcmp(argv[j],"-block")){
			a.myblksz=atoi(argv[j+1]);
		}
		else if(!strcmp(argv[j],"-assoc")){
			a.myassoc=atoi(argv[j+1]);
		}
		else if(!strcmp(argv[j],"-repl")){
			if(!strcmp(argv[j+1],"LRU")){
				a.mypol=1;
			}
			else a.mypol=0;
		}
	
	}
	cout<< "********** Cache Simulator by Gaurav Mittal, Kaushal Yagnik & Rahul Mittal **********\n";
	cout<< "Running Cache Simulator....\n\n";
	mycache_init(&a);
	while((scanf("%d",&i))!=EOF)			//checking for read or write
	{
		tot_ins++;
		if(i)
		{
			scanf("%x%x",&x,&y);
			mycache_access(&a,x,1);
		}
		else
		{
			scanf("%x",&x);
			tot_rins++;
			mycache_access(&a,x,0);
		}
	}
	cout<< ">>>>>>>>>>>>> STATISTICS <<<<<<<<<<<<<\n"<<endl;				//displaying statistics
	cout<< "Instructions : "<<tot_ins<<" "<<tot_rins<<" "<<tot_ins-tot_rins<<endl;
	cout<< "Misses : "<<tot_miss << " " << tot_rmiss << " " << tot_miss - tot_rmiss << endl;
	cout<< "Miss Rate : "<< (float)tot_miss/tot_ins << " " << (float)tot_rmiss/tot_rins << " " << (float)(tot_miss - tot_rmiss)/(tot_ins - tot_rins) << endl;
	cout<< "Number of dirty blocks evicted from the cache : "<<  dirty_blk << endl;
	cout<< "Total Number of Cycles : "<< lat << endl;
	cout<< "Average CPI : "<< (float)lat/tot_ins<<endl;
	
	cout<< "\nCACHE Contents: "<<endl<<endl;
	display(&a);
	
	
	return 0;
}
