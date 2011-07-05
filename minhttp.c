#include <sys/sendfile.h> /*               _       _   _ _____ _____ ____     */
#include   <sys/socket.h> /*     _ __ ___ (_)_ __ | | | |_   _|_   _|  _ \    */
#include   <netinet/in.h> /*    | '_ ` _ \| | '_ \| |_| | | |   | | | |_) |   */
#include     <sys/time.h> /*    | | | | | | | | | |  _  | | |   | | |  __/    */
#include     <sys/stat.h> /*    |_| |_| |_|_|_| |_|_| |_| |_|   |_| |_|       */
#include       <unistd.h> /*   A tiny and surprisingly featureful webserver   */
#include       <stdlib.h> /* -Source is smaller than Apache's config file.    */
#include       <string.h> /* -Compiles faster than lighttpd starts up.        */
#include       <dirent.h> /* -Actual code fits in an 80x24 screen.            */
#include       <signal.h> /* -Portable (pure POSIX), Unicode support.         */
#include        <fcntl.h> /* -Directory listings (if no index.html is found). */
#include        <stdio.h> /* -Fast sendfile() I/O, timeouts to prevent DoS.   */
/*** Configuration (defaults for testing only, try the values in brackets). ***/
#define N 3    /*[20]     Max number of concurrent requests (# of processes). */
#define P 8000 /*[80]     Port to listen on.                                  */
#define E 0    /*["/404"] Path to custom 404 error message, or 0 for default. */
#define C 0    /*["/web"] Document root to chroot() into (0 to serve from /). */
#define V 0    /*[1]      HTTP VHosts: if 1, docs are at DocRoot/Host/Path.   */
#define U 0    /*[65534]  UID to drop privilege to (0 to leave UID unchanged).*/
#define T(k)(i=(x[k]|32)-48,i-=39*(i>9),i) /***********************************/
#define I(x,y);if(d,x){y;close(c);goto g;} /*                                 */
#define Y(f)(close(d),d=open(f,O_RDONLY))< /*    (c) Stephen Dolan, 2010      */
#define O(o)setsockopt(c,L,o,&i,sizeof(i)) /*  minHTTP is provided under the  */
#define S(f,a)write(i=c,r,sprintf(r,f,a)); /*  terms of the X11 License.      */
#define H()"\r\nConnection: close\r\n\r\n" /*                                 */
#define R(c,h,d,a)S("HTTP/1.1 "c h H()d,a) /***********************************/
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
