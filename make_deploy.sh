#!/bin/bash

#==================================================================
#0) 준비
#==================================================================
echo "Making deploy directory is started."
PWD_DIR=`pwd`
echo ">> current directory: ${PWD_DIR}"

#==================================================================
#1) 폴더 생성 
#==================================================================
# deploy (있으면 안 만들어요)
DEPLOY_DIR=${PWD_DIR}/deploy
if [ ! -d $DEPLOY_DIR ]; then
	echo ">> making $DEPLOY_DIR"
	mkdir $DEPLOY_DIR
fi
#==================================================================
# deploy/include (있으면 안 만들어요)
INC_DIR=$DEPLOY_DIR/include
if [ ! -d $INC_DIR ]; then
	echo ">> making $INC_DIR"
	mkdir $INC_DIR
fi
# deploy/include/iotclient (있으면 폴더를 지우고 새로 만들어요)
INC_BIOT_DIR=$INC_DIR/iotclient
if [ -d $INC_BIOT_DIR ]; then
	echo ">> deleting $INC_BIOT_DIR"
	rm -fr $INC_BIOT_DIR
fi
echo ">> making $INC_BIOT_DIR"
mkdir $INC_BIOT_DIR
# deploy/include/ 오픈소스 디렉터리들 (있으면 안 만들어요)
folder_names=("curl" "json" "openssl" "mqtt" "zstd")
for i in ${folder_names[@]}; do
	INC_SUB_DIR=$INC_DIR/$i
	if [ ! -d $INC_SUB_DIR ]; then
		echo ">> making $INC_SUB_DIR"
		mkdir $INC_SUB_DIR
	fi
done
#==================================================================
# $LIB_DIR 폴더 (있으면 안 만들어요)
LIB_DIR=$DEPLOY_DIR/lib
if [ ! -d $LIB_DIR ]; then
        echo ">> making $LIB_DIR"
        mkdir $LIB_DIR
fi
#==================================================================
# $LIB_DIR/json/pkgconfig 폴더
PKGCONFIG_DIR=$LIB_DIR/pkgconfig
if [ ! -d $PKGCONFIG_DIR ]; then
	echo ">> making $PKGCONFIG_DIR"
	mkdir $PKGCONFIG_DIR
fi

#==================================================================
# deploy/src 와 obj (있으면 안 만들어요)
folder_names=("src" "obj")
for i in ${folder_names[@]}; do
	DIR_PATH=$DEPLOY_DIR/$i
	if [ ! -d $DIR_PATH ]; then
		echo ">> making $DIR_PATH"
		mkdir $DIR_PATH
	fi
done

#==================================================================
#2) SDK 헤더와 라이브러리 가져오기
#==================================================================
file_names=("IotClient.h" "OCPManager.h" "OCPMessage.h" "OCPTypes.h" "IotClient_ErrCode.h")
FROM_DIR=$PWD_DIR/sdk_out/include/ocp
if [ -d $FROM_DIR ]; then
	for i in ${file_names[@]}; do
		SOURCE_FILE=$FROM_DIR/$i
		if [ -e $SOURCE_DIR ]; then
			echo ">> copying $SOURCE_FILE to $INC_BIOT_DIR"
			cp -f $SOURCE_FILE $INC_BIOT_DIR
		fi
	done
fi
FROM_LIB_FILE=$PWD_DIR/sdk_out/lib/ocp/libiotclient.a
if [ -e $FROM_LIB_FILE ]; then
	echo ">> copying $FROM_LIB_FILE to $LIB_DIR"
	cp -f $FROM_LIB_FILE $LIB_DIR
fi

#==================================================================
#3) 오픈소스 헤더와 라이브러리 가져오기
#==================================================================
FROM_INC_DIR=/usr/local/include
FROM_LIB_DIR=/usr/local/lib
#==================================================================
#3-1 openssl
FROM_DIR=$FROM_INC_DIR/openssl
TO_DIR=$INC_DIR/openssl
if [ -d $FROM_DIR ] && [ -d $TO_DIR ]; then
	echo ">> copying files $FROM_DIR to $TO_DIR"
	cp -f $FROM_DIR/* $TO_DIR
fi

TO_DIR=$LIB_DIR/
if [ -d $TO_DIR ]; then
	file_names=("libssl.so" "libcrypto.so")
	for i in ${file_names[@]}; do
		SOURCE_FILE=$FROM_LIB_DIR/$i
		echo ">> copying $SOURCE_FILE to $TO_DIR"
		cp -f $FROM_LIB_DIR/$i $TO_DIR
	done
fi
#==================================================================
#3-2 curl
FROM_DIR=$FROM_INC_DIR/curl
TO_DIR=$INC_DIR/curl
if [ -d $FROM_DIR ] && [ -d $TO_DIR ]; then
	echo ">> copying files $FROM_DIR to $TO_DIR"
	cp -f $FROM_DIR/* $TO_DIR
fi

TO_DIR=$LIB_DIR/
if [ -d $TO_DIR ]; then
	SOURCE_FILE=$FROM_LIB_DIR/libcurl.so
	echo ">> copying $SOURCE_FILE to $TO_DIR"
	cp -f $SOURCE_FILE $TO_DIR
fi
#==================================================================
#3-3 zstd
TO_DIR=$INC_DIR/zstd
if [ -d $TO_DIR ]; then
	file_names=("zbuff.h" "zdict.h" "zstd.h" "zstd_errors.h")
	for i in ${file_names[@]}; do
		SOURCE_FILE=$FROM_INC_DIR/$i
		if [ -e $SOURCE_FILE ]; then
			echo ">> copying $SOURCE_FILE to $TO_DIR"
			cp -f $SOURCE_FILE $TO_DIR
		fi
	done
fi
TO_DIR=$LIB_DIR/
if [ -d $TO_DIR ]; then
	SOURCE_FILE=$FROM_LIB_DIR/libzstd.so
	if [ -e $SOURCE_FILE ]; then
		echo ">> copying $SOURCE_FILE to $TO_DIR"
		cp -f $FROM_LIB_DIR/libzstd.so $TO_DIR
	fi
fi
#==================================================================
#3-4 paho.mqtt.c
TO_DIR=$INC_DIR/mqtt
if [ -d $TO_DIR ]; then
	file_names=("MQTTAsync.h" "MQTTClient.h" "MQTTClientPersistence.h" "MQTTProperties.h" "MQTTReasonCodes.h" "MQTTSubscribeOpts.h")
	for i in ${file_names[@]}; do
		SOURCE_FILE=$FROM_INC_DIR/$i
		if [ -e $SOURCE_FILE ]; then
			echo ">> copying $SOURCE_FILE to $TO_DIR"
			cp -f $SOURCE_FILE $TO_DIR
		else
			echo ">>>>>>>>> no file $SOURCE_FILE"
		fi
	done
fi
TO_DIR=$LIB_DIR/
if [ -d $TO_DIR ]; then
	# only as file. asynchronous SSL
	SRC_NAME=libpaho-mqtt3as.so.1
	SOURCE_FILE=$FROM_LIB_DIR/$SRC_NAME
	if [ -e $SOURCE_FILE ]; then
		echo ">> copying $SOURCE_FILE to $TO_DIR"
		cp -f $SOURCE_FILE $TO_DIR
	fi
	TO_NAME=libpaho-mqtt3as.so
	SRC_PATH=$TO_DIR/$SRC_NAME
	TO_PATH=$TO_DIR/$TO_NAME
	if [ ! -e $TO_PATH ]; then
		echo ">> make a symbolic link of $TO_NAME"
		ln -sr $SRC_PATH $TO_PATH
	fi
fi
#==================================================================
#3-5 jansson
TO_DIR=$INC_DIR/json
if [ -d $TO_DIR ]; then
	file_names=("jansson.h" "jansson_config.h")
	for i in ${file_names[@]}; do
		SOURCE_FILE=$FROM_INC_DIR/$i
		if [ -e $SOURCE_FILE ]; then
			echo ">> copying $SOURCE_FILE to $TO_DIR"
			cp -f $SOURCE_FILE $TO_DIR
		else
			echo ">>>>>>>>> no file $SOURCE_FILE"
		fi
	done
fi
TO_DIR=$LIB_DIR/
if [ -d $TO_DIR ]; then
	SRC_NAME=libjansson.so.4
	SOURCE_FILE=$FROM_LIB_DIR/$SRC_NAME
	if [ -e $SOURCE_FILE ]; then
		echo ">> copying $SOURCE_FILE to $TO_DIR"
		cp -f $SOURCE_FILE $TO_DIR
	fi
	TO_NAME=libjansson.so
	SRC_PATH=$TO_DIR/$SRC_NAME
	TO_PATH=$TO_DIR/$TO_NAME
	if [ ! -e $TO_PATH ]; then
		echo ">> make a symbolic link of $TO_NAME"
		ln -sr $SRC_PATH $TO_PATH
	fi
	DIR_1=$FROM_LIB_DIR/pkgconfig/jansson.pc
	if [ -d $TO_DIR/pkgconfig ] && [ -e $DIR_1 ]; then
		DIR_2=$TO_DIR/pkgconfig
		echo ">> copying $DIR_1 to $DIR_2"
		cp -f $DIR_1 $DIR_2
	fi
fi
#==================================================================
#4) 기타 필요한 파일들 가져오기
#==================================================================
PROPERTY_FILE=biot_client.properties
PROPERTY_FROM=$PWD_DIR/sample/$PROPERTY_FILE
PROPERTY_TO=$DEPLOY_DIR/$PROPERTY_FILE
if [ -e $PROPERTY_FROM ] && [ ! -e $PROPERTY_TO ]; then
	echo ">> copying Property file, $PROPERTY_FILE to $DEPLOY_DIR"
	cp $PROPERTY_FROM $PROPERTY_TO
fi

PTHREAD_FILE=libpthread.so
TO_DIR=$LIB_DIR/
find /usr/lib -name $PTHREAD_FILE -exec cp "{}" $TO_DIR \;
#==================================================================

