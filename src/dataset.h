#ifndef _DATASET_H
#define _DATASET_H
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

class DataSet;
typedef void (DataSet::*File_func)(const char* filename);


class DataSet{
    public:
    int         m_num; /* number of the feature */
    int         m_dim; /* dimension of the feature */

    /*! feature -> filename */
    vector<string> m_vfilename;/* file names */
    int*        m_featureFileIdx;
    float*      m_features;

    public:
    DataSet():m_num(0),m_dim(0),\
              m_featureFileIdx(NULL),m_features(NULL){
    }
    void readData(const char *feature_dir);
    bool initDataSet(const char* feature_dir);
    float* getFtrs(){
        return m_features;
    }
    
    bool freeFtrs() {
        if(m_features){
            free(m_features);
            m_features=NULL;
        }
    }

    string getFtrFileName(int fileIdx) {
        return m_vfilename[fileIdx];
    }
    int getFtrFileIdx(int ftrIdx){
        return m_featureFileIdx[ftrIdx];
    }

    int getFeatureDim() {
        return m_dim;
    }
    int getFeatureNum() {
        return m_num;
    }
    ~DataSet(){
        if(m_features){
            free(m_features);
            m_features=NULL;
        }
        if(m_featureFileIdx) {
            free(m_featureFileIdx);
            m_featureFileIdx=NULL;
        }
    }

    private:
    void getHeader(const char* filename);
    void readFeatureFile(const char *filepath);
    void explore_dir(const char*,File_func pfunc);
};

#endif /*_DATASET_H*/
