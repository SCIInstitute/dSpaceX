function compile()

mex ReadLinalgMatrix.cpp -I../linalg/ -output ReadLinalgMatrixDouble -DDOUBLE
mex ReadLinalgMatrix.cpp -I../linalg/ -output ReadLinalgMatrixSingle -DSINGLE
mex ReadLinalgMatrix.cpp -I../linalg/ -output ReadLinalgMatrixInt64 -DINT64
mex ReadLinalgMatrix.cpp -I../linalg/ -output ReadLinalgMatrixInt32 -DINT32

mex ReadLinalgVector.cpp -I../linalg/ -output ReadLinalgVectorDouble -DDOUBLE
mex ReadLinalgVector.cpp -I../linalg/ -output ReadLinalgVectorSingle -DSINGLE
mex ReadLinalgVector.cpp -I../linalg/ -output ReadLinalgVectorInt64 -DINT64
mex ReadLinalgVector.cpp -I../linalg/ -output ReadLinalgVectorInt32 -DINT32

mex WriteLinalgMatrix.cpp -I../linalg/ -output WriteLinalgMatrixDouble -DDOUBLE
mex WriteLinalgMatrix.cpp -I../linalg/ -output WriteLinalgMatrixSingle -DSINGLE
mex WriteLinalgMatrix.cpp -I../linalg/ -output WriteLinalgMatrixInt64 -DINT64
mex WriteLinalgMatrix.cpp -I../linalg/ -output WriteLinalgMatrixInt32 -DINT32

mex WriteLinalgVector.cpp -I../linalg/ -output WriteLinalgVectorDouble -DDOUBLE
mex WriteLinalgVector.cpp -I../linalg/ -output WriteLinalgVectorSingle -DSINGLE
mex WriteLinalgVector.cpp -I../linalg/ -output WriteLinalgVectorInt64 -DINT64
mex WriteLinalgVector.cpp -I../linalg/ -output WriteLinalgVectorInt32 -DINT32
