#/bin/bash

if [ $# -ne 1 ]
then
    echo args error!
else
    testfile=$1
    echo test the file $testfile
    for ((i=0;i<10;i++))
    do
        echo *****************************
        echo        do test ${i}
        echo *****************************
        echo @updatescript/${testfile} | secondo
        mv /home/hhr/secondo/bin/testdata1 /home/hhr/secondo/bin/testdata1${i}
        mv /home/hhr/secondo/bin/testdata2 /home/hhr/secondo/bin/testdata2${i}
        mv /home/hhr/secondo/bin/testdata3 /home/hhr/secondo/bin/testdata3${i}
    done
fi
