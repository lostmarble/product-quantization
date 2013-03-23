#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>




#include "config.h"


#define MAX_VAR_NUM     (16)
#define MAX_VAR_NAME_LEN     (1024)
#define MAX_VAR_VALUE_LEN   (MAX_PATH_LEN)

#define COMMENT_CHARACTER '#'
#define LINE_SIZE   (1024)

char ga_variables[MAX_VAR_NUM][MAX_VAR_VALUE_LEN+1];
char ga_values[MAX_VAR_NUM][MAX_VAR_VALUE_LEN+1];
int g_var_num=0;

#define ErrSyslog printf
#define ErrPrintf printf


void remove_trailing_chars(char *path, char c){
    size_t len;
    len = strlen(path);
    while (len>0 && path[len-1] == c)
        path[--len]='\0';
}

int get_key(char **line, char **key, char **value) {
    char *linepos;
    char *temp;
    linepos = *line; 
    if (!linepos) {
        return (-1);
    }

    /*skip whitespace*/
    while (isspace(linepos[0]))
        linepos++;

    if (linepos[0] == '\0') {
        return -1;
    }
    
    /*get the key*/
    *key = linepos;
    while (1) {
        linepos++;
        if (linepos[0] == '\0') {
            return -1;
        }
        if (isspace(linepos[0]))
            break;
        if (linepos[0] == '=')
            break;
    }

    /*terminate key*/
    linepos[0]='\0';
    while (1) {
        linepos++;
        if (linepos[0] == '\0') {
            return -1;
        }
        if (isspace(linepos[0])){
            continue;
        }
        if (linepos[0]=='='){
            continue;
        }
        break;
    }

    /*get the value*/
    if (linepos[0]=='"') {
        linepos++;
    }
    else {
        return -1;
    }
    *value = linepos;
    temp = strchr(linepos, '"');
    if (!temp) {
        return -1;
    }
    temp[0]='\0';
    return 0;
}

int parse_config_file(char *path_to_config_file) {
    char line[LINE_SIZE+2];
    char *buffline;
    char *variable;
    char *value;
    char *buf;
    char *linepos;
    size_t bufsize;
    size_t cur;
    size_t count;
    int lineno;
    int retval=0;

    FILE *cfg_file = fopen(path_to_config_file,"r");
    if (NULL==cfg_file){
        ErrSyslog("can't open '%s' as config file:%s",path_to_config_file, strerror(errno));
        goto EXIT;
    }

    /*loop through the whole file*/
    lineno=0;
    cur=0;
    while (NULL != fgets(line, sizeof(line), cfg_file)) {
        lineno++;
        buffline = line;
        count = strlen(line);
        
        if (count>LINE_SIZE) {
            ErrSyslog("line too long , conf line skipped %s, line %d", path_to_config_file,lineno);
            continue;
        }

        /* eat the whilespace */
        while ((count > 0) && isspace(buffline[0])) {
            buffline++;
            count--;
        }
        if (count==0)
            continue;

        /* see if this is a comment */
        if (buffline[0] == COMMENT_CHARACTER)
            continue;
        memcpy(line, buffline, count);
        line[count]='\0';
        linepos = line;
        retval = get_key(&linepos, &variable, &value);
        if (retval != 0) {
            ErrSyslog("error parsing %s, line %d:%d", path_to_config_file, lineno,(int)(linepos-line));
            continue;
        }

        if (g_var_num >= MAX_VAR_NUM) {
            ErrSyslog("too many vars in %s, line %d:%d", path_to_config_file, lineno, (int)(linepos-line));
            continue;
        }

        if (strlen(variable) > MAX_VAR_NAME_LEN) {
            ErrSyslog("var name to long %s, line %d:%d", path_to_config_file, lineno, (int)(linepos-line));
            continue;
        }

        if (strlen(value) > MAX_VAR_VALUE_LEN) {
            ErrSyslog("value too long %s, line %d:%d", path_to_config_file, lineno, (int)(linepos-line));
            continue;
        }

        strncpy(ga_variables[g_var_num], variable, sizeof(ga_variables[g_var_num]));
        remove_trailing_chars(value,'/');
        strncpy(ga_values[g_var_num], value, sizeof(ga_values[g_var_num]));
        g_var_num++;
        continue;
    }

EXIT:fclose(cfg_file);
     return g_var_num;
}

char *get_config_var(char *var_name) {
    int i;
    for (i=0;i<g_var_num;i++) {
        if (strcasecmp(ga_variables[i],var_name) == 0) {
            return ga_values[i];
        }
    }
    ErrSyslog("get %s failed", var_name);
    return NULL;
}

void print_all_vars() {
    int i;
    ErrPrintf("g_var_num==%d", g_var_num);
    for(i=0; i<g_var_num; i++) {
        printf("%s=%s\n", ga_variables[i], ga_values[i]);
    }
}


        
