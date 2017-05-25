#ifndef _HTTP_CHUNK_
#define _HTTP_CHUNK_

#define MAXNUM_SIZE 16

typedef enum {
		CHUNK_FIRST, //不用

		CHUNK_HEX,
		CHUNK_POSTHEX,
		CHUNK_CR,
		CHUNK_DATA,
		CHUNK_POSTCR,
		CHUNK_POSTLF,
		CHUNK_STOPCR,
		CHUNK_STOP,
		CHUNK_TRAILER,
		CHUNK_TRAILER_CR,
		CHUNK_TRAILER_POSTCR,
		
		CHUNK_LAST //不用
} CHUNK_STATE;

typedef enum {
		CHUNKE_STOP = -1,
		CHUNKE_OK = 0,
		CHUNKE_TOO_LONG_HEX = 1,
		CHUNKE_ILLEGAL_HEX,
		CHUNKE_BAD_CHUNK,
		CHUNKE_WRITE_ERROR,
		CHUNKE_STATE_ERROR,
		CHUNKE_BAD_ENCODING,
		CHUNKE_OUT_OF_MEMORY,
		CHUNKE_LAST
} CHUNK_CODE;

struct http_chunker {
	char hexbuffer[ MAXNUM_SIZE + 1];
	int hexindex;
	CHUNK_STATE state;
	size_t datasize;		// 剩余纯数据块大小，会影响下一次调用。
	size_t dataleft;        // 解析完整个Chunk体后,datap还剩下的长度。 
};


struct chunk_trailer{
	size_t trlPos;
	size_t trlMax;
	char * trailer;
};

typedef int (* write_decode_data)(char *data,size_t datalen,void * arg);

extern void decode_chunked_init(write_decode_data write_body_data,void * arg1,write_decode_data write_trailer_data,void * arg2,bool btrailer);

extern CHUNK_CODE decode_chunked(char *datap,size_t datalen,size_t *wrote);

extern void decode_chunked_cleanup();

#endif //_HTTP_CHUNK
