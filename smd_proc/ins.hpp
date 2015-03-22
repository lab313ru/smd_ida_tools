//
//	Instruction set for M68000 (enum)
//	Link: http://info.sonicretro.org/SCHG:68000_Instruction_Set
//

#ifndef INS_HPP_INCLUDED
#define INS_HPP_INCLUDED

#include <idp.hpp>

extern instruc_t Instructions[];

enum instructno_t ENUM_SIZE(uint8)
{
	opc_null = 0,
		opc_abcd,

		opc_addaw,
		opc_addal,

		opc_addib,
		opc_addiw,
		opc_addil,

		opc_addqb,
		opc_addqw,
		opc_addql,

		opc_addb,
		opc_addw,
		opc_addl,

		opc_addxb,
		opc_addxw,
		opc_addxl,

		opc_andib,
		opc_andiw,
		opc_andil,
		opc_andi,

		opc_andb,
		opc_andw,
		opc_andl,
		opc_and,

		opc_aslb,
		opc_aslw,
		opc_asll,

		opc_asrb,
		opc_asrw,
		opc_asrl,

		opc_bhiw,
		opc_blsw,
		opc_bccw,
		opc_bcsw,
		opc_bnew,
		opc_beqw,
		opc_bvcw,
		opc_bvsw,
		opc_bplw,
		opc_bmiw,
		opc_bgew,
		opc_bltw,
		opc_bgtw,
		opc_blew,

		opc_bhis,
		opc_blss,
		opc_bccs,
		opc_bcss,
		opc_bnes,
		opc_beqs,
		opc_bvcs,
		opc_bvss,
		opc_bpls,
		opc_bmis,
		opc_bges,
		opc_blts,
		opc_bgts,
		opc_bles,

		opc_bchg,

		opc_bclr,

		opc_braw,
		opc_bras,

		opc_bset,

		opc_bsrw,
		opc_bsrs,

		opc_btst,

		opc_chkl,
		opc_chkw,

		opc_clrb,
		opc_clrw,
		opc_clrl,

		opc_cmpaw,
		opc_cmpal,

		opc_cmpib,
		opc_cmpiw,
		opc_cmpil,

		opc_cmpmb,
		opc_cmpmw,
		opc_cmpml,

		opc_cmpb,
		opc_cmpw,
		opc_cmpl,

		opc_dbcc,
		opc_dbcs,
		opc_dbeq,
		opc_dbf,
		opc_dbge,
		opc_dbgt,
		opc_dbhi,
		opc_dble,
		opc_dbls,
		opc_dblt,
		opc_dbmi,
		opc_dbne,
		opc_dbpl,
		opc_dbt,
		opc_dbvc,
		opc_dbvs,

		opc_divsw,

		opc_divuw,

		opc_eorib,
		opc_eoriw,
		opc_eoril,
		opc_eori,

		opc_eorb,
		opc_eorw,
		opc_eorl,
		opc_eor,

		opc_exg,

		opc_extw,
		opc_extl,

		opc_illegal,

		opc_jmp,

		opc_jsr,

		opc_lea,

		opc_linkw,
		opc_link,

		opc_lslb,
		opc_lslw,
		opc_lsll,

		opc_lsrb,
		opc_lsrw,
		opc_lsrl,

		opc_moveal,
		opc_moveaw,

		opc_movemw,
		opc_moveml,

		opc_movepw,
		opc_movepl,

		opc_moveq,

		opc_moveb,
		opc_movew,
		opc_movel,
		opc_move,

		opc_mulsw,

		opc_muluw,

		opc_nbcd,

		opc_negb,
		opc_negw,
		opc_negl,

		opc_negxb,
		opc_negxw,
		opc_negxl,

		opc_nop,

		opc_notb,
		opc_notw,
		opc_notl,

		opc_orib,
		opc_oriw,
		opc_oril,
		opc_ori,

		opc_orb,
		opc_orw,
		opc_orl,
		opc_or,

		opc_pea,

		opc_reset,

		opc_rolb,
		opc_rolw,
		opc_roll,

		opc_rorb,
		opc_rorw,
		opc_rorl,

		opc_roxlb,
		opc_roxlw,
		opc_roxll,

		opc_roxrb,
		opc_roxrw,
		opc_roxrl,

		opc_rte,

		opc_rtr,

		opc_rts,

		opc_sbcd,

		opc_trapv,

		opc_scc,
		opc_scs,
		opc_seq,
		opc_sf,
		opc_sge,
		opc_sgt,
		opc_shi,
		opc_sle,
		opc_sls,
		opc_slt,
		opc_smi,
		opc_sne,
		opc_spl,
		opc_st,
		opc_svc,
		opc_svs,

		opc_stop,

		opc_subal,
		opc_subaw,

		opc_subib,
		opc_subiw,
		opc_subil,

		opc_subqb,
		opc_subqw,
		opc_subql,

		opc_subb,
		opc_subw,
		opc_subl,

		opc_subxb,
		opc_subxw,
		opc_subxl,

		opc_swap,

		opc_tas,

		opc_trap,

		opc_tstb,
		opc_tstw,
		opc_tstl,

		opc_unlk,

		opc_last
};

#endif
