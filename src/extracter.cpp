#include <dirent.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include "featureflfmt.h"
#include "../contrib/surf/surflib.h"
#include "extracter.h"

using namespace std;
void Extracter::extractDirFeatures(char* dir) {
    DIR             *pDir;
    struct dirent   *ent;

    m_feature_num=0;
    pDir = opendir(dir);
    while (NULL !=(ent=readdir(pDir))) {
        if (ent->d_type&DT_DIR) {
            continue;
        }
        else {
            _extFtr(string(dir)+"/"+string(ent->d_name));
        }
    }
    closedir(pDir);

    char        summery[512];
    sprintf(summery, "%s/summery.txt",m_ftrDir);
    ofstream ofile(summery,ios::binary);
    ofile<<m_feature_num<<"\t"<<FEATURE_DIM<<endl;
    ofile.close();
}
void Extracter::extractFileFeatures(char *filename) {
    ifstream ifile(filename);
    if(!ifile.good()) {
        cout<<"can't read the file "<<filename<<endl;
        ifile.close();
        return;
    }
    string pic;
    while(!ifile.eof()) {
        ifile>>pic;
        _extFtr(pic);
    }
    ifile.close();

    char        summery[512];
    sprintf(summery, "%s/summery.txt",m_ftrDir);
    ofstream ofile(summery,ios::binary);
    ofile<<m_feature_num<<"\t"<<FEATURE_DIM<<endl;
    ofile.close();
}


void Extracter::_extFtr(string picPath) {

    //char        picPath[512];
    //sprintf(picPath, "%s/%s",m_picDir,picName);
    string spicPath(picPath);
    int pos=spicPath.find_last_of("/");
    IpVec       ipts;
    IplImage    *img=cvLoadImage(spicPath.c_str());

    if(!img) {
        cerr<<"broken picture:"<<picPath<<endl;
        return;
    }

    // Detect and describe interest points in the image
    clock_t start = clock();
    surfDetDes(img,ipts,false,5,4,3,0.0004f);
    clock_t end = clock();
    cvReleaseImage(&img);
    //cout<<picPath<<"found:"<<ipts.size() <<" interest points"\
        <<"\t took:"<<float(end-start) /CLOCKS_PER_SEC<<" seconds"\
        <<endl;
    cout<<"."<<" ";
    if(ipts.size()>0){

        char ftrPath[512];
        sprintf(ftrPath, "%s/%s_ftr",m_ftrDir,spicPath.substr(pos+1).c_str());
        ofstream ofile(ftrPath,ios::binary);

        FtrFileHeader file_header;
        file_header.m_version = FEATURE_FILE_VER;
        file_header.m_ftr_num = ipts.size();
        file_header.m_ftr_len = FEATURE_LEN;
        file_header.m_ftr_dim = FEATURE_DIM;

        m_feature_num+=ipts.size();
        ofile.write((char*)&file_header,sizeof(file_header));

        MetaFeature tmp_ftr;
        assert(FEATURE_DIM<=64);
        IpVec::iterator iter;
        for (iter=ipts.begin();iter!=ipts.end();++iter) {
            tmp_ftr.m_scale       =iter->scale;
            tmp_ftr.m_orientation =iter->orientation;
            tmp_ftr.x             =iter->x;
            tmp_ftr.y             =iter->y;
            memcpy(tmp_ftr.m_descriptor,iter->descriptor,FEATURE_LEN);

            ofile.write((char*)&tmp_ftr,sizeof(tmp_ftr));

        }
        ofile.close();
    }

}
    
void Extracter::testExtracter() {
    DIR             *pDir;
    struct dirent   *ent;
    char            filepath[512];

    char *m_picDir="";
    pDir = opendir(m_picDir);
    while (NULL !=(ent=readdir(pDir))) {
        if (ent->d_type&DT_DIR) {
            continue;
        }
        else {
            _testExtFtr(ent->d_name); 
        }
    }
    closedir(pDir);
}

// for test 
void Extracter::_testExtFtr(char *picName) {
    IpVec       ipts;
    char        picPath[512];
    char* m_picDir="";
    sprintf(picPath, "%s/%s",m_picDir,picName);
    IplImage    *img=cvLoadImage(picPath);

    if(!img) {
        cout<<"broken picture:"<<picPath<<endl;
        return;
    }

    // Detect and describe interest points in the image
    clock_t start = clock();
    surfDetDes(img,ipts,false,5,4,3,0.0004f);
    clock_t end = clock();

    char ftrPath[512];
    sprintf(ftrPath, "%s/%s_ftr",m_ftrDir,picName);
    ifstream ifile(ftrPath,ios::binary);

    FtrFileHeader file_header;
    ifile.read((char*)&file_header,sizeof(file_header));

    assert(file_header.m_version == FEATURE_FILE_VER);
    assert(file_header.m_ftr_num == ipts.size());
    assert(file_header.m_ftr_len == FEATURE_LEN);
    assert(file_header.m_ftr_dim == FEATURE_DIM);

    MetaFeature tmp_ftr;
    assert(FEATURE_DIM<=64);
    IpVec::iterator iter;
    for (iter=ipts.begin();iter!=ipts.end();++iter) {
        ifile.read((char*)&tmp_ftr,sizeof(tmp_ftr));
        assert(tmp_ftr.m_scale       -iter->scale<0.1);
        assert(tmp_ftr.m_orientation -iter->orientation<0.1);
        for (int i=0;i<FEATURE_DIM;i++) {
            assert(tmp_ftr.m_descriptor[i]-iter->descriptor[i]<=0.1);
        }
    }
    ifile.close();
}
    
