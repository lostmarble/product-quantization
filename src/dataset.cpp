#include "dataset.h"
#include <unistd.h>
#include "featureflfmt.h"

extern "C" {
#include "../contrib/yael/kmeans.h"
#include "../contrib/yael/nn.h"
#include "../contrib/yael/vector.h"
#include "../contrib/yael/sorting.h"
#include "../contrib/yael/machinedeps.h"
}

void DataSet::explore_dir(const char* dir,File_func pfunc) {
    DIR             *pDir;
    struct dirent   *ent;

    pDir=opendir(dir);

    char filepath[512];

    while(NULL!=(ent=readdir(pDir))) {
        if (ent->d_type&DT_DIR) {
            continue;
        }
        else {
            sprintf(filepath,"%s/%s",dir,ent->d_name);
            (this->*pfunc)(filepath);
        }
    }
}
bool DataSet::initDataSet(const char *feature_dir) {
    cout<<"->init dataset "<<feature_dir<<endl;
    char summery_file[512];
    sprintf(summery_file,"%s/summery.txt",feature_dir);
    ifstream ifile(summery_file);
    if (!ifile.good()){
        cout<<"->dataset summery file do not exits!"<<endl;
        ifile.close();
        File_func pfunc=&DataSet::getHeader;

        explore_dir(feature_dir,pfunc);
        ofstream ofile(summery_file);
        ofile<<m_num<<"\t"<<m_dim<<endl;
        ofile.close();
        ifile.open(summery_file);
    }
    ifile>>m_num>>m_dim;
    cout<<"->feature number:"<<m_num<<"\tfeature dim:"<<m_dim<<endl;
    ifile.close();

    //m_features=(float*)malloc(m_dim*sizeof(float)*m_num);
    m_features=fvec_new(m_dim*m_num);
    //m_featureFileIdx=(int*)malloc(m_num*sizeof(int));
    m_featureFileIdx=ivec_new(m_num);
    if (!m_features||!m_featureFileIdx) {
        std::cout<<"can't malloc memory for DataSet m_features or m_featureFileIdx!"<<endl;
        return false;
    }
    return true;
}

void DataSet::getHeader(const char* filepath) {
    if (strstr(filepath,"summery.txt")) return;
    ifstream ifile(filepath,ios::binary);
    if(ifile.good()){
        // read feature file header
        FtrFileHeader header;
        ifile.read((char*)&header,sizeof(FtrFileHeader));
        m_num+=header.m_ftr_num;
        if(!m_dim) {
            m_dim = header.m_ftr_dim;
        }
    }
    ifile.close();
}

void DataSet::readData(const char* feature_dir) {
    cout<<"->read data set:"<<feature_dir<<endl;
    const char *pdir=feature_dir;
    explore_dir(pdir,&DataSet::readFeatureFile);
}

void DataSet::readFeatureFile(const char *filepath) {
    /* skip the summery file */
    if (strstr(filepath,"summery.txt")){
        cout<<"skip the summery.txt"<<endl;
        return;
    }

    /* start to read features in file */
    static size_t read_ftr_num=0;

    ifstream ifile(filepath,ios::binary);
    FtrFileHeader header;
    size_t ftr_num=0;
    if (ifile.good()) {

        m_vfilename.push_back(string(filepath));
        int ftr_file_idx=m_vfilename.size()-1;

        // read feature file header
        ifile.read((char*)&header,sizeof(FtrFileHeader));
        ftr_num=header.m_ftr_num;
        for (int i=0;i<ftr_num;i++) {
            MetaFeature tmp_ftr;
            ifile.read((char*)&(tmp_ftr), sizeof(MetaFeature));
            memcpy(m_features+(read_ftr_num+i)*FEATURE_DIM ,\
                   tmp_ftr.m_descriptor,\
                   FEATURE_LEN);
            m_featureFileIdx[read_ftr_num+i]=ftr_file_idx;
        }
        read_ftr_num+=ftr_num;

    }
}
