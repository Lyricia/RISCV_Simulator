#pragma once
#include <iostream>
#include <iomanip>
#include <string>
#include <bitset>
#include <set>

#define DEBUG true;

constexpr int MASK_OP		= 0b00000000'00000000'00000000'01111111;
constexpr int MASK_REG_IMM	= 0b00000000'00000000'00000000'00011111;
constexpr int MASK_FUNCT2	= 0b00000000'00000000'00000000'00000011;
constexpr int MASK_FUNCT3	= 0b00000000'00000000'00000000'00000111;
constexpr int MASK_FUNCT7	= 0b00000000'00000000'00000000'01111111;
constexpr int MASK_IMM12	= 0b00000000'00000000'00001111'11111111;
constexpr int MASK_IMM20	= 0b00000000'00001111'11111111'11111111;
constexpr int MASK_AQ_RL	= 0b00000000'00000000'00000000'00000001;
constexpr int MASK_ATOMIC	= 0b00000000'00000000'00000000'00011111;

std::set<std::string> collector;

enum inst_type {
	R_TYPE,
	R4_TYPE,
	I_TYPE,
	S_TYPE,
	B_TYPE,
	U_TYPE,
	J_TYPE,
	NOTDEFINED
};

struct decode
{
	std::string op;
	uint32_t opcode;			/* (>256 entries) nearly full */

	uint32_t rd;			/* (5 bits) byte aligned for performance */
	uint32_t rs1;			/* (5 bits) byte aligned for performance */
	uint32_t rs2;			/* (5 bits) byte aligned for performance */
	uint32_t rs3;			/* (5 bits) byte aligned for performance */

	uint32_t imm;			/* decoded immediate */

	uint32_t codec;			/* (>32 entries) can grow */
	uint32_t rm;			/* round mode for some FPU ops */
	uint32_t aq;			/* acquire for atomic ops */
	uint32_t rl;			/* release for atomic ops */
	uint32_t pred;			/* pred for fence */
	uint32_t succ;			/* succ for fence */
};

void int_to_bin_digit(unsigned int in, int count, int* out)
{
	/* assert: count <= sizeof(int)*CHAR_BIT */
	unsigned int mask = 1U << (count - 1);
	int i;
	for (i = 0; i < count; i++) {
		out[i] = (in & mask) ? 1 : 0;
		in <<= 1;
	}
}

decode Decode_R(int inst) {
	decode d;
	std::string op;
	d.opcode	= (inst&MASK_OP);
	d.rd		= (inst >> 7) & MASK_REG_IMM;
	d.rs1		= (inst >> 15) & MASK_REG_IMM;
	d.rs2		= (inst >> 20) & MASK_REG_IMM;
	d.aq		= (inst >> 25) & MASK_AQ_RL;
	d.rl		= (inst >> 26) & MASK_AQ_RL;

	int funct3	= (inst >> 12) & MASK_FUNCT3;
	int funct7	= (inst >> 25) & MASK_FUNCT7;

	switch (d.opcode) {
	case 0b0'0110011:
	{
		switch (funct3) {
		case 0b0'000:
			if (funct7 == 0b0'0100000)
				op = "SUB";
			else
				op = "ADD";
			break;

		case 0b0'001:	op = "SLL";		break;
		case 0b0'010:	op = "SLT";		break;
		case 0b0'011:	op = "SLTU";	break;
		case 0b0'100:	op = "XOR";		break;

		case 0b0'101:
			if (funct7 == 0b0'0100000)
				op = "SRA";
			else
				op = "SRL";
			break;

		case 0b0'110:	op = "OR";		break;
		case 0b0'111:	op = "AND";		break;
		}
		break;
	}

	case 0b0'0111011:
	{
		switch (funct3) {
		case 0b0'000: {
			switch (funct7){
			case 0b0'0000000:	op = "ADDW";	break;
			case 0b0'0100000:	op = "SUBW";	break;
			case 0b0'0000001:	op = "MULW";	break;
			}
			break;
		}

		case 0b0'001:	op = "SLLW";	break;
		case 0b0'100:	op = "DIVW";	break;

		case 0b0'101: {
			switch (funct7) {
			case 0b0'0000000:	op = "SRLW";	break;
			case 0b0'0100000:	op = "SRAW";	break;
			case 0b0'0000001:	op = "DIVUW";	break;
			}
			break;
		}

		case 0b0'110:	op = "REMW";	break;
		case 0b0'111:	op = "REMUW";	break;
		}
		break;
	}

	case 0b0'0101111:
	{
		int amofunc = (inst >> 27) & MASK_ATOMIC;
		switch (amofunc) {
		case 0b0'00010:	op = "LR";		break;
		case 0b0'00011:	op = "SC";		break;
		case 0b0'00001:	op = "AMOSWAP";	break;
		case 0b0'00000:	op = "AMOADD";	break;
		case 0b0'00100:	op = "AMOXOR";	break;
		case 0b0'01100:	op = "AMOAND";	break;
		case 0b0'01000:	op = "AMOOR";	break;
		case 0b0'10000:	op = "AMOMIN";	break;
		case 0b0'10100:	op = "AMOMAX";	break;
		case 0b0'11000:	op = "AMOMINU";	break;
		case 0b0'11100:	op = "AMOMAXU";	break;
		}

		if (funct3 == 010)			
			op += "_W";
		else 
			op += "_D";

		break;
	}

	case 0b0'1010011:
	{
		
		switch (funct7) {
		case 0b0'0000000:	op= "FADD_S";	break;
		case 0b0'0000100:	op= "FSUB_S";	break;
		case 0b0'0001000:	op= "FMUL_S";	break;
		case 0b0'0001100:	op= "FDIV_S";	break;
		case 0b0'0101100:	op= "FSQRT_S";	break;
		case 0b0'0010000:
		{
			switch (funct3) {
			case 0b0'000:	op = "FSGNJ_S";		break;
			case 0b0'001:	op = "FSGNJN_S";	break;
			case 0b0'010:	op = "FSGNJX_S";	break;
			}
			break;
		}
		case 0b0'0010100:
		{
			switch (funct3) {
			case 0b0'000:	op = "FMIN_S";		break;
			case 0b0'001:	op = "FMAX_S";		break;
			}
			break;
		};
		case 0b0'1100000:
		{
			switch (d.rs2) {
			case 0b0'00000:	op = "FCVT_W_S";	break;
			case 0b0'00001:	op = "FCVT_WU_S";	break;
			case 0b0'00010:	op = "FCVT_L_S";	break;
			case 0b0'00011:	op = "FCVT_LU_S";	break;
			}
			break;
		}; 
		case 0b0'1110000: {
			switch (funct3) {
			case 0b0'000:	op = "FMV_X_W";		break;
			case 0b0'001:	op = "FCLASS_S";	break;
			}
			break;
		}
		case 0b0'1010000:
		{
			switch (funct3) {
			case 0b0'000:	op = "FEQ_S";		break;
			case 0b0'001:	op = "FLT_S";		break;
			case 0b0'010:	op = "FLE_S";		break;
			}
			break;
		}
		case 0b0'1101000:
		{
			switch (d.rs2) {
			case 0b0'00000:	op = "FCVT_S_W";	break;
			case 0b0'00001:	op = "FCVT_S_WU";	break; 
			case 0b0'00010:	op = "FCVT_S_L";	break;
			case 0b0'00011:	op = "FCVT_S_LU";	break;
			}
			break;
		}
		case 0b0'1111000:	op = "FMV_W_X"; break;
//////////////////////////////////////////////////////////

		case 0b0'0000001:	op = "FADD_D";	break;
		case 0b0'0000101:	op = "FSUB_D";	break;
		case 0b0'0001001:	op = "FMUL_D";	break;
		case 0b0'0001101:	op = "FDIV_D";	break;
		case 0b0'0101101:	op = "FSQRT_D";	break;
		case 0b0'0010001:
		{
			switch (funct3) {
			case 0b0'000:	op = "FSGNJ_D";		break;
			case 0b0'001:	op = "FSGNJN_D";	break;
			case 0b0'010:	op = "FSGNJX_D";	break;
			}
			break;
		}
		case 0b0'0010101:
		{
			switch (funct3) {
			case 0b0'000:	op = "FMIN_D";		break;
			case 0b0'001:	op = "FMAX_D";		break;
			}
			break;
		};
		case 0b0'0100000:	op = "FCVT_S_D";	break;
		case 0b0'0100001:	op = "FCVT_D_S";	break;

		case 0b0'1100001:
		{
			switch (d.rs2) {
			case 0b0'00000:	op = "FCVT_W_D";	break;
			case 0b0'00001:	op = "FCVT_WU_D";	break;
			case 0b0'00010:	op = "FCVT_L_D";	break;
			case 0b0'00011:	op = "FCVT_LU_D";	break;
			}
			break;
		};
		case 0b0'1110001: {
			switch (funct3) {
			case 0b0'000:	op = "FMV_X_D";		break;
			case 0b0'001:	op = "FCLASS_D";	break;
			}
			break;
		}
		case 0b0'1010001:
		{
			switch (funct3) {
			case 0b0'000:	op = "FLE_D";		break;
			case 0b0'001:	op = "FLT_D";		break;
			case 0b0'010:	op = "FEQ_D";		break;
			}
			break;
		}
		case 0b0'1101001:
		{
			switch (d.rs2) {
			case 0b0'00000:	op = "FCVT_D_W";	break;
			case 0b0'00001:	op = "FCVT_D_WU";	break;
			case 0b0'00010:	op = "FCVT_D_L";	break;
			case 0b0'00011:	op = "FCVT_D_LU";	break;
			}
			break;
		}
		case 0b0'1111001:	op = "FMV_D_X"; break;
		}
		break;
	}

		
	default:
		op = std::to_string(d.opcode);
		while (true);
		break;
	}

#ifdef DEBUG
	if (op == "")		while (true);
#endif

	d.op = op;
	return d;

	//std::cout << std::setw(10) << op << std::setw(5) << rd << std::setw(5) << rs1 << std::setw(5) << rs2 << std::endl;
}

decode Decode_U(int inst) 
{
	
	decode d;
	std::string op;
	d.opcode = (inst&MASK_OP);
	d.rd = (inst >> 7) & MASK_REG_IMM;
	d.imm = (inst >> 12) & MASK_IMM20;

	switch (d.opcode) {
	case 0b0'0110111:	op = "LUI";		break;
	case 0b0'0010111:	op = "AUIPC";	break;
	}

#ifdef DEBUG
	if (op == "")		while (true);
#endif

	d.op = op;
	return d;
}

decode Decode_J(int inst)
{
	decode d;
	std::string op;
	d.opcode = (inst&MASK_OP);
	d.rd = (inst >> 7) & MASK_REG_IMM;
	int imm = (inst >> 12) & MASK_IMM20;
	int imm20 = (inst >> 31) & 0b0'1;
	int imm11 = (inst >> 20) & 0b0'1;
	int imm19_12 = (inst >> 12) & 0b0'11111111;
	int imm10_1 = (inst >> 21) & 0b0'1111111111;
	d.imm =
		(imm10_1 << 1)	|
		(imm11 << 11)	|
		(imm19_12 << 12) |
		(imm20 << 20);

	switch (d.opcode) {
	case 0b0'1101111:	op = "JAL";		break;
	}

#ifdef DEBUG
	if (op == "")		while (true);
#endif

	d.op = op;
	return d;
}

decode Decode_B(int inst)				//branch
{
	decode d;
	std::string op;
	d.opcode = (inst&MASK_OP);
	d.rs1 = (inst >> 15) & MASK_REG_IMM;
	d.rs2 = (inst >> 20) & MASK_REG_IMM;

	int funct3 = (inst >> 12) & MASK_FUNCT3;
	int imm1 = (inst >> 7) & MASK_REG_IMM;
	int imm2 = (inst >> 25) & MASK_FUNCT7;

	int imm12 = (imm2 >> 6) & 0b0'1;
	int imm10_5 = imm2 & 0b0111111;
	int imm4_1 = (imm1 >> 1) & 0b0111111;
	int imm11 = imm1 & 0b0'1;
	d.imm =
		(imm4_1 << 1)	|
		(imm10_5 << 5)	|
		(imm11 << 11)	|
		(imm12 << 12);

	switch (funct3) {
	case 0b0'000: op = "BEQ"; break;
	case 0b0'001: op = "BNE"; break;
	case 0b0'100: op = "BLT"; break;
	case 0b0'101: op = "BGE"; break;
	case 0b0'110: op = "BLTU"; break;
	case 0b0'111: op = "BGEU"; break;
	}

	// imm reordering

#ifdef DEBUG
	if (op == "")		while (true);
#endif

	d.op = op;
	return d;

}

decode Decode_S(int inst) 
{
	decode d;
	std::string op;
	d.opcode = (inst&MASK_OP);
	d.rs1 = (inst >> 15) & MASK_REG_IMM;
	d.rs2 = (inst >> 20) & MASK_REG_IMM;

	int funct3 = (inst >> 12) & MASK_FUNCT3;
	int imm1 = (inst >> 7) & MASK_REG_IMM;
	int imm2 = (inst >> 25) & MASK_FUNCT7;

	switch (funct3) 
	{
	case 0b0'000: op = "SB"; break;
	case 0b0'001: op = "SH"; break;
	case 0b0'010: op = "SW"; break;
	case 0b0'011: op = "SD"; break;
	}

#ifdef DEBUG
	if (op == "")		while (true);
#endif

	d.op = op;
	return d;

}

decode Decode_R4(int inst)
{
	decode d;
	std::string op;
	d.opcode = (inst&MASK_OP);
	d.rd = (inst >> 7) & MASK_REG_IMM;
	d.rm = (inst >> 12) & MASK_FUNCT3;
	d.rs1 = (inst >> 15) & MASK_REG_IMM;
	d.rs2 = (inst >> 20) & MASK_REG_IMM;
	d.rs3 = (inst >> 27) & MASK_REG_IMM;
	int funct2 = (inst >> 25) & MASK_FUNCT2;

	switch (d.opcode) {
	case 0b0'1000011:	op = "FMADD";	break;
	case 0b0'1000111:	op = "FMSUB";	break;
	case 0b0'1001011:	op = "FNMSUB";	break;
	case 0b0'1001111:	op = "FNMADD";	break;
	}
	if (funct2 == 0)	op += "_S";
	else				op += "_D";

#ifdef DEBUG
	if (op == "")		while (true);
#endif

	return d;

}

decode Decode_I(int inst) 
{
	decode d;
	std::string op;
	d.opcode = (inst&MASK_OP);
	d.rd = (inst >> 7) & MASK_REG_IMM;
	d.rs1 = (inst >> 15) & MASK_REG_IMM;
	d.rs2 = (inst >> 20) & MASK_REG_IMM;
	d.aq = (inst >> 25) & MASK_AQ_RL;
	d.rl = (inst >> 26) & MASK_AQ_RL;
	int funct3 = (inst >> 12) & MASK_FUNCT3;
	int funct7 = (inst >> 25) & MASK_FUNCT7;

	switch (d.opcode) {
	case 0b0'0000111:	op = "FLW";			break;
	case 0b0'1100111:	op = "JALR";		break;
	case 0b0'0000011:
	{
		switch (funct3) {
		case 0b0'000:	op = "LB";		break;
		case 0b0'001:	op = "LH";		break;
		case 0b0'010:	op = "LW";		break;
		case 0b0'100:	op = "LBU";		break;
		case 0b0'101:	op = "LHU";		break;
		case 0b0'110:	op = "LWU";		break;
		case 0b0'011:	op = "LD";		break;
		}
		break;
	}

	case 0b0'0010011:
	{	
		switch (funct3) {
		case 0b0'001:	op = "SLLI";	break;
		case 0b0'101:
		{
			if (funct7 == 0b0'0100000) { op = "SRLI";	break; }
			else						 op = "SRAI";	break;
		}
		case 0b0'000:	op = "ADDI";	break;
		case 0b0'010:	op = "SLTI";	break;
		case 0b0'011:	op = "SLTIU";	break;
		case 0b0'100:	op = "XORI";	break;
		case 0b0'110:	op = "ORI";	break;
		case 0b0'111:	op = "ANDI";	break;
		}
		break;
	}

	case 0b0'1110011:
	{
		switch (funct3) {
		case 0b0'000:
		{
			if (((inst >> 20)&MASK_IMM12) == 0b0'0) {	
				op = "ECALL";	break; 
			}
			else										
				op = "EBREAK";	break;
		}
		case 0b0'001:	op = "CSRRW";		break;
		case 0b0'010:	op = "CSRRS";		break;
		case 0b0'011:	op = "CSRRC";		break;
		case 0b0'101:	op = "CSRRWI";		break;
		case 0b0'110:	op = "CSRRSI";		break;
		case 0b0'111:	op = "CSRRCI";		break;
		}
		break;
	}
	case 0b0'0011011: {
		switch (funct3) {
		case 0b0'000:	op = "ADDIW";		break;
		case 0b0'001:	op = "SLLIW";		break;
		case 0b0'101: {
			if (funct7 == 0b0'0000000) {
				op = "SRLIW";		break;
			}
			else
				op = "SRAIW";		break;
		}
		}
		break;
	}

	}

#ifdef DEBUG
	if (op == "")		while (true);
#endif

	d.op = op;
	return d;

}

void Inst_Decode(uint32_t inst)
{
	decode d;
	inst_type type;
	switch (inst&MASK_OP)
	{
	case 0b0'0110111:
	case 0b0'0010111:
		type = U_TYPE;
		d = Decode_U(inst);
		break;

	case 0b0'1101111:
		type = J_TYPE;
		d = Decode_J(inst);
		break;


	case 0b0'1100011:
		type = B_TYPE;
		d = Decode_B(inst);
		break;

	case 0b0'0100011:
	case 0b0'0100111:
		type = S_TYPE;
		d = Decode_S(inst);
		break;

	case 0b0'0001111:		// FENCE
		type = NOTDEFINED;
		break;

	case 0b0'0000111:
	case 0b0'1100111:
	case 0b0'0000011:
	case 0b0'0010011:
	case 0b0'0011011:
	case 0b0'1110011:
		type = I_TYPE;
		d = Decode_I(inst);
		break;

	case 0b0'0110011:
	case 0b0'0111011:
	case 0b0'0101111:
	case 0b0'1010011:
		type = R_TYPE;
		d = Decode_R(inst);
		break;

	case 0b0'1000011:
	case 0b0'1000111:
	case 0b0'1001011:
	case 0b0'1001111:
		type = R4_TYPE;
		d = Decode_R4(inst);
		break;

	default:

		std::cout << "unknown opcode " << std::bitset<7>(inst&MASK_OP) << std::endl;
		break;
	}

	if(type == R_TYPE)
		std::cout << std::setw(10) << d.op << std::setw(5) << d.rd << std::setw(5) << d.rs1 << std::setw(5) << d.rs2 << std::endl;;
}