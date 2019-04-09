#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <stddef.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "cmdline.h"
#include "request.h"
#include "jbuffer.h"
#include "dirlisting.h"
#include "i4httools.h"

#define MAX_REQUEST 8192

static const char *path;

static int cmpstring(const void *p1, const void *p2){
	return strcmp(* (char *const *) p1, *(char *const *) p2);
}

/* send the given file in fullpath
 * returns -1 on error, 0 on succes */
static int send(const char *fullpath, FILE *tx){



	FILE *open = fopen(fullpath, "r");
	/* check fopen errors */
	if(open == NULL){
		switch(errno){
			case EACCES:
				//fprintf(stderr, "forbidden file\n");
				httpForbidden(tx, fullpath);
				return -1;
			case ENOENT:
				httpNotFound(tx, fullpath);
				return -1;
			default:
				httpInternalServerError(tx,fullpath);
				perror("fopen request");
				return -1;
		}

	}

	httpOK(tx);

	/* send to client */
	int c = 0;

  while (c != EOF) {
      c = fgetc(open);
      if (c == EOF) {
          if (ferror (open) != 0) {
              perror ("getc");
              fclose (open);
              httpInternalServerError(tx,fullpath);
              return -1;
          }
      } else {
          fputc (c, tx);
      }

  }
  fclose (open);
  return 0;
}

int initRequestHandler(void){
	path = cmdlineGetValueForKey("wwwpath");
	if((path == NULL) || *path == '\0'){
		return -1;
	}


	return 0;
}

void handleRequest(FILE *rx, FILE *tx){
/* ------------------check request------------------------------*/
	char request[MAX_REQUEST + 1];

	if (fgets(request, MAX_REQUEST + 1, rx) == NULL){
		if (ferror(rx)){
			fprintf(stderr, "fgets failed!");
			httpInternalServerError(tx,NULL);
			return;
		}
	}
	//for saving
	char *saveptr;
	/* check GET */
	char *valid_GET = strtok_r(request, " ", &saveptr);

	if(strcmp(valid_GET, "GET") != 0){
		fprintf(stderr, "get\n");
		httpBadRequest(tx);
		return;
	}

	/* check path */
	char *valid_PATH = strtok_r(NULL, " ", &saveptr);

	if(valid_PATH == NULL){
		fprintf(stderr, "path\n");
		httpBadRequest(tx);
		return;
	}
	//printf("%s\n",valid_PATH);
	/* check version */
	char *valid_HTTP = strtok_r(NULL, " \r\n", &saveptr);

	if(((strcmp(valid_HTTP, "HTTP/1.0")) != 0 )&&
		((strcmp(valid_HTTP, "HTTP/1.1")) != 0)){
		fprintf(stderr, "http");
		httpBadRequest(tx);
		return;
	}

	/* check acces */
	if(checkPath(valid_PATH) == -1){
		fprintf(stderr, "forbidden path\n");
		httpForbidden(tx, valid_PATH);
		return;
	}

	/* create fullpath */
	char fullpath[strlen(valid_PATH) + strlen(path) +1];
	strcpy(fullpath, path);
	strcat(fullpath, valid_PATH);

	//printf("%s\n",fullpath);

	/*check lstat errors*/
	struct stat buf;
	if(lstat(fullpath, &buf) == -1){
		switch(errno){
			case EACCES:
				//fprintf(stderr, "forbidden link\n");
				httpForbidden(tx, fullpath);
				return;
			case EFAULT:
			case ENOTDIR:
			case ELOOP:
			case ENAMETOOLONG:
			case ENOENT:
				httpNotFound(tx, fullpath);
				return;
			default:
				httpInternalServerError(tx,fullpath);
				perror("lstat");
				return;
		}
	}

/*-----------------list files if directory--------------------------*/

	if(S_ISDIR(buf.st_mode)){
		size_t len = strlen(valid_PATH);
		if(valid_PATH[len - 1] != '/'){
			char newpath[len + 2];
			strncpy(newpath, valid_PATH, len);
			newpath[len] = '/';
			newpath[len + 1] = '\0';
			//printf("%s\n", newpath);
			httpMovedPermanently(tx, newpath);
			return;
		}

		DIR *dir = opendir(fullpath);
		if (dir == NULL){
			perror("opendir");
			httpInternalServerError(tx, fullpath);
			return;
		}


		struct dirent *result;

		//initialize entry buffer (see manpage of readdir)
		long name_max = pathconf(path, _PC_NAME_MAX);
		if(name_max == -1){
			name_max = 255;
		}
		size_t size = offsetof(struct dirent, d_name) + name_max + 1;
		struct dirent *curdir = (struct dirent*) malloc(size);
		if (curdir == NULL){
			perror("malloc");
			httpInternalServerError(tx, fullpath);
			exit(EXIT_FAILURE);
		}

		//initialize namelist
		size_t nmemb = 0;
		size_t listlength = 1000;
		char **namelist = (char **) malloc(sizeof(char *)*listlength);
		if (namelist == NULL){
			perror("malloc");
			free(curdir);
			httpInternalServerError(tx, fullpath);
			exit(EXIT_FAILURE);
		}

		int isindex = 1;

		while(1){
			errno = readdir_r(dir, curdir, &result);

			//Error
			if(errno != 0){
				perror("readdir");
				free(curdir);
				free(namelist);
				httpInternalServerError(tx, fullpath);
				return;
			}

			//End of directory
			if(result == NULL){
				break;
			}

			//directory contains index.html
			if(strcmp(curdir->d_name, "index.html") == 0 && isindex == 1){
				char tmppath[strlen(fullpath)+strlen("index.html")+1];
				strcpy(tmppath, fullpath);
				strcat(tmppath, "index.html");

				//check if regular file
				struct stat in;
				if(lstat(tmppath, &in) == -1){
					switch(errno){
						case EACCES:
							//fprintf(stderr, "forbidden link\n");
							httpForbidden(tx, tmppath);
							return;
						case EFAULT:
						case ENOTDIR:
						case ELOOP:
						case ENAMETOOLONG:
						case ENOENT:
							httpNotFound(tx, tmppath);
							return;
						default:
							httpInternalServerError(tx,tmppath);
							perror("lstat");
							return;
					}
				}

				//if not, do not send it
				if(S_ISREG(in.st_mode) == 0){
					httpForbidden(tx, tmppath);
					fprintf(stderr,
					"index.html is not a regular file!\n");
					isindex = 0;
					continue;
				}

				send(tmppath, tx); 

				if(closedir(dir) == -1){
					perror("closedir");
				}
				free(namelist);
				free(curdir);
				return;
			}

			if(strncmp(curdir->d_name, ".",1) == 0){ //ignore "."
				continue;
			}


			//lenght of list is too small
			if(nmemb > listlength){
				char **tmplist;
				tmplist = (char **) realloc(namelist, sizeof(char *)
				 * (listlength + 100));
				if(tmplist == NULL){
					perror("realloc");
					free(namelist);
					free(curdir);
					httpInternalServerError(tx, fullpath);
					exit(EXIT_FAILURE);
				}
				namelist = tmplist;
			}

			//insert name of entry in namelist

			size_t namelen = strlen(curdir->d_name) + 1;
			char *name = (char *) malloc(sizeof(char) * namelen);
			strcpy(name, curdir->d_name);

			namelist[nmemb] = name;
			nmemb++;



		}
		//sort entries
		qsort(namelist, nmemb, sizeof(char *), cmpstring);

		httpOK(tx);

		//list sorted directories
		dirlistingBegin(tx, valid_PATH);

		for(int i = 0; i < nmemb; i++){
			dirlistingPrintEntry(tx, valid_PATH, namelist[i]);
		}
		dirlistingEnd(tx);

		free(namelist);
		free(curdir);
		return;
	}
/*-------------------check file and execute if perl script----------*/
	//regular file?
	if(S_ISREG(buf.st_mode) == 0){
		httpForbidden(tx, fullpath);
		return;
	}

	//check if the path ends with ".pl"
	int isperl = 0;
	char *dot = strrchr(fullpath, '.');
	if(dot != NULL && strlen(dot) == 3 && strcmp(dot, ".pl") == 0){
			isperl = 1;
	}

	//it's a perl script and executable!!
	if (isperl && buf.st_mode & S_IXUSR){
		 pid_t pid = fork();

		 if(pid == -1){
			 perror("fork");
			 httpInternalServerError(tx, fullpath);
			 return;

		 }else if(pid == 0){ //child process

			 if (fflush(stdout) == EOF){
				perror("fflush");
				exit(EXIT_FAILURE);
			}

			if(dup2(fileno(tx), STDOUT_FILENO) == -1){
				perror("dup2");
				exit(EXIT_FAILURE);
			}

			fclose(tx);
			fclose(rx);

			execl(fullpath,valid_PATH, (char *) NULL);
			perror("exec");
			exit(EXIT_FAILURE);

		 }
		 //father process
		return;
	}

	send(fullpath, tx);	//go away
	return;

}
