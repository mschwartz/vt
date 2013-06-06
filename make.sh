#!/bin/sh

TOPDIR=$PWD
set -e
trap 'cd $PWD' INT TERM EXIT

PREFIX=">>>>>>>>>>>>>>>>>>"

# which version to build: debug or release
# BUILD=debug
BUILD=release

# determine OS and other useful information
if [ "$OSTYPE" = "darwin12" ]; then
	OS="OSX"
	CPUS=`sysctl -n hw.ncpu`
else
	OS="LINUX"
	CPUS=`grep vendor_id /proc/cpuinfo | wc -l`
fi

####### Logic to build v8

V8PATH=v8-read-only
if [ "$BUILD" = "debug" ]; then
	V8TARGET=x64.debug
else
	V8TARGET=x64.release
fi

function build_v8() {
	if [ -d $V8PATH ]; then
		echo "$PREFIX update v8"
		cd $V8PATH && svn update && cd ..
	else
		echo "$PREFIX checkout v8"
		svn checkout http://v8.googlecode.com/svn/trunk $V8PATH
	fi

	echo "$PREFIX build v8"
	cd $V8PATH
	make dependencies >/dev/null 2>&1
	GYP_GENERATORS=make make $V8TARGET library=shared -j $CPUS >/dev/null
	cd ..
}

function clean_v8() {
	if [ -d $V8PATH ]; then
		echo "$PREFIX clean v8"
		cd $V8PATH && make clean >/dev/null 2>&1 && cd ..
	fi
}

####### Logic to build vt

### Set up compiler and linker options

if [ "$OS" = "OSX" ]; then
	LDLIBS="-lv8_base.x64 -lv8_snapshot"
	# LDLIBS="-lv8"
	LDRPATH="-Wl,-rpath,/usr/local/vt/src/v8,-rpath,$TOPDIR/src/$V8PATH/out/$V8TARGET"
else
	LDLIBS="-lv8"
	LDRPATH="-Wl,-rpath=/usr/local/vt/src/v8,-rpath=$TOPDIR/src/$V8PATH/out/$V8TARGET"
fi

CC="g++"
CCFLAGS="-D_REENTRANT -fno-rtti -fno-exceptions"
INCLUDE="-I$V8PATH/include -I$V8PATH/src"

LD="g++"
LDFLAGS="-D_REENTRANT -rdynamic -L$V8PATH/out/$V8TARGET"
LDLIBS="$LDLIBS -lpthread"
if [ "$BUILD" = "debug" ]; then
	CCFLAGS="-g $CCFLAGS"
	LDFLAGS="-g $LDFLAGS"
fi

### Set up files to be compiled

SOURCES="main console process v8 mem pthread fs net async"

function compile() {
	if [ "$1.cpp" -nt "$1.o" ] || [ "vt.h" -nt "$1.o" ]; then
		echo ">>> CXX $1.cpp"
		$CC $CCFLAGS $INCLUDE -c $1.cpp
	fi
}

function link() {
	OBJ=""
	for FILE in $SOURCES
	do
		OBJ="$OBJ $FILE.o"
	done
	echo ">>> LD vt"
	$LD -o vt $LDFLAGS $OBJ $LDLIBS $LDRPATH
}

function build_vt() {
	if [ ! -d $V8PATH ]; then
		build_v8
	fi
	echo "$PREFIX build vt"
	for FILE in $SOURCES
	do
		compile $FILE
	done
	link
	# $CC $CCFLAGS $INCLUDE -c main.cpp
	# $LD -o vt $LDFLAGS main.o $LDLIBS $LDRPATH
}

function clean_vt() {
	echo "$PREFIX clean vt"
	rm -f *.o vt
}

#########

function build_all() {
	echo "$PREFIX build"
	build_v8
	build_vt
}

function clean() {
	clean_v8
	clean_vt
}

cd src
case $1 in
	clean) clean ;;
	clean_vt) clean_vt ;;
	build_v8) build_v8 ;;
	all) build_all ;;
	*) build_vt ;;
esac

