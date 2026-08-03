/***************************************************************************************************
**    Copyright (C) 1999-2026 HMS Technology Center Ravensburg GmbH, all rights reserved
****************************************************************************************************
**
**        File: benchmark_m33.c
**     Summary: ETX project specific configuration
**              NXP MCXN947, ARM Cortex-M33 (FRDM-MCXN947, ETX benchmark)
**
**      Author: Isaac L. L. Yuki
**
****************************************************************************************************
**    Template Version 4
***************************************************************************************************/

/***************************************************************************************************
**    include-files
***************************************************************************************************/

#include "benchmark_common.h"

/***************************************************************************************************
**    definitions
***************************************************************************************************/

/*
    The Flash Patch and Breakpoint unit (FPB) has no CMSIS type in core_cm33.h, so declare the two
    registers we need to inspect. Only FP_CTRL and FP_COMP[n] are touched, read-only.
*/
typedef struct
{
    __IOM uint32_t CTRL;
    __IOM uint32_t REMAP;
    __IOM uint32_t COMP[8U]; /* Cortex-M33 implements at most 8 instruction-address comparators */
} BENCHMARK_FPB_Type;

#define BENCHMARK_FPB ((BENCHMARK_FPB_Type *) 0xE0002000UL)

#define FPB_CTRL_ENABLE_MASK (1UL << 0U)
#define FPB_CTRL_NUM_CODE_LO_MASK (0xFUL << 4U)
#define FPB_CTRL_NUM_CODE_LO_SHIFT (4U)
#define FPB_CTRL_NUM_CODE_HI_MASK (0x7UL << 12U)
#define FPB_CTRL_NUM_CODE_HI_SHIFT (12U)
#define FPB_COMP_BE_MASK (1UL << 0U)

/*
    ETMv4 instruction trace unit: optional in silicon, absent from CMSIS, and located wherever the
    ROM table says. Registers are therefore addressed relative to the base the table reports, never
    a hardcoded one, which also means no read ever touches an unimplemented window.
*/
#define ETM_REG_AT(base, offset) (*(const volatile uint32_t *) ((base) + (offset)))

#define ETM_TRCPRGCTLR_OFFSET (0x004U)
#define ETM_TRCSTATR_OFFSET (0x00CU)
#define ETM_TRCSTALLCTLR_OFFSET (0x02CU)
#define ETM_TRCPDSR_OFFSET (0x314U)

#define ETM_TRCPRGCTLR_EN_MASK (1UL << 0U)
#define ETM_TRCSTATR_IDLE_MASK (1UL << 0U)
#define ETM_TRCPDSR_POWER_MASK (1UL << 0U)

/*
    CoreSight ROM table. Always implemented on Cortex-M33 and the authoritative record of which
    debug components exist and at what address. Entry format: bit 0 marks the component present,
    bits [31:12] hold a signed 4 KB-granular offset from the table base, a zero word ends the list.
    Entries occupy the space below the table's own ID registers at offset 0xF00.
*/
#define CORESIGHT_ROM_TABLE_BASE (0xE00FF000UL)
#define CORESIGHT_ROM_ENTRY_MAX (0xF00U / 4U)
#define CORESIGHT_ROM_ENTRY_PRESENT_MASK (1UL << 0U)
#define CORESIGHT_ROM_ENTRY_OFFSET_MASK (0xFFFFF000UL)

#define CORESIGHT_COMP_DEVARCH_OFFSET (0xFBCU)
#define CORESIGHT_COMP_DEVTYPE_OFFSET (0xFCCU)

#define ETM_DEVTYPE_TRACE_SOURCE_CORE (0x00000013UL) /* MAJOR 0x3 trace source, SUB 0x1 core */
#define ETM_DEVARCH_PRESENT_MASK (1UL << 20U)
#define ETM_DEVARCH_ARCHITECT_ARM (0x23BUL) /* DEVARCH[31:21]: JEP106 code of Arm Ltd */
#define ETM_DEVARCH_ARCHID_ETMV4 (0x4A13UL) /* DEVARCH[15:0]: ETMv4 architecture */

/* DWT->CTRL bits that may legally be set: CYCCNT enable plus the read-only ID/capability fields. */
#define DWT_CTRL_ALLOWED_MASK                                                                                          \
    (DWT_CTRL_CYCCNTENA_Msk | DWT_CTRL_NUMCOMP_Msk | DWT_CTRL_NOTRCPKT_Msk | DWT_CTRL_NOEXTTRIG_Msk                    \
     | DWT_CTRL_NOCYCCNT_Msk | DWT_CTRL_NOPRFCNT_Msk)

/* DEMCR bits that may legally be set: TRCENA (required for DWT) plus the read-only SDME status. */
#define DCB_DEMCR_ALLOWED_MASK (DCB_DEMCR_TRCENA_Msk | DCB_DEMCR_SDME_Msk)

/***************************************************************************************************
**    prototypes
***************************************************************************************************/

static uint32_t BENCHMARK_findCoreSightComponent(uint32_t devtype, uint32_t archid);

/***************************************************************************************************
**    variables
***************************************************************************************************/

/***************************************************************************************************
**    code
***************************************************************************************************/

void BENCHMARK_assertNeededComponents(void)
{
    /* --- required for the measurement itself ------------------------------------------------- */
    assert((DWT->CTRL & DWT_CTRL_NOCYCCNT_Msk) == 0u);  /* cycle counter is implemented           */
    assert((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) != 0u); /* cycle counter is running               */
    assert((DWT->CTRL & DWT_CTRL_CYCDISS_Msk) == 0u);   /* cycle counter not inhibited            */
    assert((DCB->DEMCR & DCB_DEMCR_TRCENA_Msk) != 0u);  /* DWT is clocked/accessible              */
}

/*
    Walk the CoreSight ROM table and return the base address of the first present component whose
    DEVTYPE and DEVARCH architecture ID match, or 0 if the table lists none. Single level: the core
    ROM table lists the core's own debug components, which is where an ETM would appear. SoC-level
    tables chained from here (funnels, trace buffers) are not followed.
*/
static uint32_t BENCHMARK_findCoreSightComponent(uint32_t devtype, uint32_t archid)
{
    const volatile uint32_t *const rom = (const volatile uint32_t *) CORESIGHT_ROM_TABLE_BASE;

    for (uint32_t i = 0U; i < CORESIGHT_ROM_ENTRY_MAX; i++)
    {
        const uint32_t entry = rom[i];
        if (entry == 0U)
        {
            break; /* end of table */
        }
        if ((entry & CORESIGHT_ROM_ENTRY_PRESENT_MASK) == 0U)
        {
            continue; /* slot defined but component not fitted */
        }

        /* The offset field is signed: components sit both above and below the table itself. */
        const uint32_t base = CORESIGHT_ROM_TABLE_BASE + (uint32_t) (int32_t) (entry & CORESIGHT_ROM_ENTRY_OFFSET_MASK);
        const volatile uint32_t *const comp = (const volatile uint32_t *) base;

        if (comp[CORESIGHT_COMP_DEVTYPE_OFFSET / 4U] != devtype)
        {
            continue;
        }
        const uint32_t devarch = comp[CORESIGHT_COMP_DEVARCH_OFFSET / 4U];
        if ((devarch & ETM_DEVARCH_PRESENT_MASK) == 0U)
        {
            continue;
        }
        if (((devarch >> 21U) & 0x7FFU) != ETM_DEVARCH_ARCHITECT_ARM)
        {
            continue; /* architecture defined by someone other than Arm */
        }
        if ((devarch & 0xFFFFU) == archid)
        {
            return base;
        }
    }

    return 0U;
}

/*
    Assert that nothing but halting debug (so the IDE can still attach/halt) and the DWT cycle
    counter is active. Every other debug/trace facility either steals bus or CPU cycles, stalls the
    core, or diverts execution, and therefore perturbs a cycle-count measurement.

    NOTE: all of this is compiled out when NDEBUG is defined - keep asserts enabled for the
    benchmark build, otherwise these preconditions are silently unchecked.
*/
void BENCHMARK_assertQuietSystem(void)
{
    /* --- DWT: everything except CYCCNT must be off ------------------------------------------- */
    assert((DWT->CTRL & DWT_CTRL_PCSAMPLENA_Msk) == 0u);  /* PC sampling off      */
    assert((DWT->CTRL & DWT_CTRL_EXCTRCENA_Msk) == 0u);   /* exception trace off  */
    assert((DWT->CTRL & DWT_CTRL_SLEEPEVTENA_Msk) == 0u); /* sleep event trace off */
    assert((DWT->CTRL & DWT_CTRL_CPIEVTENA_Msk) == 0u);   /* cycle count event trace off */
    assert((DWT->CTRL & DWT_CTRL_EXCEVTENA_Msk) == 0u);   /* exception event trace off */
    assert((DWT->CTRL & DWT_CTRL_CYCEVTENA_Msk) == 0u);   /* cycle count event trace off */
    assert((DWT->CTRL & DWT_CTRL_LSUEVTENA_Msk) == 0u);   /* LSU event trace off */
    assert((DWT->CTRL & DWT_CTRL_FOLDEVTENA_Msk) == 0u);  /* fold event trace off */
    assert((DWT->CTRL & DWT_CTRL_SYNCTAP_Msk) == 0u);     /* sync tap off */
    assert((DWT->CTRL & DWT_CTRL_CYCTAP_Msk) == 0u);      /* cycle tap off (PC/data sample rate) */
    assert((DWT->CTRL & DWT_CTRL_POSTINIT_Msk) == 0u);    /* post init off */
    assert((DWT->CTRL & DWT_CTRL_POSTPRESET_Msk) == 0u);  /* post preset off */
    assert((DWT->CTRL & ~DWT_CTRL_ALLOWED_MASK) == 0u);   /* and nothing else, incl. future bits */

    /*
        No comparator armed. MATCH == 0 disables a comparator outright, which kills every path out
        of it: trace packet, watchpoint debug event, and the CTITRIGIN lines the SoC wires
        comparators 0-2 into. FUNCTION[31:27] is a read-only ID field, hence the mask.
        Addressed by offset because core_cm33.h names only FUNCTION0..3 while NUMCOMP may report
        more - a fixed array of four would silently skip the rest.
    */
    const uint32_t dwt_numcomp = (DWT->CTRL & DWT_CTRL_NUMCOMP_Msk) >> DWT_CTRL_NUMCOMP_Pos;
    for (uint32_t i = 0U; i < dwt_numcomp; i++)
    {
        const volatile uint32_t *const function = (const volatile uint32_t *) (DWT_BASE + 0x028U + (0x010U * i));
        assert((*function & (DWT_FUNCTION_MATCH_Msk | DWT_FUNCTION_ACTION_Msk)) == 0u);
    }

    /* --- ITM/SWO: no stimulus, no timestamps, no processor stalling -------------------------- */
    assert((ITM->TCR & ITM_TCR_ITMENA_Msk) == 0u);   /* ITM disabled         */
    assert((ITM->TCR & ITM_TCR_STALLENA_Msk) == 0u); /* no core stall to deliver trace */
    assert((ITM->TCR & ITM_TCR_TSENA_Msk) == 0u);    /* local timestamps off */
    assert((ITM->TCR & ITM_TCR_GTSFREQ_Msk) == 0u);  /* global timestamps off */
    assert((ITM->TCR & ITM_TCR_SYNCENA_Msk) == 0u);  /* sync packets off */
    assert((ITM->TCR & ITM_TCR_DWTENA_Msk) == 0u);   /* DWT->ITM forwarding off */
    assert((ITM->TCR & ITM_TCR_SWOENA_Msk) == 0u);   /* SWO timestamp clock off */
    assert(ITM->TER == 0u);                          /* no stimulus port enabled (ITM printf) */

    /* --- ETM: the only unit that can stall the core for trace --------------------------------- */
    /*
        Ask the ROM table whether this part has a core trace source at all. No entry means no ETM
        exists and there is nothing to check. An entry gives us its base, and then the trace unit
        must be off: either unpowered (it cannot trace) or powered but not programmed.
    */
    const uint32_t etm_base = BENCHMARK_findCoreSightComponent(ETM_DEVTYPE_TRACE_SOURCE_CORE, ETM_DEVARCH_ARCHID_ETMV4);
    if (etm_base != 0U)
    {
        if ((ETM_REG_AT(etm_base, ETM_TRCPDSR_OFFSET) & ETM_TRCPDSR_POWER_MASK) != 0u)
        {
            assert((ETM_REG_AT(etm_base, ETM_TRCPRGCTLR_OFFSET) & ETM_TRCPRGCTLR_EN_MASK) == 0u); /* not programmed */
            assert((ETM_REG_AT(etm_base, ETM_TRCSTATR_OFFSET) & ETM_TRCSTATR_IDLE_MASK) != 0u);   /* confirmed idle */
            assert(ETM_REG_AT(etm_base, ETM_TRCSTALLCTLR_OFFSET) == 0u); /* no ISTALL/DSTALL back-pressure */
        }
    }

    /* --- DebugMonitor exception and vector catch --------------------------------------------- */
    assert((DCB->DEMCR & DCB_DEMCR_MON_EN_Msk) == 0u);       /* DebugMonitor exception off */
    assert((DCB->DEMCR & DCB_DEMCR_UMON_EN_Msk) == 0u);      /* unprivileged DebugMonitor off */
    assert((DCB->DEMCR & DCB_DEMCR_MON_PEND_Msk) == 0u);     /* no pending DebugMonitor */
    assert((DCB->DEMCR & DCB_DEMCR_MON_REQ_Msk) == 0u);      /* no DebugMonitor request */
    assert((DCB->DEMCR & DCB_DEMCR_MON_STEP_Msk) == 0u);     /* monitor single step off */
    assert((DCB->DEMCR & DCB_DEMCR_VC_CORERESET_Msk) == 0u); /* vector catch: reset */
    // assert((DCB->DEMCR & DCB_DEMCR_VC_MMERR_Msk) == 0u);   /* vector catch: MemManage */
    // assert((DCB->DEMCR & DCB_DEMCR_VC_NOCPERR_Msk) == 0u); /* vector catch: NOCP */
    // assert((DCB->DEMCR & DCB_DEMCR_VC_CHKERR_Msk) == 0u);  /* vector catch: check error */
    // assert((DCB->DEMCR & DCB_DEMCR_VC_STATERR_Msk) == 0u); /* vector catch: state error */
    // assert((DCB->DEMCR & DCB_DEMCR_VC_BUSERR_Msk) == 0u);  /* vector catch: BusFault */
    // assert((DCB->DEMCR & DCB_DEMCR_VC_INTERR_Msk) == 0u);  /* vector catch: interrupt error */
    // assert((DCB->DEMCR & DCB_DEMCR_VC_HARDERR_Msk) == 0u); /* vector catch: HardFault */
    // assert((DCB->DEMCR & DCB_DEMCR_VC_SFERR_Msk) == 0u);   /* vector catch: SecureFault */
    // assert((DCB->DEMCR & ~DCB_DEMCR_ALLOWED_MASK) == 0u);  /* and nothing else but TRCENA */

    /* --- halting debug: attached is fine, intrusive modes are not ---------------------------- */
    // assert((DCB->DHCSR & DCB_DHCSR_C_HALT_Msk) == 0u);      /* not halt-requested */
    // assert((DCB->DHCSR & DCB_DHCSR_C_STEP_Msk) == 0u);      /* single step off */
    // assert((DCB->DHCSR & DCB_DHCSR_C_MASKINTS_Msk) == 0u);  /* debugger not masking interrupts */
    // assert((DCB->DHCSR & DCB_DHCSR_C_SNAPSTALL_Msk) == 0u); /* no imprecise-halt stall */
    /*
        Opt-in: an armed FPB comparator only costs cycles when it is hit, but a breakpoint inside
        the measured region silently invalidates the run. Enable this for unattended reference runs;
        leave it off while actually debugging in the IDE.
    */
    const uint32_t fp_ctrl = BENCHMARK_FPB->CTRL;
    if ((fp_ctrl & FPB_CTRL_ENABLE_MASK) != 0u)
    {
        uint32_t fp_numcode = ((fp_ctrl & FPB_CTRL_NUM_CODE_LO_MASK) >> FPB_CTRL_NUM_CODE_LO_SHIFT)
                              | (((fp_ctrl & FPB_CTRL_NUM_CODE_HI_MASK) >> FPB_CTRL_NUM_CODE_HI_SHIFT) << 4U);
        if (fp_numcode > 8U)
        {
            fp_numcode = 8U;
        }
        for (uint32_t i = 0U; i < fp_numcode; i++)
        {
            assert((BENCHMARK_FPB->COMP[i] & FPB_COMP_BE_MASK) == 0u); /* no hardware breakpoint set */
        }
    }

    /* --- SoC-level noise sources ------------------------------------------------------------- */
    assert(SYSCON0->ECC_ENABLE_CTRL == 0u);                                  /* RAM ECC off       */
    assert((SYSCON0->CPUCTRL & SYSCON_CPUCTRL_CPU1RSTEN_MASK) != 0u);        /* CPU1 in reset     */
    assert((SYSCON0->AHBCLKCTRL0 & SYSCON_AHBCLKCTRL0_DMA0_MASK) == 0u);     /* eDMA0 clock gated */
    assert((SYSCON0->AHBCLKCTRL1 & SYSCON_AHBCLKCTRL1_SmartDMA_MASK) == 0u); /* SmartDMA clock gated */
    assert((SYSCON0->AHBCLKCTRL2 & SYSCON_AHBCLKCTRL2_DMA1_MASK) == 0u);     /* eDMA1 clock gated */
    assert((CMX_PERFMON0->PMCR[0].PMCR & SYSPM_PMCR_SSC_MASK) == 0u);
    assert((SYSCON->LPCAC_CTRL & SYSCON_LPCAC_CTRL_DIS_LPCAC_MASK) != 0u); /* LPCAC disabled */
}
