#!/bin/bash
echo $$ > pid.txt
exec $*
