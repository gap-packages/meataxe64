#!/bin/bash
#
# Comparte two logs for correctness
# Outputs differences
#
# $1: log
# $2: check file
#
# Returns 0: success
#         1: failure

Usage="check_log.sh <log file> <check file>"
if [ 2 -ne $# ]; then
    echo "$0: Bad parameters" 1>&2
    echo $Usage 1>&2
    exit 1
fi
log=$1
check=$2
if [ ! -e ${log} ]; then
    echo "$0: log file not found" 1>&2
    exit 1
fi
if [ ! -e ${check} ]; then
    echo "$0: check file not found" 1>&2
    exit 1
fi
i=0
while read line; do
    logfile[$i]=$line
    let i=$i+1
done < ${log}
log_length=$i
tmp=tmp${PPID}
logtmp=${tmp}.1
checktmp=${tmp}.2
i=0
while read line; do
    checkfile[$i]=$line
    let i=$i+1
done < ${check}
check_length=$i
i=0
ret=0
while [ $i -lt ${log_length} ]; do
    # Read a line from one, compare with the other
    log_line=${logfile[$i]}
    check_line=${checkfile[$i]}
    progname=`echo ${log_line} | sed -e "s/[^ ]* //;s/ .*$//"`
    # Fail if different
    if [ "${log_line}" != "${check_line}" ]; then
        echo "$0: different program lines \"${log_line}\" and \"${check_line}\" found" 1>&2
        exit 1
    fi
    if ! echo ${progname} | grep "^z" 1>/dev/null; then
        echo "$0: program name ${progname} invalid" 1>&2
        exit 1
    fi
    # Now set j=i+1
    let j=$i+1
    # Read lines j until we find a different program name
    cat /dev/null > ${logtmp}
    cat /dev/null > ${checktmp}
    while [ $j -lt ${log_length} ]; do
        line=${logfile[$j]}
        tmpprg=`echo ${line} | sed -e "s/[^ ]* //;s/ .*$//"`
        if echo ${tmpprg} | grep "^z" 1>/dev/null; then
            # Found the next program
            break
        fi
        echo ${line} >> ${logtmp}
        echo ${checkfile[$j]} >> ${checktmp}
        let j=$j+1
    done
    # Now compare lines i+1 to j (not inclusive) after sorting
    sort < ${logtmp} > ${logtmp}.s
    sort < ${checktmp} > ${checktmp}.s
    if ! diff ${logtmp}.s ${checktmp}.s; then
        ret=1
    fi
    # Move i onto j
    i=$j
done
rm ${tmp}*
exit $ret
