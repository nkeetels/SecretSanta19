// Button Bashers Secret Santa 2019 Gameboy Advance demo source code
// 
// File: KDEngine.c
// Description: KDEngine handles polygon rasterization for GBA Mode 4. It features texture-mapping and bump-mapping 
//              support. The texture should be a 128 x 128 indexed color 8-bit bitmap, whereas the bump-map should
//              be a 128 x 128 greyscale 8-bit bitmap. 
//
// Note: the shading is honestly a bit of a hack. I created a tool in Python that quantizes an indexed color 8-bit
//       bitmap image to 32 colors and create 7 shades of that 32-color palette. This is why g_paletteOffset exists. 

#include "KDTypes.h"
#include "KDEngine.h"

#define MAX_FACES 140

u8 *g_currentBumpMap = 0;
u8 *g_currentTexture = 0;
u8 g_paletteOffset = 0;
u16 *g_currentBuffer = FRONTBUFFER;

KDModel currentModel;
KDFace EWRAM_DATA currentFaces[MAX_FACES];
KDFace transformedFaces[MAX_FACES];
u16 drawOrder[MAX_FACES];
u16 depthBuffer[MAX_FACES];

extern unsigned char phong_lut[16384];

void swap_vertex(KDVertex *a, KDVertex *b)
{
  KDVertex temp = *a;
  *a = *b;
  *b = temp;
}

void SetTexture(u8 *texture, u8 palette_offset)
{
  g_currentTexture = texture;
  g_paletteOffset = palette_offset;
}

void SetBumpMap(u8 *bumpmap)
{
    g_currentBumpMap = bumpmap;
}

void IWRAM_CODE HLine(int x1, int x2, int y, u16 color)
{
  u8 *vscreen = (u8*)((u32)g_currentBuffer + x1 + y * SCREENWIDTH);

  int length = x2 - x1 + 2;
  int n = (length + 7) >> 3;

  switch (length & 7)
  {
    case 0: do {  *vscreen++ = color;   
    case 7:       *vscreen++ = color;
    case 6:       *vscreen++ = color;
    case 5:       *vscreen++ = color; 
    case 4:       *vscreen++ = color;   
    case 3:       *vscreen++ = color;
    case 2:       *vscreen++ = color;
    case 1:       *vscreen++ = color;  
    } while (--n > 0); 
  }
}

#define SWAPPED 1 << 1

void IWRAM_CODE TextureTriangle(KDFace *face, u8 *texture, u8 *bumpmap, u8 shade)
{
  if (face->m_vertices[1].m_position.y < face->m_vertices[0].m_position.y)
   swap_vertex(&face->m_vertices[0], &face->m_vertices[1]);
  if (face->m_vertices[2].m_position.y < face->m_vertices[0].m_position.y)
   swap_vertex(&face->m_vertices[0], &face->m_vertices[2]);
  if (face->m_vertices[2].m_position.y < face->m_vertices[1].m_position.y)
   swap_vertex(&face->m_vertices[1], &face->m_vertices[2]);

  if (face->m_vertices[0].m_position.y == face->m_vertices[2].m_position.y || face->m_vertices[2].m_position.y <= 0 || face->m_vertices[0].m_position.y >= 160)
    return;

  int x0 = face->m_vertices[0].m_position.x << 16;
  int y0 = face->m_vertices[0].m_position.y;  
  int u0 = face->m_vertices[0].u << 8;
  int v0 = face->m_vertices[0].v << 8;
  int s0 = face->m_vertices[0].s << 8;
  int t0 = face->m_vertices[0].t << 8;  

  int x1 = face->m_vertices[1].m_position.x << 16;
  int y1 = face->m_vertices[1].m_position.y;
  int u1 = face->m_vertices[1].u << 8;
  int v1 = face->m_vertices[1].v << 8;
  int s1 = face->m_vertices[1].s << 8;
  int t1 = face->m_vertices[1].t << 8;

  int x2 = face->m_vertices[2].m_position.x << 16;
  int y2 = face->m_vertices[2].m_position.y;
  int u2 = face->m_vertices[2].u << 8;
  int v2 = face->m_vertices[2].v << 8;
  int s2 = face->m_vertices[2].s << 8;
  int t2 = face->m_vertices[2].t << 8;

  if (y0 == y1)
  {
    if (x1 < x0)
    {
      _swap(x0, x1);
      _swap(u0, u1);
      _swap(v0, v1);
      _swap(s0, s1);
      _swap(t0, t1);
    }

    y1++; 
  }

  if (y1 == y2)
    y2++;

  y1 = min(y1, 160);
  y2 = min(y2, 160);

  int length, dxAC, duAC, dvAC, dxBC, duBC, dvBC, dxAB, duAB, dvAB, dsAC, dtAC, dsAB, dtAB, dsBC, dtBC;

  length = y2 - y0;
  dxAC = _div((x2 - x0), length);
  duAC = _div((u2 - u0), length);
  dvAC = _div((v2 - v0), length);
  dsAC = _div((s2 - s0), length);
  dtAC = _div((t2 - t0), length);

  length = y1 - y0;
  dxAB = _div((x1 - x0), length);
  duAB = _div((u1 - u0), length);
  dvAB = _div((v1 - v0), length);
  dsAB = _div((s1 - s0), length);
  dtAB = _div((t1 - t0), length);

  length = y2 - y1;
  dxBC = _div((x2 - x1), length);
  duBC = _div((u2 - u1), length);
  dvBC = _div((v2 - v1), length);
  dsBC = _div((s2 - s1), length);
  dtBC = _div((t2 - t1), length);

  u8 flags = 0;

  if (dxAB < dxAC)
  {
    flags |= SWAPPED;

    _swap(dxAB, dxAC);
    _swap(duAB, duAC);
    _swap(dvAB, dvAC);
    _swap(dsAB, dsAC);
    _swap(dtAB, dtAC);
  }

  x2 = x1;
  u2 = u1;
  v2 = v1;
  s2 = s1;
  t2 = t1;
  x1 = x0;
  u1 = u0;
  v1 = v0;    
  s1 = s0;
  t1 = t0;

  if (y0 < 0)
  {
    int overclip = y0;

    x0 -= overclip * dxAC;
    x1 -= overclip * dxAB;
    u0 -= overclip * duAC;
    u1 -= overclip * duAB;
    v0 -= overclip * dvAC;
    v1 -= overclip * dvAB;
    s0 -= overclip * dsAC;
    s1 -= overclip * dsAB;
    t0 -= overclip * dtAC;
    t1 -= overclip * dtAB;

    y0 = 0;
  }

  int dy, b0, b1, du, dv, ds, dt;

  dy = y1 - y0;
  b0 = x0 + (dy * dxAC);
  b1 = x1 + (dy * dxAB);
  length = ((b1 - b0) >> 16);

  b0 = u0 + (dy * duAC);
  b1 = u1 + (dy * duAB);
  du = _div(((b1 - b0) << 8), length) >> 8;

  b0 = v0 + (dy * dvAC);
  b1 = v1 + (dy * dvAB);
  dv = _div(((b1 - b0) << 8), length) >> 8;

  b0 = s0 + (dy * dsAC);
  b1 = s1 + (dy * dsAB);
  ds = _div(((b1 - b0) << 8), length) >> 8;

  b0 = t0 + (dy * dtAC);
  b1 = t1 + (dy * dtAB);
  dt = _div(((b1 - b0) << 8), length) >> 8;  

	u16 *line;
  
  int stride, n, y, tu, tv, ts, tt;

  if (y1 >= 0)
  {
    line = (u16*)(g_currentBuffer + 120 * y0);

    y = y1 - y0;

    u16 texelOffset;
    u16 up, dw, le, ri, tc;
    u8 bu, bv, sh;

    do 
    {
      u16* endx = (u16*)(line + (x1 >> 17));
      u16 *curx = (u16*)(line + (x0 >> 17));

      tu = u0;
      tv = v0;
      ts = s0;
      tt = t0; 

      #define PLOT \
      { \
        texelOffset = ((tu >> 8) & 0xFF) + (((tv >> 8) & 0xFF) << 8); \
        up = (texelOffset - (128 << 3)) & 0x3FFF; \
        dw = (texelOffset + (128 << 3)) & 0x3FFF; \
        le = (texelOffset - (001 << 3)) & 0x3FFF; \
        ri = (texelOffset + (001 << 3)) & 0x3FFF; \
        bu = (((bumpmap[le] << 3) - (bumpmap[ri] << 3)) >> 2) + 63; \
        bv = (((bumpmap[up] << 3) - (bumpmap[dw] << 3)) >> 2) + 63; \
        sh = (phong_lut[(((((bu << 4) - ts) >> 8) & 0xFF) + (((((bv << 4) - tt) >> 8) & 0xFF) << 8)) & 0x3FFF]) >> 5; \
        sh = (max(1, sh)) * 32; \
        tc = texture[(((tu >> 8) & 0xFF) + (((tv >> 8) & 0xFF) << 8)) & 0x3FFF] - g_paletteOffset + sh; \
        tu += du; tv += dv; ts += ds; tt += dt; \
        tc |= (texture[(((tu >> 8) & 0xFF) + (((tv >> 8) & 0xFF) << 8)) & 0x3FFF] - g_paletteOffset + sh) << 8; \
        tu += du; tv += dv; ts += ds; tt += dt; \
        *curx++ = tc; \
      }

      stride = endx - curx + 2;
      n = (stride + 7) >> 3;

      switch (stride & 7)
      {
        case 0: do { PLOT;   
        case 7: PLOT;
        case 6: PLOT;
        case 5: PLOT; 
        case 4: PLOT;   
        case 3: PLOT;
        case 2: PLOT;
        case 1: PLOT;  
        } while (--n); 
      }

      x0 += dxAC;
      x1 += dxAB;
      u0 += duAC;
      v0 += dvAC;
      s0 += dsAC;
      t0 += dtAC;

      line += 120;

    } while (--y);
  }

  if (flags & SWAPPED)
  {
    x0 = x2;
    u0 = u2;
    v0 = v2;
    s0 = s2;
    t0 = t2;

    u1 += duAB * dy;
    v1 += dvAB * dy;
    s1 += dsAB * dy;
    t1 += dtAB * dy;

    dxAC = dxBC;
    duAC = duBC;
    dvAC = dvBC;
    dsAC = dsBC;
    dtAC = dtBC;

    dxBC = dxAB;
    duBC = duAB;
    dvBC = dvAB;
    dsBC = dsAB;
    dtBC = dtAB;
  }
  else
  {
    x1 = x2;
    u1 = u2;
    v1 = v2;
    s1 = s2;
    t1 = t2;
  }
          
  if (y1 < 0)
  {
    if (flags & SWAPPED)
    {
      int overclip = y1;

      x0 -= overclip * dxAC;
      x1 -= (y0 - 0) * dxBC;
      u0 -= overclip * duAC;
      v0 -= overclip * dvAC;
      s0 -= overclip * dsAC;
      t0 -= overclip * dtAC;
    }
    else
    {
      int overclip = y2;

      x0 -= overclip * dxAC;
      x1 -= (y1 - 0) * dxBC;
      u0 -= overclip * duAC;
      v0 -= overclip * dvAC;
      s0 -= overclip * dsAC;
      t0 -= overclip * dtAC;
    }

    y1 = 0;
  }
  
  line = (u16*)(g_currentBuffer + 120 * y1);

  y = y2 - y1;

  u16 texelOffset;
  u16 up, dw, le, ri, tc;
  u8 bu, bv, sh;  

  do
  {
    u16* endx = (u16*)(line + (x1 >> 17));
    u16 *curx = (u16*)(line + (x0 >> 17));

    tu = u0;
    tv = v0;
    ts = s0;
    tt = t0;

    #define PLOT \
    { \
      texelOffset = ((tu >> 8) & 0xFF) + (((tv >> 8) & 0xFF) << 8); \
      up = (texelOffset - (128 << 3)) & 0x3FFF; \
      dw = (texelOffset + (128 << 3)) & 0x3FFF; \
      le = (texelOffset - (001 << 3)) & 0x3FFF; \
      ri = (texelOffset + (001 << 3)) & 0x3FFF; \
      bu = (((bumpmap[le] << 3) - (bumpmap[ri] << 3)) >> 2) + 63; \
      bv = (((bumpmap[up] << 3) - (bumpmap[dw] << 3)) >> 2) + 63; \
      sh = (phong_lut[(((((bu << 4) - ts) >> 8) & 0xFF) + (((((bv << 4) - tt) >> 8) & 0xFF) << 8)) & 0x3FFF]) >> 5; \
      sh = (max(1, sh)) * 32; \
      tc = texture[(((tu >> 8) & 0xFF) + (((tv >> 8) & 0xFF) << 8)) & 0x3FFF] - g_paletteOffset + sh; \
      tu += du; tv += dv; ts += ds; tt += dt; \
      tc |= (texture[(((tu >> 8) & 0xFF) + (((tv >> 8) & 0xFF) << 8)) & 0x3FFF] - g_paletteOffset + sh) << 8; \
      tu += du; tv += dv; ts += ds; tt += dt; \
      *curx++ = tc; \
    }

    stride = endx - curx + 2;
    n = (stride + 7) >> 3;
    
    switch (stride & 7)
    {
      case 0: do { PLOT;   
      case 7: PLOT;
      case 6: PLOT;
      case 5: PLOT; 
      case 4: PLOT;   
      case 3: PLOT;
      case 2: PLOT;
      case 1: PLOT;  
      } while (--n); 
    }
      
    x0 += dxAC;
    x1 += dxBC;
    u0 += duAC;
    v0 += dvAC;
    s0 += dsAC;
    t0 += dtAC;

    line += 120;

  } while (--y);
}

void SwapFace(KDFace *a, KDFace * b)
{
  KDFace t = *a; 
  *a = *b; 
  *b = t;
}

void IWRAM_CODE SwapIndices(u16 *a, u16 *b, u16 l, u16 r)
{
  u16 t = *a;
  *a = *b;
  *b = t;

  t = drawOrder[l];
  drawOrder[l] = drawOrder[r];
  drawOrder[r] = t;
}

void IWRAM_CODE QuickSort(u16 *list, u16 start, u16 end)
{
  u16 z, l, r;

  if (end > start + 1)
  {
    z = list[start];
    l = start + 1;
    r = end;

    while (l < r)
    {
      if (list[l] > z)
      { 
        l++;
      }
      else
      {
        r--;
        SwapIndices(&list[l], &list[r], l, r);
      }
    }
    l--;
    SwapIndices(&list[l], &list[start], l, start);
    QuickSort(list, start, l);
    QuickSort(list, r, end);
  }
} 

#define LUTSIZE 255
void Render(KDModel *model)
{
  int i, nx, ny, nz, normal;

  Matrix3x3 m;
  Rotate(m, model->m_rotation.x, model->m_rotation.y, model->m_rotation.z);

  int numFaces = 0;
  KDFace *idx = model->m_faces;
  while (idx != 0) 
  {
    KDFace face;
    for (i = 0; i < 3; i++)
    {
      face.m_vertices[i].m_position.x = idx->m_vertices[i].m_position.x;
      face.m_vertices[i].m_position.y = idx->m_vertices[i].m_position.y;
      face.m_vertices[i].m_position.z = idx->m_vertices[i].m_position.z;
    }

    // rotate
    for (i = 0; i < 3; i ++)    
    { 
      nx = (m[0][0] * face.m_vertices[i].m_position.x + m[0][1] * face.m_vertices[i].m_position.y + m[0][2] * face.m_vertices[i].m_position.z) >> 8;
      ny = (m[1][0] * face.m_vertices[i].m_position.x + m[1][1] * face.m_vertices[i].m_position.y + m[1][2] * face.m_vertices[i].m_position.z) >> 8;
      nz = (m[2][0] * face.m_vertices[i].m_position.x + m[2][1] * face.m_vertices[i].m_position.y + m[2][2] * face.m_vertices[i].m_position.z) >> 8;

      face.m_vertices[i].m_position.x = nx;
      face.m_vertices[i].m_position.y = ny;
      face.m_vertices[i].m_position.z = nz;


#if 1
      // Pseudo-normal approximation looks good enough for convex object
      nx = (nx) + 64;//(_div(nx << 8, len)) + 63;
      ny = (ny >> 1) + 32;//(_div(nx << 8, len)) + 63;

#else
      nx = (nx >> 1) + 64;//(_div(nx << 8, len)) + 63;
      ny = (ny >> 2) + 32;//(_div(nx << 8, len)) + 63;
#endif

      face.m_vertices[i].s = nx;
      face.m_vertices[i].t = ny;
    }

    // Translate
    for (i = 0; i < 3; i++)
    {
      face.m_vertices[i].m_position.x += model->m_position.x;
      face.m_vertices[i].m_position.y += model->m_position.y;
      face.m_vertices[i].m_position.z += model->m_position.z;
    }

    // Project
    for (i = 0; i < 3; i++)
    {
      face.m_vertices[i].m_position.x = ((face.m_vertices[i].m_position.x * (face.m_vertices[i].m_position.z + 256)) >> 8);
      face.m_vertices[i].m_position.y = ((face.m_vertices[i].m_position.y * (face.m_vertices[i].m_position.z + 256)) >> 8);
    }

    // Calculate normal for culling and lighting
    normal = ((face.m_vertices[1].m_position.x - face.m_vertices[0].m_position.x) * 
              (face.m_vertices[2].m_position.y - face.m_vertices[1].m_position.y) - 
              (face.m_vertices[1].m_position.y - face.m_vertices[0].m_position.y) * 
              (face.m_vertices[2].m_position.x - face.m_vertices[1].m_position.x));  

    face.m_normal = normal;

    // Add visible faces to the list
    if (model->m_flags & MODEL_BACKFACECULLING)
    {     
      if (normal > 0)
      {           
        transformedFaces[numFaces].m_normal = face.m_normal;
        //transformedFaces[numFaces].m_depth = face.m_depth;
        transformedFaces[numFaces].m_vertices[0].m_position.x = face.m_vertices[0].m_position.x + HALFWIDTH;
        transformedFaces[numFaces].m_vertices[0].m_position.y = face.m_vertices[0].m_position.y + HALFHEIGHT;
        transformedFaces[numFaces].m_vertices[1].m_position.x = face.m_vertices[1].m_position.x + HALFWIDTH;
        transformedFaces[numFaces].m_vertices[1].m_position.y = face.m_vertices[1].m_position.y + HALFHEIGHT;
        transformedFaces[numFaces].m_vertices[2].m_position.x = face.m_vertices[2].m_position.x + HALFWIDTH;
        transformedFaces[numFaces].m_vertices[2].m_position.y = face.m_vertices[2].m_position.y + HALFHEIGHT;
        transformedFaces[numFaces].m_vertices[0].u = idx->m_vertices[0].u;
        transformedFaces[numFaces].m_vertices[0].v = idx->m_vertices[0].v;
        transformedFaces[numFaces].m_vertices[1].u = idx->m_vertices[1].u;
        transformedFaces[numFaces].m_vertices[1].v = idx->m_vertices[1].v;
        transformedFaces[numFaces].m_vertices[2].u = idx->m_vertices[2].u;
        transformedFaces[numFaces].m_vertices[2].v = idx->m_vertices[2].v;        
        transformedFaces[numFaces].m_vertices[0].s = face.m_vertices[0].s;
        transformedFaces[numFaces].m_vertices[0].t = face.m_vertices[0].t;
        transformedFaces[numFaces].m_vertices[1].s = face.m_vertices[1].s;
        transformedFaces[numFaces].m_vertices[1].t = face.m_vertices[1].t;
        transformedFaces[numFaces].m_vertices[2].s = face.m_vertices[2].s;
        transformedFaces[numFaces].m_vertices[2].t = face.m_vertices[2].t;             
        transformedFaces[numFaces].m_next = NULL;        

        // Calculate depth for sorting based on average Z
        depthBuffer[numFaces] = face.m_vertices[0].m_position.z + face.m_vertices[1].m_position.z + face.m_vertices[2].m_position.z;
        
        numFaces++;
      }
    }
    idx = idx->m_next;
  }

  // Index triangles
  for (i = 0; i < numFaces; i++)
    drawOrder[i] = i;

  // Rearrange indices based on average depth per triangle
  if (model->m_flags & MODEL_SORTFACES)
  {
    QuickSort(depthBuffer, 0, numFaces);
  }

  u16 faceIndex;

  // Draw back to front
  for (i = 0; i < numFaces; i++) 
  {
    faceIndex = drawOrder[i];

    if (model->m_flags & MODEL_TEXTURED)
    {
      normal = transformedFaces[faceIndex].m_normal >> 2;

      TextureTriangle(&transformedFaces[faceIndex], g_currentTexture, g_currentBumpMap, 0);
		}
  }
}

void LoadModel(KDModel *model, KDFace f[], const s16 positions[], const s16 normals[], const u16 texcoords[], int numVertices, int scale, int textured)
{
  int i, j;
  int k = 0, l = 0;

  for (i = 0; i < numVertices / 3; i++)
  {
    for (j = 0; j < 3; j++) 
    {
      f[i].m_vertices[j].m_position.x = (int)(positions[k++] * scale) >> 16;
      f[i].m_vertices[j].m_position.y = (int)(positions[k++] * scale) >> 16;
      f[i].m_vertices[j].m_position.z = (int)(positions[k++] * scale) >> 16;

      f[i].m_vertices[j].u = texcoords[l++];
      f[i].m_vertices[j].v = texcoords[l++];  

      f[i].m_vertices[j].s = (normals[k - 3] + 255) >> 2;
      f[i].m_vertices[j].t = (normals[k - 2] + 255) >> 2;        
    }
  }

  model->m_faces = &f[0];
  int numFaces = numVertices / 3;

  for (j = 0; j <numFaces - 1; j++)
    f[j].m_next = &f[j+1];

  f[numFaces-1].m_next = 0;

  // Default settings
  model->m_rotation.x = 0;
  model->m_rotation.y = 0;
  model->m_rotation.z = 0;
  model->m_position.x = 0;
  model->m_position.y = 0;
  model->m_position.z = 0;
  model->m_flags = MODEL_BACKFACECULLING | MODEL_SORTFACES | MODEL_TEXTURED;
}


const s8 points[] = {
        -1.0, -1.0, -1.0,
        -1.0, -1.0,  1.0,
        -1.0,  1.0,  1.0,
         1.0,  1.0, -1.0,
        -1.0, -1.0, -1.0,
        -1.0,  1.0, -1.0,
         1.0, -1.0,  1.0,
        -1.0, -1.0, -1.0,
         1.0, -1.0, -1.0,
         1.0,  1.0, -1.0,
         1.0, -1.0, -1.0,
        -1.0, -1.0, -1.0,
        -1.0, -1.0, -1.0,
        -1.0,  1.0,  1.0,
        -1.0,  1.0, -1.0,
         1.0, -1.0,  1.0,
        -1.0, -1.0,  1.0,
        -1.0, -1.0, -1.0,
        -1.0,  1.0,  1.0,
        -1.0, -1.0,  1.0,
         1.0, -1.0,  1.0,
         1.0,  1.0,  1.0,
         1.0, -1.0, -1.0,
         1.0,  1.0, -1.0,
         1.0, -1.0, -1.0,
         1.0,  1.0,  1.0,
         1.0, -1.0,  1.0,
         1.0,  1.0,  1.0,
         1.0,  1.0, -1.0,
        -1.0,  1.0, -1.0,
         1.0,  1.0,  1.0,
        -1.0,  1.0, -1.0,
        -1.0,  1.0,  1.0,
         1.0,  1.0,  1.0,
        -1.0,  1.0,  1.0,
         1.0, -1.0,  1.0
    };  


const u8 textureCoords[] = {
        0, 0,
        1, 0,
        0, 1,
        0, 1,
        1, 0,
        1, 1,
        0, 0,
        1, 0,
        0, 1,
        0, 1,
        1, 0,
        1, 1,
        0, 0,
        1, 0,
        0, 1,
        0, 1,
        1, 0,
        1, 1,
        0, 0,
        1, 0,
        0, 1,
        0, 1,
        1, 0,
        1, 1,
        0, 0,
        1, 0,
        0, 1,
        0, 1,
        1, 0,
        1, 1,
        0, 0,
        1, 0,
        0, 1,
        0, 1,
        1, 0,
        1, 1,
    };
/*

const u8 textureCoords[] = {
        0, 0,
        1, 0,
        0, 1,
        0, 1,
        1, 0,
        1, 1,
        0, 0,
        1, 0,
        0, 1,
        0, 1,
        1, 0,
        1, 1,
        0, 0,
        1, 0,
        0, 1,
        0, 1,
        1, 0,
        1, 1,
        0, 0,
        1, 0,
        0, 1,
        0, 1,
        1, 0,
        1, 1,
        0, 0,
        1, 0,
        0, 1,
        0, 1,
        1, 0,
        1, 1,
        0, 0,
        1, 0,
        0, 1,
        0, 1,
        1, 0,
        1, 1,
    };
*/
void make_cube(KDModel *model, KDFace f[], u16 size, int textured) 
{
  int i, j;
  int hsize = size>>1;
  int numtris = 12;

  model->m_faces = &f[0];
  for (j = 0; j < numtris-1; j++)
  {
    f[j].m_next = &f[j+1];
  }

  int uvindex = 0;
  int idx = 0;
  for (j = 0; j < numtris; j++)
  for (i = 0; i < 3; i++)
  {
    f[j].m_vertices[i].m_position.x = points[idx + 0] * hsize;
    f[j].m_vertices[i].m_position.y = points[idx + 1] * hsize;
    f[j].m_vertices[i].m_position.z = points[idx + 2] * hsize;
    f[j].m_vertices[i].u = textureCoords[uvindex] * 128;
    f[j].m_vertices[i].v = textureCoords[uvindex + 1] * 64;

    // Compute pseudo-normals for lighting
    f[j].m_vertices[i].s = (points[idx + 0] * 63) + 64;
    f[j].m_vertices[i].t = (points[idx + 1] * 63) + 64;  

    uvindex+=2;
    idx += 3;
  }

  f[numtris-1].m_next = 0;
  // Default settings

  model->m_rotation.x = 0;
  model->m_rotation.y = 0;
  model->m_rotation.z = 0;
  model->m_position.x = 0;
  model->m_position.y = 0;
  model->m_position.z = 0;
  model->m_flags = MODEL_BACKFACECULLING | MODEL_TEXTURED;
}
