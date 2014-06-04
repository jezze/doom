#ifndef __R_DRAW__
#define __R_DRAW__

enum column_pipeline_e
{

    RDC_PIPELINE_STANDARD,
    RDC_PIPELINE_TRANSLATED,
    RDC_PIPELINE_FUZZ,
    RDC_PIPELINE_MAXPIPELINES,

};

enum draw_filter_type_e
{

    RDRAW_FILTER_NONE,
    RDRAW_FILTER_POINT,
    RDRAW_FILTER_LINEAR,
    RDRAW_FILTER_ROUNDED,
    RDRAW_FILTER_MAXFILTERS

};

enum sloped_edge_type_e
{

    RDRAW_MASKEDCOLUMNEDGE_SQUARE,
    RDRAW_MASKEDCOLUMNEDGE_SLOPED

};

typedef struct
{

    int x;
    int yl;
    int yh;
    fixed_t z;
    fixed_t iscale;
    fixed_t texturemid;
    int texheight;
    fixed_t texu;
    const byte *source;
    const byte *prevsource;
    const byte *nextsource;
    const lighttable_t *colormap;
    const lighttable_t *nextcolormap;
    const byte *translation;
    int edgeslope;
    int drawingmasked;
    enum sloped_edge_type_e edgetype;

} draw_column_vars_t;

typedef struct
{

    int y;
    int x1;
    int x2;
    fixed_t z;
    fixed_t xfrac;
    fixed_t yfrac;
    fixed_t xstep;
    fixed_t ystep;
    const byte *source;
    const lighttable_t *colormap;
    const lighttable_t *nextcolormap;

} draw_span_vars_t;

typedef struct
{

    byte *byte_topleft;
    int byte_pitch;
    enum draw_filter_type_e filterwall;
    enum draw_filter_type_e filterfloor;
    enum draw_filter_type_e filtersprite;
    enum draw_filter_type_e filterz;
    enum draw_filter_type_e filterpatch;
    enum sloped_edge_type_e sprite_edges;
    enum sloped_edge_type_e patch_edges;
    fixed_t mag_threshold;

} draw_vars_t;

extern draw_vars_t drawvars;
extern byte *translationtables;
typedef void (*R_DrawColumn_f)(draw_column_vars_t *dcvars);
R_DrawColumn_f R_GetDrawColumnFunc(enum column_pipeline_e type, enum draw_filter_type_e filter, enum draw_filter_type_e filterz);
typedef void (*R_DrawSpan_f)(draw_span_vars_t *dsvars);
R_DrawSpan_f R_GetDrawSpanFunc(enum draw_filter_type_e filter, enum draw_filter_type_e filterz);
void R_DrawSpan(draw_span_vars_t *dsvars);
void R_InitBuffer(int width, int height);
void R_InitTranslationTables(void);
void R_ResetColumnBuffer(void);
void R_SetDefaultDrawColumnVars(draw_column_vars_t *dcvars);
void R_VideoErase(int x, int y, int count);

#endif
