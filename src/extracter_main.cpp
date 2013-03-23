#include "../src/extracter.h"
#include "../src/extracter.cpp"
#include <sys/stat.h>
void usage() {
    cout<<"usage:Extracter img_dir img_feature_dir"<<endl;
}

int main(int argc,char *argv[]) {
    if (argc!=3) {
        usage();
        return 1;
    }
    
    Extracter ex(argv[2]);
    struct stat info;
    stat(argv[1],&info);
    if(S_ISDIR(info.st_mode)){
        //printf("This is a directory");
        ex.extractDirFeatures(argv[1]);
    }
    else {
        ex.extractFileFeatures(argv[1]);
    }
    return 0;
}

