#ifndef _UTIL_H
#define _UTIL_H
#include <iostream>
#include <ostream>

using namespace std;

#define FREE(p) do{if(p){ free(p);p=NULL;}}while(0)

struct Associator {
    float dis;
    int   idx;
    Associator(){
        dis=-1.0;
        idx=-1;
    }
    Associator(float _dis,int _idx) {
        dis=_dis;
        idx=_idx;
    }

    inline bool operator < (Associator &a)const {
        return dis<a.dis;
    }
    inline bool operator > ( Associator &a)const {
        return dis>a.dis;
    }
    friend ostream& operator << (ostream& os,const Associator&a){
        os<<a.dis<<" "<<a.idx<<endl;
        return os;
    }

};

template<class T>
class MaxHeap {
    private:
        T *heap;
        int m_cursize;
        int m_maxsize;
    public:
        MaxHeap() {
            m_cursize=0;
            m_maxsize=0;
            heap = NULL;
        }
        void init(int maxsize) {
            m_cursize=0;
            setMaxsize(maxsize);
        }

        ~MaxHeap(){
            delete[] heap;
        }
        int size() const {
            return m_cursize;
        }
        void setMaxsize(int maxsize) {
            m_maxsize=maxsize;
            if(heap)
                delete[] heap;
            heap = new T[m_maxsize+1];
        }

        void insert(const T x) {

            //find the position
            int i=1; /*right position index */
            if (m_cursize==m_maxsize) {
                if (x>heap[1]) return ;
                //delete heap[1]
                i=1;
                int ci = 2;
                while (ci<=m_cursize) {
                    if (ci<m_cursize && heap[ci]<heap[ci+1])
                        ci++;
                    if (x>heap[ci])
                        break;
                    heap[i]=heap[ci];
                    i=ci;
                    ci*=2;
                }
            }
            else {
                i = ++m_cursize;
                while (i!=1&&x>heap[i/2]){
                    heap[i] = heap[i/2];
                    i/=2;
                }
            }

            heap[i] = x;

            return ;
        }

        T *getData()const {
            return heap;
        }
        friend std::ostream& operator << (ostream& os,const MaxHeap<T>& mh){
            os<<"mh size:"<<mh.size()<<endl;
            for(int i = 1;i<=mh.size();i++) {
                //os<<mh.heap[i]<<endl;
                os<<(mh.getData())[i]<<endl;
            }
            return os;
        }
};


#ifdef _TEST
int main() {
    MaxHeap< Associator >* sift_point=new MaxHeap<Associator >[2](23);
    //MaxHeap< int > sift_point(100);

    for (int i =1000;i> 0;i--) {
        sift_point[0].insert(Associator(i*1.0,i));
        //sift_point.insert(i);
    }
    cout<<sift_point[0]<<endl;
    return 0;
}

#endif /*_TEST*/
#endif /* _UTIL_H*/

