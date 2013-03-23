#include "surflib.h"
#include <cv.h>
#include <highgui.h>
#include <ctime>
#include <iostream>
#include <sys/stat.h>

#include "extracter.h"
#include "pqsearchengine.h"

void usage() {
    cout<<"usage:pqsearch [query_ftr_dir|query_list_file]"<<endl;
}
int main(int argc, char* argv[]){
    if(argc!=2) {
        usage();
        return 0;
    }

    //parameter
    int bucket_neig=16;
    int sift_neighbor_num=5000;
    int pic_neigh_num = 50;

    PqModel* model = new PqModel();
    char* model_file = "model.dat";
    model->loadModel(model_file);

    DataBase* db = new DataBase();
    char* database_file = "database.dat";
    db->loadDataBase(database_file);

    PqSearchEngine se;
    se.setBuckNeigNum(bucket_neig);
    se.setSiftNeigNum(sift_neighbor_num);
    se.setPicNeigNum(pic_neigh_num);
    se.setDataBase(db);
    se.setModel(model);

    /* first check if this two file exits.
     * if search_engine exits, load engine;
     * else if centorid_file exits, load centorid_file
     * else train model
     */
    //char *engine_file="search_engine.txt";
    //char *centorid_file="centroids.txt";

    //ifstream ifile(engine_file);
    //if(ifile.good()){
    //    ifile.close();
    //    se.loadEngine(engine_file);
    //}
    //else {
    //    ifile.close();
    //    /* argv[1] base pic_ftr dir */
    //    se.init(argv[1],coarse_k,centorid_file,engine_file);
    //}

    struct stat info;
    stat(argv[1],&info);
    if(S_ISDIR(info.st_mode)) {
        search_withdir(&se,argv[1]);
    }
    else {
        search_withfile(&se,argv[1]);
    }
    return 0;
}

