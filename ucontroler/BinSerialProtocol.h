#ifndef BINSERIALPROTOCOL_H_
#define BINSERIALPROTOCOL_H_ 1

#define IndiTextVectorKindUid 0
#define IndiNumberVectorKindUid 1
#define IndiMaxVectorKind IndiNumberVectorKindUid

#define IndiNumberVectorMemberInt 0
#define IndiNumberVectorMemberFloat 1

struct VectorKind;
extern const VectorKind * kindsByUid[IndiMaxVectorKind + 1];

// Must be > 127
#define PACKET_MAX_DATA 240

#define PACKET_REQUEST 248
#define PACKET_MESSAGE 249
// Sent when a client is restarted
#define PACKET_RESTARTED 250
#define PACKET_ANNOUNCE 251
#define PACKET_MUTATE 252
#define PACKET_UPDATE 253
#define PACKET_DELETE 254
#define PACKET_END 255


#define MIN_PACKET_START PACKET_REQUEST
#define MAX_PACKET_START PACKET_DELETE


#define NOTIF_PACKET_MAX_SIZE 2048
#define REQUEST_PACKET_MAX_SIZE NOTIF_PACKET_MAX_SIZE
#define ACK_PACKET_MAX_SIZE 256

#endif
