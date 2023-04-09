#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include<string.h>
#include <strings.h>
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>
char * CUR_LOC;
void cleanExit(){exit(0);}

void error(char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[]){

	signal(SIGTERM, cleanExit);
	signal(SIGINT, cleanExit);
	//포트넘버 자원해제
	

	
	int servsockfd, clisockfd, reqfd;//서버소켓, 클라이언트소켓 fd, 요청파일 fd 
	int portno;// 포트넘버
	socklen_t clilen;

	char head_buf[1000];//요청메시지 리드 버퍼
	char req_file_name[1000] ={};// 요청파일이름 저장 버퍼
	char data_buf[4096];//파일전송시 write 데이터 버퍼
	
	

	struct sockaddr_in serv_addr, cli_addr;// ip주소값에 대한 구조체(서버,클라이언트 변수 선언)
	
	int n;
	if(argc <2) {
		fprintf(stderr, "ERROR, no port provided\n");// 포트넘버 인자값 전달 예외처리
		exit(1);
	}


	servsockfd = socket(AF_INET, SOCK_STREAM,0);// tcp 소켓 생성
	if(servsockfd <0)
		error("ERROR opening socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));  
	portno = atoi(argv[1]);// portno 변수에 인자값 전달
	serv_addr.sin_family = AF_INET; //ipv4사용
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //장치의 ip주소를 할당
	serv_addr.sin_port = htons(portno); // 포트넘버 할당
	


	if(bind(servsockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) <0)//bind작업 실행 및 에러처리
		error("ERROR on binding");

	

	if(listen(servsockfd , 5) == -1) printf("listen error"); //listen 클라이언트는 최대 5명까지 listen 상태에서 대기 가능

	

	


	while(1){
		clilen = sizeof(cli_addr);// 클라이언트 주소 길이 할당
		clisockfd = accept(servsockfd, (struct sockaddr *) &cli_addr , &clilen); //소켓 accept과정 밑 에러처리
		if(clisockfd <0) error("ERROR on accept");


		//리퀘스트 메시지 리드과정(Part A)
		bzero(head_buf,999); // head_buf 초기화
		n = read(clisockfd, head_buf , 999);
		if(n < 0) error("ERROR reading from socket");

		printf("Here is the msg :  %s\n", head_buf); //리퀘스트 메시지 출력
		
		

		//==============================Part A 완료 ====================================

	
	
		//요청파일이름 추출과정
	
		int x=5, i=0;//요청메시지에서 filename 시작  인덱스# x, req_file_name 저장인덱스 초기화
		
		while(head_buf[x] != 32){
		req_file_name[i] = head_buf[x];
	        i++; x++;
		} 

		

				
		char *header_msg ;//헤더메시지 변수



		//요청파일 open 및 응답메시지 + 파일데이터 전송부분 


		//요청파일이 디렉토리 내에 위치하지 않을경우
		if((reqfd = open(req_file_name, O_RDONLY)) <  0){
			 header_msg = "HTTP/1.1 404 NOT FOUND\r\nContetn-Type: text/html\r\n\r\n";
			 write(clisockfd, header_msg, strlen(header_msg));
		}

		else{
			//여러 형식의 파일 헤더메시지 전달 및 지원하지않는 형식파일요청 시 처리


			if(strstr(req_file_name,".html"))//html 파일 요청시
			{
			header_msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html;charset=UTF-8\r\n\r\n";
			if(write(clisockfd, header_msg, strlen(header_msg)) < 0)    error("ERROR writing html type file");
			}
			

			else if(strstr(req_file_name,".gif"))//gif 파일 요청시
                        {
			header_msg = "HTTP/1.1 200 OK\r\nContent-Type: image/gif;\r\n\r\n";
                        if(write(clisockfd, header_msg, strlen(header_msg)) < 0)    error("ERROR writing html type file");
                        }

			else if(strstr(req_file_name,".jpeg") || strstr(req_file_name,".jpg"))// jpeg, jpg파일 요청시
                        {
			header_msg = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg;\r\n\r\n";
                        if(write(clisockfd, header_msg, strlen(header_msg)) < 0)    error("ERROR writing html type file");
                        }
			
			else if(strstr(req_file_name,".mp3"))//mp3 파일 요청시
                        {
			header_msg = "HTTP/1.1 200 OK\r\nContent-Type: audio/mp3;\r\n\r\n";
                        if(write(clisockfd, header_msg, strlen(header_msg)) < 0)    error("ERROR writing html type file");
                        }

			else if(strstr(req_file_name,".pdf"))// pdf 파일 요청시
                        {
			header_msg = "HTTP/1.1 200 OK\r\nContent-Type: application/pdf;\r\n\r\n";
                        if(write(clisockfd, header_msg, strlen(header_msg)) < 0)    error("ERROR writing html type file");
                        }
			
			
			else
			{
			//지원하지않은 형식의 파일 헤더메시지 전달
			header_msg = "HTTP/1.1 404 NOT FOUND\r\nContetn-Type: text/html\r\n\r\n";
                        if(write(clisockfd, header_msg, strlen(header_msg)) < 0) error("ERROR writing error msg");

			}
		
		




		//요청파일 데이터 전송파트
				             
		while (1)
	            {
        	        int datalen = read(reqfd, data_buf, 4095); // 전송할 파일 Read
	                if (datalen == 0) // 더이상 없다면 read-write작업 종료
        	            break;
	                else if (datalen < 0)
	                    error("ERROR reading data"); //read 에러
	                else
	                {
	                    if (write(clisockfd,data_buf , datalen) < 0)// write버퍼에 전송할 데이터 채움
	       	            error("ERROR writing request file");
			    
        	        }
	            } 

		
		close(reqfd);// reqfd close
		}
	   close(clisockfd); //클라이언트소켓 close
	}
	close(servsockfd);//서버소켓 close
	return 0;

   
}
