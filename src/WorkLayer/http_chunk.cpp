#include <StdAfx.h>

#include "http_chunk.h"

struct http_chunker chunker;

struct chunk_trailer ch_trailer;

write_decode_data write_body,write_trailer;

void *body_arg,*trailer_arg;

bool has_trailer;

CHUNK_CODE decode_chunk(http_chunker * ch,chunk_trailer *trailer,char *datap,size_t datalen,size_t * writed);

void http_chunk_init(struct http_chunker *ch)
{
	ch->hexindex = 0;			/* start at 0 */
	ch->dataleft = 0;			/* no data left yet! */
	ch->state    = CHUNK_HEX;	/* we get hex first! */
}

void decode_chunked_init(write_decode_data write_body_data,void * arg1,write_decode_data write_trailer_data,void * arg2,bool btrailer)
{
	http_chunk_init(&chunker);

	ch_trailer.trlMax = 0;
	ch_trailer.trlPos = 0;
	ch_trailer.trailer = 0;

	write_body = write_body_data;
	write_trailer = write_trailer_data;

	body_arg = arg1;

	trailer_arg = arg2;

	has_trailer = btrailer;

}

void decode_chunked_cleanup()
{
	if (ch_trailer.trailer)
		free(ch_trailer.trailer);
}


CHUNK_CODE decode_chunked(char *datap,size_t datalen,size_t * wrote)
{
	return decode_chunk(&chunker,&ch_trailer,datap,datalen,wrote);
}

CHUNK_CODE decode_chunk(http_chunker * ch,chunk_trailer *trailer,char *datap,size_t datalen,size_t * writed)
{
	CHUNK_CODE result = CHUNKE_OK;

	size_t piece;
	size_t length = (size_t)datalen;
    size_t *wrote  = writed;

	while(length) 
	{
		switch(ch->state) 
		{
		case CHUNK_HEX:
		/* Check for an ASCII hex digit.We avoid the use of isxdigit to accommodate non-ASCII hosts.*/
			if(    (*datap >= 0x30 && *datap <= 0x39)    /* 0-9 */
				|| (*datap >= 0x41 && *datap <= 0x46)    /* A-F */
				|| (*datap >= 0x61 && *datap <= 0x66)) /* a-f */
			{
				if(ch->hexindex < MAXNUM_SIZE) 
				{
					ch->hexbuffer[ch->hexindex] = *datap;
					datap++;
					length--;
					ch->hexindex++;
				}
				else 
				{
					return CHUNKE_TOO_LONG_HEX; /* longer hex than we support */
				}
			}
			else 
			{
				if(0 == ch->hexindex) 
				{
					/* This is illegal data, we received junk where we expected a hexadecimal digit. */
					return CHUNKE_ILLEGAL_HEX;
				}
				/* length and datap are unmodified */
				ch->hexbuffer[ch->hexindex] = 0;
				ch->datasize = strtoul(ch->hexbuffer, NULL, 16);
				ch->state = CHUNK_POSTHEX;
			}
			break;

		case CHUNK_POSTHEX:
			/* In this state, we're waiting for CRLF to arrive. We support
				this to allow so called chunk-extensions to show up here before the CRLF comes. */
			if(*datap == 0x0d)
				ch->state = CHUNK_CR;
			length--;
			datap++;
			break;
		
		case CHUNK_CR:
			/* waiting for the LF */
			if(*datap == 0x0a)
			{
				/* we're now expecting data to come, unless size was zero! */
				if(0 == ch->datasize)
				{
					if (!has_trailer) //conn->bits.trailerHdrPresent!=TRUE
					{
						/* No Trailer: header found - revert to original Curl processing */
						ch->state = CHUNK_STOPCR;
						/* We need to increment the datap here since we bypass the
							increment below with the immediate break */
						length--;
						datap++;
						/* This is the final byte, continue to read the final CRLF */
						break;
					}
					else 
					{
						ch->state = CHUNK_TRAILER; /* attempt to read trailers */
						//conn->trlPos=0;
					}
				}
				else
				{
					ch->state = CHUNK_DATA;
				}
			}
			else
				/* previously we got a fake CR, go back to CR waiting! */
				ch->state = CHUNK_CR;
			datap++;
			length--;
			break;

		case CHUNK_DATA:
			/* we get pure and fine data
				We expect another 'datasize' of data. We have 'length' right now,
				it can be more or less than 'datasize'. Get the smallest piece.
			*/
			piece = (ch->datasize >= length ) ? length : ch->datasize;

			/* Write the data portion available */
			
			result = (CHUNK_CODE)write_body(datap,piece,body_arg);

			if(result)
				return CHUNKE_WRITE_ERROR;	
			
			*wrote += piece;			
			ch->datasize -= piece; /* decrease amount left to expect */ 
			datap  += piece;		/* move read pointer forward */
			length -= piece;		/* decrease space left in this round */			
			if(0 == ch->datasize)
				ch->state = CHUNK_POSTCR; /* end of data this round, we now expect a trailing CRLF */
			break;
		
		case CHUNK_POSTCR:
			if(*datap == 0x0d)
			{
				ch->state = CHUNK_POSTLF;
				datap++;
				length--;
			}
			else
			{
				return CHUNKE_BAD_CHUNK;
			}
			break;
		
		case CHUNK_POSTLF:
			if(*datap == 0x0a) 
			{
				/* The last one before we go back to hex state and start all over */
				http_chunk_init(ch); //解析完一个数据块了，并初始化新的数据块结构
				datap++;
				length--;
			}
			else 
			{
				return CHUNKE_BAD_CHUNK;
			}
			break;
		
		case CHUNK_TRAILER: /* conn->trailer is assumed to be freed in url.c on a connection basis */
			if (trailer->trlPos >= trailer->trlMax) 
			{
				char *ptr;
				if(trailer->trlMax)
				{
					trailer->trlMax *= 2;
					ptr = (char*)realloc(trailer->trailer,trailer->trlMax);
				}
				else 
				{
					trailer->trlMax=128;
					ptr = (char*)malloc(trailer->trlMax);
				}
				if(!ptr)
					return CHUNKE_OUT_OF_MEMORY;
				trailer->trailer = ptr;
			}
			trailer->trailer[trailer->trlPos++] = *datap;
			
			if(*datap == 0x0d)
				ch->state = CHUNK_TRAILER_CR;
			else
			{
				datap++;
				length--;
			}
			break;
		
		case CHUNK_TRAILER_CR:
			if(*datap == 0x0d) 
			{
				ch->state = CHUNK_TRAILER_POSTCR;
				datap++;
				length--;
			}
			else
				return CHUNKE_BAD_CHUNK;
			break;
		
		case CHUNK_TRAILER_POSTCR:
			if (*datap == 0x0a) 
			{
				trailer->trailer[trailer->trlPos++] = 0x0a;
				trailer->trailer[trailer->trlPos] = 0;
				if (trailer->trlPos == 2)
				{
					ch->state = CHUNK_STOP;
					datap++;
					length--;
					/* Note that this case skips over the final STOP states since we've already read the final CRLF and need to return */
					ch->dataleft = length;
					return CHUNKE_STOP; /* return stop */
				}
				else
				{
					if(write_trailer)
						write_trailer(trailer->trailer,trailer->trlPos,trailer_arg);

				}
				ch->state = CHUNK_TRAILER;
				trailer->trlPos = 0;
				datap++;
				length--;
			}
			else
				return CHUNKE_BAD_CHUNK;
			break;
		
		case CHUNK_STOPCR:
			/* Read the final CRLF that ends all chunk bodies */
			if(*datap == 0x0d) 
			{
				ch->state = CHUNK_STOP;
				datap++;
				length--;
			}
			else 
			{
				return CHUNKE_BAD_CHUNK;
			}
			break;
		
		case CHUNK_STOP:
			if (*datap == 0x0a)
			{
				datap++;
				length--;
				/* Record the length of any data left in the end of the buffer even if there's no more chunks to read */
				ch->dataleft = length;
				return CHUNKE_STOP; /* return stop */
			}
			else
			{
				return CHUNKE_BAD_CHUNK;
			}
		default:
			return CHUNKE_STATE_ERROR;
		}
	}
 return CHUNKE_OK;
}

