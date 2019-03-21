/*
Copyright (c) 2010-2018 Roger Light <roger@atchoo.org>

All rights reserved. This program and the accompanying materials
are made available under the terms of the Eclipse Public License v1.0
and Eclipse Distribution License v1.0 which accompany this distribution.
 
The Eclipse Public License is available at
   http://www.eclipse.org/legal/epl-v10.html
and the Eclipse Distribution License is available at
  http://www.eclipse.org/org/documents/edl-v10.php.
 
Contributors:
   Roger Light - initial implementation and documentation.
*/

#include "config.h"

#ifdef WITH_PERSISTENCE

#ifndef WIN32
#include <arpa/inet.h>
#endif
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "mosquitto_broker_internal.h"
#include "memory_mosq.h"
#include "persist.h"
#include "time_mosq.h"
#include "util_mosq.h"

int persist__chunk_cfg_write_v5(FILE *db_fptr, struct PF_cfg *chunk)
{
	struct PF_header header;

	header.chunk = htonl(DB_CHUNK_CFG);
	header.length = htonl(sizeof(struct PF_cfg));
	write_e(db_fptr, &header, sizeof(struct PF_header));
	write_e(db_fptr, chunk, sizeof(struct PF_cfg));

	return MOSQ_ERR_SUCCESS;
error:
	log__printf(NULL, MOSQ_LOG_ERR, "Error: %s.", strerror(errno));
	return 1;
}


int persist__chunk_client_write_v5(FILE *db_fptr, struct P_client *chunk)
{
	struct PF_header header;
	int id_len = chunk->F.id_len;

	chunk->F.session_expiry_interval = htonl(chunk->F.session_expiry_interval);
	chunk->F.last_mid = htons(chunk->F.last_mid);
	chunk->F.id_len = htons(chunk->F.id_len);

	header.chunk = htonl(DB_CHUNK_CLIENT);
	header.length = htonl(sizeof(struct PF_client)+id_len);

	write_e(db_fptr, &header, sizeof(struct PF_header));
	write_e(db_fptr, &chunk->F, sizeof(struct PF_client));

	write_e(db_fptr, chunk->client_id, id_len);

	return MOSQ_ERR_SUCCESS;
error:
	log__printf(NULL, MOSQ_LOG_ERR, "Error: %s.", strerror(errno));
	return 1;
}


int persist__chunk_client_msg_write_v5(FILE *db_fptr, struct P_client_msg *chunk)
{
	struct PF_header header;
	int id_len = chunk->F.id_len;

	chunk->F.mid = htons(chunk->F.mid);
	chunk->F.id_len = htons(chunk->F.id_len);

	header.chunk = htonl(DB_CHUNK_CLIENT_MSG);
	header.length = htonl(sizeof(struct PF_client_msg)+id_len);

	write_e(db_fptr, &header, sizeof(struct PF_header));
	write_e(db_fptr, &chunk->F, sizeof(struct PF_client_msg));
	write_e(db_fptr, chunk->client_id, id_len);

	return MOSQ_ERR_SUCCESS;
error:
	log__printf(NULL, MOSQ_LOG_ERR, "Error: %s.", strerror(errno));
	return 1;
}


int persist__chunk_message_store_write_v5(FILE *db_fptr, struct P_msg_store *chunk)
{
	struct PF_header header;
	int payloadlen = chunk->F.payloadlen;
	int source_id_len = chunk->F.source_id_len;
	int source_username_len = chunk->F.source_username_len;
	int topic_len = chunk->F.topic_len;

	chunk->F.payloadlen = htonl(chunk->F.payloadlen);
	chunk->F.source_mid = htons(chunk->F.source_mid);
	chunk->F.source_id_len = htons(chunk->F.source_id_len);
	chunk->F.source_username_len = htons(chunk->F.source_username_len);
	chunk->F.topic_len = htons(chunk->F.topic_len);
	chunk->F.source_port = htons(chunk->F.source_port);

	header.chunk = htonl(DB_CHUNK_MSG_STORE);
	header.length = htonl(sizeof(struct PF_msg_store) +
			topic_len + payloadlen +
			source_id_len + source_username_len);

	write_e(db_fptr, &header, sizeof(struct PF_header));
	write_e(db_fptr, &chunk->F, sizeof(struct PF_msg_store));
	if(source_id_len){
		write_e(db_fptr, chunk->source.id, source_id_len);
	}
	if(source_username_len){
		write_e(db_fptr, chunk->source.username, source_username_len);
	}
	write_e(db_fptr, chunk->topic, topic_len);
	if(payloadlen){
		write_e(db_fptr, UHPA_ACCESS(chunk->payload, payloadlen), (unsigned int)payloadlen);
	}

	return MOSQ_ERR_SUCCESS;
error:
	log__printf(NULL, MOSQ_LOG_ERR, "Error: %s.", strerror(errno));
	return 1;
}


int persist__chunk_retain_write_v5(FILE *db_fptr, struct P_retain *chunk)
{
	struct PF_header header;

	header.chunk = htonl(DB_CHUNK_RETAIN);
	header.length = htonl(sizeof(struct PF_retain));

	write_e(db_fptr, &header, sizeof(struct PF_header));
	write_e(db_fptr, &chunk->F, sizeof(struct PF_retain));

	return MOSQ_ERR_SUCCESS;
error:
	log__printf(NULL, MOSQ_LOG_ERR, "Error: %s.", strerror(errno));
	return 1;
}


int persist__chunk_sub_write_v5(FILE *db_fptr, struct P_sub *chunk)
{
	struct PF_header header;
	int id_len = chunk->F.id_len;
	int topic_len = chunk->F.topic_len;

	chunk->F.identifier = htonl(chunk->F.identifier);
	chunk->F.id_len = htons(chunk->F.id_len);
	chunk->F.topic_len = htons(chunk->F.topic_len);

	header.chunk = htonl(DB_CHUNK_SUB);
	header.length = htonl(sizeof(struct PF_sub) +
			id_len + topic_len);

	write_e(db_fptr, &header, sizeof(struct PF_header));
	write_e(db_fptr, &chunk->F, sizeof(struct PF_sub));
	write_e(db_fptr, chunk->client_id, id_len);
	write_e(db_fptr, chunk->topic, topic_len);

	return MOSQ_ERR_SUCCESS;
error:
	log__printf(NULL, MOSQ_LOG_ERR, "Error: %s.", strerror(errno));
	return 1;
}
#endif