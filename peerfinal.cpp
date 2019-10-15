#include <bits/stdc++.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h> 
#include <openssl/sha.h>

#define BUFF_SIZE 1024

using namespace std;
int fc=0;
int curserverport;
int loggedin=0;
string curtrackport;
string curtrackip;
char curuser[50];

int servport1;

struct arg_struct {
    int arg1;
    char *command;
    char * arg2;
};

struct arg_dl {
    int fd;
    char *name;
    int begin;
    int end;
    int size;
    int port;
    int partsize1;
    int lpart;
};

void communicatepartwithserver(int sockfd,char* name, 
		int begin,int end,
		int size,int port,
		int partsize1, int lpart){

		int ack=0;
		cout<<"comm wit server\n";
		
		cout<<"sockfd  is "<<sockfd<<endl;	
		cout<<"name is "<<name<<endl;
		cout<<"begin"<<begin<<endl;	
		cout<<"end "<<end<<endl;	
		cout<<"size "<<size<<endl;	
		cout<<"port "<<port<<endl;	

		char msg1[1500];
		strcpy(msg1,name);
		cout<<"msg is "<<msg1<<"sending msg"<<endl;
		send(sockfd, (char*)&msg1, strlen(msg1), 0);//send fn as msg
		cout<<"msg sent waitig for ack"<<endl;
		// send(sockfd, (char*)&name, strlen(name), 0);//filen length sent is
		recv(sockfd, &ack, sizeof(ack), 0);//ack of fn
		
		send(sockfd, &begin, sizeof(begin), 0);//filen length sent is
		recv(sockfd, &ack, sizeof(ack), 0);//ack of fn

		send(sockfd, &end, sizeof(end), 0);//filen length sent is
		recv(sockfd, &ack, sizeof(ack), 0);//ack of fn

		FILE *fp = fopen ( name  , "r+b");
		fseek ( fp , 0 , begin+1);

		char Buffer [BUFF_SIZE] ; 	
		int n;

		while ( ( n = recv( sockfd , Buffer ,   BUFF_SIZE, 0) ) > 0  && end > 0){
			// cout<<"val:"<<Buffer<<endl;
			fwrite (Buffer , sizeof (char), n, fp);
			memset ( Buffer , '\0', BUFF_SIZE);
			send(sockfd, &ack, sizeof(ack), 0);//send ack of part
			end = end - n;
		}
		// cout<<"part taken till \n\n"<<endl;
		fclose ( fp );
		close( sockfd);

}

void * copyfilepart(void * arguments){
	cout<<"in copy file part\n";
	struct arg_dl *args_dl = (struct arg_dl *)arguments;
    // printf("CopyThread %d\n", args_dl -> fd);
    // int sockfd=arg_dl->fd;
    // printf("CopyThread %s\n", args_dl->name);
	printf("CopyThread begin %d\n", args_dl->begin);
    printf("CopyThread end %d\n", args_dl->end);
    // printf("CopyThread %d\n", args_dl->size);
    printf("CopyThread %d\n", args_dl->port);	
    int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	struct sockaddr_in  serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(args_dl->port);
	// serv_addr.sin_addr.s_addr=inet_addr(ip);
	serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	cout<<"client port no - "<<serv_addr.sin_port<<"\n\n";

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed in client with port %d \n",args_dl->port); 
        return NULL; 
    }
    communicatepartwithserver(sockfd, args_dl->name, 
    		args_dl->begin,args_dl->end,
    		args_dl->size,args_dl->port,
    		args_dl->partsize1 ,args_dl->lpart);
	return NULL;
}

void* copyfileserverparts(void * p_sockfd){
	int ack=0;
	int sockfd= *((int*)p_sockfd);
	// free(p_sockfd);
	int begin,end;
	char msg[1500];
	memset(msg,'\0',1500);
	cout<<"server waiting for msg"<<endl;
	recv(sockfd, (char*)&msg, sizeof(msg), 0);//receieved fn
	cout<<"size of msg is "<<sizeof(msg)<<"len of msg"<<strlen(msg)<<endl;
	cout<<"server received msg, sending acknowlegemtn"<<endl;
	send(sockfd, &ack, sizeof(ack), 0);//send ack of fn msg
	// recv(sockfd, &file_size, sizeof(file_size), 0);
	recv(sockfd, &begin, sizeof(begin), 0);
	send(sockfd, &ack, sizeof(ack), 0);//send ack begin
	
	recv(sockfd, &end, sizeof(end), 0);
	send(sockfd, &ack, sizeof(ack), 0);//send ack of end
	cout<<"fn in server whose parts to send is \n"<<msg<<endl;
	cout<<"begin in server whose parts to send is \n"<<begin<<endl;
	cout<<"end in server whose parts to send is \n"<<end<<endl;

	FILE *fp = fopen ( msg  , "rb" );
	if (fp == NULL) {
	    cout<<"Can't open file in client\n";
	}
	fseek ( fp , 0 , begin+1);
	char Buffer [ BUFF_SIZE] ; 
	int n;
	while ( end > 0 && ( n = fread( Buffer , sizeof(char) , BUFF_SIZE , fp ) ) > 0 )
	{
			
			send (sockfd , Buffer, n, 0 );
	   	 	memset ( Buffer , '\0', BUFF_SIZE);
	   	 	recv(sockfd, &ack, sizeof(ack), 0);//recv ack of part
			end = end - n ;
	}

	cout<<"Sending file part complete\n";


	fclose ( fp );
	close( sockfd);
	return NULL;

}
void* copyfileserver(void * p_sockfd){
	int sockfd= *((int*)p_sockfd);
	// free(p_sockfd);
	int file_size;
	  char msg[1500];
	recv(sockfd, (char*)&msg, sizeof(msg), 0);
	
	cout<<"\nmod fn in server is \n"<<msg<<endl;
	recv(sockfd, &file_size, sizeof(file_size), 0);
	// FILE *fp = fopen ( "copy"  , "wb" );
	//FILE *fp = fopen ( to_string(fc++).data()  , "wb" );
	FILE *fp = fopen ( msg  , "wb" );
		char Buffer [ BUFF_SIZE] ; 
		// int file_size;

		// recv(sockfd, &file_size, sizeof(file_size), 0);
		cout<<"file size\n"<<file_size<<endl;
		int n;
		while ( ( n = recv( sockfd , Buffer ,   BUFF_SIZE, 0) ) > 0  &&  file_size > 0){
		
			fwrite (Buffer , sizeof (char), n, fp);
			memset ( Buffer , '\0', BUFF_SIZE);
			file_size = file_size - n;
		}
	fclose ( fp );
	close( sockfd);
	return NULL;

}

void* serverthread(void* vargp){
	// cout<<"enter server port\n";
	// int servport1;
	// cin>>servport1;
	curserverport=servport1;

	int server_fd = socket (AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in   addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(servport1);
	addr.sin_addr.s_addr=INADDR_ANY;
	int addrlen = sizeof(sockaddr);

	bind (server_fd  , (struct sockaddr *)&addr , sizeof ( addr ) );
	
	listen (server_fd, 3);
	cout<<"\nListening\n";
	while(1){

		int sockfd = accept ( server_fd , (struct sockaddr *)&addr , (socklen_t*)&addrlen);
		cout<<"server port no - "<<addr.sin_port<<endl;
		cout<<"Connection Established in server\n";
		
		pthread_t t;
		// pthread_create(&t,NULL,copyfileserver,(void*)&sockfd);
		pthread_create(&t,NULL,copyfileserverparts ,(void*)&sockfd);
		// pthread_join(t,NULL);
	}
	close( server_fd);
	return NULL;

}

void* copyfileclient(void *arguments){
		int ack=0;
		struct arg_struct *args = (struct arg_struct *)arguments;
	    // printf("%d\n", args -> arg1);
	    // printf("%d\n", args -> arg2);
	    int sockfd=args->arg1;
	    char *fn=(char*)malloc(100*sizeof(char));
	    fn=args->arg2;

	    char *command=(char*)malloc(100*sizeof(char));
	    command=args->command;
	    
	    // cout<<"command is "<<command<<endl;
		// cout<<"sockfd  is "<<sockfd<<endl;	
		// cout<<"args 1 is "<<args->arg1<<endl;
		// cout<<"fn is "<<fn<<endl;	
		// cout<<"args 2 is "<<args->arg2<<endl;	
		// const char * copy="copy";
		char cmd[1500];
		strcpy(cmd,command);
		send(sockfd, (char*)&cmd, strlen(cmd), 0);
		recv(sockfd, &ack, sizeof(ack), 0);
		// cout<<"\nack of command recv\n"
		
		char msg[1500];
		strcpy(msg,fn);
		send(sockfd, (char*)&msg, strlen(msg), 0);
		recv(sockfd, &ack, sizeof(ack), 0);
		int peerscount=0;
	
		recv(sockfd, &peerscount, sizeof(peerscount), 0);
		send(sockfd, &ack, sizeof(ack), 0);

		cout<<"file found in peers:"<<peerscount<<endl;
		if(peerscount==0){
			cout<<"File Not Found\n";
			close(sockfd);
			return NULL;
		}
		vector <int> portlist;
		for(int i =0; i<peerscount ; i++){
			char buffer[255];
			int ack=1;
			bzero(buffer, 255);
			recv(sockfd, &buffer, sizeof(buffer), 0);
			cout<<"ports are :"<<buffer<<endl;
			portlist.push_back(atoi(buffer));
			send ( sockfd , &ack, sizeof(ack), 0);
		}
		int sizef=-1;
		recv(sockfd, &sizef, sizeof(sizef), 0);
		cout<<"file size received is"<<sizef<<endl;
		send(sockfd, &ack, sizeof(ack), 0);
	
		FILE *fp = fopen ( fn  , "wb" );//create dummy file
		char Buffer [ BUFF_SIZE] ; 
		char nl[1];
		nl[0]='\0';
		size_t n=1;
		int size2=sizef;
		while (sizef > 0){
			fwrite (nl , sizeof (char),n , fp);
			sizef = sizef - n;
		}


		/*pthread_t copyfilepartthread[peerscount];
		int threadno=0;
		int _512chunks=size2/512000;
		cout<<"512 chunks are"<<_512chunks<<endl;
		if(size2%512000==0){
			int perpeer=_512chunks/peerscount;
			cout<<"per peer chunks are"<<perpeer<<endl;
			if(_512chunks%peerscount==0){
				//data per peer 512chunk/peerscount
				int partsize=_512chunks/peerscount;
				if(_512chunks<peerscount) partsize=1;
				cout<<"2nd case partsize is "<<partsize<<endl;
				int begin=0,end=begin+(partsize*512000);
				// cout<<"begin:"<<begin<<"end:"<<end<<endl;
				while(end!=size2){
					// sleep(1);
					struct arg_dl *args_dl=(struct arg_dl*)malloc(sizeof(struct arg_dl));
					args_dl->fd=sockfd;
					args_dl->name=fn;
					args_dl->begin=begin;
					args_dl->end=end;
					args_dl->size=size2;
					args_dl->port=portlist[threadno%peerscount];
					args_dl->partsize1=partsize;
					args_dl->lpart=0;
					// pthread_t copyfilepartthread;
					pthread_create(&copyfilepartthread[threadno],NULL,copyfilepart,(void *)args_dl);
					cout<<"Thread no"<<threadno<<endl;
					cout<<"port :"<<portlist[threadno%peerscount]<<endl;
					threadno++;
					cout<<"begin:"<<begin<<"end:"<<end<<endl<<endl;
					begin=end+1;
					if(begin+(partsize*512000)-1<=size2)
						end=begin+(partsize*512000)-1;
					else
						end=size2;
				}
				struct arg_dl *args_dl=(struct arg_dl*)malloc(sizeof(struct arg_dl));
				args_dl->fd=sockfd;
				args_dl->name=fn;
				args_dl->begin=begin;
				args_dl->end=end;
				args_dl->size=size2;
				args_dl->port=portlist[threadno%peerscount];
				args_dl->partsize1=partsize;
				args_dl->lpart=1;
				// pthread_t copyfilepartthread;
				pthread_create(&copyfilepartthread[threadno],NULL,copyfilepart,(void *)args_dl);				
				cout<<"Thread no"<<threadno<<endl;
				cout<<"port :"<<portlist[threadno%peerscount]<<endl;
				cout<<"begin:"<<begin<<"end:"<<end<<endl<<endl;
				threadno++;
			}
			else{//512cunks%peercount!=0
				int partsize=_512chunks/peerscount;
				if(_512chunks<peerscount) partsize=1;
				cout<<"1st case partsize is "<<partsize<<endl;
				int begin=0,end=begin+(partsize*512000);
				// cout<<"begin:"<<begin<<"end:"<<end<<endl;
				while(end<size2){
					// sleep(1);
					struct arg_dl *args_dl=(struct arg_dl*)malloc(sizeof(struct arg_dl));
					args_dl->fd=sockfd;
					args_dl->name=fn;
					args_dl->begin=begin;
					args_dl->end=end;
					args_dl->size=size2;
					args_dl->port=portlist[threadno%peerscount];
					args_dl->partsize1=partsize;
					args_dl->lpart=0;
					// pthread_t copyfilepartthread;
					pthread_create(&copyfilepartthread[threadno],NULL,copyfilepart,(void *)args_dl);

					cout<<"Thread no"<<threadno<<endl;
					cout<<"port :"<<portlist[threadno%peerscount]<<endl;
					threadno++;
					cout<<"begin!:"<<begin<<"end!:"<<end<<endl<<endl;
					begin=end+1;
					if(begin+(partsize*512000)-1<=size2)
						end=begin+(partsize*512000)-1;
					else
						end=size2;
				}
				struct arg_dl *args_dl=(struct arg_dl*)malloc(sizeof(struct arg_dl));
				args_dl->fd=sockfd;
				args_dl->name=fn;
				args_dl->begin=begin;
				args_dl->end=end;
				args_dl->size=size2;
				args_dl->port=portlist[threadno%peerscount];
				args_dl->partsize1=partsize;
				args_dl->lpart=1;
				// pthread_t copyfilepartthread;
				pthread_create(&copyfilepartthread[threadno],NULL,copyfilepart,(void *)args_dl);				
				cout<<"Thread no"<<threadno<<endl;
				cout<<"port :"<<portlist[threadno%peerscount]<<endl;
				cout<<"begin!:"<<begin<<"end!:"<<end<<endl<<endl;
				threadno++;
			}
		}
		else{
			_512chunks++;
			int perpeer=_512chunks/peerscount;
			cout<<"!per peer chunks are"<<perpeer<<endl;
			if(_512chunks%peerscount==0){
				//data per peer 512chunk/peerscount
				int partsize=_512chunks/peerscount;
				if(_512chunks<peerscount) partsize=1;
				cout<<"3rd case !partsize is "<<partsize<<endl;
				int begin=0,end=begin+(partsize*512000);
				// cout<<"begin:"<<begin<<"end:"<<end<<endl;
				while(end!=size2){
					// sleep(1);
					struct arg_dl *args_dl=(struct arg_dl*)malloc(sizeof(struct arg_dl));
					args_dl->fd=sockfd;
					args_dl->name=fn;
					args_dl->begin=begin;
					args_dl->end=end;
					args_dl->size=size2;
					args_dl->port=portlist[threadno%peerscount];
					args_dl->partsize1=partsize;
					args_dl->lpart=0;
					// pthread_t copyfilepartthread;
					pthread_create(&copyfilepartthread[threadno],NULL,copyfilepart,(void *)args_dl);					
					cout<<"Thread no"<<threadno<<endl;
					cout<<"port :"<<portlist[threadno%peerscount]<<endl;
					threadno++;
					cout<<"!begin:"<<begin<<"end:"<<end<<endl<<endl;
					begin=end+1;
					if(begin+(partsize*512000)-1<=size2)
						end=begin+(partsize*512000)-1;
					else
						end=size2;
				}
				struct arg_dl *args_dl=(struct arg_dl*)malloc(sizeof(struct arg_dl));
				args_dl->fd=sockfd;
				args_dl->name=fn;
				args_dl->begin=begin;
				args_dl->end=end;
				args_dl->size=size2;
				args_dl->port=portlist[threadno%peerscount];
				args_dl->partsize1=partsize;
				args_dl->lpart=1;
				// pthread_t copyfilepartthread;
				pthread_create(&copyfilepartthread[threadno],NULL,copyfilepart,(void *)args_dl);				
				cout<<"Thread no"<<threadno<<endl;
				cout<<"port :"<<portlist[threadno%peerscount]<<endl;
				cout<<"begin:"<<begin<<"end:"<<end<<endl<<endl;
				threadno++;
			}
			else{//512cunks%peercount!=0
				cout<<"512 c"<<_512chunks<<"peerscount"<<peerscount<<endl;
				int partsize=_512chunks/peerscount;
				if(_512chunks<peerscount) partsize=1;
				cout<<"4rth case partsize is "<<partsize<<endl;
				int begin=0,end=begin+(partsize*512000);
				// cout<<"begin:"<<begin<<"end:"<<end<<endl;
				while(end<size2){
					// sleep(1);
					struct arg_dl *args_dl=(struct arg_dl*)malloc(sizeof(struct arg_dl));
					args_dl->fd=sockfd;
					args_dl->name=fn;
					args_dl->begin=begin;
					args_dl->end=end;
					args_dl->size=size2;
					args_dl->port=portlist[threadno%peerscount];
					args_dl->partsize1=partsize;
					args_dl->lpart=0;
					// pthread_t copyfilepartthread;
					pthread_create(&copyfilepartthread[threadno],NULL,copyfilepart,(void *)args_dl);					
					cout<<"Thread no"<<threadno<<endl;
					cout<<"port :"<<portlist[threadno%peerscount]<<endl;
					threadno++;
					cout<<"begin!:"<<begin<<"end!:"<<end<<endl<<endl;
					begin=end+1;
					if(begin+(partsize*512000)-1<=size2)
						end=begin+(partsize*512000)-1;
					else
						end=size2;
				}
				struct arg_dl *args_dl=(struct arg_dl*)malloc(sizeof(struct arg_dl));
				args_dl->fd=sockfd;
				args_dl->name=fn;
				args_dl->begin=begin;
				args_dl->end=end;
				args_dl->size=size2;
				args_dl->port=portlist[threadno%peerscount];
				args_dl->partsize1=partsize;
				args_dl->lpart=1;
				pthread_create(&copyfilepartthread[threadno],NULL,copyfilepart,(void *)args_dl);				
				cout<<"Thread no"<<threadno<<endl;
				cout<<"port :"<<portlist[threadno%peerscount]<<endl;
				cout<<"begin!:"<<begin<<"end!:"<<end<<endl<<endl;
				threadno++;
			}			
			
		}
		*/int partsize=(size2/peerscount);
		pthread_t copyfilepartthread[peerscount];
		int threadno=0;
		if(size2%peerscount==0){
			long long begin=0,end=(size2/peerscount);
			while(end != (size2)){
				// sleep(1);

			    struct arg_dl *args_dl=(struct arg_dl*)malloc(sizeof(struct arg_dl));
				args_dl->fd=sockfd;
				args_dl->name=fn;
				args_dl->begin=begin;
				args_dl->end=end;
				args_dl->size=size2;
				args_dl->port=portlist[threadno%peerscount];
				// pthread_t copyfilepartthread;
				pthread_create(&copyfilepartthread[threadno],NULL,copyfilepart,(void *)args_dl);
				// pthread_join(copyfilepartthread[threadno],NULL);
				cout<<"dist begin"<<begin<<endl;
				cout<<"dits end"<<end<<endl;
				cout<<"dist port"<<args_dl->port<<endl<<endl;
				threadno++;
				begin=end+1;
				if(begin+partsize-1<=size2)
					end=begin+partsize-1;
				else
					end=size2;
			}
				struct arg_dl *args_dl=(struct arg_dl*)malloc(sizeof(struct arg_dl));
				args_dl->fd=sockfd;
				args_dl->name=fn;
				args_dl->begin=begin;
				args_dl->end=end;
				args_dl->port=portlist[threadno%peerscount];
				args_dl->size=size2;
				// pthread_t copyfilepartthread;
				pthread_create(&copyfilepartthread[threadno],NULL,copyfilepart,(void *)args_dl);
				// pthread_join(copyfilepartthread[threadno],NULL);
				cout<<"dist begin"<<begin<<endl;
				cout<<"dist end"<<end<<endl;
				cout<<"dist port"<<args_dl->port<<endl<<endl;
				threadno++;

				for(int i=0;i<peerscount ;i ++){
					// pthread_join(copyfilepartthread[i],NULL);
				}//wait
		}
		else{
			long long begin=0,end=(size2/peerscount)-1;
			while(end< (size2)){
				// sleep(1);
				struct arg_dl *args_dl=(struct arg_dl*)malloc(sizeof(struct arg_dl));
				args_dl->fd=sockfd;
				args_dl->name=fn;
				args_dl->begin=begin;
				args_dl->end=end;
				args_dl->port=portlist[threadno%peerscount];
				args_dl->size=size2;
				// pthread_t copyfilepartthread;
				pthread_create(&copyfilepartthread[threadno],NULL,copyfilepart,(void *)args_dl);
				// pthread_join(copyfilepartthread[threadno],NULL);
				cout<<"dist beginelse"<<begin<<endl;
				cout<<"dist endelse"<<end<<endl;
				cout<<"dist port"<<args_dl->port<<endl<<endl;
				threadno++;
				begin=end+1;
				// end=begin+partsize-1;
				if(begin+partsize-1<=size2)
					end=begin+partsize-1;
				else
					end=size2;
			}
			struct arg_dl *args_dl=(struct arg_dl*)malloc(sizeof(struct arg_dl));
			args_dl->fd=sockfd;
			args_dl->name=fn;
			args_dl->begin=begin;
			args_dl->end=end;
			args_dl->port=portlist[threadno%peerscount];
			args_dl->size=size2-1;
			// pthread_t copyfilepartthread;
			pthread_create(&copyfilepartthread[threadno],NULL,copyfilepart,(void *)args_dl);
			// pthread_join(copyfilepartthread[threadno],NULL);
			cout<<"dist beginelse"<<begin<<endl;
			cout<<"dist endelse"<<(size2)<<endl;
			cout<<"dist port"<<args_dl->port<<endl<<endl;
			threadno++;

			for(int i=0;i<peerscount ;i ++){
					// pthread_join(copyfilepartthread[i],NULL);
				}
		}

		// struct arg_dl *args_dl=(struct arg_dl*)malloc(sizeof(struct arg_dl));
		// args_dl->fd=sockfd;
		// args_dl->name=fn;
		// args_dl->begin=;
		// args_dl->end=;
		// args_dl->size=;
		

		fclose ( fp );


		close( sockfd);
		return NULL;

}

void * clientthread(void* vargp ){
	// sleep(5);
	int port=0;
	port=stoi(curtrackport);
	while(1){
		beginloop:
		int ack=0;
		char * fn=(char*)malloc(100*sizeof(char));
		char * command=(char*)malloc(100*sizeof(char));
		string line;
		// char* ip;
		// memset(fn,'/0',sizeof(fn));
		// memset(command,'/0',sizeof(command));
		// strcpy(line,"");
		// cout<<"enter port to connect\n";
		// cin>>port;
		cout<<"enter command\n";
		cin>>command;
		if(strcmp(command,"logout")==0){
			loggedin=0;
			// curuser.clear();
			memset ( curuser , '\0', 50);
			return NULL;
		}
		if(strcmp(command,"s_s")==0){
			cout<<"enter filname\n";
			cin>>fn;
			
			int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
			struct sockaddr_in  serv_addr;
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_port = htons( port );
			serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
			if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
		    { 
		        printf("\nConnection Failed in client ss sec \n"); 
		        return NULL; 
		    }
			cout<<"ss sec\n";
			char cmd[1500];
			strcpy(cmd,command);
			send(sockfd, (char*)&cmd, strlen(cmd), 0);
			recv(sockfd, &ack, sizeof(ack), 0);


			FILE *fp = fopen ( fn  , "rb" );
			if (fp == NULL) {
			    cout<<"Can't open file in client\n";
			}
			fseek ( fp , 0 , SEEK_END);
			long long size = ftell ( fp );

			line.append(fn);
			line.append(" ");

			line.append(to_string(curserverport));
			line.append(" ");

			line.append(to_string(size));


			line.append(" ");
			line.append(curuser);

			line.append("\n");

			cout<<"line to upload "<<line<<endl;
			char  linetoupload[1500];
			strcpy(linetoupload,line.data());
			cout<<"here port"<<port<<endl;
			
		    {
		    	cout<<"connect estb in client removing fn: "<<linetoupload<<endl;
		    	send(sockfd, (char*)&linetoupload, strlen(linetoupload), 0);
				recv(sockfd, &ack, sizeof(ack), 0);
				close(sockfd);
		    }
		    

			continue;

		}
		else if(strcmp(command,"l_f")==0){
			int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
			struct sockaddr_in  serv_addr;
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_port = htons( port );
			serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
			if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
		    { 
		        printf("\nConnection Failed in client upl sec \n"); 
		        return NULL; 
		    }
			cout<<"list sec\n";
			char cmd[1500];
			strcpy(cmd,command);
			send(sockfd, (char*)&cmd, strlen(cmd), 0);
			recv(sockfd, &ack, sizeof(ack), 0);
			int peerscount=0;
	
			recv(sockfd, &peerscount, sizeof(peerscount), 0);
			send(sockfd, &ack, sizeof(ack), 0);

			cout<<"file found in peers:"<<peerscount<<endl;
			if(peerscount==0){
				cout<<"No sharebale files Found\n";
				close(sockfd);
				return NULL;
			}
			cout<<"peerscount"<<peerscount<<endl;
			for(int i =0; i<peerscount; i++){
				char buffer[255];
				int ack=1;
				bzero(buffer, 255);
				recv(sockfd, &buffer, sizeof(buffer), 0);
				cout<<"shared files are :"<<buffer<<endl;
				send ( sockfd , &ack, sizeof(ack), 0);
				// cout<<"send ack\n";
			}
			// close(sockfd);
			int sizef=-1;
			recv(sockfd, &sizef, sizeof(sizef), 0);
			// cout<<"file size received is"<<sizef<<endl;
			send(sockfd, &ack, sizeof(ack), 0);
			// cout<<"conn closed\n";
			goto beginloop;

		}else{
		cout<<"enter filname\n";
		cin>>fn;
		if(strcmp(command,"u_f")==0){
			int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
			struct sockaddr_in  serv_addr;
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_port = htons( port );
			serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
			if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
		    { 
		        printf("\nConnection Failed in client upl sec \n"); 
		        return NULL; 
		    }
			cout<<"upload sec\n";
			char cmd[1500];
			strcpy(cmd,command);
			send(sockfd, (char*)&cmd, strlen(cmd), 0);
			recv(sockfd, &ack, sizeof(ack), 0);

			FILE *fp = fopen ( fn  , "rb" );
			if (fp == NULL) {
			    cout<<"Can't open file in client\n";
			}
			fseek ( fp , 0 , SEEK_END);
			long long size = ftell ( fp );
			cout<<"File To copy size sending to client\n"<<size<<endl;
			rewind ( fp );
			fclose( fp );

			line.append(fn);
			line.append(" ");

			line.append(to_string(curserverport));
			line.append(" ");

			line.append(to_string(size));


			line.append(" ");
			line.append(curuser);
			
			cout<<"line to upload "<<line<<endl;
			char  linetoupload[1500];
			strcpy(linetoupload,line.data());
			cout<<"here port"<<port<<endl;
			
		    {
		    	cout<<"connect estb in client sending fn: "<<linetoupload<<endl;
		    	send(sockfd, (char*)&linetoupload, strlen(linetoupload), 0);
				recv(sockfd, &ack, sizeof(ack), 0);
				close(sockfd);
		    }
		    

			continue;
		}		
		// cout<<"enter ip to connect\n";
		// cin>>ip;	
		// cout<<"here port"<<port<<endl;
		// cout<<"sockfd"<<sockfd;
		else if(strcmp(command,"d_f")==0){
			int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
			struct sockaddr_in  serv_addr;
			serv_addr.sin_family = AF_INET;
			cout<<"prrt in dl client "<<port<<"sockfd "<<sockfd<<endl;
			serv_addr.sin_port = htons( port );
			// serv_addr.sin_addr.s_addr=inet_addr(ip);
			serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
			// cout<<"\nclient port no - "<<serv_addr.sin_port<<"\n";


			if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
		    { 
		        printf("\nConnection Failed in client dl sec \n"); 
		        return NULL; 
		    }

		    cout<<"Connection Established\n";
		    struct arg_struct *args=(struct arg_struct*)malloc(sizeof(struct arg_struct));
			args->arg1=sockfd;
			// strcpy(args->arg2,fn);
			args->arg2=fn;
			args->command=command;
			// cout<<"sockfd in client is "<<sockfd<<endl;	
			// cout<<"args 1 in client is "<<args->arg1<<endl;
			// cout<<"fn in client is "<<fn<<endl;	
			// cout<<"args 2 in client is "<<args->arg2<<endl;	

			

			pthread_t copyfileclientthread;
			pthread_create(&copyfileclientthread,NULL,copyfileclient,(void *)args);

		}
		}
	}

	return NULL;
}
bool createlogin(){
	int ack=0;
	int port=stoi(curtrackport);
	int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	struct sockaddr_in  serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	cout<<"here\n";
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed in client cl sec \n"); 
        return NULL; 
    }
    char msg1[1500],user[1500],passwd[1500];
	cout<<"Connection Established\n";
	cout<<"enter username\n";
	cin>>user;
	cout<<"enter password\n";
	cin>>passwd;
	strcpy(msg1,"register");
	// cout<<"msg is "<<msg1<<"sending msg"<<endl;
	send(sockfd, (char*)&msg1, strlen(msg1), 0);
	recv(sockfd, &ack, sizeof(ack), 0);//ack of fn

	// send(sockfd, (char*)&cmd, strlen(cmd), 0);//send fn as msg
	// recv(sockfd, &ack, sizeof(ack), 0);//ack of fn
	send(sockfd, (char*)&user, strlen(user), 0);//send fn as msg
	recv(sockfd, &ack, sizeof(ack), 0);//ack of fn
	send(sockfd, (char*)&passwd, strlen(passwd), 0);//send fn as msg
	recv(sockfd, &ack, sizeof(ack), 0);//ack of fn
	return true;
}
bool checklogin(char arr[],char arr2[]){
	int ack=0;
	int port=stoi(curtrackport);
	int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	struct sockaddr_in  serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	cout<<"here\n";
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed in client cl sec \n"); 
        return NULL; 
    }
    char msg1[150],user[150],passwd[150],status[150];
	cout<<"Connection Established\n";
	cout<<"enter username\n";
	cin>>user;
	cout<<"enter password\n";
	cin>>passwd;
	strcpy(msg1,"login");
	// cout<<"msg is "<<msg1<<"sending msg"<<endl;
	send(sockfd, (char*)&msg1, strlen(msg1), 0);
	recv(sockfd, &ack, sizeof(ack), 0);//ack of fn

	// send(sockfd, (char*)&cmd, strlen(cmd), 0);//send fn as msg
	// recv(sockfd, &ack, sizeof(ack), 0);//ack of fn
	send(sockfd, (char*)&user, strlen(user), 0);//send fn as msg
	recv(sockfd, &ack, sizeof(ack), 0);//ack of fn
	send(sockfd, (char*)&passwd, strlen(passwd), 0);//send fn as msg
	recv(sockfd, &ack, sizeof(ack), 0);//ack of fn

	send(sockfd, &servport1, sizeof(servport1), 0);//send fn as msg
	recv(sockfd, &ack, sizeof(ack), 0);//ack of fn	

	recv(sockfd, (char*)&status, strlen(status), 0);//send fn as msg
	cout<<"status rec is"<<status<<endl;
	send(sockfd, &ack, sizeof(ack), 0);//ack of fn
	if(strcmp(status,"yes")==0){
		strcpy(curuser,user);
		return true;
	}
	return false;
}
int main(int argc, char *argv[])//./a.out  127.0.0.1 8075 trackerinfo.txt
{
	if(argc!=4){
		printf("usage ip, port, trackerinfo.txt\n");
		return 0;
	}
	servport1=stoi(argv[2]);
	cout<<"\nTracker fileReading\n";
	string track1ip , track1port,track2ip,track2port;
	cout<<"argv1 "<<argv[3]<<endl;
	ifstream myfile (argv[3]);
	
	if (myfile.is_open())
  	{
		string str; 
		std::getline(myfile, str);
		track1ip=str;
		cout<<"track1ip "<<track1ip<<endl;
		str.clear();

		std::getline(myfile, str);
		track1port=str;
		cout<<"track1p "<<track1port<<endl;
		str.clear();

		std::getline(myfile, str);
		track2ip=str;
		cout<<"track2ip "<<track2ip<<endl;
		str.clear();

		std::getline(myfile, str);
		track2port=str;
		cout<<"track2p "<<track2port<<endl;
		str.clear();    
 	    myfile.close();
  	}
  	else {
  		printf("Unable to open file\n"); 
  		return 0;
  	}
	  
		curtrackport=track1port;
		curtrackip=track1ip;

	cout<<"curtrackport"<<curtrackport<<endl;
	cout<<"curtrackip"<<curtrackip<<endl;
	while(loggedin==0){
		char command[50],user[50],passwd[50];
		cout<<"enter command\n";
		cin>>command;
		if(strcmp(command,"login")==0 || strcmp(command,"register")==0){
			if(strcmp(command,"login")==0){
				if(checklogin(user,passwd)==true){
					loggedin=1;
					cout<<"logged in successfully\n";
					break;
				}else{
					cout<<"incorrect login try again\n";
				}
			}
			else{
				if(createlogin()==true){
					cout<<"User created success\n";
				}else{
					cout<<"User created fail\n";
				}
			}
		}
		else{
			cout<<"login or register first"<<endl;
		}
	}
	pthread_t t1,t2;
	pthread_create(&t2,NULL,serverthread,NULL);
	// pthread_join(t2,NULL);

	pthread_create(&t1,NULL,clientthread,NULL);

	pthread_join(t1,NULL);

	return 0;
}