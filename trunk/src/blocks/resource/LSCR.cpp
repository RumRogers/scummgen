#include "LSCR.hpp"
#include "util/IO.hpp"
#include "types/Script.hpp"

LSCR::LSCR(Script *script)
{
}

uint32_t LSCR::getSize()
{
	uint32_t size = 0;
	size += 4 * sizeof(uint8_t); // identifier
	size += sizeof(uint32_t); // size
	size += _bytes.size() * sizeof(uint8_t); // bytes
	return size;
}

void LSCR::write(ofstream &f)
{
	IO::writeString(f, "LSCR");
	IO::writeU32BE(f, getSize());
	for (int i = 0; i < _bytes.size(); i++)
		IO::writeU8(f, _bytes[i]);
}

LSCR::~LSCR()
{
}

