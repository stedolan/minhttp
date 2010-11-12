/*bin/true;sed -n 's/.*.\/\*\(.*\)../\1/p' "$0"|I="$0" sh;exit;[minHTTP 80x24]*/
#define T(k)(i=(x[k]|32)-48,i-=39*(i>9),i)/*echo gcc -includesys/sendfile,sys\*/
#define I(x,y);if(d,x){y;close(c);goto g;}/*/socket,netinet/in,sys/time,sys/s\*/
#define Y(f)(close(d),d=open(f,O_RDONLY))</*tat,unistd,stdlib,string,dirent,s\*/
#define O(o)setsockopt(c,L,o,&i,sizeof(i))/*ignal,fcntl,stdio.h -DN=3 -DP=800\*/
#define S(f,a)write(i=c,r,sprintf(r,f,a));/*0 -DE=0 -DC=0 -DV=0 -DU=0 -om "$I\*/
#define H()"\r\nConnection: close\r\n\r\n"/*" $C|sed s/,/.h\ -include/g|sh;./m*/
#define R(c,h,d,a)S("HTTP/1.1 "c h H()d,a)/*#See full version for docs/config#*/
int main(){enum{Z=AF_INET,L=SOL_SOCKET,D=SO_RCVTIMEO,F=SIGPIPE,G=SO_REUSEADDR,X=
9001};struct dirent*w;int i=1,s,d=-1,c=socket(Z,SOCK_STREAM,0);char*x,*y,*h,q[X]
={0},M[]="Content-type:text/html;charset=UTF-8",f[X],r[X];struct sockaddr_in a={
Z};struct stat t;void*A=&a;a.sin_port=htons(P);O(G);if(C&&chroot(C)||bind(s=c,A,
sizeof(a))||listen(s,5)||U&&setuid(U))perror(0),exit(1);signal(F,SIG_IGN);for(i=
N;fork()&&--i;);i||pause();g:c=accept(s,A,(void*)q);{struct timeval i={5};O(D);O
(SO_SNDTIMEO);}I(read(c,q,X/2)<0||strncmp(q,"GET /",5),;)for(y=x=q+4;*x&~32;)if(
*x-37)*y++=*x++;else{*y=T(1)<<4;*y+++=T(2);x+=3;}*y=0;if((h=strstr(x+1,"\nHost:"
))&&(x=strchr(h+=7,13)))*x=0;h=h?h:"";sprintf(f,"/%s%s",h,x=q+4);i=y[-1]-47;if(Y
(y=V?f:x)0){R("404 Not Found\r\n","%s","",M)I(Y(E)0,S("<h1>%d File Not Found</h"
"1>",404))}else{fstat(d,&t);if(S_ISDIR(t.st_mode)){I(i,R("301 Moved\r\n","Locat"
"ion:http:/%s/","",f))strcpy(h=y+strlen(y),"index.html")I(Y(y)0,DIR*v;R("200 OK"
"\r\n","%s","<html><",M)*h=0;S("head><title>Index of %s</title></head><body>",x)
S("<h1>%s</h1>",x)for(v=opendir(y);w=readdir(v);){y=w->d_name;S("<a href=\"%s\""
">",y)S("%s</a><br>",y)}closedir(v);S("</body></html>",h))fstat(d,&t);}R("200 O"
"K\r\n","Content-Length: %d","",(int)t.st_size)}I(1,while(sendfile(c,d,0,X)>0))}
