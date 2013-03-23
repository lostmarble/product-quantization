#ifndef _STABLE_POINT
#define _STABLE_POINT
#include <vector>
#include <string>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
using namespace std;
using namespace cv;

#define PIC_NEIGHBOR_NUM 50

class Stablizer {
    char *m_out_file;
    string m_unmatch_file;

    /* parameters */
    /* find good point match for RANSAC,
     * dist<sift_good_match_threshold* minimum_dist
     * if you don't want filter point in this step,
     * just set it large enough, 
     * default 100
     */
    float m_sift_good_match_threshold;

    /* RANSAC threshold 
     * it define the distane to distinguish innerliers &outliers
     */
    float m_ransac_dis_threshold;
    /* after RANSAC, when innerliers' num is 
     * bigger than (ratio)*(RANSAC points' number),
     * it is a good match.
     * default  0.09
     */
    float m_ransac_good_point_ratio;

    /* unstable match distance ratio
     * default 0.95
     */
    float m_unstable_match_threshold;

    
    public:
        Stablizer( char *_out_file):\
            m_out_file(_out_file),
            m_sift_good_match_threshold(100),
            m_ransac_good_point_ratio(0.09),
            m_unstable_match_threshold(0.95)
            {}
        
        Mat* readFeatures(string _pic,vector<Point2f>*sift_pos);
        /* get stable points for a pic and output in a file 
         * @_pic:the pic to be stablized
         * @_v_match_pic:matched picture for _pic
         * @top_dis:第top_N distance for each point in the _pic
         * */
        void getStablePoints(string _pic,vector<string>*_v_match_pic,vector<float>* top_dis);
        void stable(char* search_result);
        void setSiftMatchThreshold(float t){
            m_sift_good_match_threshold=t;
        }
        void setRansacGoodRatio(float r) {
            m_ransac_good_point_ratio=r;
        }
        void setUnstableMatchThreshold(float t) {
            m_unstable_match_threshold=t;
        }
        void setRansacDisThreshold(float t) {
            m_ransac_dis_threshold=t;
        }
};

#endif /* _STABLE_POINT */
