#/bin/bash

if [ $# -ne 2 ]
then
    echo test.sh usage:
    echo 
    echo ./test.sh testfile testflag
    echo 
else
    testfile=$1
    testflag=$2
    echo test the file $testfile
    for ((i=0;i<10;i++))
    do
        echo *****************************
        echo        do test ${i}
        echo *****************************
        echo @updatescript/${testfile} | secondo
        mv /home/hhr/secondo/bin/testdata1 /home/hhr/secondo/bin/${testflag}${i}testdata1
        mv /home/hhr/secondo/bin/testdata2 /home/hhr/secondo/bin/${testflag}${i}testdata2
        mv /home/hhr/secondo/bin/testdata3 /home/hhr/secondo/bin/${testflag}${i}testdata3
    done
fi
