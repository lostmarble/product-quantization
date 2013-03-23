#include "pqsearchengine.h"
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <map>
#include <algorithm>
#include <assert.h>
#include <pthread.h>
#include "util.h"
#include "types.h"
extern "C" {
#include "../contrib/yael/kmeans.h"
#include "../contrib/yael/nn.h"
#include "../contrib/yael/vector.h"
#include "../contrib/yael/sorting.h"
#include "../contrib/yael/machinedeps.h"
}

//switches for different feature
//#define _DEBUG

using namespace std;

PqSearchEngine::PqSearchEngine(){
}

void search_withfile(PqSearchEngine* search_engine,char* queryfile ) {
    int nt = count_cpu();
    //nt=1;
    SearchThreadPara* se_thd_para=new SearchThreadPara[nt];
    ifstream ifile(queryfile,ios::in);
    if(!ifile.good()){
        cout<<"E->invalid query files !"<<endl;
        exit(-1);
    }
    int itr = 0;
    while(!ifile.eof()){
        string sfilename;
        ifile>>sfilename;
        //end with _ftr or _filterftr
        if (sfilename.find_last_of("_")!=(sfilename.size()-4)&&\
            sfilename.find_last_of("_")!=(sfilename.size()-10)){
            cout<<"W->invalid feature file:"<<sfilename<<endl;
            continue;
        }
        se_thd_para[itr].files.push_back(sfilename);
        itr++;
        itr%=nt;
    }

    ifile.close();

    pthread_t * pthread_t_arr = new pthread_t[nt];
    for (int i=0;i<nt;i++) {
        cout<<"->start thread :"<<i<<endl;
        se_thd_para[i].se=search_engine;
        char result_file[512];
        sprintf(result_file,"search_rslt%d.txt",i);
        se_thd_para[i].search_result_file=string(result_file);
        pthread_create(pthread_t_arr+i,NULL,search_file,(void*)(se_thd_para+i));
    }
    for (int i=0;i<nt;i++) {
        pthread_join(pthread_t_arr[i],NULL);
    }
}
void search_withdir(PqSearchEngine* search_engine,char *feature_dir){
    cout<<"->start to search in directory:"<<feature_dir<<endl;
    DIR* pdir=opendir(feature_dir);
    struct dirent   *ent;


//    /* clear all previous data */
//    ofstream of("search_result.txt");
//    of.close();

    int nt = count_cpu();
    SearchThreadPara* se_thd_para=new SearchThreadPara[nt];

    string sdir(feature_dir);
    if (sdir.find_last_of('/')!=(sdir.size()-1)) sdir.append("/");
    int itr=0;
    while(NULL!=(ent=readdir(pdir))) {
        if(ent->d_type&DT_DIR) {
            continue;
        }
        else {
            string sfilename(ent->d_name);
            //cout<<" filename:"<<sfilename<<sfilename.find_last_of("_")<<" "<<sfilename.size()<<endl;
            /* all feature file should end with _ftr*/
            if (sfilename.find_last_of("_")!=(sfilename.size()-4)) continue;
            //if (sfilename.find_last_of("_ftr")) continue;

            se_thd_para[itr].files.push_back(sdir+sfilename);
            itr++;
            itr%=nt;
        }
    }
    closedir(pdir);

    pthread_t * pthread_t_arr = new pthread_t[nt];
    for (int i=0;i<nt;i++) {
        cout<<"->start thread :"<<i<<endl;
        se_thd_para[i].se=search_engine;
        char result_file[512];
        sprintf(result_file,"search_%d.rslt",i);
        se_thd_para[i].search_result_file=string(result_file);
        pthread_create(pthread_t_arr+i,NULL,search_file,(void*)(se_thd_para+i));
    }
    for (int i=0;i<nt;i++) {
        pthread_join(pthread_t_arr[i],NULL);
    }
}

void* search_file(void* searchThreadPara) {

#ifdef _DEBUG
    clock_t start,end;
#endif /* _DEBUG */

    SearchThreadPara *se_para=(SearchThreadPara*)searchThreadPara;
    PqSearchEngine* se=se_para->se;

    PqModel*  m_model   = se->getModel();
    DataBase* m_db      = se->getDataBase();
    int       m_w       = se->getBuckNeigNum();
    int       m_sift_neighbor_num = se->getSiftNeigNum();
    int       m_pic_neighbor_num  = se->getPicNeigNum();


    int file_num=se_para->files.size();
    //cout<<"file size:"<<file_size<<endl;
    for(int file_index=0;file_index<file_num;file_index++){

#ifdef _DEBUG
        start = clock();
#endif /*_DEBUG*/
        /* store all query features */
        float * t_queryFeatures;

        const char *filepath=se_para->files[file_index].c_str();
#ifdef _DEBUG
        cout<<"->search :"<<filepath<<endl;
#else
        cout<<".";
#endif /*_DEBUG*/
        int query_num=0;
        ifstream ifile(filepath,ios::binary);
        if(!ifile.good()){
            cout<<"E->invalid query feature file"<<endl;
            ifile.close();
            continue;
        }
        // read feature file header
        FtrFileHeader header;
        ifile.read((char*)&header,sizeof(FtrFileHeader));
        if(!ifile.good()) {
            ifile.close();
            cout<<"read feature file header error!"<<endl;
            continue;
        }

        query_num=header.m_ftr_num;

        /* invalid query */
        if (query_num<=0 ) {
            cout<<"read feature file header error!"<<endl;
            continue;
        }

        t_queryFeatures = fvec_new(query_num*FEATURE_DIM);
        for (int i=0;i<header.m_ftr_num;i++) {
            MetaFeature tmp_ftr;
            ifile.read((char*)&(tmp_ftr), sizeof(MetaFeature));
            memcpy(t_queryFeatures+i*FEATURE_DIM,\
                    tmp_ftr.m_descriptor,\
                    FEATURE_LEN);
        }

        ifile.close();

#ifdef _DEBUG
        end=clock();
        cout<<"read file time:"<<(end-start)*1.0/CLOCKS_PER_SEC<<endl;
        start = end;
#endif /*_DEBUG*/

        int *assign = ivec_new(m_w*query_num);
        m_model->predict(t_queryFeatures,query_num,m_w,assign,2);

        //int i,j,k=0,l=0,bin,q=0;

#ifdef _DEBUG
        for (int i=0;i<w*nq;i++) {
            if(assign[i]>m_model.getCoarseK()){
                cout<<"assign["<<i<<"]:"<<assign[i]<<">coarse_k\t";
                exit(0);
                cout<<endl;
            }
        }
        cout<<endl;
#endif /* _DEBUG*/

        /*parameter:m_sift_neighbor_num*/
        MaxHeap<Associator>* qresultvec =
            new MaxHeap<Associator>[query_num];
        for(int query_idx=0;query_idx<query_num;query_idx++) {
            /* init objects with max size */
            qresultvec[query_idx].init(m_sift_neighbor_num);
        }

#ifdef _DEBUG
        end=clock();
        cout<<"coarse index time:"<<(end-start)*1.0/CLOCKS_PER_SEC<<endl;
        start = end;
#endif /*_DEBUG*/

        for(int bin=0;bin<m_w;bin++) {
            // compute residual vector
            float* qresidual=fvec_new(query_num*FEATURE_DIM);

            for (int j=0;j<query_num;j++) {
                for (int k=0;k<FEATURE_DIM;k++) {
                    qresidual[j*FEATURE_DIM+k]=\
                                               t_queryFeatures[j*FEATURE_DIM+k]-\
                                               (m_model->getCoarseCentroid())[assign[j*m_w+bin]*FEATURE_DIM+k];
                }
            }

            float* sub_v=fvec_new(LSQ*query_num);
            // compute the query residual and centroids distance table.
            //float* dis_tab[NSQ][KS*nq];
            float* dis_tab[NSQ];

            for (int k=0;k<NSQ;k++) {
                for (int i=0;i<query_num;i++) {
                    for (int j=0;j<LSQ;j++) {
                        /* problem */
                        sub_v[i*LSQ+j]=qresidual[i*FEATURE_DIM+j+k*LSQ];
                    }
                }

                dis_tab[k]=fvec_new(query_num*KS);
                /* check me*/
                compute_cross_distances(LSQ,query_num,KS,sub_v,\
                        m_model->getPqCentroid(k),dis_tab[k]);
            }

            free(qresidual);
            qresidual=NULL;
            free(sub_v);
            sub_v=NULL;

#ifdef _DEBUG
            end=clock();
            cout<<"compute distable time:"<<(end-start)*1.0/CLOCKS_PER_SEC<<endl;
            start = end;
#endif /* _DEBUG*/

            for (int i=0;i<query_num;i++) {
                int cl_idx=assign[i*m_w+bin];
                /* point in all database index */
                int db_idx=0;
                //for (int db_i=0;db_i<m_db.size();db_i++) {
                //    DataBase *db = m_db[db_i];
                DataBase *db = m_db;
                int cl_el_num = \
                                db->getCoarseClusterEleNum(cl_idx);
                int start_idx = \
                                db->getCoarseClusterStartIdx(cl_idx);

                //cout<<index<<"\t"<<endl;
                // for (k=0;k<cluster_element_num;k++) {
                for(int el_idx=0;el_idx<cl_el_num;el_idx++) {
                    float tmp_dis=0;
                    //int base_idx= m_model->m_coarse_cluster_assign_idx[index+k];
                    int base_idx = \
                                   db->getCoarseClusterAssignIdx(start_idx+el_idx);
                    for (int l=0;l<NSQ;l++) {
                        int pq_idx = \
                                     db->getPqAssign(l,base_idx);
                        tmp_dis+=dis_tab[l][pq_idx+KS*i];
                    }
                    //Associator dist(tmp_dis,base_idx+db_idx);
                    Associator dist(tmp_dis,base_idx);
                    qresultvec[i].insert(dist);
                    //pair<float,int> dist(tmp_dis,base_idx);
                    //qresultvec[i].push_back(dist);
                    // }
                    // db_idx+=db->getFtrNum();
            }
            }
            for(int k=0;k<NSQ;k++) {
                free(dis_tab[k]);
            }

#ifdef _DEBUG
            end=clock();
            cout<<"look up in distable time:"<<(end-start)*1.0/CLOCKS_PER_SEC<<endl;
            start = end;
#endif /* _DEBUG*/
        }

        if(assign){
            free(assign);
            assign=NULL;
        }
        //parameter
        //map<int,int> votes;
        int ftrfilenum=0;
        //for (int db_i;db_i<m_db->size();db_i++){
        //    ftrfilenum+=m_db[i]->getFtrFileNum();
        //}
        ftrfilenum = m_db->getFtrFileNum();
        int * votes = ivec_new_0(ftrfilenum);
        int* filter = ivec_new_0(ftrfilenum);
        for (int i=0;i<query_num;i++){
            //map<int,int> filter;
            memset(filter,0,ftrfilenum*sizeof(int));

            //sort(qresultvec[i].begin(),qresultvec[i].end());
            int num=qresultvec[i].size();
            //int k=qresultvec[i].size();
            //int num=k>m_sift_neighbor_num?m_sift_neighbor_num:k;

            for (int iter=1;iter<=num;iter++) {
                int baseIdx=qresultvec[i].getData()[iter].idx;
                //int baseIdx=qresultvec[i][iter].second;
                //asset(m_db->size()>0);
                DataBase * db=m_db;
                int fileIdx=0;
                //for (int db_i;db_i<m_db->size();db_i++){
                //    db=m_db[db_i];
                //    if (baseIdx<db->getFtrNum()) break;
                //    baseIdx-=db->getFtrNum();
                //    fileIdx+=db->getFtrFileNum();
                //}
                //fileIdx+=db->getFtrFileIdx(baseIdx);
                fileIdx = db->getFtrFileIdx(baseIdx);

                if (!filter[fileIdx]){
                    votes[fileIdx]++;
                    filter[fileIdx]++;
                }
            }
        }
#ifdef _DEBUG
        end=clock();
        cout<<"vote neighbours time:"<<(end-start)*1.0/CLOCKS_PER_SEC<<endl;
        start = end;
#endif /* _DEBUG*/


        int* sorted_file_index=ivec_new(ftrfilenum);
        ivec_sort_index(votes,ftrfilenum,sorted_file_index);

       // vector<pair<int,int> > invVotes(10000);
       // for (int itr=0;itr!=ftrfilenum;itr++) {
       //     invVotes.push_back(pair<int,int>(votes[itr],itr));
       // }

       // sort(invVotes.begin(),invVotes.end());

#ifdef _DEBUG
        end=clock();
        cout<<"sort neighbours time:"<<(end-start)*1.0/CLOCKS_PER_SEC<<endl;
        start = end;
#endif /* _DEBUG*/

        //vector<pair<int,int> >::reverse_iterator revitr; 
        ofstream ofile(se_para->search_result_file.c_str(),ios::app);

        
        //cout<<"filename:"<<filepath<<"\t feature num:"<<query_num<<endl;
        ofile<<"filename:"<<filepath<<"\t feature num:"<<query_num<<endl;

        //parameter
       // for(i=0,revitr=invVotes.rbegin();i<m_pic_neighbor_num&&revitr!=invVotes.rend();i++,revitr++){
       //     //   cout<<revitr->second<< " scores:"<<revitr->first<<endl;
       //     string fn=m_ds->getFtrFileName(revitr->second);
       //     ofile<<fn<< " scores:"<<revitr->first<<endl;
       // }
        if(m_pic_neighbor_num>ftrfilenum){
            m_pic_neighbor_num=ftrfilenum;
        }
        for (int reiter=0;reiter<m_pic_neighbor_num;reiter++) {
            int index = sorted_file_index[ftrfilenum-1-reiter];
            DataBase* db=m_db;
            //for(int db_i=0;db_i<m_db->size();db_i++) {
            //    db=m_db[db_i];
            //    if (index<db->getFtrFileNum()) break;
            //    index-=db->getFtrFileNum();
            //}
            string fn = db->getFtrFileName(index);
            ofile<<fn<<" scores:"<<votes[index]<<endl;
        }

        for(int i =0;i<query_num;i++) {
            float top_dis=qresultvec[i].getData()[1].dis;
            ofile<<top_dis<<"\t";
        }
        ofile<<endl;
        ofile.close();

#if _DEBUG
        ofstream top_dis_file((se_para->search_result_file+".top_dis").c_str(),ios::app);
        top_dis_file<<"filename:"<<filepath<<"\t feature num:"<<query_num<<endl;
        for (int i = 0;i<query_num;i++) {
            float top_dis=qresultvec[i].getData()[1].dis;
            top_dis_file<<"idx:"<<i<<" dis:"<<top_dis<<endl;
        }
        top_dis_file.close();
#endif /*_DEBUG*/

        if(qresultvec){
            delete[] qresultvec;
            qresultvec=NULL;
        }

        free(votes);
        free(filter);
        free(sorted_file_index);

        free(t_queryFeatures);
#ifdef _DEBUG
        end=clock();
        cout<<"output neighbours time:"<<(end-start)*1.0/CLOCKS_PER_SEC<<endl;
        start = end;
#endif /* _DEBUG*/
    }
}


//void PqSearchEngine::trainModel(DataSet* dataset,char* cenfile) {
//    DataBase* database = m_model.trainModel(dataset,cenfile);
//    //m_db.push_back(database);
//    m_db=database;
//}
//
//void PqSearchEngine::addAdditionalDataSet(DataSet* dataset) {
//    DataBase* db = m_model.getDataBase(dataset);
//    //m_db.push_back(db);
//    m_db.merge(db);
//    delete db;
//}
//
//void PqSearchEngine::getDataBaseNum() {
//    return m_db.size();
//}
//void PqSearchEngine::getDistances(int query_idx,int cl_idx,float**dis_tab) {
//    for (int db_idx=0;db_idx<m_db.size();db_idx++) {
//        DataBase *db=m_db[db_idx];
//        int cl_el_num = db->getCoarseClusterEleNum(cl_idx); 
//        int start_idx = db->getCoarseClusterStartIdx(cl_idx);
//
//        for(int el_idx=0;el_idx<cl_el_num;el_idx++) {
//            float tmp_dis=0;
//            int base_idx=\
//                db->getCoarseClusterAssignIdx(start_idx+el_idx);
//            for (int sub_idx=0;sub_idx<NSQ;sub_idx++) {
//                int pq_idx = db->getPqAssign(sub_idx,base_idx);
//                tmp_dis+=dis_tab[sub_idx][pq_idx+KS*query_idx];
//            }
//        }
//    }
//}

PqSearchEngine::~PqSearchEngine() {
}
bool PqSearchEngine::loadEngine(const char *engine_file) {
//    const char *filepath=engine_file;
//    ifstream ifile(filepath);
//    if(ifile.good()) {
//        cout<<"->start to load engine from :"<<filepath<<endl;
//        /* read data set parameter */
//        cout<<"feature num:"<<m_ds.m_num<<" "<<m_ds.m_dim<<endl;
//        ifile.read((char*)&(m_ds.m_num),sizeof(int));
//        ifile.read((char*)&(m_ds.m_dim),sizeof(int));
//        cout<<"feature num:"<<m_ds.m_num<<" "<<m_ds.m_dim<<endl;
//
//        m_ds.m_featureFileIdx=ivec_new(m_ds.m_num);
//        ifile.read((char*)m_ds.m_featureFileIdx,sizeof(int)*(m_ds.m_num));
//
//        /* read pqmodel parameter */
//        ifile.read((char*)&(m_model.m_coarse_k),sizeof(int));
//        cout<<"coarse_k:"<<m_model.m_coarse_k<<endl;
//
//        m_model.m_coarse_assign=ivec_new(m_ds.m_num);
//        m_model.m_coarse_cluster_element_num=ivec_new(m_model.m_coarse_k);
//        m_model.m_coarse_cluster_assign_idx=ivec_new(m_ds.m_num);
//        m_model.m_coarse_cluster_idx=ivec_new(m_model.m_coarse_k);
//        m_model.m_coarse_centroids=fvec_new(m_model.m_coarse_k*m_ds.m_dim);
//
//        ifile.read((char*)(m_model.m_coarse_assign),sizeof(int)*m_ds.m_num);
//        ifile.read((char*)(m_model.m_coarse_cluster_element_num),sizeof(int)*(m_model.m_coarse_k));
//        ifile.read((char*)(m_model.m_coarse_cluster_assign_idx),sizeof(int)*m_ds.m_num);
//        ifile.read((char*)(m_model.m_coarse_cluster_idx),sizeof(int)*(m_model.m_coarse_k));
//        ifile.read((char*)(m_model.m_coarse_centroids),sizeof(float)*(m_model.m_coarse_k)*(m_ds.m_dim));
//
//        for (int i=0;i<NSQ;i++) {
//            m_model.m_pq_assign[i]=ivec_new(m_ds.m_num);
//            m_model.m_pq_centroids[i]=fvec_new(LSQ*KS);
//
//            ifile.read((char*)(m_model.m_pq_assign[i]),sizeof(int)*m_ds.m_num);
//            ifile.read((char*)(m_model.m_pq_centroids[i]),sizeof(float)*LSQ*KS);
//        }
//
//        int filename_size=0;
//        ifile.read((char*)&filename_size,sizeof(int));
//        cout<<"filename size:"<<filename_size<<endl;
//        for(int i=0;i<filename_size;i++) {
//            string tmp;
//            ifile>>tmp;
//            m_ds.m_vfilename.push_back(tmp);
//            //cou<<tmp<<endl;
//        }
//    }
//    else {
//        return false;
//    }
//    ifile.close();
//    cout<<"->pq search engine loaded"<<endl;
    return true;

}

void PqSearchEngine::writeEngine(const char* engine_file) {
//    const char *filepath=engine_file;
//    ofstream ofile(filepath);
//    if(ofile.good()) {
//        cout<<"->start to write engine to :"<<filepath<<endl;
//        /* write data set parameter */
//        ofile.write((char*)&m_ds.m_num,sizeof(int));
//        ofile.write((char*)&m_ds.m_dim,sizeof(int));
//        ofile.write((char*)m_ds.m_featureFileIdx,sizeof(int)*m_ds.m_num);
//
//        /* write pqmodel parameter */
//        ofile.write((char*)&(m_model.m_coarse_k),sizeof(int));
//        ofile.write((char*)(m_model.m_coarse_assign),sizeof(int)*m_ds.m_num);
//        ofile.write((char*)(m_model.m_coarse_cluster_element_num),sizeof(int)*(m_model.m_coarse_k));
//        ofile.write((char*)(m_model.m_coarse_cluster_assign_idx),sizeof(int)*m_ds.m_num);
//        ofile.write((char*)(m_model.m_coarse_cluster_idx),sizeof(int)*(m_model.m_coarse_k));
//        ofile.write((char*)(m_model.m_coarse_centroids),sizeof(float)*(m_model.m_coarse_k)*(m_ds.m_dim));
//
//        for (int i=0;i<NSQ;i++) {
//            ofile.write((char*)(m_model.m_pq_assign[i]),sizeof(int)*m_ds.m_num);
//            ofile.write((char*)(m_model.m_pq_centroids[i]),sizeof(float)*LSQ*KS);
//        }
//
//        int filename_size=m_ds.m_vfilename.size();
//        ofile.write((char*)&filename_size,sizeof(int));
//        for(int i=0;i<filename_size;i++) {
//            ofile<<m_ds.m_vfilename[i]<<endl;
//        }
//    }
//    else {
//        cout<< "can't open output file:"<<filepath<<endl;
//    }
//
//    ofile.close();
//    cout<<"->write engine ended!"<<endl;
}





