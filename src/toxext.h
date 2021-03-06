#pragma once
#include <tox/tox.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * A handle to the tox extension library
 */
struct ToxExt;

/**
 * A handle to a registered extension
 */
struct ToxExtExtension;

/**
 * Some packets about to be sent out. May be sent to many extensions but only a
 * single friend
 */
struct ToxExtPacketList;

#define TOXEXT_MAX_PACKET_SIZE (TOX_MAX_CUSTOM_PACKET_SIZE - 7)

enum TOXEXT_ERROR {
	TOXEXT_SUCCESS = 0,
	TOXEXT_DATA_TOO_LARGE,
	TOXEXT_DOES_NOT_EXIST,
	TOXEXT_NOT_SUPPORTED,
	TOXEXT_ALLOCATE_FAIL,
	TOXEXT_INVALID_PACKET,
	TOXEXT_NOT_CONNECTED,
	TOXEXT_SEND_FAILED,
};

/**
 * Initializes a ToxExt instance and gets a handle back, a toxext instance will
 * hold onto a Tox instance, so this must be freed before the tox instance is
 */
struct ToxExt *toxext_init(struct Tox *tox);

/**
 * Frees an existing ToxExt instance
 */
void toxext_free(struct ToxExt *toxext);

/**
 * Service function
 */
void toxext_iterate(struct ToxExt *toxext);

/**
 * Callback for your extension. When the toxext library receives a message from
 * a friend with the same uuid as was registered, this callback will be called
 */
typedef void (*toxext_recv_callback)(struct ToxExtExtension *extension,
				     uint32_t friend_id, void const *data,
				     size_t size, void *userdata,
				     struct ToxExtPacketList *response_packet);

/**
 * Negotiation callback. This is called after a friend acknowledges your
 * negotiation request.
 *
 * You might think that there should be some way for the extension to version
 * itself and reject the negotiation request, but I argue that on new versions
 * the UUID should just be updated
 */
typedef void (*toxext_negotiate_connection_cb)(
	struct ToxExtExtension *extension, uint32_t friend_id, bool compatible,
	void *userdata, struct ToxExtPacketList *response_packet);

/**
 * Creates a registered extension item. Must be used when sending messages for
 * this extension.
 */
struct ToxExtExtension *toxext_register(struct ToxExt *toxext,
					uint8_t const *uuid, void *userdata,
					toxext_recv_callback recv_cb,
					toxext_negotiate_connection_cb neg_cb);

/**
 * Frees/deregisters a toxext item
 */
void toxext_deregister(struct ToxExtExtension *extension);

/**
 * Creates a packet list. Each extension that wants to talk to a friend appends
 * onto the list. A packet list may fold multiple packets into one toxcore
 * packet. The list will be sent and freed with toxext_send
 */
struct ToxExtPacketList *toxext_packet_list_create(struct ToxExt *toxext,
						   uint32_t friend_id);

/**
 * Appends an extension message onto an existing packet. This is the endpoint
 * extensions should use when trying to send their own data.
 */
int toxext_packet_append(struct ToxExtPacketList *packet /*in/out*/,
			 struct ToxExtExtension *extension, void const *data,
			 size_t size);

/**
 * Sends and frees an existing packet
 */
int toxext_send(struct ToxExtPacketList *packet);

/**
 * Determines if the input data is meant for ToxExt
 */
bool is_toxext_packet(uint8_t const *data, size_t size);

/**
 * Callback for lossless custom packets from toxcore. Clients are expected to
 * attach this to their toxcore callbacks
 */
int toxext_handle_lossless_custom_packet(struct ToxExt *toxext,
					 uint32_t friend_id, void const *data,
					 size_t size);

/**
 * Negotiate a connection with a friend. This must be called for a client to
 * signal to a friend that any given extension is supported. Negotiation is
 * completed through the toxext_negotiate_connection_cb
 */
int toxext_negotiate_connection(struct ToxExtExtension *extension,
				uint32_t friend_id);

/**
 * Revokes a connection with a friend for the given extension. If a client wants
 * to stop advertising extension support to a friend they can call this function
 */
int toxext_revoke_connection(struct ToxExtExtension *extension,
			     uint32_t friend_id);
