#ifndef _DATABASE_H
#define _DATABASE_H
#include <vector>
#include <string>
#include "types.h"
using namespace::std;

/*
 * dataset is store as database in memory
 * in order to save memory
 */
class DataBase {
    int     m_feature_num;
    int     m_coarse_k;
    int*    m_coarse_assign;     /* feature_num */
    int*    m_pq_assign[NSQ];         /* NSQ*feature_num */

    /* invert feature table */
    int*    m_coarse_cluster_start_idx;     /* cluster start idex in m_coarse_cluster_assign_idx */
    int*    m_coarse_cluster_element_num;
    int*    m_coarse_cluster_assign_idx;

    /* feature filename */
    int*        m_featureFileIdx;
    vector<string> m_vfilename;/* feature file names */

    public:
    DataBase();

    /* merge database */
    bool merge(DataBase*db);
    /* build the invert file */
    void buildIvf();

    /* toggle with disk */
    bool saveDataBase(char* dbfile);
    bool loadDataBase(char* dbfile);

    void  setFtrNum(int feature_num);
    int  getFtrNum();

    void setCoarseK(int coarse_k);
    int  getCoarseK();

    string getFtrFileName(int fileIdx);
    void setFtrFileName(vector<string> ftrfilename);
    int  getFtrFileNum();
    int  getFtrFileIdx(int ftrIdx);
    void setFtrFileIdx(int* ftrfileIdx);

    void setPqAssign(int idx,int* assign);
    int  getPqAssign(int sub_idx,int base_idx);
    void setCoarseAssign(int* coarse_assign);
    int  getCoarseAssign(int feature_idx);
    void setCoarseClusterStartIdx(int* idx);
    int  getCoarseClusterStartIdx(int idx);
    void setCoarseClusterEleNum(int *en);
    int  getCoarseClusterEleNum(int cluster_idx);
    void setCoarseClusterAssignIdx(int *idx);
    int  getCoarseClusterAssignIdx(int idx);

    ~DataBase();
};
#endif /* _DATABASE_H*/

