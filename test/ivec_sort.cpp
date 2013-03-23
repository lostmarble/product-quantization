
extern "C" {
#include "../contrib/yael/kmeans.h"
#include "../contrib/yael/nn.h"
#include "../contrib/yael/vector.h"
#include "../contrib/yael/sorting.h"
#include "../contrib/yael/machinedeps.h"
}

#include <iostream>
using namespace std;
int main() {
    int n=5;
    int stand[]={3,1,2,5,4};
    
    int* index=new int[n];
    ivec_sort_index(stand,n,index );
    for (int i=0;i<n;i++) {
        cout<<stand[i]<<"\t";
    }
    cout<<endl;
    for (int i=0;i<n;i++) {
        cout<<index[i]<<"\t";
    }
    delete[] index;
    return 0;
}

