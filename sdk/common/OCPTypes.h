/**
 * 
 * 
 * @author mw50.jeong
 * @since 2018. 2. 1.
 */

#ifndef _OCPTYPES_H_
#define _OCPTYPES_H_

// properties length
#define OCPMANAGER_PROPERTY_LEN   1024
// sdk version
#define OCPSDK_VERSION        "2.9.0"
// return success value
#define OCPSDK_SUCCESS        1
// return failure value
#define OCPSDK_FAILURE        0
// ssloption isSsl false
#define OCPSSLOPTS_TCP			'0'
// ssloption isSsl true
#define OCPSSLOPTS_ONEWAY		'1'
// ssloption isTwowaySsl true
#define OCPSSLOPTS_TWOWAY		'2'
// authType 0
#define OCPAUTHTYPE_NO				"0"
// authType 1
#define OCPAUTHTYPE_DH				"1"
// authType 2
#define OCPAUTHTYPE_SEAL			"2"
// authType 3
#define OCPAUTHTYPE_OAUTH			"3"
// authType 4
#define OCPAUTHTYPE_WBC				"4"
// authType 5
#define OCPAUTHTYPE_ITA				"5"
// authType 6
#define OCPAUTHTYPE_SSL				"6"
// msgHeaderType D
#define OCPMESSAGECONVERTER_DELIMETER 	"D"
// msgHeaderType J
#define OCPMESSAGECONVERTER_JSON  		"J"
// dataformat JSON
#define OCPDATAFORMAT_JSON			"application/json"
// dataformat DELIMETER
#define OCPDATAFORMAT_DELIMETER		"application/x-delimiter"
// dataformat XML
#define OCPDATAFORMAT_XML			"application/xml"
// dataformat FIXED
#define OCPDATAFORMAT_FIXED 		"application/x-fixed"
// dataformat GPB
#define OCPDATAFORMAT_GPB			"application/x-protobuf"
// module mqtt
#define OCPPROTOCOL_MQTT 	"mqtt"
// module oag 
#define OCPPROTOCOL_CURL 	"curl"
// encType 0
#define OCPENCTYPE_PLAIN			"0"
// encType 1
#define OCPENCTYPE_DH				"1"
// encType 2
#define OCPENCTYPE_SEAL				"2"
// encType 3
#define OCPENCTYPE_AES128			"3"
// encType 4
#define OCPENCTYPE_WBC				"4"
// encType 5
#define OCPENCTYPE_AES256			"5"

#endif