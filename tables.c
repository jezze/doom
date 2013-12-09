#include <stddef.h>
#include "w_wad.h"
#include "tables.h"

int SlopeDiv(unsigned num, unsigned den)
{
  unsigned ans;

  if (den < 512)
    return SLOPERANGE;
  ans = (num<<3)/(den>>8);
  return ans <= SLOPERANGE ? ans : SLOPERANGE;
}

fixed_t finetangent[4096];
fixed_t finesine[10240];
angle_t tantoangle[2049];

#include "m_swap.h"
#include "lprintf.h"

void R_LoadTrigTables(void)
{
  int lump;
  {
    lump = (W_CheckNumForName)("SINETABL",ns_prboom);
    if (lump == -1) I_Error("Failed to locate trig tables");
    if (W_LumpLength(lump) != sizeof(finesine))
      I_Error("R_LoadTrigTables: Invalid SINETABL");
    W_ReadLump(lump,(unsigned char*)finesine);
  }
  {
    lump = (W_CheckNumForName)("TANGTABL",ns_prboom);
    if (lump == -1) I_Error("Failed to locate trig tables");
    if (W_LumpLength(lump) != sizeof(finetangent))
      I_Error("R_LoadTrigTables: Invalid TANGTABL");
    W_ReadLump(lump,(unsigned char*)finetangent);
  }
  {
    lump = (W_CheckNumForName)("TANTOANG",ns_prboom);
    if (lump == -1) I_Error("Failed to locate trig tables");
    if (W_LumpLength(lump) != sizeof(tantoangle))
      I_Error("R_LoadTrigTables: Invalid TANTOANG");
    W_ReadLump(lump,(unsigned char*)tantoangle);
  }

  {
    size_t n;
    lprintf(LO_INFO, "Endianness...");

    if ((10 < finesine[1]) && (finesine[1] < 100)) {
      lprintf(LO_INFO, "ok.");
      return;
    }

#define CORRECT_TABLE_ENDIAN(tbl) \
    for (n = 0; n<sizeof(tbl)/sizeof(tbl[0]); n++) tbl[n] = doom_swap_l(tbl[n])

    CORRECT_TABLE_ENDIAN(finesine);
    CORRECT_TABLE_ENDIAN(finetangent);
    CORRECT_TABLE_ENDIAN(tantoangle);
    lprintf(LO_INFO, "corrected.");
  }
}
