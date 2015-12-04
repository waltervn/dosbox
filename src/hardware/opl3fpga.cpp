#include "config.h"
#include "inout.h"
#include "logging.h"
#include "stdio.h"
#include <math.h>
#include "adlib.h"

static Adlib::Chip *chip = 0;

void MIDI_RawOutByte(Bit8u data);

// Some parts borrowed from hardopl.c from DOSBox Daum

static Bitu regIndex[2];

static void writeReg(Bitu bank, Bitu reg, Bitu val) {
	// Handle timers
	if (bank == 0 && chip->Write(reg, val))
		return;

	MIDI_RawOutByte(0xf0);
	MIDI_RawOutByte(0x7d);
	MIDI_RawOutByte(((bank & 1) << 6) | (reg >> 2));
	MIDI_RawOutByte(((reg & 3) << 5) | (val >> 3));
	MIDI_RawOutByte((val & 7) << 4);
	MIDI_RawOutByte(0xf7);
}

static void writePort(Bitu port, Bitu val, Bitu /* iolen */) {
	Bitu bank = (port & 2) >> 1;

	if (port & 1)
		writeReg(bank, regIndex[bank], val);
	else
		regIndex[bank] = val;
}

static Bitu readPort(Bitu port, Bitu /* iolen */) {
	return chip->Read();
}

static IO_ReadHandleObject *readHandler[10] ;
static IO_WriteHandleObject *writeHandler[10];

static const Bit16u oplPorts[] = {
	0x0, 0x1, 0x2, 0x3, 0x8, 0x9,
	0x388, 0x389, 0x38A, 0x38B
};

static void reset() {
	int i;
	for (i = 0; i < 256; i++) {
		writeReg(0, i, 0);
		writeReg(1, i, 0);
	}

	// unmute output
	writeReg(1, 2, 1);
}

void OPL3FPGA_Init(Bitu sbAddr) {
	chip = new Adlib::Chip();

	reset();

	for (int i = 0; i < 10; ++i)	{
		readHandler[i] = new IO_ReadHandleObject();
		writeHandler[i] = new IO_WriteHandleObject();
		Bit16u port = oplPorts[i];
		if (i < 6)
			port += sbAddr; 
		readHandler[i]->Install(port, readPort, IO_MB);
		writeHandler[i]->Install(port, writePort, IO_MB);
	}
}

void OPL3FPGA_Shutdown() {
	for (int i = 0; i < 10; ++i) {
		delete readHandler[i];
		delete writeHandler[i];
	}

	reset();

	delete chip;
}
