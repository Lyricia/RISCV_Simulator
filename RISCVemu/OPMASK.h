#pragma once
#include <iostream>
#include <iomanip>
#include <string>
#include <bitset>

constexpr int MASK_OP		= 0b00000000'00000000'00000000'01111111;
constexpr int MASK_REG_IMM	= 0b00000000'00000000'00000000'00011111;
constexpr int MASK_FUNCT2	= 0b00000000'00000000'00000000'00000011;
constexpr int MASK_FUNCT3	= 0b00000000'00000000'00000000'00000111;
constexpr int MASK_FUNCT7	= 0b00000000'00000000'00000000'01111111;
constexpr int MASK_IMM11	= 0b00000000'00000000'00000111'11111111;
constexpr int MASK_IMM20	= 0b00000000'00001111'11111111'11111111;
constexpr int MASK_AQ_RL	= 0b00000000'00000000'00000000'00000001;
constexpr int MASK_ATOMIC	= 0b00000000'00000000'00000000'00011111;

void Decode_R(int inst);

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

struct INST_DATA {
	int opcode;
	int rd;
	int funct3;
	int rs1;
	int rs2;
	int funct7;
	int aq;
	int rl;
};

void Inst_Decode(uint32_t inst) 
{
	inst_type type;
	switch (inst&MASK_OP)
	{
	case 0b0'0110111:
	case 0b0'0010111:
		type = U_TYPE;
		break;

	case 0b0'1101111:
		type = J_TYPE;
		break;


	case 0b0'1100011:
		type = B_TYPE;
		break;

	case 0b0'0100011:
	case 0b0'0100111:
		type = S_TYPE;
		break;

	case 0b0'0001111:		// FENCE
		type = NOTDEFINED;
		break;

	case 0b0'0000111:
	case 0b0'1100111:
	case 0b0'0000011:
		type = I_TYPE;
		break;

	case 0b0'0010011: {
		type = I_TYPE;
		int funct3 = (inst >> 12) & MASK_FUNCT3;
		switch (funct3) {
		case 0b001:
		case 0b101:
			break;
		default:
			break;
		}
		break; 
	}

	case 0b0'1110011:		// ECALL
		if ((inst >> 7) == 0)
			type = NOTDEFINED;
		else
			type = I_TYPE;
		break;

	case 0b0'0011011:
	{
		type = I_TYPE;
		int funct3 = (inst >> 12) & MASK_FUNCT3;
		switch (funct3) {
		case 0b001:
		case 0b101:
			break;
		default:
			break;
		}
		break;
	}

	case 0b0'0110011:	
	case 0b0'0111011:	
	case 0b0'0101111:	
	case 0b0'1010011:	
		type = R_TYPE;	Decode_R(inst);	break;

	case 0b0'1000011:
	case 0b0'1000111:
	case 0b0'1001011:
	case 0b0'1001111:
		type = R4_TYPE;
		break;



	default:
		std::cout << "unknown opcode " << std::bitset<32>(inst) << std::endl;
		break;
	}
}

//R_TYPE==
//R4_TYPE==
//I_TYPE
//S_TYPE==
//B_TYPE==
//U_TYPE==
//J_TYPE==
//NOTDEFINED

void Decode_R(int inst) {
	std::string op;
	int opcode	= (inst&MASK_OP);
	int rd		= (inst >> 7) & MASK_REG_IMM;
	int funct3	= (inst >> 12) & MASK_FUNCT3;
	int rs1		= (inst >> 15) & MASK_REG_IMM;
	int rs2		= (inst >> 20) & MASK_REG_IMM;
	int funct7	= (inst >> 25) & MASK_FUNCT7;
	int aq		= (inst >> 25) & MASK_AQ_RL;
	int rl		= (inst >> 26) & MASK_AQ_RL;

	switch (opcode) {
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
			switch (rs2) {
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
			switch (rs2) {
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
			switch (rs2) {
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
			switch (rs2) {
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
		op = std::to_string(opcode);
		while (true);
		break;
	}
	//std::cout << std::setw(10) << op << std::setw(5) << rd << std::setw(5) << rs1 << std::setw(5) << rs2 << std::endl;
}

void Decode_U(int inst) 
{
	std::string op;
	int opcode = (inst&MASK_OP);
	int rd = (inst >> 7) & MASK_REG_IMM;
	int imm = (inst >> 12) & MASK_IMM20;

	switch (opcode) {
	case 0b0'0110111:	op = "LUI";		break;
	case 0b0'0010111:	op = "AUIPC";	break;
	}
}

void Decode_J(int inst)
{
	std::string op;
	int opcode = (inst&MASK_OP);
	int rd = (inst >> 7) & MASK_REG_IMM;
	int imm = (inst >> 12) & MASK_IMM20;

	switch (opcode) {
	case 0b0'1101111:	op = "JAL";		break;
	}
}

void Decode_B(int inst)				//branch
{
	std::string op;
	int opcode = (inst&MASK_OP);
	int imm1 = (inst >> 7) & MASK_REG_IMM;
	int funct3 = (inst >> 12) & MASK_FUNCT3;
	int rs1 = (inst >> 15) & MASK_REG_IMM;
	int rs2 = (inst >> 20) & MASK_REG_IMM;
	int imm2 = (inst >> 25) & MASK_FUNCT7;

	switch (funct3) {
	case 0b0'000: op = "BEQ"; break;
	case 0b0'001: op = "BNE"; break;
	case 0b0'100: op = "BLT"; break;
	case 0b0'101: op = "BGE"; break;
	case 0b0'110: op = "BLTU"; break;
	case 0b0'111: op = "BGEU"; break;
	}

	// imm reordering

}

void Decode_S(int inst) 
{
	std::string op;
	int opcode = (inst&MASK_OP);
	int imm1 = (inst >> 7) & MASK_REG_IMM;
	int funct3 = (inst >> 12) & MASK_FUNCT3;
	int rs1 = (inst >> 15) & MASK_REG_IMM;
	int rs2 = (inst >> 20) & MASK_REG_IMM;
	int imm2 = (inst >> 25) & MASK_FUNCT7;

	switch (funct3) 
	{
	case 0b0'000: op = "SB"; break;
	case 0b0'001: op = "SH"; break;
	case 0b0'010: op = "SW"; break;
	case 0b0'011: op = "SD"; break;
	}

}

void Decode_R4(int inst)
{
	std::string op;
	int opcode = (inst&MASK_OP);
	int rd = (inst >> 7) & MASK_REG_IMM;
	int rm = (inst >> 12) & MASK_FUNCT3;
	int rs1 = (inst >> 15) & MASK_REG_IMM;
	int rs2 = (inst >> 20) & MASK_REG_IMM;
	int rs3 = (inst >> 27) & MASK_REG_IMM;
	int funct2 = (inst >> 25) & MASK_FUNCT2;

	switch (opcode) {
	case 0b0'1000011:	op = "FMADD";	break;
	case 0b0'1000111:	op = "FMSUB";	break;
	case 0b0'1001011:	op = "FNMSUB";	break;
	case 0b0'1001111:	op = "FNMADD";	break;
	}
	if (funct2 == 0)	op += "_S";
	else				op += "_D";

}
