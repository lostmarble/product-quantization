#include "../src/extracter.h"
#include "../src/extracter.cpp"
void usage() {
    cout<<"usage:testExtracter img_dir img_feature_dir"<<endl;
}

int main(int argc,char *argv[]) {
    if (argc!=3) {
        usage();
        return 1;
    }
    Extracter ex(argv[1],argv[2]);
    ex.extractFeatures();
    ex.testExtracter();
    return 0;
}

