
#include "chirp_protocol.h"
#include "chirp_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include <unistd.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

static int tcp_connect( const char *host, int port );
static void chirp_fatal_request( const char *name );
static void chirp_fatal_response();
static int get_result( FILE *s );
static int convert_result( int response );
static int simple_command(struct chirp_client *c,char const *fmt,...);
static void vsprintf_command(char *command,char const *fmt,va_list args);

struct chirp_client {
	FILE *stream;
};

struct chirp_client * chirp_client_connect_default()
{
	FILE *file;
	int fields;
	int save_errno;
	struct chirp_client *client;
	char host[CHIRP_LINE_MAX];
	char cookie[CHIRP_LINE_MAX];
	int port;
	int result;

	file = fopen("chirp.config","r");
	if(!file) return 0;

	fields = fscanf(file,"%s %d %s",host,&port,cookie);
	fclose(file);

	if(fields!=3) {
		errno = EINVAL;
		return 0;
	}

	client = chirp_client_connect(host,port);
	if(!client) return 0;

	result = chirp_client_cookie(client,cookie);
	if(result!=0) {
		save_errno = errno;
		chirp_client_disconnect(client);
		errno = save_errno;
		return 0;
	}

	return client;
}

struct chirp_client * chirp_client_connect( const char *host, int port )
{
	struct chirp_client *c;
	int save_errno;
	int fd;

	c = malloc(sizeof(*c));
	if(!c) return 0;

	fd = tcp_connect(host,port);
	if(fd<0) {
		save_errno = errno;
		free(c);
		errno = save_errno;
		return 0;
	}

	c->stream = fdopen(fd,"w+");
	if(!c->stream) {
		save_errno = errno;
		close(fd);
		free(c);
		errno = save_errno;
		return 0;
	}

	return c;
}

void chirp_client_disconnect( struct chirp_client *c )
{
	fclose(c->stream);
	free(c);
}

int chirp_client_cookie( struct chirp_client *c, const char *cookie )
{
	return simple_command(c,"cookie %s\n",cookie);
}

int chirp_client_login( struct chirp_client *c, const char *name, const char *password )
{
	return simple_command(c,"login %s %s\n",name,password);
}

int chirp_client_open( struct chirp_client *c, const char *path, const char *flags, int mode )
{
	return simple_command(c,"open %s %s %d\n",path,flags,mode);
}

int chirp_client_close( struct chirp_client *c, int fd )
{
	return simple_command(c,"close %d\n",fd);
}

int chirp_client_read( struct chirp_client *c, int fd, char *buffer, int length )
{
	int result;
	int actual;

	result = simple_command(c,"read %d %d\n",fd,length);

	if( result>0 ) {
		actual = fread(buffer,1,result,c->stream);
		if(actual!=result) chirp_fatal_response("read");
	}

	return result;
}

int chirp_client_write( struct chirp_client *c, int fd, const char *buffer, int length )
{
	int actual;
	int result;

	result = fprintf(c->stream,"write %d %d\n",fd,length);
	if(result<0) chirp_fatal_request("write");

	result = fflush(c->stream);
	if(result<0) chirp_fatal_request("write");

	actual = fwrite(buffer,1,length,c->stream);
	if(actual!=length) chirp_fatal_request("write");

	return convert_result(get_result(c->stream));
}

int chirp_client_unlink( struct chirp_client *c, const char *path )
{
	return simple_command(c,"unlink %s\n",path);
}

int chirp_client_rename( struct chirp_client *c, const char *oldpath, const char *newpath )
{
	return simple_command(c,"rename %s %s\n",oldpath,newpath);
}
int chirp_client_fsync( struct chirp_client *c, int fd )
{
	return simple_command(c,"fsync %d\n",fd);
}

int chirp_client_lseek( struct chirp_client *c, int fd, int offset, int whence )
{
	return simple_command(c,"lseek %d %d %d\n",fd,offset,whence);
}

int chirp_client_mkdir( struct chirp_client *c, char const *name, int mode )
{
	return simple_command(c,"mkdir %s %d\n",name,mode);
}

int chirp_client_rmdir( struct chirp_client *c, char const *name )
{
	return simple_command(c,"rmdir %s\n",name);
}


static int convert_result( int result )
{
	if(result>=0) {
		return result;
	} else {
		switch(result) {
			case CHIRP_ERROR_NOT_AUTHENTICATED:
			case CHIRP_ERROR_NOT_AUTHORIZED:
				errno = EACCES;
				break;
			case CHIRP_ERROR_DOESNT_EXIST:
				errno = ENOENT;
				break;
			case CHIRP_ERROR_ALREADY_EXISTS:
				errno = EEXIST;
				break;
			case CHIRP_ERROR_TOO_BIG:
				errno = EFBIG;
				break;
			case CHIRP_ERROR_NO_SPACE:
				errno = ENOSPC;
				break;
			case CHIRP_ERROR_NO_MEMORY:
				errno = ENOMEM;
				break;
			case CHIRP_ERROR_INVALID_REQUEST:
				errno = EINVAL;
				break;
			case CHIRP_ERROR_TOO_MANY_OPEN:
				errno = EMFILE;
				break;
			case CHIRP_ERROR_BUSY:
				errno = EBUSY;
				break;
			case CHIRP_ERROR_TRY_AGAIN:
				errno = EAGAIN;
				break;
			case CHIRP_ERROR_UNKNOWN:
				chirp_fatal_response();
				break;
		}
		return -1;
	}
}

static int get_result( FILE *s )
{
	char line[CHIRP_LINE_MAX];
	char *c;
	int result;
	int fields;

	c = fgets(line,CHIRP_LINE_MAX,s);
	if(!c) chirp_fatal_response();

	fields = sscanf(line,"%d",&result);
	if(fields!=1) chirp_fatal_response();

	return result;
}

static void chirp_fatal_request( const char *name )
{
	fprintf(stderr,"chirp: couldn't %s: %s\n",name,strerror(errno));
	abort();
}

static void chirp_fatal_response()
{
	fprintf(stderr,"chirp: couldn't get response from server: %s\n",strerror(errno));
	abort();
}

static int tcp_connect( const char *host, int port )
{
	struct hostent *h;
	struct sockaddr_in address;
	int success;
	int fd;

	h = gethostbyname(host);
	if(!h) return -1;

	address.sin_port = htons(port);
	address.sin_family = h->h_addrtype;
	memcpy(&address.sin_addr.s_addr,h->h_addr_list[0],sizeof(address.sin_addr.s_addr));

	fd = socket( AF_INET, SOCK_STREAM, 0 );
	if(fd<0) return -1;

	success = connect( fd, (struct sockaddr *) &address, sizeof(address) );
	if(success<0) {
		close(fd);
		return -1;
	}

	return fd;
}

/*
 *vsprintf_command -- simple sprintf capabilities with character escaping
 *
 *The following format characters are interpreted:
 *
 *%d -- decimal
 *%s -- word (whitespace is escaped)
 */

void vsprintf_command(char *command,char const *fmt,va_list args)
{
  char *c;
  char const *f;

  c = command;
  f = fmt;
  while(*f) {
	  if(*f == '%') {
		  switch(*(++f)) {
		  case 'd':
			  f++;
			  sprintf(c,"%d",va_arg(args,int));
			  c += strlen(c);
			  break;
		  case 's': {
			  char const *w = va_arg(args,char const *);
			  f++;
			  while(*w) {
				  switch(*w) {
				  case ' ':
				  case '\t':
				  case '\n':
				  case '\r':
				  case '\\':
					  *(c++) = '\\';
					  /*fall through*/
				  default:
					  *(c++) = *(w++);
				  }
			  }
			  break;
		  }
		  default:
			  chirp_fatal_request(f);
		  }
	  }
	  else {
		  *(c++) = *(f++);
	  }
  }
  *(c++) = '\0';
}

int simple_command(struct chirp_client *c,char const *fmt,...)
{
  int result;
  char command[CHIRP_LINE_MAX];
  va_list args;

  va_start(args,fmt);
  vsprintf_command(command,fmt,args);
  va_end(args);

  result = fputs(command,c->stream);



  if(result < 0) chirp_fatal_request(fmt);

  result = fflush(c->stream);
  if(result < 0) chirp_fatal_request(fmt);

  return convert_result(get_result(c->stream));
}

