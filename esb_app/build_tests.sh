set -x
PROJ_DIR=`pwd`
SRC_DIR=$PROJ_DIR/src
OUT_DIR=$PROJ_DIR/output
OUT_BIN=test_runner
# Create the output directory
mkdir -p $OUT_DIR

cd $OUT_DIR
# Clean the output directory
rm *

gcc -v -I$SRC_DIR/adapter -I$SRC_DIR/test $SRC_DIR/test/*.c $SRC_DIR/esb/*.c -o $OUT_BIN