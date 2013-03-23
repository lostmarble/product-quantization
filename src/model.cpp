
#include <iostream>
#include "featureflfmt.h"
#include "dataset.h"
#include "model.h"
#include "util.h"

extern "C" {
#include "../contrib/yael/kmeans.h"
#include "../contrib/yael/nn.h"
#include "../contrib/yael/vector.h"
#include "../contrib/yael/sorting.h"
#include "../contrib/yael/machinedeps.h"
}

using namespace std;
PqModel::PqModel():
           m_coarse_k(0),\
           m_coarse_centroids(NULL) {
    for(int i=0;i<NSQ;i++) {
        m_pq_centroids[i]=NULL;
    }
}

void PqModel::setCoarseK(int coarse_k) {
    m_coarse_k=coarse_k;
}

int PqModel::getCoarseK() {
    return m_coarse_k;
}
float* PqModel::getCoarseCentroid(){
    return m_coarse_centroids;
}
float* PqModel::getPqCentroid(int k) {
    return m_pq_centroids[k];
}

DataBase* PqModel::trainModel(DataSet*ds,char* centroid_file) {
    DataBase *db = new DataBase();
    db->setFtrNum(ds->getFeatureNum());
    db->setCoarseK(m_coarse_k);
    ifstream ifile(centroid_file,ios::binary);
    if(!ifile.good()){
        ifile.close();
        cout<<"->coarse index without coarse.centroids file !"<<endl;
        trainAndCoarseIdx(ds,db);
       // writeCentroids(centroid_file);
    }
    else{
        ifile.close();
        cout<<"->coarse index with coarse.centroids file !"<<endl;
        trainAndCoarseIdxWithCentroid(ds,db,centroid_file);
    }
    trainAndPq(ds,db);

    db->setFtrFileName(ds->m_vfilename);
    int* ftrFileIdx= ivec_new_0(ds->getFeatureNum());
    memcpy(ftrFileIdx,ds->m_featureFileIdx,ds->getFeatureNum());
    db->setFtrFileIdx(ftrFileIdx);
    return db;
}

void PqModel::trainAndCoarseIdx(DataSet* dataset,DataBase* db) {
    if(m_coarse_k<=0) {
        cout<<"E-->m_coarse_k should not be:"<<m_coarse_k<<"\t";
        cout<<"please set m_coarse_k first"<<endl;
        exit(0);
    }

    int d       = dataset->getFeatureDim();
    int n       = dataset->getFeatureNum();

    if (2*n<m_coarse_k) {
        cout<<"->base feature num < coarse_k/2"<<endl;
        m_coarse_k=n/2;
    }

    /* cluster number should not be bigger than samples */
    int coarse_k= m_coarse_k;
    int niter   = 100;
    float * v   = dataset->getFtrs();
    
    cout<<"->train and coarse index..."<<endl;
    cout<<"->coarse k="<<coarse_k<<endl;
    cout<<"->dataset feature dim:"<<d<<"num:"<<n<<"coarsek"<<coarse_k<<endl;
    int nt      = count_cpu();
    //nt      = 1;
    cout<<"->cpu number:"<<nt<<endl;
    int flags   = nt | KMEANS_INIT_RANDOM | KMEANS_QUIET; 
    //int flags   = nt | KMEANS_INIT_RANDOM  ; 
    
    int* coarse_assign = ivec_new(n);
    int* coarse_cluster_element_num = ivec_new_0(coarse_k); 
    if (m_coarse_centroids) free(m_coarse_centroids);
    m_coarse_centroids=fvec_new(d*coarse_k);

    int redo    = 1;
    int seed    = 0;

    float err   = kmeans(d,n,coarse_k,niter,v,\
                            flags,seed,redo,\
                            m_coarse_centroids,NULL,\
                            coarse_assign,\
                            coarse_cluster_element_num );
	//fprintf (stderr, "->kmeans err = %.3f\n", err);
	cout<< "->coarse index kmeans err = "<<err<<endl;
    free(coarse_cluster_element_num);
    //int* coarse_cluster_assign_idx = ivec_new(n); /* inverted file elements */
    //ivec_sort_index(coarse_assign,n,coarse_cluster_assign_idx );

    /* inverted file elements */
    //int* coarse_cluster_start_idx=ivec_new(m_coarse_k);
    /* compute the coarse cluster index */
    //coarse_cluster_start_idx[0]=0;
    //for (i=1;i<m_coarse_k;i++) {
    //   coarse_cluster_start_idx[i]=coarse_cluster_start_idx[i-1]+\
    //                          coarse_cluster_element_num[i-1];
    //}

    db->setCoarseAssign(coarse_assign);
    //db->setCoarseClusterEleNum(coarse_cluster_element_num);
    //db->setCoarseClusterAssignIdx(coarse_cluster_assign_idx);
    //db->setCoarseClusterStartIdx(coarse_cluster_start_idx);
	//fprintf (stderr, "->kmeans err = %.3f\n", err);

    cout<<"->coarse index end!"<<endl;
}

void PqModel::trainAndCoarseIdxWithCentroid(DataSet* ds,DataBase*db,char* modelfile) {
    loadModel(modelfile);

    int* coarse_assign = ivec_new(ds->m_num);
    int* coarse_cluster_element_num = ivec_new_0(m_coarse_k); 

    int nq = ds->m_num;
    int nb = m_coarse_k;
    int d  = FEATURE_DIM;
    int w  = 1;
    int *assign = coarse_assign;
    float *base = m_coarse_centroids;
    float *query = ds->getFtrs();
    float* dis = fvec_new(w*nq);
    int distype = 2;
    /*! number of threads */
    int nt=count_cpu();
    //nt=1;

    knn_full_thread(distype,nq,nb,d,w,base,\
                    query,NULL,assign,dis,nt);
    free(dis);

   // for (int i=0;i<ds->m_num;i++) {
   //     coarse_cluster_element_num[assign[i]]++;
   // }
   // int* coarse_cluster_assign_idx = ivec_new(ds->m_num); /* inverted file elements */
   // ivec_sort_index(coarse_assign,ds->m_num,coarse_cluster_assign_idx );

   // /* inverted file elements */
   // int* coarse_cluster_start_idx=ivec_new(m_coarse_k);
   // /* compute the coarse cluster index */
   // coarse_cluster_start_idx[0]=0;
   // for (int i=1;i<m_coarse_k;i++) {
   //     coarse_cluster_start_idx[i]=coarse_cluster_start_idx[i-1]+\
   //                           coarse_cluster_element_num[i-1];
   // }
    db->setCoarseAssign(coarse_assign);
    //db->setCoarseClusterStartIdx(coarse_cluster_start_idx);
    //db->setCoarseClusterEleNum(coarse_cluster_element_num);
    //db->setCoarseClusterAssignIdx(coarse_cluster_assign_idx);
    return;
}

/*! product quantization */
void PqModel::trainAndPq(DataSet* ds,DataBase* db){
    cout<<"->product quantization..."<<endl;
    if(m_coarse_k==0) {
        cout<<"error:m_coarse_k should not be 0"<<"\t";
        cout<<"please set m_coarse_k"<<endl;
        exit(0);
    }

    /*! compute the residual vector. */
    for (int i=0;i<ds->getFeatureNum();i++) {
        for (int j=0;j<ds->getFeatureDim();j++) {
            (ds->getFtrs())[i*FEATURE_DIM+j]-=\
                m_coarse_centroids[db->getCoarseAssign(i)*FEATURE_DIM+j];
        }
    }

    /*! parameter for kmeans */
    int redo    = 1;
    int seed    = 0;
    int nt      =count_cpu();
    //nt      =1;
    int flags   = nt | KMEANS_INIT_RANDOM | KMEANS_QUIET; 
    int niter   = 100;
    
    float* subv = fvec_new(ds->getFeatureNum()*LSQ);

    for (int k=0;k<NSQ;k++) {
        int* pq_assign      = ivec_new(ds->getFeatureNum());
        m_pq_centroids[k]   = fvec_new(LSQ*KS);

        for (int i=0;i<ds->getFeatureNum();i++) {
            for(int j=0;j<LSQ;j++) {
                subv[i*LSQ+j]=(ds->getFtrs())[i*FEATURE_DIM+k*LSQ+j];
            }
        }
        float err = kmeans(LSQ,ds->getFeatureNum(),KS,\
                            niter,subv,flags,seed,redo,\
                            m_pq_centroids[k],NULL,\
                            pq_assign,NULL);
        db->setPqAssign(k,pq_assign);
    }
    free(subv);
    subv=NULL;
    cout<<"->product quantization end "<<endl;
}

DataBase* PqModel::getDataBase(DataSet* ds ){
    DataBase* db = new DataBase();
    db->setFtrNum(ds->getFeatureNum());
    db->setCoarseK(m_coarse_k);
    coarseIdx(ds,db);
    pq(ds,db);

    db->setFtrFileName(ds->m_vfilename);
    int* ftrFileIdx= ivec_new_0(ds->getFeatureNum());
    memcpy(ftrFileIdx,ds->m_featureFileIdx,ds->getFeatureNum());
    db->setFtrFileIdx(ftrFileIdx);
    return db;
}
void PqModel::coarseIdx(DataSet*ds ,DataBase*db){

    int feature_num=ds->getFeatureNum();
    int *coarse_assign = ivec_new(feature_num);
    int *coarse_cluster_element_num = ivec_new_0(m_coarse_k); 

    int nq = feature_num;
    int nb = m_coarse_k;
    int d  = FEATURE_DIM;
    int w  = 1;
    int *assign = coarse_assign;
    float *base = m_coarse_centroids;
    float *query = ds->getFtrs();
    float* dis = fvec_new(w*nq);
    int distype = 2;
    /* number of threads */
    int nt=count_cpu();

    knn_full_thread(distype,nq,nb,d,w,base,\
                    query,NULL,assign,dis,nt);
    free(dis);

    //for (int i=0;i<ds->m_num;i++) {
    //    coarse_cluster_element_num[assign[i]]++;
    //}
    //int *coarse_cluster_assign_idx = ivec_new(ds->m_num); /* inverted file elements */
    //ivec_sort_index(coarse_assign,ds->m_num,coarse_cluster_assign_idx );

    ///* inverted file elements */
    //int* coarse_cluster_start_idx=ivec_new(m_coarse_k);
    ///* compute the coarse cluster index */
    //coarse_cluster_start_idx[0]=0;
    //for (int i=1;i<m_coarse_k;i++) {
    //   coarse_cluster_start_idx[i]=coarse_cluster_start_idx[i-1]+\
    //                          coarse_cluster_element_num[i-1];
    //}

    db->setCoarseAssign(assign);
    //db->setCoarseClusterStartIdx(coarse_cluster_start_idx);
    //db->setCoarseClusterEleNum(coarse_cluster_element_num);
    //db->setCoarseClusterAssignIdx(coarse_cluster_assign_idx);
    return ;
}

void PqModel::pq(DataSet* ds, DataBase* db) {
    cout<<"->product quantization..."<<endl;

    /*! compute the residual vector. */
    for (int i=0;i<ds->getFeatureNum();i++) {
        for (int j=0;j<ds->getFeatureDim();j++) {
            int coarse_assign_idx=db->getCoarseAssign(i);
            (ds->getFtrs())[i*FEATURE_DIM+j]-=\
              m_coarse_centroids[coarse_assign_idx*FEATURE_DIM+j];
        }
    }

    /*! parameter for kmeans */
    float* subv = fvec_new(ds->getFeatureNum()*LSQ);

    for (int k=0;k<NSQ;k++) {
        for (int i=0;i<ds->getFeatureNum();i++) {
            for(int j=0;j<LSQ;j++) {
                subv[i*LSQ+j]=(ds->getFtrs())[i*FEATURE_DIM+k*LSQ+j];
            }
        }
        int nq = ds->getFeatureNum();
        int nb = KS;
        int d  = LSQ;
        int w  = 1;
        int *assign = ivec_new(nq);
        float *base = m_pq_centroids[k];
        float *query = subv;
        float* dis = fvec_new(w*nq);
        int distype = 2;
        /* number of threads */
        int nt=count_cpu();

        knn_full_thread(distype,nq,nb,d,w,base,\
                query,NULL,assign,dis,nt);
        free(dis);

        db->setPqAssign(k,assign);
    }
    free(subv);
    subv=NULL;
    cout<<"->product quantization end "<<endl;
}


bool PqModel::saveModel(char*centroid_file){
    ofstream ofile(centroid_file);
    if(!ofile.good()){
        cout<<"cann't open centroid_file:"<<centroid_file<<endl;
        return false;
    }
    ofile.write((char*)&m_coarse_k,sizeof(int));
    ofile.write((char*)m_coarse_centroids,m_coarse_k*FEATURE_LEN);
    for(int i=0;i<NSQ;i++) {
        ofile.write((char*)m_pq_centroids[i],sizeof(float)*LSQ*KS);
    }
    ofile.close();
    return true;
}

bool PqModel::loadModel(char*centroid_file){

    ifstream ifile(centroid_file,ios::binary);
    if(!ifile.good()){
        cout<<"No coarse.centroids file !"<<endl;
        ifile.close();
        return false;
    }
    ifile.read((char*)&m_coarse_k,sizeof(int));
    FREE(m_coarse_centroids);
    m_coarse_centroids=fvec_new(FEATURE_DIM*m_coarse_k);
    //for(int i=0;i<m_coarse_k;i++) {
    //    ifile.read((char*)(m_coarse_centroids+i*FEATURE_DIM),\
    //            FEATURE_LEN);
    //}
    ifile.read((char*)(m_coarse_centroids),m_coarse_k*FEATURE_LEN);
    for(int i=0;i<NSQ;i++) {
        FREE(m_pq_centroids[i]);
        m_pq_centroids[i] = fvec_new(LSQ*KS*sizeof(float));
        ifile.read((char*)m_pq_centroids[i],sizeof(float)*LSQ*KS);
    }
    ifile.close();
    return true;
}

void PqModel::predict(float* q_ar,int q_num,int neig_num,\
                            int *assign,int distype){
    /*since we use predict in multithread, 
     * multithread here is just a burden;
     */
    int thread_num = 1;
    float *dis = fvec_new(neig_num*q_num);
    knn_full_thread(distype,q_num,m_coarse_k,FEATURE_DIM,neig_num,\
            m_coarse_centroids,q_ar,NULL,assign,dis,thread_num);
    free(dis);
}

PqModel::~PqModel() {

    if(m_coarse_centroids) {
        cout<<"free coarse centroids"<<endl;
        free( m_coarse_centroids);
        m_coarse_centroids=NULL;
    }
    int i=0;
    for (i=0;i<NSQ;i++) {
        free( m_pq_centroids[i]);
        m_pq_centroids[i]=NULL;
    }
}

