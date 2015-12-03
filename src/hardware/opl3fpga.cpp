#include "config.h"
#include "inout.h"
#include "logging.h"
#include "stdio.h"

void MIDI_RawOutByte(Bit8u data);

// Some parts borrowed from hardopl.c from DOSBox Daum

static Bitu regIndex[2];

static void writeReg(Bitu bank, Bitu reg, Bitu val) {
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
	// TODO: emulate timer
	return 0;
}

static IO_ReadHandleObject *readHandler[10] ;
static IO_WriteHandleObject *writeHandler[10];

static const Bit16u oplPorts[] = {
	0x0, 0x1, 0x2, 0x3, 0x8, 0x9,
	0x388, 0x389, 0x38A, 0x38B
};

void OPL3FPGA_Init(Bitu sbAddr) {
	for (int i = 0; i < 10; ++i)	{
		readHandler[i] = new IO_ReadHandleObject();
		writeHandler[i] = new IO_WriteHandleObject();
		Bit16u port = oplPorts[i];
		if (i < 6)
			port += sbAddr; 
		readHandler[i]->Install(port, readPort, IO_MB);
		writeHandler[i]->Install(port, writePort, IO_MB);
	}

	// Disable OPL3 mode
	writeReg(1, 0x05, 0x00);
	// Unmute
	writeReg(1, 0x02, 0x01);
}

void OPL3FPGA_Shutdown() {
	for (int i = 0; i < 10; ++i) {
		delete readHandler[i];
		delete writeHandler[i];
	}
}
