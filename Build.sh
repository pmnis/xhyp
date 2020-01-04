#!/bin/ksh

BASE=$(pwd)
export INCLUDE=${BASE}/include



# try find the board
[[ -r .config ]] && source ./.config


[[ ${CONFIG_ARM} =~ y ]] && ARCH=arm
[[ ${CONFIG_BOARD_VERSATILE} =~ y ]] && BOARD=versatile
[[ ${CONFIG_BOARD_REALVIEW} =~ y ]] && BOARD=realview

# set defaults
export ARCH=${ARCH:-arm}
export BOARD=${BOARD:-versatile}
export CROSS_COMPILE=${CROSS_COMPILE:-arm-linux-}
export CC=${CROSS_COMPILE}gcc
export LD=${CROSS_COMPILE}ld
export AS=${CROSS_COMPILE}as
export OBJCOPY=${CROSS_COMPILE}objcopy


[[ ${ARCH} ]] && [[ ! ${CROSS_COMPILE} ]] && {
	print "Unknown compiler for architecture $ARCH"
	print "Please export CROSS_COMPILE"
	exit 1
	}

[[ $(whence ${CROSS_COMPILE}gcc) ]] || {
	print "I did not find ${CROSS_COMPILE}gcc in $PATH"
	print "Please setup your PATH"
	exit 1
}

print "Using compiler ${CROSS_COMPILE}gcc for architecture $ARCH"
export DEBFLGS="-g"

# export tool chain you find in toolchain.inc
source arch/${ARCH}/${BOARD}/toolchain.inc


# export directories
DIRS="kernel lib arch/${ARCH}/${BOARD} arch/${ARCH}/common/ arch/${ARCH}/lib domains scripts drivers tests"

mkdir -p objs

function die
{
	print $*
	exit 1
}

function clean_all
{
	for dir in ${DIRS}
	do
		print cleaning ${dir}
		( cd ${dir} ; ./Build.sh clean )
		print "done"
	done
	print "cleaning objects"
	rm -rf generated
	rm -f os/*.dom
	rm -f xhyp xhyp.bin xhyp.ld xhyp.nm
	rm -f files.in
	rm -f objs/*
	rm -f cscope.out cscope.files
	print "done"
}

function mrproper
{
	clean_all
	rm -f .config
	rm -rf doc
	rm -f include/autoconf.h
}

function save_all
{
	set -x
	d=${PWD##/*/}
	file=${d}_$(date +%Y%m%d_%H%M)_$$.tgz
	print "saving directory $d to ../${file}"
	#(cd ..; print excluding files in ${d}/TAR.EXCLUDE)
	#(cd .. ; d=$(readlink $d); tar zcvf ${file} -X ${d}/TAR.EXCLUDE ${d} )
	#[[ -l ../${d} ]] && d=$(readlink $d)
	(cd .. ; tar zcvf ${file} ${d} )
	exit 0
}

function save_all_all
{
	cp /root/OSEO/linux/patches/xhyp.patch patches/
	cp /data/linux_xhyp.tgz ports/
	cp /data/freertos_xhyp.tgz ports/
	d=${PWD##/*/}
	file=${d}_$(date +%Y%m%d_%H%M)_$$.tgz
	print "saving directory $d to ../${file}"
	(cd .. ; d=$(readlink $d); tar zcvf ${file} $d )
	exit 0
}

function build_scripts
{
	(
	cd scripts
	./Build.sh $*
	)
}

function build_config
{
	build_scripts
	./scripts/kconfig/mconf Kconfig
}

function help
{
	print "
usage:
	./Build [option]

Default: Build xhyp

where option is one of
	cscope	:	Build cscope files
	scripts	:	Build configuration scripts
	menu	:	configure using menu
	menuconfig:	configure using menu
	clean	:	cleanup source tree
	mrproper:	cleanup source tree and cscope files
	save	:	save tree in ../xhyp_YYYYMMMDDD.tgz
";
}

function build_cscope
{
	find  . -type f -name "*.c"  > cscope.files 
	find  . -type f -name "*.S"  >> cscope.files 
	find  . -type f -name "*.h"  >> cscope.files
	cscope -I ./include -k -b -i cscope.files
}

function build_doc
{
	mkdir -p doc
	doxygen && ( cd doc/latex &&  make )
	exit $?
}

[[ $1 == "help" ]] && help && exit 0
[[ $1 == "cscope" ]] && build_cscope && exit 0
[[ $1 == "scripts" ]] && build_scripts && exit 0
[[ $1 == "menuconfig" ]] && build_config && exit 0
[[ $1 == "menu" ]] && build_config && exit 0
[[ $1 == "config" ]] && build_config && exit 0
[[ $1 == "mrproper" ]] && mrproper && exit 0
[[ $1 == "clean" ]] && clean_all && exit 0
[[ $1 == "save" ]] && save_all && exit 0
[[ $1 == "doc" ]] && build_doc && exit 0

[[ -r .config ]] || {
	build_config
}

[[ -r .config ]] || {
	print "Please configure using ./Build.sh config"
	exit 1
}

#clean_all

source ./.config

i=1
nb_domains=0
while read line
do
	line=${line%#*}
	[[ $line =~ CONFIG_DOMAIN.= ]] && {
		: $(( nb_domains++))
	}
	[[ $line ]] && export $line
done < .config

[[ -x ./os/Build.sh ]] && {
	./os/Build.sh || exit 1
	}

./scripts/Build_ld || exit 1


DIRS="lib kernel"
LIBS="lib/xhyp.a lib/mlib.a lib/lib.a"
GENERATED="generated/domain_table.o"
[[ ${CONFIG_SCHED_ARINC} =~ y ]] && {
	GENERATED="${GENERATED} generated/arinc_table.o"
	./scripts/Build_arinc > generated/arinc_table.c
}

print "Build board dependencies $*"
( cd arch/${ARCH}/${BOARD} && ./Build.sh $* ) || exit 1
print "done"

print "Build arch dependencies $*"
( cd arch/${ARCH}/ && ./Build.sh $* ) || exit 1
print "done"

print "Build standard libraries $*"
( cd lib && ./Build.sh $* ) || exit 1
print "done"

print "Build kernel $*"
( cd kernel && ./Build.sh $* ) || exit 1
print "done"

print "Build drivers $*"
( cd drivers && ./Build.sh $* )  || exit 1

print "Build tests $*"
( cd tests && ./Build.sh $* )  || exit 1

( cd domains && ./Build.sh $* ) || exit 1
i=1
read nb_domains reste < <(ls -d domains/domain?? | wc -l)

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


printf "Compute domain tables ... "
${CC} ${CFLAGS} -c generated/domain_table.c -o generated/domain_table.o

[[ ${CONFIG_SCHED_ARINC} =~ y ]] && {
	printf "Compute arinc tables ... "
	${CC} ${CFLAGS} -c generated/arinc_table.c -o generated/arinc_table.o
}

printf "Linking xhyp ... "

export LDFLAGS="-Bstatic -T xhyp.ld "
${LD} ${LDFLAGS} ${LIBS} ${GENERATED} ${DOMAINS} -o xhyp
${CROSS_COMPILE}nm xhyp > xhyp.nm


[[ $1 == "clean" ]] && exit 0

build_cscope

print "done"

exit 0

