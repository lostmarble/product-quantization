#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "featureflfmt.h"
using namespace std;
using namespace cv;
void usage() {
    cout<<"usage:draw stablize_result.txt pic_dir"<<endl;
}

int main(int args, char *argv[]) {
    if (args!=3) {
        usage();
        return -1;
    }

    FILE* ifile=fopen(argv[1],"r");
    if (!ifile) {
        cout<<"can't read stablize result file :"<<argv[1]<<endl;
        fclose(ifile);
        exit(0);
    }
    while(!feof(ifile)) {
        char ftr_filename[512];
        int ftr_num=0;

        /* read all good feature index */
        fscanf(ifile,"filename:%s num:%d\n",ftr_filename,&ftr_num);
        vector<int> good_ftr_index;
        vector<int> unstable_ftr_index;

        good_ftr_index.clear();
        unstable_ftr_index.clear();
        for (int i=0;i<ftr_num;i++) {
            int idx=0,score=0;
            int status = 0;
            fscanf(ifile,"index:%d\tscores:%d\tstatus:%d\n",&idx,&score,&status);
            if(status) 
                unstable_ftr_index.push_back(idx);
            else 
                good_ftr_index.push_back(idx);

        }

        /* read all good feature position */
        vector<Point2f> ftr_pos;
        ifstream ftr_file(ftr_filename,ios::binary);
        if (ftr_file.good()) {
            // read feature file header
            FtrFileHeader header;
            ftr_file.read((char*)&header,sizeof(FtrFileHeader));
            for (int i=0;i<header.m_ftr_num;i++) {
                MetaFeature tmp_ftr;
                ftr_file.read((char*)&(tmp_ftr), sizeof(MetaFeature));
                Point2f sift_pos(tmp_ftr.x,tmp_ftr.y);
                ftr_pos.push_back(sift_pos);
            }
            ftr_file.close();
        }
        else {
            ftr_file.close();
            cout<<"can't read "<<ftr_filename<<" features"<<endl;
            return -1;
        }

        vector<Point2f> good_ftr_pos;
        vector<Point2f> unstable_ftr_pos;
        for (int i=0;i<good_ftr_index.size();i++) {
            good_ftr_pos.push_back(ftr_pos[good_ftr_index[i]]);
        }
        for (int i=0;i<unstable_ftr_index.size();i++) {
        unstable_ftr_pos.push_back(ftr_pos[unstable_ftr_index[i]]);
        }

        string sftrfilename(ftr_filename);
        int name_idx=sftrfilename.find_last_of("/");
        int name_idx_end=sftrfilename.find_last_of("_");
        if(name_idx==-1) {
            cout<<"can't find file name in filenamepath"<<endl;
            exit(0);
        }
        string  filename = sftrfilename.substr(name_idx+1,name_idx_end-name_idx-1);
        char pic_path[512];
        sprintf(pic_path,"%s/%s",argv[2],filename.c_str());

        cout<<"read image:"<<pic_path<<endl;
        Mat img = imread(pic_path);
       // vector<KeyPoint> points;
       // vector<KeyPoint> points1;
       // vector<KeyPoint> points2;
       // KeyPoint::convert(good_ftr_pos,points);
       // for (int i=0;i<points.size()/2;i++) {
       //     points1.push_back(points[i]);
       // }
       // for (int i=points.size()/2;i<points.size();i++) {
       //     points2.push_back(points[i]);
       // }
        vector<KeyPoint> good_points;
        vector<KeyPoint> unstable_points;
        KeyPoint::convert(good_ftr_pos,good_points);
        KeyPoint::convert(unstable_ftr_pos,unstable_points);
        Mat showImg=img.clone();
        drawKeypoints(img,good_points,showImg,Scalar(255,0,255),DrawMatchesFlags::DRAW_OVER_OUTIMG|DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        drawKeypoints(img,unstable_points,showImg,Scalar(0,255,0),DrawMatchesFlags::DRAW_OVER_OUTIMG|DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        imshow( filename, showImg );
        waitKey(0);
    }
    fclose(ifile);
    return 0;
}

