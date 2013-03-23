SET(LIBYAEL_SRC vector.c nn.c kmeans.c sorting.c matrix.c machinedeps.c binheap.c eigs.c)
ADD_LIBRARY(libyael SHARED ${LIBYAEL_SRC})
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
