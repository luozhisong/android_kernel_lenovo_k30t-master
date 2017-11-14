/*
 * Copyright (c) 2001-2002 by David Brownell
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* this file is part of ehci-hcd.c */

#ifdef	DEBUG

/* check the values in the HCSPARAMS register
 * (host controller _Structural_ parameters)
 * see EHCI spec, Table 2-4 for each value
 */
static void dbg_hcs_params (struct ehci_hcd *ehci, char *label)
{
	u32	params = ehci_readl(ehci, &ehci->caps->hcs_params);

	ehci_dbg (ehci,
		"%s hcs_params 0x%x dbg=%d%s cc=%d pcc=%d%s%s ports=%d\n",
		label, params,
		HCS_DEBUG_PORT (params),
		HCS_INDICATOR (params) ? " ind" : "",
		HCS_N_CC (params),
		HCS_N_PCC (params),
		HCS_PORTROUTED (params) ? "" : " ordered",
		HCS_PPC (params) ? "" : " !ppc",
		HCS_N_PORTS (params)
		);
	/* Port routing, per EHCI 0.95 Spec, Section 2.2.5 */
	if (HCS_PORTROUTED (params)) {
		int i;
		char buf [46], tmp [7], byte;

		buf[0] = 0;
		for (i = 0; i < HCS_N_PORTS (params); i++) {
			// FIXME MIPS won't readb() ...
			byte = readb (&ehci->caps->portroute[(i>>1)]);
			sprintf(tmp, "%d ",
				((i & 0x1) ? ((byte)&0xf) : ((byte>>4)&0xf)));
			strcat(buf, tmp);
		}
		ehci_dbg (ehci, "%s portroute %s\n",
				label, buf);
	}
}
#else

static inline void dbg_hcs_params (struct ehci_hcd *ehci, char *label) {}

#endif

#ifdef	DEBUG

/* check the values in the HCCPARAMS register
 * (host controller _Capability_ parameters)
 * see EHCI Spec, Table 2-5 for each value
 * */
static void dbg_hcc_params (struct ehci_hcd *ehci, char *label)
{
	u32	params = ehci_readl(ehci, &ehci->caps->hcc_params);

	if (HCC_ISOC_CACHE (params)) {
		ehci_dbg (ehci,
			"%s hcc_params %04x caching frame %s%s%s\n",
			label, params,
			HCC_PGM_FRAMELISTLEN(params) ? "256/512/1024" : "1024",
			HCC_CANPARK(params) ? " park" : "",
			HCC_64BIT_ADDR(params) ? " 64 bit addr" : "");
	} else {
		ehci_dbg (ehci,
			"%s hcc_params %04x thresh %d uframes %s%s%s%s%s%s%s\n",
			label,
			params,
			HCC_ISOC_THRES(params),
			HCC_PGM_FRAMELISTLEN(params) ? "256/512/1024" : "1024",
			HCC_CANPARK(params) ? " park" : "",
			HCC_64BIT_ADDR(params) ? " 64 bit addr" : "",
			HCC_LPM(params) ? " LPM" : "",
			HCC_PER_PORT_CHANGE_EVENT(params) ? " ppce" : "",
			HCC_HW_PREFETCH(params) ? " hw prefetch" : "",
			HCC_32FRAME_PERIODIC_LIST(params) ?
				" 32 periodic list" : "");
	}
}
#else

static inline void dbg_hcc_params (struct ehci_hcd *ehci, char *label) {}

#endif

#ifdef	DEBUG

static void __maybe_unused
dbg_qtd (const char *label, struct ehci_hcd *ehci, struct ehci_qtd *qtd)
{
	ehci_dbg(ehci, "%s td %p n%08x %08x t%08x p0=%08x\n", label, qtd,
		hc32_to_cpup(ehci, &qtd->hw_next),
		hc32_to_cpup(ehci, &qtd->hw_alt_next),
		hc32_to_cpup(ehci, &qtd->hw_token),
		hc32_to_cpup(ehci, &qtd->hw_buf [0]));
	if (qtd->hw_buf [1])
		ehci_dbg(ehci, "  p1=%08x p2=%08x p3=%08x p4=%08x\n",
			hc32_to_cpup(ehci, &qtd->hw_buf[1]),
			hc32_to_cpup(ehci, &qtd->hw_buf[2]),
			hc32_to_cpup(ehci, &qtd->hw_buf[3]),
			hc32_to_cpup(ehci, &qtd->hw_buf[4]));
}

static void __maybe_unused
dbg_qh (const char *label, struct ehci_hcd *ehci, struct ehci_qh *qh)
{
	struct ehci_qh_hw *hw = qh->hw;

	ehci_dbg (ehci, "%s qh %p n%08x info %x %x qtd %x\n", label,
		qh, hw->hw_next, hw->hw_info1, hw->hw_info2, hw->hw_current);
	dbg_qtd("overlay", ehci, (struct ehci_qtd *) &hw->hw_qtd_next);
}

static void __maybe_unused
dbg_itd (const char *label, struct ehci_hcd *ehci, struct ehci_itd *itd)
{
	ehci_dbg (ehci, "%s [%d] itd %p, next %08x, urb %p\