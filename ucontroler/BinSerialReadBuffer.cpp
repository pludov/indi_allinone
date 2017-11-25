#ifdef ARDUINO
#include <Arduino.h>
#endif


#include "BinSerialReadBuffer.h"
#include "BinSerialProtocol.h"
#include "IndiProtocol.h"
#include "IndiDeviceMutator.h"
#include "IndiVector.h"
#include "IndiVectorMember.h"
#include "IndiTextVector.h"
#include "IndiNumberVector.h"
#include "CommonUtils.h"
#include "Symbol.h"
#include "Utils.h"

const VectorKind * kindsByUid[IndiMaxVectorKind + 1] = {
    &IndiTextVectorKind,
    &IndiNumberVectorKind
};

BinSerialReadBuffer::BinSerialReadBuffer(uint8_t * buffer, int size) : ReadBuffer::ReadBuffer(buffer, size)
{
}

void BinSerialReadBuffer::internalReadAndApply(IndiDevice & applyTo, IndiProtocol &proto, BinSerialWriteBuffer & answer)
{
    uint8_t ctrl = readPacketControl();
    DEBUG(F("[PACKET START]"));
    switch(ctrl) {
        case PACKET_RESTARTED:
        {
            DEBUG(F("[RESTART]"));
#ifdef ARDUINO
            // Reset the whole protocol
            proto.reset();
            answer.appendPacketControl(PACKET_RESTARTED);
            answer.appendPacketControl(PACKET_END);
#endif
            break;
        }
        
#ifndef ARDUINO
        case PACKET_MESSAGE:
        {
            char buffer[128];
            int id = 0;
            while(!isAtEnd()) {
                uint8_t v = readUint7();
                buffer[id++] = v;
                if (id == 127) {
                    break;
                }
            }
            buffer[id] = 0; 
            DEBUG(F("DEBUG: "), buffer);
            break;
        }

        case PACKET_ANNOUNCE:
        {
            DEBUG(F("[ANNOUNCE]"));
        	IndiDeviceMutator * mutator = proto.getMutator();
        	if (!mutator) {
        		fail(F("Announce not supported here"));
        	}
            uint8_t typeUid = readUid();
            DEBUG(F(" [TYPE]:"), (int)typeUid);

            if (typeUid >IndiMaxVectorKind) {
                fail(F("Wrong vector kind"));
            }

            char name[65];
            readSymbol(name, 64);
            DEBUG(F(" [NAME]"), name);
            char label[65];
            readSymbol(label, 64);
            DEBUG(F(" [LABEL]"), label);
            uint8_t flag = readUint7();;
            DEBUG(F(" [FLAG]"), (int)flag);
            
            uint8_t clientUid = readUid();
            DEBUG(F(" [UID]"), (int)clientUid);

            // FIXME: delete this when fail !
            const VectorKind & kind = *(kindsByUid[typeUid]);

            IndiVector * vec = kind.newVector(name, label);
            vec->uid = clientUid;
            vec->flag = flag;
            while(!isAtEnd()) {
                uint8_t subType = 0;
                if (kind.hasMemberSubtype()) {
                    subType = readUid();
                }
                DEBUG(F(" [Member] subtype="), (int)subType);

                readSymbol(name, 64);
                DEBUG(F("  [NAME]"), name);
                
                readSymbol(label, 64);
                DEBUG(F("  [LABEL]"), label);
                
                IndiVectorMember * member = kind.newMember(vec, name, label, subType);

                if (!member) {
                    fail(F("Wrong vector subtype"));
                }
                member->readValue(*this);
            }
            // FIXME: bound check
            applyTo.list[vec->uid] = vec;
            mutator->announced(vec);
            DEBUG(F("Done with announcement"));
            break;
        }
        case PACKET_UPDATE:
        {
        	DEBUG(F("[UPDATE]"));
			IndiDeviceMutator * mutator = proto.getMutator();

			uint8_t clientUid = readUid();
			DEBUG(F(" [UID]"), (int)clientUid);
			// FIXME: bound check
			IndiVector * target = applyTo.list[clientUid];
			if (!target) {
				fail(F("Wrong target vector for update"));
			}
			uint8_t flag = readUint7();
			target->flag = (target->flag & ~VECTOR_BUSY) | (flag & VECTOR_BUSY);

			for(IndiVectorMember * mem = target->first; mem && !isAtEnd(); mem=mem->next)
			{
				// FIXME: partial read leads to corrupted value
				mem->readValue(*this);
			}

			if (mutator) {
				mutator->updated(target);
			}
            DEBUG(F("Done with update"));
			break;
        }
#endif
        case PACKET_REQUEST:
        {
        	// Une requete.
        	// Trouve le buffer
        	DEBUG(F("[REQUEST]"));
			uint8_t clientUid = readUid();
			DEBUG(F(" [UID]"), (int)clientUid);
			// FIXME: bound check
			IndiVector * target = applyTo.list[clientUid];
			if (!target) {
				fail(F("Wrong target vector for update"));
			}

			// Rely on correct synchronisation of members here
			// Maybe not so clever for moving options in set

			int memberCount = target->getMemberCount();
			uint16_t offset[memberCount];
			IndiVectorMember * updates[memberCount];
			IndiVectorUpdateRequest indiVectorUpdateRequest(this, updates, offset);

			IndiVectorMember *current = target->first;
			while(!isAtEnd()) {
				int skip;
				do {
					skip = readUint7();
					DEBUG(F("skip "), (int)skip);
					for(int i = 0; i < skip; ++i) {
						if (!current) fail(F("Skip too many member"));
						current = current->next;
						if (!current) fail(F("Skip too many member"));
					}
				} while(skip >= 127);
				indiVectorUpdateRequest.addItem(current, this->getCurrentPos());
				current->skipUpdateValue(*this);
				current = current->next;
			}
			// We have all values ready.
			indiVectorUpdateRequest.markEnd();

			// Reply immediately with a pseudo busy and mark the buffer busy
			answer.appendPacketControl(PACKET_UPDATE);
			answer.appendUid(clientUid);
			answer.appendUint7(VECTOR_BUSY);
			answer.appendPacketControl(PACKET_END);
			DEBUG(F("Doing update"));
			target->doUpdate(indiVectorUpdateRequest);
			indiVectorUpdateRequest.unseek();
			// If the vector is not busy, ensure it gets back to non busy on the client side
			if (!(target->flag & VECTOR_BUSY)) {
				// Make sure the target is dirty
				if (target->setDirty(proto.getClientId(), VECTOR_VALUE)) {
					DEBUG(F("Client made ready to send non busy ASAP"));
					proto.dirtied(target);
				}
			}

			DEBUG(F("Done with update"));
			break;
        }


        default:
            DEBUG("Wrong kind: ", (int)ctrl);
            fail(F("Wrong kind"));

    }

    if (readPacketControl() != PACKET_END) {
        fail(F("Packet too big"));
    }
    DEBUG(F("[PACKET END]"));
}


void BinSerialReadBuffer::readString(char * buffer, int maxSize)
{
	readSymbol(buffer, maxSize);
}

void BinSerialReadBuffer::skipString(int maxSize)
{
	skipSymbol(maxSize);
}


uint8_t BinSerialReadBuffer::readUint7()
{
    uint8_t r = readOne();
    if (r > 127) {
        fail(F("Wrong uint7"));
    }
    return r;
}

uint8_t BinSerialReadBuffer::readStringChar()
{
    uint8_t c = readOne();
    if (c > PACKET_MAX_DATA) fail(F("Malformed packet"));
    return c;
}

uint8_t BinSerialReadBuffer::readUid()
{
    uint8_t v = readOne();
    if (v == PACKET_MAX_DATA) {
        v += readOne();
    }
    return v;
}

uint8_t BinSerialReadBuffer::readPacketControl()
{
    uint8_t v = readOne();
    if (v >= MIN_PACKET_START && v <= MAX_PACKET_START) {
        return v;
    }
    if (v == PACKET_END) {
        return v;
    }
    fail(F("Packet control not found"));
}

bool BinSerialReadBuffer::isAtEnd()
{
    uint8_t v = peekOne();
    return (v == PACKET_END);
}

void BinSerialReadBuffer::readSymbol(char * buffer, int maxLength)
{
    int i = 0;
    while(i <= maxLength) {
        uint8_t v = readStringChar();
        buffer[i ++] = v;
        if (!v) return;
    }
    fail(F("Symbol overflow"));
}

void BinSerialReadBuffer::skipSymbol(int maxLength)
{
    int i = 0;
    while(i < maxLength) {
        uint8_t v = readStringChar();
        if (!v) return;
    }
    fail(F("Symbol overflow"));
}

float BinSerialReadBuffer::readFloat()
{
    char buffer[33];
    readSymbol(buffer, 32);
    float f;
    if (sscanf(buffer, "%f", &f) != 1) {
        DEBUG(F("Failing float: "), buffer);
        fail(F("Float format error"));
    }
    return f;
}

int32_t BinSerialReadBuffer::readInt()
{
    int32_t result = 0;
    for(int i = 0; i < 5; ++i)
    {
        int32_t v = readOne();
        if (v & 128) {
            fail(F("Invalid int"));
        }
        result |= (v << (7*i));
    }
    return result;
}

