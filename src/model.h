
#ifndef _MODEL_H
#define _MODEL_H
#include <vector>
#include <string>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>
#include "featureflfmt.h"
#include "dataset.h"
#include "database.h"
#include "types.h"
using namespace std;


class PqModel {
    public:
        /* coarse index */
        int         m_coarse_k;      /*! coarse clusters */
        float*      m_coarse_centroids;   /*! (m_corse_k,\
                                                FEATURE_DIM) */

        /*! pq centroids of base vectors.(NSQ,LSQ,KS) */
        float*      m_pq_centroids[NSQ];

    public:
        PqModel();  
        void setCoarseK(int coarse_k);
        inline int  getCoarseK();
        float* getCoarseCentroid();
        float* getPqCentroid(int k);
        

        DataBase* trainModel(DataSet*ds,char* centroid_file=NULL);
        /* tansfer a dataset to database
         * coarse_centroids & pq_centroids must be present
         */
        DataBase* getDataBase(DataSet* ds);

        /*parameters:
         * in - query_array,query_num,neig_num,distype
         * out- assign
         */
        void predict(float* query_array,int query_num,\
                int neig_num,int* assign,int distype=2);
        bool saveModel(char*centorid_file);
        bool loadModel(char*centorid_file);
        
        ~PqModel();
    private:
        /*! coarse index */
        void trainAndCoarseIdx(DataSet *ds,DataBase* db);
        void trainAndCoarseIdxWithCentroid(DataSet*,DataBase*db,\
                char* modelfile);
        /* product quantization */
        void trainAndPq(DataSet *ds,DataBase* db);

        void coarseIdx(DataSet* ds , DataBase* db);
        void pq(DataSet* ds , DataBase* db);

};

#endif /* _MODEL_H */
