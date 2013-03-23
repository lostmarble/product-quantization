#include <iostream>
#include <fstream>
#include <stdio.h>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#include "featureflfmt.h"
#include "stablizer.h"

extern "C" {
#include "../contrib/yael/kmeans.h"
#include "../contrib/yael/nn.h"
#include "../contrib/yael/vector.h"
#include "../contrib/yael/sorting.h"
#include "../contrib/yael/machinedeps.h"
}
using namespace cv;
using namespace std;
/* bottle neck is in the findHomography */
//#define _STABLIZER_CPP_DEBUG

void Stablizer::getStablePoints(string _pic,\
               vector<string>*_v_match_pic,vector<float>*top_dis){
    //string feature_dir ="/home/xzx/workspace/surf/test_base_pic_ftr1/";
    //string pic=feature_dir+_pic;
#ifdef _STABLIZER_CPP_DEBUG
    clock_t start, end;
    start=clock();
#endif /* _STABLIZER_CPP_DEBUG */
    string pic=_pic;
    vector<Point2f> origin_pos;
    origin_pos.reserve(100);
    Mat* origin_pic = readFeatures(pic,&origin_pos);
    int feature_num = origin_pic->rows;

   // FileStorage fs("descriptors",FileStorage::WRITE);
   // fs<<"mat"<<(*origin_pic);
   // fs.release();

   // FileStorage fsr("descriptors",FileStorage::READ);
   // fsr["mat"]>>(*origin_pic);
   // fsr.release();
    vector<string> unmatch_filename;
    vector<Mat> unstable_discriptors;

    /* measures for each point */
    vector<int> stable_metric(feature_num,0);
    vector<int> discriminant_metric(feature_num,0);
    vector<int> confusion_metric(feature_num,0);

    /* index and vote */
    //map<int,int> feature_vote;
    int* feature_vote = ivec_new_0(feature_num);
    for(int i_=0;i_<_v_match_pic->size();i_++) {
#ifdef _STABLIZER_CPP_DEBUG
        end=clock();
        cout<<"read feature time:"<<end-start<<endl;
        start=clock();
#endif /* _STABLIZER_CPP_DEBUG */
        //string match_ftr=feature_dir+(*_v_match_pic)[i];
        string match_ftr=(*_v_match_pic)[i_];
        vector<Point2f> match_pos;
        match_pos.reserve(200);
        //cout<<"match_pos size:"<< match_pos.size()<<endl;
        Mat* match = readFeatures(match_ftr,&match_pos);
        if (NULL==match) {
            cout<<"read "<<match_ftr<<" feature error"<<endl;
            continue;
        }
        
        vector<DMatch> good_matches;
        good_matches.reserve(200);

#ifdef _STABLIZER_CPP_DEBUG
        end=clock();
        cout<<"read feature time:"<<end-start<<endl;
        start=clock();
#endif /* _STABLIZER_CPP_DEBUG */

        vector<int>tmp_dis_con;
        tmp_dis_con.reserve(feature_num);
#ifdef GOOD_MATCH
        float good_match_threshold = 0.9;
        vector<vector<DMatch> > knnMatches;
        BruteForceMatcher< L2<float> > matcher;
        matcher.knnMatch((*origin_pic),(*match),knnMatches,2);
        for (int i=0;i<knnMatches.size();i++) {
            double dist1 = knnMatches[i][0].distance;
            double dist2 = knnMatches[i][1].distance;
            if ((dist1/dist2)>=good_match_threshold) {
                good_matches.push_back(knnMatches[i][0]);
            }
        }

#else /* GOOD_MATCH */
        FlannBasedMatcher matcher;
        vector<DMatch> matches;
        matcher.match((*origin_pic),(*match),matches);

        double max_dist = 0;
        double min_dist = 100000;
        for (int i=0; i<(*origin_pic).rows;i++) {
            double dist = matches[i].distance;
            if (dist<(*top_dis)[i]){
                tmp_dis_con.push_back(1);
            }
            else{
                tmp_dis_con.push_back(0);
            }
            if (dist<min_dist ) min_dist = dist;
            if (dist>max_dist ) max_dist = dist;
        }

        //parameter to define a good match 
        //dist<alpha*min_dist
        float alpha=m_sift_good_match_threshold;
        /* find good featue and vote */
        for (int i=0;i<(*origin_pic).rows;i++) {
            if (matches[i].distance <= alpha*min_dist) {
                good_matches.push_back(matches[i]); 
            }
        }
    
#endif /* GOOD_MATCH */

#ifdef _STABLIZER_CPP_DEBUG
        end=clock();
        cout<<"match point time:"<<end-start<<endl;
        start=clock();
#endif /* _STABLIZER_CPP_DEBUG */

        vector<Point2f> origin_good_pos;
        vector<Point2f> match_good_pos;
        for (int i=0;i<good_matches.size();i++) {
            origin_good_pos.push_back(origin_pos[good_matches[i].queryIdx]);
            match_good_pos.push_back(match_pos[good_matches[i].trainIdx]);
        }
        vector<uchar> status;
        //cout<<origin_good_pos.size()<<"  "<<match_good_pos.size()<<endl;
        if (origin_good_pos.size()>=4) {
            Mat H = findHomography(origin_good_pos,match_good_pos,CV_RANSAC,m_ransac_dis_threshold,status);
#ifdef _STABLIZER_CPP_DEBUG
            end=clock();
            cout<<"findHomography time:"<<end-start<<endl;
            start=clock();
#endif /* _STABLIZER_CPP_DEBUG */

            int good_point_count = 0;
            for(int i=0;i<origin_good_pos.size();i++) {
                if(status[i]) {
                    good_point_count++;
                }
            }
            
            /* parameter:good point ratio */
            float good_point_ratio=m_ransac_good_point_ratio;
            /* this is a good match */
            if (good_point_count>=origin_good_pos.size()*good_point_ratio){
                for(int i=0;i<origin_good_pos.size();i++) {
                    if(status[i]) {
                        feature_vote[good_matches[i].queryIdx]++;
                        stable_metric[good_matches[i].queryIdx]++;
                    }
                }

                for(int i=0;i<feature_num;i++) {
                    discriminant_metric[i]+=tmp_dis_con[i];
                }
            }
            else { /* bad match */
                unmatch_filename.push_back(match_ftr);
                unstable_discriptors.push_back((*match).clone());
                for(int i=0;i<feature_num;i++) {
                    confusion_metric[i]+=tmp_dis_con[i];
                }
            }
        }
        else /* unmatch picture */
        {
            unmatch_filename.push_back(match_ftr);
            unstable_discriptors.push_back((*match).clone());
            for(int i=0;i<feature_num;i++) {
                confusion_metric[i]+=tmp_dis_con[i];
            }
        }

        delete match; 
#ifdef _STABLIZER_CPP_DEBUG
        end=clock();
        cout<<"vote feature time:"<<end-start<<endl;
        start=clock();
#endif /* _STABLIZER_CPP_DEBUG */
    }
    
    /* get the bad matches */
    if (unmatch_filename.size()>0) {
        ofstream unmatch_file(m_unmatch_file.c_str(),ios::app);
        unmatch_file<<"filename:"<<pic<<"\tunmatch number:"<<unmatch_filename.size()<<endl;
        for (int i = 0;i<unmatch_filename.size();i++) {
            unmatch_file<<unmatch_filename[i]<<endl;
        }
        unmatch_file<<endl;
        unmatch_file.close();
    }

    int* feature_status = ivec_new_0(feature_num); /*stable or not*/
    cout<<"unstable file num:"<<unstable_discriptors.size()<<endl;
    if (unstable_discriptors.size()>0){
        /* parameter */
        float unstable_match_threshold=m_unstable_match_threshold;
        FlannBasedMatcher unstable_matcher;
        vector<vector<DMatch> > knnunstable_matches;
        vector<DMatch> bad_matches;
        unstable_matcher.add(unstable_discriptors);
        //unstable_matcher.train();
        unstable_matcher.knnMatch(*origin_pic,knnunstable_matches,2);
        //assert(knnunstable_matches.size()==feature_num);
        for (int i=0;i<knnunstable_matches.size();i++) {
            double dist1 = knnunstable_matches[i][0].distance;
            double dist2 = knnunstable_matches[i][1].distance;
            if (dist1<dist2*unstable_match_threshold) {
                bad_matches.push_back(knnunstable_matches[i][0]);
                int index = knnunstable_matches[i][0].queryIdx;
                feature_status[index]++;
            }
        }
    }
   // vector<Point2f> origin_bad_pos;
   // for (int i=0;i<bad_matches.size();i++) {
   //     origin_bad_pos.push_back(origin_pos[bad_matches[i].queryIdx]);
   //     cout<<"bad match pos:"<<origin_pos[bad_matches[i].queryIdx]<<endl;
   // }
    //FIXME
    /* now what to do? */

    //vector<pair<int,int> > invVotes;
    //map<int,int>::iterator itr;
    //for (itr=feature_vote.begin();itr!=feature_vote.end();itr++) {
    //    invVotes.push_back(pair<int,int>(itr->second,itr->first));
    //}
    //sort(invVotes.begin(),invVotes.end());

    int* sorted_sift_index = ivec_new_0(feature_num);
    ivec_sort_index(feature_vote,feature_num,sorted_sift_index);

#ifdef _STABLIZER_CPP_DEBUG
    end=clock();
    cout<<"sort time:"<<end-start<<endl;
    start=clock();
#endif /* _STABLIZER_CPP_DEBUG */

    ofstream ofile(m_out_file,ios::app);
    ofile<<"filename:"<<_pic<<" num:"<<origin_pic->rows<<endl;

    //parameter
    int stable_point_num=1000;

    //vector<pair<int,int> >::reverse_iterator revitr;
    //int i=0;
   // for(i=0,revitr=invVotes.rbegin();i<stable_point_num&&revitr!=invVotes.rend();i++,revitr++){
   //     //cout<<revitr->second<< " scores:"<<revitr->first<<endl;
   //     ofile<<"index:"<<revitr->second<< " scores:"<<revitr->first<<endl;
   // }
    
    for(int i=0;i<feature_num;i++) {
        int sift_index=sorted_sift_index[feature_num-i-1];
        ofile\
         <<"index:\t"<<sift_index\
         <<"\tscores:\t"<<feature_vote[sift_index]\
         <<"\tstatus:\t"<<feature_status[sift_index]\
         <<"\tM:\t"<<_v_match_pic->size()-unmatch_filename.size()\
         <<"\ts:\t"<<stable_metric[sift_index]\
         <<"\td:\t"<<discriminant_metric[sift_index]\
         <<"\tc:\t"<<confusion_metric[sift_index]\
         <<endl;
    }
    free(feature_vote);
    free(feature_status);
    free(sorted_sift_index);
    ofile<<endl;
    ofile.close();
    delete origin_pic;

#ifdef _STABLIZER_CPP_DEBUG
    end=clock();
    cout<<"sort time:"<<end-start<<endl;
    start=clock();
#endif /* _STABLIZER_CPP_DEBUG */

}


Mat* Stablizer::readFeatures(string _pic,vector<Point2f>*pos) {
    ifstream ifile(_pic.c_str(),ios::binary);

    if (ifile.good()) {
        // read feature file header
        FtrFileHeader header;
        ifile.read((char*)&header,sizeof(FtrFileHeader));
        Mat* feature = new Mat(header.m_ftr_num,header.m_ftr_dim,CV_32F);

        //cout<<_pic<<": "<<header.m_ftr_num<<endl;
        for (int i=0;i<header.m_ftr_num;i++) {
            MetaFeature tmp_ftr;
            ifile.read((char*)&(tmp_ftr), sizeof(MetaFeature));
            for (int j=0;j<header.m_ftr_dim;j++) {
                feature->at<float>(i,j)=tmp_ftr.m_descriptor[j];
            }
            Point2f sift_pos(tmp_ftr.x,tmp_ftr.y);
            pos->push_back(sift_pos);
        }
        ifile.close();
        return feature;
    }
    else {
        ifile.close();
        cout<<"can't read "<<_pic<<" features"<<endl;
        return NULL;
    }
}


void Stablizer::stable(char* search_result) {

    /* clean previous results in the output file */
    ofstream ofile(m_out_file);
    ofile.close();

    char unmatch_out[512];
    sprintf(unmatch_out,"%s.unmatch_file",search_result);
    m_unmatch_file=string(unmatch_out);
    ofstream ounfile(unmatch_out);
    ounfile.close();

    //ifstream ifile(search_result);
    FILE* ifile;
    ifile=fopen(search_result,"r");
    while(!feof(ifile)) {
        char pic_name[512];
        int feature_num=0;
        fscanf(ifile,"\nfilename:%s\t feature num:%d\n",pic_name,&feature_num);
        cout<<"-->"<<pic_name<<endl;
        string pic(pic_name);
        vector<string> match_file;
        for (int i=0;i<PIC_NEIGHBOR_NUM;i++) {
            fscanf(ifile,"%s ",pic_name);
            string match(pic_name);
            match_file.push_back(match);

            /* ignore the scores */
            fscanf(ifile,"%s\n",pic_name);
        }
        vector<float> top_dis;
        top_dis.reserve(feature_num);
        float dis_tmp=0.0;
        /* get the dis for each point */
        for (int i=0;i<feature_num;i++) {
            char sdis[512];
            fscanf(ifile,"%s",sdis);
            dis_tmp=atof(sdis);
            //cout<<dis_tmp<<"\t";
            top_dis.push_back(dis_tmp);
        }
        //fscanf(ifile,"%s",&pic_name);
        
        getStablePoints(pic,&match_file,&top_dis);
        match_file.clear();
    }
    fclose(ifile);
}

//void Stablizer::unstable(char * unmatch_filename){
//    FILE* ifile;
//    ifile = fopen(unmatch_filename,"r");
//    while (!feof(ifile)) {
//        char pic_name[512];
//        int unmatch_num=0;
//        fscanf(ifile,"filename:%s\tunmatch number:%d\n",pic_name,&unmatch_num);
//        string pic(pic_name);
//        vector<string> unmatch_file;
//        for (int i=0;i<unmatch_num;i++) {
//            fscanf(ifile,"%s\n",pic_name);
//            unmatch_file.push_back(pic_name);
//        }
//        getUnstablePoints(pic,&unmatch_file);
//        unmatch_file.clear();
//    }
//    fclose(ifile);
//}
//
//void Stablizer::getUnstablePoints(string pic,vector<string>*unmatchfile){
//    vector<Point2f> origin_pos;
//    Mat* origin_pic = readFeatures(pic,&origin_pos);
//    for(int i=0;i<unmatchfile->size();i++) {
//        //string match_ftr=feature_dir+(*_v_match_pic)[i];
//        string match_ftr=(*unmatchfile)[i];
//        vector<Point2f> match_pos;
//        Mat* match = readFeatures(match_ftr,&match_pos);
//    
//}
