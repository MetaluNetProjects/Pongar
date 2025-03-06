#!/bin/bash

WAVS=$1
#echo wavs: $WAVS
#exit

for dirnum in {1..99} ; do
    dirname=$(printf %02d $dirnum)
    if [ -d $WAVS/$dirname ] && [ "x$(ls $WAVS/$dirname/*.wav)" != x ]; then
        for wavnum in {0..255} ; do
            wavname=$WAVS/$dirname/$(printf %03d $wavnum).wav
            if [ -f $wavname ] ; then
                duration=$(soxi -s $wavname)
                duration=$(printf %d $((duration / 441)))
                echo $duration
            else echo 0
            fi
        done
    else 
        for wavnum in {0..255} ; do echo 0 ; done
    fi
done

exit

echo -e "/* wavs_duration[1][2] is duration of wavs/01/002.wav in hundredth of second (100->1s) */\n"
echo -e "const uint16_t wavs_duration[][256] = {"
for dirnum in {1..99} ; do
    dirname=$(printf %02d $dirnum)
    if [ -d $WAVS/$dirname ] && [ "x$(ls $WAVS/$dirname/*.wav)" != x ]; then
        echo -en "\t{ "
        for wavnum in {0..255} ; do
            wavname=$WAVS/$dirname/$(printf %03d $wavnum).wav
            if [ -f $wavname ] ; then
                duration=$(soxi -s $wavname)
                duration=$(printf %d $((duration / 441)))
                echo -en $duration", "
            else echo -en "0, "
            fi
        done
        echo -e "},"
    else 
        echo -e "\t{ 0 },"
    fi
done
echo -e "\t0 \n};"

exit

echo "char wavsinfo[][] = {"
for dir in $(ls -d wavs/*/); do
    dirnum=$((`basename $dir` + 0))
    #declare -a MY_ARRAY=( $(for i in {1..255}; do echo 0; done) )
    for wav in $dir/*.wav; do
    wavnum=$((`basename $wav .wav` + 0))
        echo "\t{" $dirnum, $wavnum, $(soxi -D $wav) "},"
    done
done
echo "\t0 \n};"
