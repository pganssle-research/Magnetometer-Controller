:#!/bin/sh
:# This shell script which creates the fill.nc file from fill.cdl.
:# $Id: create_fills.sh,v 1.2 2009/01/25 14:33:45 ed Exp $

echo off
echo %1
echo "*** Testing creating file with fill values."
:set -e
..\%1\ncgen -o fills.nc fills.cdl
copy fills.nc ..\%1\fills.nc
echo "*** SUCCESS!"
:exit 0
