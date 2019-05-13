#!/bin/bash

# validar execucoes
main_p=$1

for p1 in "./output/"$main_p"/"*/
do
    for p2 in $p1*/
    do
        for p3 in $p2*/
        do
            echo $p3
            cat $p3"etapa_time.txt"
        done
    done
done