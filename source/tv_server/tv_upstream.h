/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef __TV_UPSTREAM_H
#define __TV_UPSTREAM_H

#include "tv_local.h"

#include "tv_relay.h"

struct upstream_s {
	connstate_t state;

	packet_t *packetqueue;
	packet_t *packetqueue_head;
	packet_t *packetqueue_discarded;

	int number;
	char *name;
	char *customname, *backupname;
	netadr_t serveraddress;
	char *servername;
	char *password;

	char *userinfo;
	bool userinfo_modified;

	bool individual_socket;
	socket_t *socket;
	socket_t socket_real;
	netchan_t netchan;

	int64_t connect_time;
	int connect_count;
	int challenge;
	bool rejected;

	int timeoutcount;
	int64_t lastPacketReceivedTime;
	int64_t lastPacketSentTime;

	int64_t reliableSequence;          // the last one we put in the list to be sent
	int64_t reliableSent;              // the last one we sent to the server
	int64_t reliableAcknowledge;       // the last one the server has executed
	char reliableCommands[MAX_RELIABLE_COMMANDS][MAX_STRING_CHARS];

	int64_t framenum;
	int64_t lastExecutedServerCommand;
	int64_t lastUcmdTime;

	bool reliable;
	bool multiview;                     // are we receiving multiview data?
	bool precacheDone;

	// serverdata
	int playernum;
	int servercount;
	int64_t serverTime;
	int64_t serverFrame;
	unsigned int snapFrameTime;
	char game[MAX_QPATH];
	char basegame[MAX_QPATH];
	char levelname[MAX_QPATH];
	int sv_bitflags;
	purelist_t *purelist;
	char configstrings[MAX_CONFIGSTRINGS][MAX_CONFIGSTRING_CHARS];
	entity_state_t baselines[MAX_EDICTS];

	struct mempool_s *mempool;

	struct {
		bool recording;
		bool waiting;
		int64_t basetime;
		unsigned int duration;
		bool autorecording;

		bool playing;
		int filehandle;
		int filelen;
		char *filename, *tempname;
		bool random;

		time_t localtime;

		char meta_data[SNAP_MAX_DEMO_META_DATA_SIZE];
		size_t meta_data_realsize;
	} demo;

	char *audiotrack;

	// relays
	relay_t relay;
};

#define TV_Upstream_CopyString( upstream,in ) _TVCopyString_Pool( ( upstream )->mempool, in, __FILE__, __LINE__ )
bool TV_UpstreamForText( const char *text, upstream_t **upstream );
void TV_Upstream_UpdateReliableCommandsToServer( upstream_t *upstream, msg_t *msg );

#ifndef _MSC_VER
void TV_Upstream_Error( upstream_t *upstream, const char *format, ... ) __attribute__( ( format( printf, 2, 3 ) ) ) __attribute__( ( noreturn ) );
void TV_Upstream_Disconnect( upstream_t *upstream, const char *format, ... ) __attribute( ( format( printf, 2, 3 ) ) );
void TV_Upstream_Shutdown( upstream_t *upstream, const char *format, ... ) __attribute( ( format( printf, 2, 3 ) ) );
#else
__declspec( noreturn ) void TV_Upstream_Error( upstream_t *upstream, _Printf_format_string_ const char *format, ... );
void TV_Upstream_Disconnect( upstream_t *upstream, _Printf_format_string_ const char *format, ... );
void TV_Upstream_Shutdown( upstream_t *upstream, _Printf_format_string_ const char *format, ... );
#endif

void TV_Upstream_ClearState( upstream_t *upstream );
void TV_Upstream_AddReliableCommand( upstream_t *upstream, const char *cmd );
void TV_Upstream_Run( upstream_t *upstream, int msec );
void TV_Upstream_SavePacket( upstream_t *upstream, msg_t *msg, int timeBias );
void TV_Upstream_SendConnectPacket( upstream_t *upstream );
void TV_Upstream_Connect( upstream_t *upstream, const char *servername, const char *password, socket_type_t type, netadr_t *address );
void TV_Upstream_Reconnect_f( upstream_t *upstream );
upstream_t *TV_Upstream_New( const char *name, const char *customname, int delay );
void TV_Upstream_SetName( upstream_t *upstream, const char *name );
void TV_Upstream_NameNotify( upstream_t *upstream, client_t *client );
void TV_Upstream_SetAudioTrack( upstream_t *upstream, const char *track );

#endif // __TV_UPSTREAM_H
