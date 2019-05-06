#include "elf/elf-file.h"
#include "OPMASK.h"
#include <iostream>
#include <bitset>

int main() 
{
	elf_file e("data/hello.elf");

	int total_inst_count = e.sections[1].size / 4;
	auto pc = (uint32_t*)(e.buf.data() + e.ehdr.e_entry);	// Start Program counter

	

	for (int i = 0; i < total_inst_count; ++i) 
	{
		auto inst = ((uint32_t*)e.sections[1].buf.data())[i];
		Inst_Decode(inst);
		//std::cout << std::bitset<32>(inst) << std::endl;
	}

	system("pause");
}