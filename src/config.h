/**
 * @file config_file.h
 * @Synopsis   read configure file
 * @author xuzuoxin@ict.ac.cn
 * @version 0.1
 * @date 2012-06-02
 */

/**
 * format likes:
#comment
varname1 = "xx"     #comment

varname2 = "xxx"    #comment
*/

#include <stdint.h>
#include <stdio.h>
//#include <syslog.h>

#define MAX_PATH_LEN    (512)
#define MAX_FILE_NAME_LEN   (128)

int parse_config_file( char *path_to_config_file);
char * get_config_var(char *var_name);

