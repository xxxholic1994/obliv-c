#include<stdio.h>
#include<bcrandom.h>
#include<obliv.h>
#include<obliv_common.h>

#ifndef REMOTEHOST
#define REMOTEHOST "localhost"
#endif

void unpackBitString(bool dest[],char src[],size_t boolCount)
{
  unsigned i,j;
  for(i=0;8*i<boolCount;i++) 
  { char ch = src[i];
    for(j=0;j<8 && i*8+j<boolCount;++j,ch>>=1) dest[i*8+j] = (ch&1);
  }
}

int main(int argc, char* argv[])
{
  int n;
  const int len = 10;
  if(argc<5 || sscanf(argv[4],"%d",&n)!=1) 
  { fprintf(stderr,"Usage %s <port> <party> <H|M> <n>\n",argv[0]); 
    return 1;
  }
	ProtocolDesc pd;
  dhRandomInit();
  int me = argv[2][0]=='1'?1:2;
  int err;
	//protocolUseStdio(&pd);
  if(me==1) err = protocolAcceptTcp2P(&pd,argv[1],1);
  else err = protocolConnectTcp2P(&pd,REMOTEHOST,argv[1],1);
  if(err!=0) { fprintf(stderr,"TCP connection error\n"); return 2; }
	setCurrentParty(&pd,me);
  BCipherRandomGen *gen = newBCipherRandomGen();
	if(me==1)
	{
		char opt0[n*len], opt1[n*len];
    randomizeBuffer(gen,opt0,n*len);
    randomizeBuffer(gen,opt1,n*len);
    OTsender s;
    if(argv[3][0]=='H') 
      s = honestOTExtSenderAbstract(honestOTExtSenderNew(&pd,2));
    else
      s = maliciousOTExtSenderAbstract(maliciousOTExtSenderNew(&pd,2));
    s.send(s.sender,opt0,opt1,n,len);
    otSenderRelease(&s);
	}
	else 
	{
		int i;
    char selpack[(n+7)/8];
    bool sel[n];
    randomizeBuffer(gen,selpack,sizeof(selpack));
    unpackBitString(sel,selpack,n);
    char output[n*len];
    OTrecver r;
    if(argv[3][0]=='H')
      r = honestOTExtRecverAbstract(honestOTExtRecverNew(&pd,1));
    else
      r = maliciousOTExtRecverAbstract(maliciousOTExtRecverNew(&pd,1));
    r.recv(r.recver,output,sel,n,len);
    otRecverRelease(&r);
	}
  releaseBCipherRandomGen(gen);
	cleanupProtocol(&pd);
	return 0;
}
