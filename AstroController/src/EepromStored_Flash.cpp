#include <Arduino.h>
#include "CommonUtils.h"
#include "EepromStored.h"

EepromStored::EepromStored(uint32_t addr): addr(addr) {
    this->root = FlashStore::instance;
    this->nextDirty = this->prevDirty = nullptr;
    this->next = root->first;
    this->root->first = this;
    DEBUG("Initialized", addr, (uint32_t)this->root);
    dirty = false;
}

EepromStored::~EepromStored() {
    // This is not supported
    panic("eepromstored deletion not implemented");
}

void EepromStored::setDirtyness(bool dirty) {
    if (dirty == this->dirty) {
        return;
    }

    this->dirty = dirty;

    // Add to the dirty list
    if (dirty) {
        prevDirty = root->lastDirty;
        nextDirty = nullptr;
        if (root->lastDirty)
            root->lastDirty->nextDirty = this;
        else {
            root->firstDirty = this;
        }
        root->lastDirty = this;
    } else {
        // Remove from the dirty list
        if (prevDirty)
            prevDirty->nextDirty = nextDirty;
        else
            root->firstDirty = nextDirty;
        if (nextDirty)
            nextDirty->prevDirty = prevDirty;
        else
            root->lastDirty = prevDirty;
        this->dirty = false;
    }
}


uint32_t EepromStored::get24BitsAddress() {
    // Pack the 4x 6 bits address into 3 bytes
    return (addr & 0x3f000000) >> 6
        |  (addr & 0x003f0000) >> 4
        |  (addr & 0x00003f00) >> 2
        |  (addr & 0x0000003f);
}

int EepromStored::getLength() {
    return getEepromSize();
}

void EepromStored::getData(uint8_t * t) {
    encodeEepromValue(t, getEepromSize());
}

void EepromStored::loadDataFrom(const uint8_t * data, int length) {
    decodeEepromValue((void *) data, length);
}

void EepromStored::setClean() {
    setDirtyness(false);
}

void EepromStored::write() {
    if (dirty && this->root->lastDirty == this) {
        // Already dirty & last ? Skip
        return;
    }
    setDirtyness(false);
    setDirtyness(true);
    // Schedule something
    root->schedule();
}
