#include "elf/elf-file.h"
#include "OPMASK.h"
#include <iostream>
#include <bitset>

int main() 
{
	elf_file e("data/hello.elf");
	uint32_t* inst_buf = nullptr;

	int total_inst_count = -1;
	for (auto& section : e.sections) {
		if (section.name == ".text") {
			total_inst_count = section.size / 4;
			inst_buf = (uint32_t*)section.buf.data();
		
			break;
		}
	}

	uint32_t pc = reinterpret_cast<uint32_t>(e.fbuf.data() + e.ehdr.e_entry);	// Start Program counter

	for (int i = 0; i < total_inst_count; ++i) 
	{
		uint32_t inst = *reinterpret_cast<uint32_t*>(pc);
		pc += 4;
		Inst_Decode(inst);
		//std::cout << std::bitset<32>(inst) << std::endl;
	}

	system("pause");
}