#include <iostream>
#include "stablizer.h"
extern "C" {
#include "config.h"
}
using namespace std;
void usage() {
    cout<<"stablize search_result.txt stablizer_result.txt"<<endl;
    return;
}
int main(int argc, char* argv[]) {
    if (argc!=3) {
        usage();
        return 0;
    }
    Stablizer stable(argv[2]);
    char* conf = "stablizer.conf";
    if (parse_config_file(conf)) {
        float sift_good_match_threshold=\
               atof(get_config_var("sift_good_match_threshold")); 
        float ransac_good_point_ratio = \
               atof(get_config_var("ransac_good_point_ratio"));
        float unstable_match_threshold=\
               atof(get_config_var("unstable_match_threshold"));
        float ransac_dis_threshold=\
               atof(get_config_var("ransac_dis_threshold"));
        cout<<"pare config file:"<<endl;
        cout<<sift_good_match_threshold<<endl;
        cout<<ransac_good_point_ratio<<endl;
        cout<<unstable_match_threshold<<endl;
        cout<<ransac_dis_threshold<<endl;
        stable.setSiftMatchThreshold(sift_good_match_threshold);
        stable.setRansacGoodRatio(ransac_good_point_ratio);
        stable.setUnstableMatchThreshold(unstable_match_threshold);
        stable.setRansacDisThreshold(unstable_match_threshold);
    }
        
    stable.stable(argv[1]);

    return 0;

}
