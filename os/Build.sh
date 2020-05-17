#!/bin/ksh


while (( i < (nb_domains + 1) ))
do
        printf "Linking domain %02d ... " $i
        dir=$(printf "domains/domain%02d" $i)
        file="${dir}/domain"
        [[ -d $dir ]] || { die "${dir} does not exist"
                        }
        [[ -r ${file}.bin ]] || { die "${file} does not exist"
                        }

        ${CROSS_COMPILE}objcopy  \
                --input-target binary \
                --output-target elf32-littlearm  \
                --binary-architecture arm \
                --rename-section .data=.dm${i} \
                ${file}.bin ${file}.ho

        DOMAINS="${DOMAINS} ${file}.ho"
        : $((i++))
        print "done"
done

