/*                           _       _   _ _____ _____ ____                   */
/*                 _ __ ___ (_)_ __ | | | |_   _|_   _|  _ \                  */
/*                | '_ ` _ \| | '_ \| |_| | | |   | | | |_) |                 */
/*                | | | | | | | | | |  _  | | |   | | |  __/                  */
/*                |_| |_| |_|_|_| |_|_| |_| |_|   |_| |_|                     */
/*                                                                            */
/*                            commented version                               */
/*                                                                            */
/*                This file contains exactly the same code as                 */
/*               minhttp.c, except for whitespace and comments                */
/*                                                                            */
/*                              Stephen Dolan                                 */

#include <sys/sendfile.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>

/*** Configuration (defaults for testing only, try the values in brackets). ***/
#define N 3    /*[20]     Max number of concurrent requests (# of processes). */
#define P 8000 /*[80]     Port to listen on.                                  */
#define E 0    /*["/404"] Path to custom 404 error message, or 0 for default. */
#define C 0    /*["/web"] Document root to chroot() into (0 to serve from /). */
#define V 0    /*[1]      HTTP VHosts: if 1, docs are at DocRoot/Host/Path.   */
#define U 0    /*[65534]  UID to drop privilege to (0 to leave UID unchanged).*/

/* Decode a hex digit. T(k) is x[k] parsed as 0-9, a-f or A-F.
   i is used as scratch. x[k]|32 is a cheap trick for converting to lowercase.
   (see an ASCII table if you don't believe me :P). Subtracting 48 maps '0' to
   zero, and subtracting 39 maps 'a' to 10. */
#define T(k) ( i = (x[k]|32) - 48, i -= 39 * (i>9), i )

/* Possibly terminate the current connection. I(cond, action) performs "action"
   and accepts a new connection if "cond" is true. The "d," is a no-op to make
   the code layout fit better. It starts with a semicolon since that saves a
   semicolon at each point where it's used. */
#define I(x,y) ; if (d, x){ y; close(c); goto g; }

/* Try opening a local file. Y(filename)0 evaluates to true if the open failed,
   otherwise "d" is the file descriptor. The 0 is not part of the macro for
   space reasons (that again :D). It first closes d, to limit the number of open
   filehandles (otherwise, the server dies after a few thousand requests). */
#define Y(f) (close(d), d=open(f,O_RDONLY))<

/* O(option) performs a call to
      setsockopt(current_socket, SOL_SOCKET, option, &i, sizeof(i))
   which sets the option according to the current value of "i". */
#define O(o) setsockopt(c, L, o, &i, sizeof(i))

/* Send a response to the current client. S(format, arg) where format is a
   printf-style format and arg is an argument to the format sends the data by
   sprintf'ing it into the buffer "r" and writing it to the socket. The sprintf
   call returns the number of written bytes, so it can be passed directly to
   write. Assigning to i is filler. */
#define S(f,a) write(i=c, r, sprintf(r,f,a));

/* A useful string to prevent HTTP/1.1 clients trying to pipeline requests.
   Again, the () is just to make it fit better. */
#define H() "\r\nConnection: close\r\n\r\n"

/* Send a HTTP response. R(response, headers, data, arg) sends a HTTP response,
   where response, headers and data may contain at most one printf-style
   argument, filled by arg. It also sends the H() headers. */
#define R(c,h,d,a) S("HTTP/1.1 " c h H() d, a)

int main(){
  /* This enum gives short names to constants that are used multiple times, or
     that just fit in better here. */
  enum{ Z = AF_INET, L = SOL_SOCKET,   D = SO_RCVTIMEO,
        F = SIGPIPE, G = SO_REUSEADDR, X = 9001         };

  /* C89-compatible, so we have to declare all variables at the start of
     a block. */
  struct dirent* w; /* used to generate directory listings */
  int   i = 1,      /* used as a temporary */
        s,          /* server socket file descriptor */
        d = -1,     /* file descriptor for files being served */
        /* c is the client socket file descriptor, but it is initialised with
           the server socket since the O macro for setsockopt is defined in
           terms of c. */
        c = socket(Z/*AF_INET*/, SOCK_STREAM, 0);
  char *x,          /* points to request URI (usually) */
       *y,          /* points to request URI or physical path (usually) */
       *h,          /* points to hostname */
        q[X] = {0}, /* buffer for requests */
        M[]="Content-type:text/html;charset=UTF-8", /* constant string */
        f[X],       /* buffer for physical paths when using VHosts */
        r[X];       /* buffer for responses (used in S above) */

  /* Some C hackery: if a structure is given an incomplete initialiser, the rest
     of it is set to zero. So, initialising "a" to {AF_INET} sets the sin_port
     and sin_addr parts to zero. */
  struct sockaddr_in a={Z/*AF_INET*/};
  struct stat t;

  /* More C hackery: casts to void* and back are implicit, so getting a void*
     pointer to "a" means we don't have to write out the full cast for bind
     below. Otherwise, we'd have to write "(struct sockaddr*)a". */
  void* A = &a;

  /* since sin_addr was initialised to zero, this sets a to 0.0.0.0:P */
  a.sin_port = htons(P);

  /* set SO_REUSEADDR to 1 (i = 1 earlier) */
  O(G/*SO_REUSEADDR*/);

  /* && has higher precedence than ||, so we can avoid brackets below. Most Unix
     syscalls return 0 on success and nonzero on failure, so combining several
     with || gives you a condition which is true if any failed. */
  if(  C && chroot(C)          /* chroot if configured (i.e. C is not null) */
    || bind(s=c, A, sizeof(a)) /* bind the socket to an address */
    || listen(s, 5)            /* listen for connections */
    || U && setuid(U))         /* drop privileges if configured */
    /* If setup failed, print an error and exit. Separating the function calls
       using C's comma operator means we don't need braces around the if body,
       since it's all "one statement". */
    perror(0),
    exit(1);

  /* We will be sent a SIGPIPE if we write to a socket that the other end has
     closed. So, ignoring this signal means a client dropping won't bring down
     the server */
  signal(F,SIG_IGN);

  /* Fork off a number of worker processes to handle connections. fork() returns
     0 in the child and nonzero in the parent, so the condition in this for loop
     makes the children exit the loop immediately. */
  for(i=N; fork() && --i; );
  /* In the parent, i == 0 by this point. The parent thus calls pause and the
     children do the work. */
  i || pause();

  /* Main connection loop. This is a label because "g:goto g;" is shorter than
     "while (1){continue;}" */
 g:

  /* Accept a connection. Accept also takes a struct sockaddr* in which to store
     the peer address, but we don't care so we just pass it a random buffer. */
  c=accept(s,A,(void*)q);

  {
    /* Hackery: we make a new variable i, inside a nested block. This shadows
       the outer copy of i, so the O macro refers to our struct timeval instead
       of the enclosing scope's int. We also reuse the partial initialiser
       trick from earlier to avoid needing to initialise i.tv_usec */
    struct timeval i={5};
    O(D/*SO_RCVTIMEO*/);
    O(SO_SNDTIMEO);
  }

  /* Read from the socket into the buffer "q". If the request isn't a GET
     request, or if the read failed, we just close the socket and accept a new
     connection. This will fail on very slow HTTP clients which don't send a
     full request in the first packet (since read can return without having read
     a full request), but in practice it works. */
  I( read(c,q,X/2) < 0 ||
     strncmp(q,"GET /",5),
     ;)

  /* Percent-encoding for URLs: URLs may contain %NN where the Ns are hex
     digits. These are generally used to encode non-ascii or whitespace
     characters. They should be decoded into the byte 0xNN before the file
     is looked up in the filesystem.

     The Request-URI starts just after the "GET ", so at position q+4. The
     decoding can only make the string shorter, so we can write the output
     to the same location as the input and be sure we won't clobber unread
     data. We set x (input) and y (output) to point to the start of the URI
     and loop until we find a space character (ascii 32). */
  for(y = x = q+4; *x & ~32;)
    if (*x - 37)       /* if (x != '%'), but shorter */
      *y++ = *x++;     /* just copy the byte, not a percent-encoding */
    else{
      *y = T(1) << 4;  /* decode first hex digit (high 4 bits) */
      *y++ += T(2);    /* decode second hex digit and advance y */
      x+=3;            /* advance x 3 places ('%' and two digits) */
    }
  /* y now points to the end of the URI, which may be before the original
     end if %-encoded characters were found. The following line makes the
     URI (at q+4) a null-terminated string. */
  *y=0;

  /* HTTP/1.1 VHosting: we need to find a Host header. Technically, parsing out
     a HTTP header is quite complex: the header name is case-insensitive, it may
     be followed by any amount of whitespace, and the value may contain
     "continuation lines" (newlines followed by whitespace). However, every HTTP
     client ever emits "Host: " with that capitalisation and a single space
     and no continuation lines, so that's what we parse. */
  if(/* If a Host header is found, set h to point to the start of the line */
     (h=strstr(x+1,"\nHost:")) &&
     (x=strchr(h += 7 /* h now points to hostname */,13)))
    *x=0; /* Put a 0 at the end of the hostname */
  /* Now, h either points to a null-terminated hostname (if a Host header was
     found), or is 0 (if strstr failed). */
  h = h ? h : ""; /* set h to empty string if no hostname found */

  /* f is set to "/hostname/filename", which is the correct physical path if
     VHosts are being used. A side-effect is that "x" is set to q+4, which is
     the URI-string without a host part. */
  sprintf(f,"/%s%s",h,x=q+4);

  /* We care whether the last character of the URI is a '/' since we want to
     produce a redirect from /foo to /foo/ if foo is a directory, as this makes
     relative URIs work properly in /foo/index.html.
     if (x[strlen(x) - 1] == '/'){ i = 0; }else{ i = <nonzero>; } but shorter */
  i = y[-1] - 47;

  /* We want to open the file given by f if VHosts are enabled, or x if not. We
     use the config variable V and save the correct filename in y for later. */
  if(Y(y=V?f:x)0){
    /* Open failed, send a 404 response. */
    R("404 Not Found\r\n","%s","",M) /* M specifies a HTML Content-type. */

    /* If the config variable E is set, it gives the filename of a custom 404
       error page. If we manage to open E, we fall through to the server code
       below and serve E as if it were the original request. Otherwise, we
       send a default 404 message, close the connection and accept a new one. */
    I(Y(E)0,
      S("<h1>%d File Not Found</h" "1>",404)
    )
  }else{
    /* The file we opened may be a directory. If so:
         - if the URL doesn't end with a /, redirect to URL + "/" since relative
           links in HTML documents will fail otherwise (relative link to "bar"
           from directory "/foo" should go to "/foo/bar", not "/foobar", but
           browsers just concatenate strings for relative links).
         - next, we search for an index.html and serve that if we can open it
         - otherwise, we generate a directory listing.
    */
    fstat(d,&t);
    if (S_ISDIR(t.st_mode)){
      /* It's a directory */
      I(i, /* i is nonzero if the URL does not end with a '/', see above */
        /* The string below was broken for layout purposes (end of a line) */
        R("301 Moved\r\n","Locat" "ion:http:/%s/","",f)
      )
      /* Set h to point to the end of the URI, and append "index.html" */
      strcpy(h=y+strlen(y),"index.html")
      /* Try to open the index.html file. If we could open it, we fall through
         to the code below and serve as if it was the original requested file */
      I(Y(y)0,
        /* Couldn't open index.html, so we make a directory listing. */
        DIR* v;
        R("200 OK" "\r\n","%s","<html><", M) /* M gives a HTML Content-type */

        /* h points to the end of the path just before we added index.html, so
           putting a 0 there gets us the original directory path back in y. */
        *h=0;

        /* x points to the original request-URI without any VHosts prepended
           to form a path, so it gives us the user-visible directory name. */
        S("head><title>Index of %s</title></head><body>",x)
        S("<h1>%s</h1>",x)

        /* For each item in the directory */
        for (v = opendir(y); w = readdir(v); ){
          y = w->d_name;
          /* Output a link to this item */
          S("<a href=\"%s\"" ">",y)
          S("%s</a><br>",y)
        }
        closedir(v);

        /* Correct HTML, strangely enough. */
        S("</body></html>",h)
      )
      /* If we get here, d now points to a different file than it did when we
         last called fstat (it is now either E or an index.html file), so we
         re-run fstat to get the correct file length. */
      fstat(d,&t);
    }
    /* We're serving a file, one of:
        - the file the user asked for
        - a custom 404 error message (E)
        - an index.html file.
       Adding a Content-Length header is not strictly neccessary (since we're
       closing the connection after the file is sent), but it means we get
       proper progress bars on most browsers for large or slow downloads. */
    R("200 O" "K\r\n","Content-Length: %d","",(int)t.st_size)
  }
  /* We've gotten a file open, so we use sendfile to serve it up. sendfile will
     return 0 on end-of-file and <0 on error, so we keep going as long as it
     keeps returning a positive value. */
  I(1,
    while (sendfile(c,d,0,X)>0)
  )
  /* the previous unconditional "I" will have goto'd the accept loop, so this
     is unreachable. */
}
