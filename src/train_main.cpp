#include "model.h"
#include "database.h"
#include "dataset.h"

void usage() {
    cout<<"usage:pqtrain feature_dir"<<endl;
}
int main(int args, char* argv[]) {
    if (args!=2){
        usage();
        return -1;
    }

    //parameter
    int coarse_k=1024;

    DataSet* ds = new DataSet();
    ds->initDataSet(argv[1]);
    ds->readData(argv[1]);

    PqModel model;
    model.setCoarseK(coarse_k);
    char* centroid_file=NULL;
    DataBase* db = model.trainModel(ds,centroid_file);
    char* model_file = "model.dat";
    model.saveModel(model_file);
    char* database_file = "database.dat";
    db->saveDataBase(database_file);

    delete db;
    delete ds;
    return 0;
}

