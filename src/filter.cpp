/**
 * filter the unstable point from original ftr file
 * 2012年 12月 23日 星期日 14:51:36 CST
 */
#include <stdio.h>
#include <iostream>
#include <set>
#include <assert.h>
#include "featureflfmt.h"
#include <string>  /*for index and substr*/

using namespace std;

#define VALID_FILE(x) if(!x) {\
                            cout<<"can't open file:"<<x<<endl;\
                            return -1;\
                       }

int filter_ftr(char* pic_ftr_path,set<int>&stable_idx);
void usage() {
    cout<<"usage:filter stable_rslt.txt output_dir"<<endl;
    return;
}

char* output_dir;
int main(int argc ,char* argv[]) {
    if(argc!=3){
        usage();
        return 0;
    }
    output_dir=argv[2];

    FILE* stable_rslt_file; /*to read stable result*/

    stable_rslt_file=fopen(argv[1],"r");
    VALID_FILE(stable_rslt_file);

    set<int> stable_idx;
    while(!feof(stable_rslt_file)) {
        stable_idx.clear();
        char pic_ftr_path[512];
        int feature_num=0;
        fscanf(stable_rslt_file,"filename:%s num:%d\n",pic_ftr_path,&feature_num);
        cout<<"-->"<<pic_ftr_path<<endl;
        assert(feature_num>0);

        /*get stable points */
        int ftr_idx=0;
        int ftr_status=0;
        int tmp_score;
        int M;
        int s;
        int d;
        int c;
        for(int i=0;i<feature_num;i++) {
            MetaFeature tmp_ftr;
            fscanf(stable_rslt_file,"index:\t%d\tscores:\t%d\tstatus:\t%d\tM:\t%d\ts:\t%d\td:\t%d\tc:\t%d\t\n",\
                    &ftr_idx,&tmp_score,&ftr_status,&M,&s,&d,&c);
            /* write stable points */
            if(!ftr_status){
                stable_idx.insert(ftr_idx);
            }

        }

        /* write stable points */
        filter_ftr(pic_ftr_path,stable_idx);
    }
    return 0;
}

int filter_ftr(char* pic_ftr_path,set<int>&stable_idx) {
    FILE* ftr_file;         /*to read features*/
    FILE* output_file;      /*to output the filtered results*/

    /* open ftr file */
    ftr_file = fopen(pic_ftr_path,"r");
    VALID_FILE(ftr_file);

    /* open output file */
    string spfp=string(pic_ftr_path);
    string pic_name=spfp.substr(spfp.find_last_of("/")+1,spfp.find_last_of("_")-spfp.find_last_of("/")-1);
    char output_file_path[512];
    sprintf(output_file_path,"%s/%s_filterftr",output_dir,pic_name.c_str());
    cout<<"==>"<<output_file_path<<endl;
    output_file=fopen(output_file_path,"w");
    VALID_FILE(output_file);


    FtrFileHeader header;
    fread((char*)&header,sizeof(header),1,ftr_file);
    int feature_num = header.m_ftr_num;
    header.m_ftr_num=stable_idx.size();
    fwrite((char*)&header,sizeof(header),1,output_file);
    for (int i=0;i<feature_num;i++) {
        MetaFeature tmp_ftr;
        fread((char*)&tmp_ftr,sizeof(tmp_ftr),1,ftr_file);
        /* write stable points */
        if(stable_idx.count(i)){
            fwrite((char*)&tmp_ftr,sizeof(tmp_ftr),1,output_file);
        }
    }

    fclose(ftr_file);
    fclose(output_file);
    return 0;
}

