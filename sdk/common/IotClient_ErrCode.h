#ifndef _IOTCLIENT_ERRCODE_H_
#define _IOTCLIENT_ERRCODE_H_

#include <stdio.h>

typedef enum _iot_status_code
{
	ERR_OK						= 1,
	ERR_CANT_MAKE_MESSAGE		= 2,
	ERR_TIMEOUT_RESPONSE		= 3,
	ERR_PROTOCOL_STILL_EXIST	= 4,
	ERR_CREATE_MQTT_FAIL		= 5,
	ERR_UNSUPPORTED_PROTOCOL	= 6,
	ERR_INVALID_PARAMETER		= 7,
	ERR_PROTOCOL_NOT_CREATED	= 8,
	ERR_PROTOCOL_TYPE_NOT_SET	= 9,
	ERR_MQTT_CONNECT_FAIL		= 10,
	ERR_MQTT_SUBSCRIBE_FAIL		= 11,
	ERR_MQTT_DISCONNECT_FAIL	= 12,
	ERR_WRONG_AUTHTYPE			= 13,

	ERR_NOT_CONNECT				= 14,
	ERR_NO_RESPONSE				= 15,

	/** 200 - OK */
    SUCCESS_OK 					= 200,
    /** 201 - Created */
    SUCCESS_CREATED 			= 201,
    /** 202 - Accepted */
    SUCCESS_ACCEPTED 			= 202,
    /** 203 - Non-Authoritative Information */
    SUCCESS_NON_AUTH_INFO 		= 203,
    /** 204 - No Content */
    SUCCESS_NO_CONTENT 			= 204,
    /** 302 - Found */
    REDIRECTION_FOUND 			= 302,

    /** 400 - Bad Request */
    CLIENT_ERROR_BAD_REQUEST 			= 400,
    /** 401 - Unauthorized */
    CLIENT_ERROR_UNAUTHORIZED 			= 401,
    /** 402 - Payment Required */
    CLIENT_ERROR_PAYMENT_REQUIRED 		= 402,
    /** 403 - Forbidden */
    CLIENT_ERROR_FORBIDDEN 				= 403,
    /** 404 - Not Found */
    CLIENT_ERROR_NOT_FOUND 				= 404,
    /** 405 - Method Not Allowed */
    CLIENT_ERROR_METHOD_NOT_ALLOWED 	= 405,
    /** 406 - Not Acceptable */
    CLIENT_ERROR_NOT_ACCEPTABLE 		= 406,
    /** 407 - Proxy Authentication Required */
    CLIENT_ERROR_PROXY_AUTHENTICATION 	= 407,
    /** 408 - Request Timeout */
    CLIENT_ERROR_REQUEST_TIMEOUT 		= 408,
    /** 409 - Conflict */
    CLIENT_ERROR_CONFLICT 				= 409,
    /** 412 - Precondition Failed */
    CLIENT_ERROR_PRECONDITION_FAILED 	= 412,
    /** 415 - Unsupported Media Type */
    CLIENT_ERROR_UNSUPPORTED_MEDIA_TYPE = 415,

    /** 500 - Internal Server Error */
    SERVER_ERROR_INTERNAL 						= 500,
    /** 501 - Not Implemented */
    SERVER_ERROR_NOT_IMPLEMENTED 				= 501,
    /** 502 - Bad Gateway */
    SERVER_ERROR_BAD_GATEWAY 					= 502,
    /** 503 - Service Unavailable */
    SERVER_ERROR_SERVICE_UNAVAILABLE 			= 503,
    /** 504 - Gateway Timeout */
    SERVER_ERROR_GATEWAY_TIMEOUT 				= 504,
    /** 505 - HTTP Version Not Supported */
    SERVER_ERROR_HTTP_VERSION_NOT_SUPPORTED 	= 505,

    /** 999 - Unclassified Status - 서버에서 분류할 수 없는 오류가 발생했을 때 */
    UNCLASSIFIED_FAIL = 999

} iot_status_code;

#endif // _IOTCLIENT_ERRCODE_H_