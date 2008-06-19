#include "VERB.hpp"
#include "util/IO.hpp"
#include "types/Script.hpp"

VERB::VERB(Script *script)
{
	for (int i = 0; i < script->getNumberOfBytes(); i++)
		_bytes.push_back(script->getByte(i));
}

uint32_t VERB::getSize()
{
	uint32_t size = 0;
	size += sizeof(uint32_t); // identifier
	size += sizeof(uint32_t); // size
	size += _bytes.size() * sizeof(uint8_t); // bytes
	return size;
}

void VERB::write(fstream &f)
{
	IO::writeString(f, "VERB");
	IO::writeU32BE(f, getSize());
	for (int i = 0; i < _bytes.size(); i++)
		IO::writeU8(f, _bytes[i]);
}

VERB::~VERB()
{
}

