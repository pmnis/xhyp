#!/bin/ksh



D_TITLE="Domains_configuration"
D_BTITLE="Xhyp_Configuration"
LXDIALOG="dialog --title ${D_TITLE} --backtitle ${D_BTITLE} --clear"
export DIALOGOPTS="--title ${D_TITLE} --backtitle ${D_BTITLE}"
XY="18 60"
#set -x

NB_Domains=0
VAL="/tmp/retval.$$"
max_access=16
NB_access=0
sched_type="Unknown"
current=0


HELP="'Use the arrow keys to navigate this window or press the hotkey of
the item you wish to select followed by the <SPACE BAR>.'"


function save_config
{
	TEXT="Do you want to save your configuration ?  "
	dialog  --yesno "${TEXT}" 5 60
}

function save_and_exit
{
	save_config
	tput clear
	print retval: $retval
	exit 0
}

trap "save_and_exit" INT

function get_dom
{
	trap "save_config; exit 1" INT
	TEXT="
You actually configured ${NB_Domains} domains.
Do you want to configure a new one ?
"
	dialog  --yesno "${TEXT}" 6 60
	return 0
}


#while get_dom
#do
#	
	#:$((NB_Domains++))
#done

function menu_list
{
	i=0
	print "Actual configuration" > /tmp/file.$$
	printf "Scheduler ..... : %s\n" ${sched_type} >> /tmp/file.$$
	printf "Domains defined : %d\n\n" ${NB_Domains} >> /tmp/file.$$
	printf "%2s %12s %6s %5s %3s\n" \
		"id" "Name" "type" "size" "MMU" \
		>> /tmp/file.$$
	while (( i++ < NB_Domains))
	do
	printf "%2d %12s %6s %3dMb %3s\n" \
		$i \
		${dommain[${i}].name} \
		${dommain[${i}].type} \
		${dommain[${i}].size} \
		${dommain[${i}].mp} \
		>> /tmp/file.$$
	done
	dialog  --textbox /tmp/file.$$ 18 60
}

function menu_dom_size
{
	typeset -i size

	HELP="
domain number.: ${current}
Name..........: ${dommain[${current}].name}
Type..........: ${dommain[${current}].type}
Memory Protect: ${dommain[${current}].mp}
Memory size...: ${dommain[${current}].size} Mb
"
	dialog  --form "${HELP}" 18 60 5 \
	"Size in Megabytes" 0 0 "" 0 20 3 3 \
	2>${VAL}

	(( $? == 0)) || return 0
	read  size < ${VAL}
	(( size < 1 || size > 32)) && size=0
	dommain[${current}].size=${size}
}

function menu_dom_mmu
{
	HELP="
domain number.: ${current}
Name..........: ${dommain[${current}].name}
Type..........: ${dommain[${current}].type}
Memory Protect: ${dommain[${current}].mp}
Memory size...: ${dommain[${current}].size} Mb
"
	dialog  --radiolist "${HELP}" 18 60 5 \
	"NONE" "Domain does not use MPU" OFF \
	"MPU" "Domain uses static MPU" OFF \
	"MMU" "Domain uses dynamic MMU" OFF \
	2>${VAL}

	(( $? == 0)) || return 0
	read  name < ${VAL}
	dommain[${current}].mp=${name}
}

function menu_dom_type
{
	HELP="
domain number.: ${current}
Name..........: ${dommain[${current}].name}
Type..........: ${dommain[${current}].type}
Memory Protect: ${dommain[${current}].mp}
Memory size...: ${dommain[${current}].size} Mb
"
	dialog  --radiolist "${HELP}" 18 60 5 \
	"XhLib" "Standalone using Xhyp library" ON \
	"RTOS" "Real Time OS like FreeRTOS" OFF \
	"GPOS" "Global Purpose OS like Linux" OFF \
	2>${VAL}

	(( $? == 0)) || return 0
	read  name < ${VAL}
	dommain[${current}].type=${name}
	[[ ${name} == "GPOS" ]] && dommain[${current}].mp="MMU"
	[[ ${name} == "GPOS" ]] && dommain[${current}].size=32
}

function menu_dom_name
{
	HELP="
domain number.: ${current}
Name..........: ${dommain[${current}].name}
Type..........: ${dommain[${current}].type}
Memory Protect: ${dommain[${current}].mp}
Memory size...: ${dommain[${current}].size} Mb
"
	dialog  --inputbox "${HELP}" 18 60 2>${VAL}

	(( $? == 0)) || return 0
	read  name < ${VAL}
	dommain[${current}].name=${name}

}

function menu_add
{
	current=${NB_Domains}
	dommain[${current}].type="RTOS"
	dommain[${current}].name="domain_${NB_Domains}"
	dommain[${current}].mp="NONE"
	dommain[${current}].size="16"
    while true
    do
	HELP="
domain number.: ${current}
Name..........: ${dommain[${current}].name}
Type..........: ${dommain[${current}].type}
Memory protect: ${dommain[${current}].mp}
Memory size...: ${dommain[${current}].size} Mb
"

	dialog  --menu "${HELP}" 18 60 5 \
		Name	"Define name" \
		Type	"Define type" \
		MP	"Define memory protection" \
		Size	"Define memory" \
		Exit	"Abandon" \
		Save	"Save the domain domain"  2>${VAL}

	read retval < ${VAL}
	case ${retval} in
	(Name) menu_dom_name
		;;
	(Type) menu_dom_type
		;;
	(MP) menu_dom_mmu
		;;
	(Size) menu_dom_size
		;;
	(Save)
		;;
	(Exit) return 1
		;;
	esac
   done
	[[ $? == 0 ]] && $(( NB_Domains++ ))
	return 0
}

function menu_choose
{
	HELP="
The scheduler you choose must be compatible with
the X-Hyp binary you intend to link with.
Scheduler: ${sched_type}
"

	dialog  --radiolist "${HELP}" 18 60 5 \
		Posix	"Define type" ON\
		Arinc	"Define type" OFF\
		EDF	"Save the domain domain" OFF 2>${VAL}
	(( $? == 1)) || read sched_type < ${VAL}
}
function menu_1
{
	HELP="
You have defined ${NB_Domains} domains
and used ${NB_access}/${max_access} subdomains.
Scheduler type: ${sched_type}
"

	dialog  --menu "${HELP}" 18 60 6 \
		Choose	"Choose scheduler" \
		Add	"Add a new domain" \
		List	"List existing domains" \
		Mod 	"Modify an existing domain" \
		Del 	"Delete an existing domain" \
		Exit	"Exit the menu" 2>${VAL}

	read retval < ${VAL}
	case ${retval} in
	(Choose)
		menu_choose
		;;
	(Add)
		menu_add
		;;
	(List)
		menu_list
		;;
	(Mod)
		menu_mod
		;;
	(Del)
		menu_del
		;;
	(*)
		return 1
		;;
	esac
	return 0
}

while true
do
	menu_1
	(( $? == 1 )) && save_and_exit
done

save_and_exit

