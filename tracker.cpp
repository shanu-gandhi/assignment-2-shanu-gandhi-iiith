#include<bits/stdc++.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h> 


#define BUFF_SIZE 512
using namespace std;
string curtrackport;
string curtrackip;

/*void changeCurUserPort(int curport, char usernm[]){
	ofstream myfile2 ("trackerdata",ios::app);
	if (myfile2.is_open())
	{
		string strline; 
		while(getline(myfile, strline))
		{
			// cout<<"line is"<<strline<<endl;
			char temp[1024];
			char * str[1024];

			strcpy(temp,strline.data());
			int index=0;
			char *ptr= strtok(temp," ");
			while(ptr != NULL)
			{
				str[index]=ptr;
				ptr = strtok(NULL, " ");
				index++;
			}
			str[index]=NULL;
			// cout<<"tok string is \n";
			if(strcmp(str[3],usernm)==0)
			{
				str[1]=to_string(curport);
			}

		}

		myfile2.close();
	}

}*/

void * thrackerThread(void * p_sockfd){
	//tracker work
	int ack=0;
	int sockfd= *((int*)p_sockfd);
	int file_size;
	char cmd[1500];
	recv(sockfd, (char*)&cmd, sizeof(cmd), 0);
	cout<<"command in tracker is \n"<<cmd<<endl;
	send(sockfd, &ack, sizeof(ack), 0);

	if(strcmp(cmd,"upload_file")==0){
		char msg[1500];
		recv(sockfd, (char*)&msg, sizeof(msg), 0);
		cout<<"line rec in tracker is \n"<<msg<<endl;
		send(sockfd, &ack, sizeof(ack), 0);
		ofstream myfile2 ("trackerdata",ios::app);
		if (myfile2.is_open())
  		{
  			myfile2<<msg<<endl;
  			myfile2.close();
		}
		else{
			cout<<"trackerdata opening error\n ";
		}
	}
	else if(strcmp(cmd,"register")==0){
		char uname[150],pass[150],line[300];
		recv(sockfd, (char*)&uname, sizeof(uname), 0);
		cout<<"uname in tracker is \n"<<uname<<endl;
		send(sockfd, &ack, sizeof(ack), 0);
		recv(sockfd, (char*)&pass, sizeof(pass), 0);
		cout<<"pass in tracker is \n"<<pass<<endl;
		send(sockfd, &ack, sizeof(ack), 0);
		strcat(uname," ");
		strcat(uname,pass);
		cout<<"final "<<uname<<"\n";
		ofstream myfile2 ("userlogindata.txt",ios::app);
		if (myfile2.is_open())
  		{
  			cout<<"writing "<<uname<<"to file\n";
  			myfile2<<uname<<endl;
  			myfile2.close();
		}
		else{
			cout<<"trackerdata opening error\n ";
		}
	}
	else if(strcmp(cmd,"login")==0){
		char uname[150],unameold[150],pass[150],line[300];
		int servport;
		recv(sockfd, (char*)&uname, sizeof(uname), 0);
		cout<<"uname in tracker is \n"<<uname<<endl;
		send(sockfd, &ack, sizeof(ack), 0);
		recv(sockfd, (char*)&pass, sizeof(pass), 0);
		cout<<"pass in tracker is \n"<<pass<<endl;
		send(sockfd, &ack, sizeof(ack), 0);

		recv(sockfd, &servport, sizeof(servport), 0);
		cout<<"serv port in tracker is \n"<<servport<<endl;
		send(sockfd, &ack, sizeof(ack), 0);

		strcpy(unameold,uname);
		strcat(uname," ");
		strcat(uname,pass);
		cout<<"final "<<uname<<"\n";
		ifstream myfile2 ("userlogindata.txt");
		int flag=0;
		if (myfile2.is_open())
  		{
  			std::string line;
		    while (getline(myfile2, line)) {
		        if(strcmp(line.c_str(),uname)==0)
		        {
		        	flag=1;
		        	//change ports corresp user
		        	// changeCurUserPort(servport,unameold);
		        	break;
		        }
		    }
		    myfile2.close();
		}
		else{
			cout<<"trackerdata opening error\n ";
				char logged[5];
				strcpy(logged,"no");
				send(sockfd, (char*)&logged, sizeof(logged), 0);
				recv(sockfd, &ack, sizeof(ack), 0);
		}
		if(flag==1){
			char logged[5];
			strcpy(logged,"yes");
			send(sockfd, (char*)&logged, sizeof(logged), 0);
			recv(sockfd, &ack, sizeof(ack), 0);
		}
		else{
			char logged[5];
			strcpy(logged,"no");
			send(sockfd, (char*)&logged, sizeof(logged), 0);
			recv(sockfd, &ack, sizeof(ack), 0);
		}
		
	}
	else if(strcmp(cmd,"d_f")==0)
  {
	char msg[1500];
	recv(sockfd, (char*)&msg, sizeof(msg), 0);
	cout<<"msg in tracker is \n"<<msg<<endl;
	send(sockfd, &ack, sizeof(ack), 0);

	ifstream myfile ("trackerdata");
	vector <string> ports;
	
	int ctprt=0, size=-1;
	if (myfile.is_open())
  	{
		string strline; 
		while(getline(myfile, strline))
		{
			// cout<<"line is"<<strline<<endl;
			char temp[1024];
			char * str[1024];

			strcpy(temp,strline.data());
			int index=0;
			char *ptr= strtok(temp," ");
			while(ptr != NULL)
			{
				str[index]=ptr;
				ptr = strtok(NULL, " ");
				index++;
			}
			str[index]=NULL;
			// cout<<"tok string is \n";
			if(strcmp(str[0],msg)==0)
			{
				printf("%s\n",str[1]);
				ports.push_back(str[1]);
				if(size==-1){
					size=atoi(str[2]);
				}
			}

		}
		for(int i =0;i<ports.size();i++){
			// cout<<"ports :"<<ports[i]<<endl;
			ctprt++;
		}
		send ( sockfd , &ctprt, sizeof(ctprt), 0);
		recv(sockfd, &ack, sizeof(ack), 0);

		
		for(int i =0;i< ctprt; i++){
			char buffer[255];
			bzero(buffer, 255);
			strcpy(buffer, ports[i].data());
			send ( sockfd , &buffer, strlen(buffer), 0);
			recv(sockfd, &ack, sizeof(ack), 0);
		}
		cout<<"sending file size :"<<size<<endl;
		send ( sockfd , &size, sizeof(size), 0);
		recv(sockfd, &ack, sizeof(ack), 0);
		
		myfile.close();
  	}
  	else{
  		cout<<"tracker data opening error";
  	}

	// recv(sockfd, &file_size, sizeof(file_size), 0);
	// // FILE *fp = fopen ( "copy"  , "wb" );
	// //FILE *fp = fopen ( to_string(fc++).data()  , "wb" );
	// FILE *fp = fopen ( msg  , "wb" );
	// 	char Buffer [ BUFF_SIZE] ; 
	// 	// int file_size;

	// 	// recv(sockfd, &file_size, sizeof(file_size), 0);
	// 	cout<<"file size\n"<<file_size<<endl;
	// 	int n;
	// 	while ( ( n = recv( sockfd , Buffer ,   BUFF_SIZE, 0) ) > 0  &&  file_size > 0){
		
	// 		fwrite (Buffer , sizeof (char), n, fp);
	// 		memset ( Buffer , '\0', BUFF_SIZE);
	// 		file_size = file_size - n;
	// 	}
	// fclose ( fp );
  }
  else if(strcmp(cmd,"l_f")==0){
  	cout<<"list files section\n";
  	ifstream myfile ("trackerdata");
	vector <string> ports;
	
	int ctprt=0, size=-1;
	if (myfile.is_open())
  	{
		string strline; 
		while(getline(myfile, strline))
		{
			// cout<<"line is"<<strline<<endl;
			char temp[1024];
			char * str[1024];

			strcpy(temp,strline.data());
			int index=0;
			char *ptr= strtok(temp," ");
			while(ptr != NULL)
			{
				str[index]=ptr;
				ptr = strtok(NULL, " ");
				index++;
			}
			str[index]=NULL;
			// cout<<"tok string is \n";
				printf("%s\n",str[0]);
				ports.push_back(str[0]);
		
		}
		auto end = ports.end();
		for (auto it = ports.begin(); it != end; ++it) {
			end = std::remove(it + 1, end, *it);
		}

		ports.erase(end, ports.end());
		for(int i =0;i<ports.size();i++){
			// cout<<"ports :"<<ports[i]<<endl;
			// cout<<"ctprt :"<<ctprt<<endl;
			ctprt++;
		}
		send ( sockfd , &ctprt, sizeof(ctprt), 0);
		recv(sockfd, &ack, sizeof(ack), 0);

		cout<<"ctprt"<<ctprt<<endl;
		for(int i =0;i< ctprt; i++){
			char buffer[255];
			bzero(buffer, 255);
			strcpy(buffer, ports[i].data());
			send ( sockfd , &buffer, strlen(buffer), 0);
			recv(sockfd, &ack, sizeof(ack), 0);
			cout<<"recved ack\n";
		}
		send ( sockfd , &size, sizeof(size), 0);
		recv(sockfd, &ack, sizeof(ack), 0);
		
		myfile.close();
  	}
  	else{
  		cout<<"tracker data opening error";
  	}

 } else if(strcmp(cmd,"s_s")==0){
  	cout<<"s s section\n";
  	ifstream myfile ("trackerdata");
  	FILE * fptr1, *fptr2;
  	char msg[1500];
	recv(sockfd, (char*)&msg, sizeof(msg), 0);
	cout<<"line rec in tracker is \n"<<msg<<endl;
	send(sockfd, &ack, sizeof(ack), 0);

	char  temp[] = "temp.txt";
	    fptr1 = fopen("trackerdata", "r");
        if (!fptr1) 
		{
                printf(" File not found or unable to open the input file!!\n");
                return 0;
        }
        fptr2 = fopen(temp, "w"); // open the temporary file in write mode 
        if (!fptr2) 
		{
                printf("Unable to open a temporary file to write!!\n");
                fclose(fptr1);
                return 0;
        }
        char * line = NULL;
	    size_t len = 0;
	    ssize_t read;
         while ((read = getline(&line, &len, fptr1)) != -1) {
	        // printf("%s", line);
	        // cout<<"line:"<<line<<endl;
	        // cout<<"mesg:"<<msg<<endl;
	        // cout<<"diff:"<<strcmp(line,msg)<<endl;
	        // cout<<"diff2:"<<line==msg<<endl;
	        if(strcmp(line,msg)!=0){
	        	fprintf(fptr2, "%s", line);
	    	}
	    }
     
        fclose(fptr1);
        fclose(fptr2);
        remove("trackerdata");  		// remove the original file 
        rename(temp, "trackerdata"); 


}

	close( sockfd);
	return NULL;
}

void * serverThread(void * arg){
	int tracker_fd = socket (AF_INET, SOCK_STREAM, 0);
  	struct sockaddr_in   addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(stoi( curtrackport ));
	addr.sin_addr.s_addr=INADDR_ANY;


	cout<<"\ncur track port no - "<<stoi( curtrackport )<<"\n";
	// cout<<"\ncur track addres - "<<addr.sin_addr.s_addr<<"\n";
	int addrlen = sizeof(sockaddr);
	
	if(	bind (tracker_fd  , (struct sockaddr *)&addr , sizeof ( addr ) ) < 0) { 
	 printf("Error binding\n");
	 return 0;
	}

	
	listen (tracker_fd, 20);
	cout<<"\nTraker Started Listening";
	while(1){
		cout<<"\nTracker while\n";
		int sockfd = accept ( tracker_fd , (struct sockaddr *)&addr , (socklen_t*)&addrlen);
		cout<<"\nport no - "<<addr.sin_port<<"\n";
		cout<<"\nConnection Established";
		
		pthread_t t2;
		pthread_create(&t2,NULL,thrackerThread,(void*)&sockfd);

	}

	close(tracker_fd);
	return NULL;


}
int main(int argc, char *argv[])
{
	if(argc!=3){
		printf("usage tracker config file , tracker no\n");
		return 0;
	}
	if(stoi(argv[2])!=1 && stoi(argv[2])!=2){
		printf("Tracker no is 1 or 2 only\n");
		return 0;
	}

	cout<<"\nTracker Starting\n";
	string track1ip , track1port,track2ip,track2port;
	cout<<"argv1 "<<argv[1]<<endl;
	ifstream myfile (argv[1]);
	
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
	  
	if(stoi(argv[2])==1){
		curtrackport=track1port;
		curtrackip=track1ip;
	}
	if(stoi(argv[2])==2){
		curtrackport=track2port;
		curtrackip=track2ip;
	}
	pthread_t t1;
	pthread_create(&t1, NULL, serverThread, NULL);
	pthread_join(t1,NULL);

  	return 0;

} 
