#ifndef _EXTRACTER
#define _EXTRACTER
#include <iostream>

using namespace std;

class Extracter {
    private:
        // feature dir to write features
        char *m_ftrDir;
        
        int  m_feature_num;
        // extract features from a picture.
        void _extFtr(string picName);

        // for test
        void _testExtFtr(char*filename);
    public:
        // construct with a  picture directory
        Extracter(char* pFtrDir): m_ftrDir(pFtrDir),\
                                              m_feature_num(0){
        
        cout<<"m_ftrDir:"<<m_ftrDir<<endl;
        
                                              }

        void extractFileFeatures(char *file);
        void extractDirFeatures(char *dir);

        // for test
        void testExtracter();
};

#endif /* _EXTRACTER */
