#include "config.h"
#include "inout.h"
#include "logging.h"
#include "stdio.h"
#include <math.h>
#include "adlib.h"
#include <string.h>

static Adlib::Chip *chip;

void MIDI_RawOutByte(Bit8u data);

// Some parts borrowed from hardopl.c from DOSBox Daum

static Bitu regIndex;

static void writeReg(Bitu reg, Bitu val) {
	// Handle timers
	if (chip->Write(reg, val))
		return;

	MIDI_RawOutByte(0x90 | ((val >> 7) << 2) | (reg >> 7));
	MIDI_RawOutByte(reg & 0x7f);
	MIDI_RawOutByte(val & 0x7f);
}

static void writePort(Bitu port, Bitu val, Bitu /* iolen */) {
	if (port & 1)
		writeReg(regIndex, val);
	else
		regIndex = val;
}

static Bitu readPort(Bitu port, Bitu /* iolen */) {
	return chip->Read();
}

static IO_ReadHandleObject *readHandler[6];
static IO_WriteHandleObject *writeHandler[6];

static const Bit16u oplPorts[] = {
	0x0, 0x1, 0x8, 0x9, 0x388, 0x389
};

static void reset() {
	MIDI_RawOutByte(0x98);
	MIDI_RawOutByte(0x00);
	MIDI_RawOutByte(0x00);
}

void OPL2USB_Init(Bitu sbAddr) {
	chip = new Adlib::Chip();

	reset();

	for (int i = 0; i < 6; ++i)	{
		readHandler[i] = new IO_ReadHandleObject();
		writeHandler[i] = new IO_WriteHandleObject();
		Bit16u port = oplPorts[i];
		if (i < 4)
			port += sbAddr;
		readHandler[i]->Install(port, readPort, IO_MB);
		writeHandler[i]->Install(port, writePort, IO_MB);
	}
}

void OPL2USB_Shutdown() {
	for (int i = 0; i < 6; ++i) {
		delete readHandler[i];
		delete writeHandler[i];
	}

	reset();

	delete chip;
}
