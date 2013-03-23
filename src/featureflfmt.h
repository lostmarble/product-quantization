#ifndef _FEATUREFLFMT_H
#define _FEATUREFLFMT_H
#define FEATURE_FILE_VER 1
#define FEATURE_DIM 64
#define FEATURE_LEN FEATURE_DIM*sizeof(float)
typedef struct _FtrFileHeader{
    int    m_version; // feature file format version.
    int    m_ftr_num; // feature number of a picture.
    int    m_ftr_len; // feature lens of bytes.
    int    m_ftr_dim; // feature dimension.
}FtrFileHeader;

typedef struct _MetaFeature{
    float m_scale;
    float m_orientation;
    /* added */
    float x;
    float y;
    
    float m_descriptor[FEATURE_DIM];
}MetaFeature;

#endif /* _FEATUREFLFMT_H */
