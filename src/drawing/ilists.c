#include "medit.h"
#include "extern.h"
#include "sproto.h"

#define BASETR   0.2
#define NBCOL    9


/* build lists for iso-surfaces */
GLuint listTriaIso(pScene sc,Mesh*mesh) {
  FILE      *out;
  GLuint     dlist = 0;
  pTriangle  pt;
  pPoint     p0,p1;
  pMaterial  pm;
  pSolution  ps0,ps1;
  double     rgb[3];
  float      iso,c[3][3],cc,kc;
  int        m,k,i,l,l1,nc,ncol,zi;
  char      *ptr,data[256];
  static double hsv[3]  = { 0.0, 1.0, 0.9 };
  static int    idir[5] = {0,1,2,0,1};

  /* default */
  if ( !mesh->nt || !mesh->nbb || mesh->typage == 1 )  return(0);
  if ( ddebug ) printf("create iso-values map list / TRIA\n");
  if ( egal(sc->iso.val[0],sc->iso.val[MAXISO-1]) )  return(0);

  /* create display list */
  dlist = glGenLists(1);
  glNewList(dlist,GL_COMPILE);
  if ( glGetError() )  return(0);

  if ( ddebug ) {
    strcpy(data,mesh->name);
    ptr = strstr(data,".mesh");
    if ( ptr ) *ptr = '\0';
    strcat(data,".data");
    out = fopen(data,"w");
    fprintf(stdout,"  %% Opening %s\n",data);
  }
  
  /* build list */
  glBegin(GL_LINES);
  ncol = NBCOL;
  for (i=0; i<=ncol*(MAXISO-1); i++) {
    if ( i < ncol*(MAXISO-1) ) {
      l   = i / ncol;
      kc  = (i % ncol) / (float)ncol;
      iso = sc->iso.val[l]*(1.0-kc)+sc->iso.val[l+1]*kc;
      hsv[0] = sc->iso.col[l]*(1.0-kc)+sc->iso.col[l+1]*kc;
    }
    else {
      iso    = sc->iso.val[MAXISO-1];
      hsv[0] = sc->iso.col[MAXISO-1];
    }
    /* convert to color */
    hsvrgb(hsv,rgb);
    glColor3dv(rgb);

    for (m=0; m<sc->par.nbmat; m++) {
      pm = &sc->material[m];
      k  = pm->depmat[LTria];
      if ( !k || pm->flag )  continue;

      while ( k != 0 ) {
        pt = &mesh->tria[k];
        if ( !pt->v[0] ) {
          k = pt->nxt;
          continue;
        }

        /* analyze edges */
        nc = 0;
        zi = 0;
        memset(c,0,9*sizeof(float));
        for (l=0; l<3; l++) {
          l1  = idir[l+1];
          p0  = &mesh->point[pt->v[l]];
          p1  = &mesh->point[pt->v[l1]];
          ps0 = &mesh->sol[pt->v[l]];
          ps1 = &mesh->sol[pt->v[l1]];
          if ( (ps0->bb > iso && ps1->bb < iso) || (ps0->bb < iso && ps1->bb > iso) ) {
            cc = fabs((iso-ps0->bb) / (ps1->bb-ps0->bb));
            c[nc][0] = p0->c[0] + cc*(p1->c[0]-p0->c[0]);
            c[nc][1] = p0->c[1] + cc*(p1->c[1]-p0->c[1]);
            if ( mesh->dim == 3)  c[nc][2] = p0->c[2]+cc*(p1->c[2]-p0->c[2]);
            nc++;
          }
          else if ( ps0->bb == iso  && ps1->bb == iso ) {
            if ( mesh->dim == 2 ) {
              c[0][0] = p0->c[0];  c[0][1] = p0->c[1];
              c[1][0] = p1->c[0];  c[1][1] = p1->c[1];
            }
            else {
              c[0][0] = p0->c[0];  c[0][1] = p0->c[1];  c[0][2] = p0->c[2];
              c[1][0] = p1->c[0];  c[1][1] = p1->c[1];  c[1][2] = p1->c[2];
            }
            nc = 2;
            break;
          }
          else {
            if ( ps0->bb == iso  && zi != pt->v[l] ) {
              if ( mesh->dim == 2 ) {
                c[nc][0] = p0->c[0];  c[nc][1] = p0->c[1];
              }
              else {
                c[nc][0] = p0->c[0];  c[nc][1] = p0->c[1];  c[nc][2] = p0->c[2];
              }
              nc++;
              zi = pt->v[l];
            }
            if ( ps1->bb == iso && zi != pt->v[l1] ) {
              if ( mesh->dim == 2 ) {
                c[nc][0] = p1->c[0];  c[nc][1] = p1->c[1];
              }
              else {
                c[nc][0] = p1->c[0];  c[nc][1] = p1->c[1];  c[nc][2] = p1->c[2];
              }  
              nc++;
              zi = pt->v[l1];
            } 
          }
        }
        if ( nc == 2 ) {
          if ( mesh->dim == 2 ) {
            glVertex2f(c[0][0],c[0][1]);
            glVertex2f(c[1][0],c[1][1]);
            if ( ddebug ) {
              fprintf(out,"%f %f\n",c[0][0]+mesh->xtra,c[0][1]+mesh->ytra);
              fprintf(out,"%f %f\n",c[1][0]+mesh->xtra,c[1][1]+mesh->ytra);
            }
          }
          else {
            glVertex3f(c[0][0],c[0][1],c[0][2]);
            glVertex3f(c[1][0],c[1][1],c[1][2]);
            if ( ddebug ) {
              fprintf(out,"%f %f %f\n",c[0][0]+mesh->xtra,c[0][1]+mesh->ytra,c[0][2]+mesh->ztra);
              fprintf(out,"%f %f %f\n",c[1][0]+mesh->xtra,c[1][1]+mesh->ytra,c[1][2]+mesh->ztra);
            }
          }
          if ( ddebug )  fprintf(out,"\n");
        }

        k = pt->nxt;
      }
    }
  }
  glEnd();
  glEndList();

  if ( ddebug )  fclose(out);
  return(dlist);
}

/* build lists for isolines (isovalues given in file) */
GLuint listTriaIso2(pScene sc,Mesh*mesh) {
  FILE      *in;
  GLuint     dlist = 0;
  pTriangle  pt;
  pPoint     p0,p1;
  pMaterial  pm;
  pSolution  ps0,ps1;
  double     rgb[3];
  float     *col,iso,c[3][3],cc,kc;
  int        m,k,i,l,l1,nc,ncol,zi;
  char      *ptr,data[256];
  static double hsv[3]  = { 0.0, 1.0, 0.9 };
  static int    idir[5] = {0,1,2,0,1};

  /* default */
  if ( !mesh->nt || !mesh->nbb || mesh->typage == 1 )  return(0);
  if ( ddebug ) printf("create iso-values map list / TRIA\n");
  if ( egal(sc->iso.val[0],sc->iso.val[MAXISO-1]) )  return(0);

  /* read isovalues in file */
  strcpy(data,mesh->name);
  ptr = strstr(data,".mesh");
  if ( ptr ) *ptr = '\0';
  strcat(data,".data");
  in = fopen(data,"r");
  if ( !in ) {
    fprintf(stdout," No data file %s found\n",data);
    return(0);
  }
  fprintf(stdout,"  %% Opening %s\n",data);
  fscanf(in,"%d",&ncol);
  col = (float*)calloc(ncol+2,sizeof(double));
  assert(col);
  for (i=0; i<ncol; i++) {
    fscanf(in,"%f",&col[i]);
  }
  fclose(in);

  /* create display list */
  dlist = glGenLists(1);
  glNewList(dlist,GL_COMPILE);
  if ( glGetError() )  return(0);

  /* build list */
  glBegin(GL_LINES);
  for (i=0; i<ncol; i++) {
    iso = col[i];

    /* convert to color */
    for (l=0; l<MAXISO; l++)
      if ( sc->iso.val[l] > iso )  break;
    if ( l == 0 || l == MAXISO )
      hsv[0] = sc->iso.col[l];
    else {
      kc = (sc->iso.val[l]-iso) / (sc->iso.val[l] - sc->iso.val[l-1]); 
      hsv[0] = sc->iso.col[l]*(1.0-kc) + sc->iso.col[l-1]*kc;
      hsvrgb(hsv,rgb);
    }
    glColor3dv(rgb);

    for (m=0; m<sc->par.nbmat; m++) {
      pm = &sc->material[m];
      k  = pm->depmat[LTria];
      if ( !k || pm->flag )  continue;
      while ( k != 0 ) {
        pt = &mesh->tria[k];
        if ( !pt->v[0] ) {
          k = pt->nxt;
          continue;
        }

        /* analyze edges */
        nc = 0;
        zi = 0;
        memset(c,0,9*sizeof(float));
        for (l=0; l<3; l++) {
          l1  = idir[l+1];
          p0  = &mesh->point[pt->v[l]];
          p1  = &mesh->point[pt->v[l1]];
          ps0 = &mesh->sol[pt->v[l]];
          ps1 = &mesh->sol[pt->v[l1]];
          if ( (ps0->bb > iso && ps1->bb < iso) || (ps0->bb < iso && ps1->bb > iso) ) {
            cc = fabs((iso-ps0->bb) / (ps1->bb-ps0->bb));
            c[nc][0] = p0->c[0] + cc*(p1->c[0]-p0->c[0]);
            c[nc][1] = p0->c[1] + cc*(p1->c[1]-p0->c[1]);
            if ( mesh->dim == 3)  c[nc][2] = p0->c[2]+cc*(p1->c[2]-p0->c[2]);
            nc++;
          }
          else if ( ps0->bb == iso  && ps1->bb == iso ) {
            c[0][0] = p0->c[0];  c[0][1] = p0->c[1];
            c[1][0] = p1->c[0];  c[1][1] = p1->c[1];
            nc = 2;
            break;
          }
          else {
            if ( ps0->bb == iso  && zi != pt->v[l] ) {
              c[nc][0] = p0->c[0];  c[nc][1] = p0->c[1];
              nc++;
              zi = pt->v[l];
            }
            if ( ps1->bb == iso && zi != pt->v[l1] ) {
              c[nc][0] = p1->c[0];  c[nc][1] = p1->c[1];
              nc++;
              zi = pt->v[l1];
            } 
          }
        }
        if ( nc == 2 ) {
          glVertex2f(c[0][0],c[0][1]);
          glVertex2f(c[1][0],c[1][1]);
        }
        k = pt->nxt;
      }
    }
  }
  glEnd();
  glEndList();
  return(dlist);
}

/* build lists for iso-surfaces */
GLuint listQuadIso(pScene sc,Mesh*mesh) {
  GLuint     dlist = 0;
  pQuad      pq;
  pPoint     p0,p1;
  pMaterial  pm;
  pSolution  ps0,ps1;
  double     rgb[3];
  float      iso,cx,cy,cz,cc,kc;
  int        m,k,i,l,l1,ncol;
  static double hsv[3]   = { 0.0f, 1.0f, 0.8f };
  static int idir[6] = {0,1,2,3,0,1};

  /* default */
  if ( !mesh->nq || !mesh->nbb || mesh->typage == 1 )  return(0);
  if ( ddebug ) printf("create iso-values map list / QUAD\n");
  if ( egal(sc->iso.val[0],sc->iso.val[MAXISO-1]) )  return(0);

  /* build display list */
  dlist = glGenLists(1);
  glNewList(dlist,GL_COMPILE);
  if ( glGetError() )  return(0);

  /* build list */
  glBegin(GL_LINES);
  ncol = NBCOL;
  for (i=0; i<=ncol*(MAXISO-1); i++) {
    if ( i < ncol*(MAXISO-1) ) {
      l   = i / ncol;
      kc  = (i % ncol) / (float)ncol;
      iso = sc->iso.val[l]*(1.0-kc)+sc->iso.val[l+1]*kc;
      hsv[0] = sc->iso.col[l]*(1.0-kc)+sc->iso.col[l+1]*kc;
    }
    else {
      iso    = sc->iso.val[MAXISO-1];
      hsv[0] = sc->iso.col[MAXISO-1];
    }
    
    hsvrgb(hsv,rgb);
    glColor3dv(rgb);

    for (m=0; m<sc->par.nbmat; m++) {
      pm = &sc->material[m];
      k  = pm->depmat[LQuad];
      if ( !k || pm->flag )  continue;

      while ( k != 0 ) {
        pq = &mesh->quad[k];
        if ( !pq->v[0] ) {
          k = pq->nxt;
          continue;
        }

        /* analyze edges */
        for (l=0; l<4; l++) {
          p0  = &mesh->point[pq->v[l]];
          ps0 = &mesh->sol[pq->v[l]];
          l1  = idir[l+1];
          p1  = &mesh->point[pq->v[l1]];
          ps1 = &mesh->sol[pq->v[l1]];
          if ( (ps0->bb > iso && ps1->bb <= iso) ||
               (ps0->bb < iso && ps1->bb >= iso) ) {
            cc = 0.0f;
            if ( fabs(ps1->bb-ps0->bb) > 0.0f )
              cc = (iso-ps0->bb) / (ps1->bb-ps0->bb);
            if ( cc == 0.0f || cc == 1.0f )  continue;
            cx = p0->c[0]+cc*(p1->c[0]-p0->c[0]);
            cy = p0->c[1]+cc*(p1->c[1]-p0->c[1]);
            if ( mesh->dim == 2 )
              glVertex2f(cx,cy);
            else {
              cz = p0->c[2]+cc*(p1->c[2]-p0->c[2]);
              glVertex3f(cx,cy,cz);
            }
          }
        }
        k = pq->nxt;
      }
    }
  }
  glEnd();
  glEndList();
  return(dlist);
}

/* build lists for iso-surfaces */
GLuint listTetraIso(pScene sc,Mesh*mesh) {
  FILE      *outv,*outf;
  GLuint     dlist = 0;
  pTetra     pt;
  pPoint     p0,p1;
  pMaterial  pm;
  pSolution  ps0,ps1;
  double     delta,rgb[4],d,ax,ay,az,bx,by,bz,cc;
  float      iso,n[3],cx[4],cy[4],cz[4];
  int        m,k,k1,k2,i,l,pos[4],neg[4],nbpos,nbneg,nbnul,nv,nf,idxf,posv,posf;
  char       data[64];
  static double hsv[3]   = { 0.0, 1.0, 0.8 };
  static int tn[4] = {0,0,1,1};
  static int tp[4] = {0,1,1,0};

  /* default */
  if ( !mesh->ntet || !mesh->nbb || mesh->typage == 1 )  return(0);
  if ( ddebug ) printf("create iso-values map list / TETRA\n");
  if ( egal(sc->iso.val[0],sc->iso.val[MAXISO-1]) )  return(0);
  delta = sc->iso.val[MAXISO-1] - sc->iso.val[0];

  /* build display list */
  dlist = glGenLists(1);
  glNewList(dlist,GL_COMPILE);
  if ( glGetError() )  return(0);

  /* build list */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE);

  glBegin(GL_TRIANGLES);
  for (i=MAXISO-1; i>=0; i--) {
    iso = sc->iso.val[i];

    if ( ddebug ) {
      idxf = 1;
      sprintf(data,"vertex%d.%d.mesh",i,idxf);
			outv = fopen(data,"w");
      fprintf(outv,"MeshVersionFormatted 1\n Dimension\n 3\n\nVertices\n");
      posv = ftell(outv);
      fprintf(outv,"xxxxxxxx\n");

      sprintf(data,"faces%d.%d.mesh",i,idxf);
      outf = fopen(data,"w");
      fprintf(outf,"\n Triangles\n");
      posf = ftell(outf);
      fprintf(outf,"xxxxxxxx\n");
    }
    nv = nf = 0;

    /* base color */
    /*hsv[0] = 240.0f*(1.0f - (iso-sc->iso.val[0])/delta);*/
    hsv[0] = sc->iso.col[i];
    hsvrgb(hsv,rgb);
    rgb[0] = min(1.0,rgb[0]+BASETR);
    rgb[1] = min(1.0,rgb[1]+BASETR);
    rgb[2] = min(1.0,rgb[2]+BASETR);
    rgb[3] = BASETR + (float)(i-1)/(float)MAXISO*(1.0-BASETR);
    /*rgb[3] = 0.5; */  
    glColor4dv(rgb);

    if ( i == MAXISO-1 )  iso -= 0.001*fabs(iso)/delta;
    else if ( i == 0 )    iso += 0.001*fabs(iso)/delta;

    for (m=0; m<sc->par.nbmat; m++) {
      pm = &sc->material[m];
      k  = pm->depmat[LTets];
      if ( !k || pm->flag )  continue;

      while ( k != 0 ) {
        pt = &mesh->tetra[k];
        if ( !pt->v[0] ) {
          k = pt->nxt;
          continue;
        }

        /* analyze vertices */
        nbpos = nbneg = nbnul = 0;
        for (l=0; l<4; l++) {
          p0  = &mesh->point[pt->v[l]];
          ps0 = &mesh->sol[pt->v[l]];
          /*if ( ps0->bb < sc->iso.val[0] )  ps0->bb = sc->iso.val[0];*/
          
          if ( ps0->bb > iso )      pos[nbpos++] = l;
          else if ( ps0->bb < iso ) neg[nbneg++] = l;
          else                      nbnul++;
        }
        if ( nbneg == 4 || nbpos == 4 ) {
          k = pt->nxt;
          continue;
        }

        if ( nbneg == 2 && nbpos == 2 ) {
          for (l=0; l<4; l++) {
            k1  = neg[tn[l]];
            k2  = pos[tp[l]];
            p0  = &mesh->point[pt->v[k1]];
            p1  = &mesh->point[pt->v[k2]];
            ps0 = &mesh->sol[pt->v[k1]];
            ps1 = &mesh->sol[pt->v[k2]];
            cc = 0.0;
            if ( fabs(ps1->bb-ps0->bb) > 0.0 )
              cc = (iso-ps0->bb) / (ps1->bb-ps0->bb);
            cx[l] = p0->c[0]+cc*(p1->c[0]-p0->c[0]);
            cy[l] = p0->c[1]+cc*(p1->c[1]-p0->c[1]);
            cz[l] = p0->c[2]+cc*(p1->c[2]-p0->c[2]);
          }

          /* compute face normal */
          ax = cx[1]-cx[0]; ay = cy[1]-cy[0]; az = cz[1]-cz[0];
          bx = cx[2]-cx[0]; by = cy[2]-cy[0]; bz = cz[2]-cz[0];
          n[0] = ay*bz - az*by;
          n[1] = az*bx - ax*bz;
          n[2] = ax*by - ay*bx;
          d = n[0]*n[0] + n[1]*n[1] + n[2]*n[2];
          if ( d > 0.0 ) {
            d = 1.0 / sqrt(d);
            n[0] *= d;  
            n[1] *= d;  
            n[2] *= d;
          }
          glNormal3fv(n);
          glVertex3f(cx[0],cy[0],cz[0]);
          glVertex3f(cx[1],cy[1],cz[1]);
          glVertex3f(cx[2],cy[2],cz[2]);
          
          glNormal3fv(n);
          glVertex3f(cx[0],cy[0],cz[0]);
          glVertex3f(cx[2],cy[2],cz[2]);
          glVertex3f(cx[3],cy[3],cz[3]);

          if ( ddebug ) {
            fprintf(outv,"%f %f %f 0\n",cx[0]+mesh->xtra,cy[0]+mesh->ytra,cz[0]+mesh->ztra);
            fprintf(outv,"%f %f %f 0\n",cx[1]+mesh->xtra,cy[1]+mesh->ytra,cz[1]+mesh->ztra);
            fprintf(outv,"%f %f %f 0\n",cx[2]+mesh->xtra,cy[2]+mesh->ytra,cz[2]+mesh->ztra);
            fprintf(outv,"%f %f %f 0\n",cx[3]+mesh->xtra,cy[3]+mesh->ytra,cz[3]+mesh->ztra);
            
            fprintf(outf,"%d %d %d 0\n",nv+1,nv+2,nv+3);
            fprintf(outf,"%d %d %d 0\n",nv+1,nv+3,nv+4);
            nv+= 4;
            nf+= 2;

            if ( nf >= 2000000 ) {
              rewind(outv);
              printf("  Vertices %d   Triangles %d\n",nv,nf);
              fseek(outv,posv,0);
              fprintf(outv,"%8d\n",nv);
              fclose(outv);
              
              fprintf(outf,"\nEnd\n");
              rewind(outf);
              fseek(outf,posf,0);
              fprintf(outf,"%8d\n",nf);
              fclose(outf);
              nv = 0;
              nf = 0;
              
              idxf++;
              sprintf(data,"vertex%d.%d.mesh",i,idxf);
              outv = fopen(data,"w");
              fprintf(outv,"MeshVersionFormatted 1\n Dimension\n 3\n\nVertices\n");
              posv = ftell(outv);
              fprintf(outv,"xxxxxxxx\n");
              
              sprintf(data,"faces%d.%d.mesh",i,idxf);
              outf = fopen(data,"w");           
              fprintf(outf,"\nTriangles\n");
              posf = ftell(outf);
              fprintf(outf,"xxxxxxxx\n");   
            }
          }
          else {
            nv+= 4;
            nf+= 2;
          }
        }
        else if ( !nbnul ) {
          for (l=0; l<3; l++) {
            k1 = nbneg == 3 ? neg[l] : pos[l];
            k2 = nbneg == 3 ? pos[0] : neg[0];
            p0 = &mesh->point[pt->v[k1]];
            p1 = &mesh->point[pt->v[k2]];
            ps0 = &mesh->sol[pt->v[k1]];
            ps1 = &mesh->sol[pt->v[k2]];
            cc = 0.0;
            if ( fabs(ps1->bb-ps0->bb) > 0.0 ) 
              cc = (iso-ps0->bb) / (ps1->bb-ps0->bb);
            cx[l] = p0->c[0]+cc*(p1->c[0]-p0->c[0]);
            cy[l] = p0->c[1]+cc*(p1->c[1]-p0->c[1]);
            cz[l] = p0->c[2]+cc*(p1->c[2]-p0->c[2]);
          }
          /* compute face normal */
          ax = cx[1]-cx[0]; ay = cy[1]-cy[0]; az = cz[1]-cz[0];
          bx = cx[2]-cx[0]; by = cy[2]-cy[0]; bz = cz[2]-cz[0];
          n[0] = ay*bz - az*by;
          n[1] = az*bx - ax*bz;
          n[2] = ax*by - ay*bx;
          d = n[0]*n[0] + n[1]*n[1] + n[2]*n[2];
          if ( d > 0.0 ) {
            d = 1.0f / sqrt(d);
            n[0] *= d;
            n[1] *= d;
            n[2] *= d;
          }
          glNormal3fv(n);
          glVertex3f(cx[0],cy[0],cz[0]);
          glVertex3f(cx[1],cy[1],cz[1]);
          glVertex3f(cx[2],cy[2],cz[2]);

          if ( ddebug ) {
            fprintf(outv,"%f %f %f 0\n",cx[0]+mesh->xtra,cy[0]+mesh->ytra,cz[0]+mesh->ztra);
            fprintf(outv,"%f %f %f 0\n",cx[1]+mesh->xtra,cy[1]+mesh->ytra,cz[1]+mesh->ztra);
            fprintf(outv,"%f %f %f 0\n",cx[2]+mesh->xtra,cy[2]+mesh->ytra,cz[2]+mesh->ztra);
            
            fprintf(outf,"%d %d %d 0\n",nv+1,nv+2,nv+3);
            nv += 3;
            nf += 1;

            if ( nf >= 10000000 ) {
              printf("  Vertices %d   Triangles %d\n",nv,nf);
              rewind(outv);
              fseek(outv,posv,0);
              fprintf(outv,"%8d\n",nv);
              fclose(outv);
              
              fprintf(outf,"\nEnd\n");
              rewind(outf);
              fseek(outf,posf,0);
              fprintf(outf,"%8d\n",nf);
              fclose(outf);
              
              nv = 0;
              nf = 0;
              idxf++;
              sprintf(data,"vertex%d.%d.mesh",i,idxf);
              outv = fopen(data,"w");
              fprintf(outv,"MeshVersionFormatted 1\n Dimension\n 3\n\nVertices\n");
              posv = ftell(outv);
              fprintf(outv,"xxxxxxxx\n");
              
              sprintf(data,"faces%d.%d.mesh",i,idxf);
              outf = fopen(data,"w");
              fprintf(outf,"\nTriangles\n");
              posf = ftell(outf);
              fprintf(outf,"xxxxxxxx\n");   
            }
          }
          else {
            nv += 3;
            nf += 1;
          }
        }
        k = pt->nxt;
      }
    }

    if ( ddebug ) {
      printf("  Vertices %d   Triangles %d\n",nv,nf);
      rewind(outv);
      fseek(outv,posv,0);
      fprintf(outv,"%8d\n",nv);
      fclose(outv);

      fprintf(outf,"\nEnd\n");
      rewind(outf);
      fseek(outf,posf,0);
      fprintf(outf,"%8d\n",nf);
      fclose(outf);
    }

  }
  glEnd();
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);

  glEndList();
  return(dlist);
}


int tetraIsoPOVray(pScene sc,Mesh*mesh) {
  FILE      *isofil;
  pTetra     pt;
  pPoint     p0,p1;
  pMaterial  pm;
  pSolution  ps0,ps1;
  double     delta;
  float      iso,cx[4],cy[4],cz[4],cc;
  int        m,k,k1,k2,i,l,pos[4],neg[4],nbpos,nbneg,nbnul;
  char      *ptr,data[128];
  static int tn[4] = {0,0,1,1};
  static int tp[4] = {0,1,1,0};

  /* default */
  if ( !mesh->ntet || !mesh->nbb || mesh->typage == 1 )  return(0);
  if ( ddebug ) printf("create isosurfaces POVray\n");
  if ( egal(sc->iso.val[0],sc->iso.val[MAXISO-1]) )  return(0);
  delta = sc->iso.val[MAXISO-1] - sc->iso.val[0];

  strcpy(data,mesh->name);
  ptr = strstr(data,".mesh");
  if ( ptr )  ptr = '\0';
  strcat(data,".pov"); 
  if ( ddebug )  fprintf(stdout,"  Writing POVRay file %s\n",data);
  isofil = fopen(data,"w");
  if ( !isofil )  return(0);

  for (i=MAXISO-1; i>=0; i--) {
    iso = sc->iso.val[i];

    if ( i == MAXISO-1 )  iso -= 0.001*fabs(iso)/delta;
    else if ( i == 0 )    iso += 0.001*fabs(iso)/delta;

    fprintf(isofil,"\n#declare isosurf%d = mesh {\n",i);

    for (m=0; m<sc->par.nbmat; m++) {
      pm = &sc->material[m];
      k  = pm->depmat[LTets];
      if ( !k || pm->flag )  continue;

      while ( k != 0 ) {
        pt = &mesh->tetra[k];
        if ( !pt->v[0] ) {
          k = pt->nxt;
          continue;
        }

        /* analyze vertices */
        nbpos = nbneg = nbnul = 0;
        for (l=0; l<4; l++) {
          p0  = &mesh->point[pt->v[l]];
          ps0 = &mesh->sol[pt->v[l]];
          
          if ( ps0->bb > iso )      pos[nbpos++] = l;
          else if ( ps0->bb < iso ) neg[nbneg++] = l;
          else                      nbnul++;
        }
        if ( nbneg == 4 || nbpos == 4 ) {
          k = pt->nxt;
          continue;
        }

        if ( nbneg == 2 && nbpos == 2 ) {
          for (l=0; l<4; l++) {
            k1  = neg[tn[l]];
            k2  = pos[tp[l]];
            p0  = &mesh->point[pt->v[k1]];
            p1  = &mesh->point[pt->v[k2]];
            ps0 = &mesh->sol[pt->v[k1]];
            ps1 = &mesh->sol[pt->v[k2]];
            cc = 0.0f;
            if ( fabs(ps1->bb-ps0->bb) > 0.0f )
              cc = (iso-ps0->bb) / (ps1->bb-ps0->bb);
            cx[l] = p0->c[0]+cc*(p1->c[0]-p0->c[0]);
            cy[l] = p0->c[1]+cc*(p1->c[1]-p0->c[1]);
            cz[l] = p0->c[2]+cc*(p1->c[2]-p0->c[2]);
          }

    fprintf(isofil,"triangle {\n");
    fprintf(isofil,"  <%f,%f,%f>,\n",
    cx[0]+mesh->xtra,cy[0]+mesh->ytra,cz[0]+mesh->ztra);
    fprintf(isofil,"  <%f,%f,%f>,\n",
    cx[1]+mesh->xtra,cy[1]+mesh->ytra,cz[1]+mesh->ztra);
    fprintf(isofil,"  <%f,%f,%f>\n" ,
    cx[2]+mesh->xtra,cy[2]+mesh->ytra,cz[2]+mesh->ztra);
          fprintf(isofil,"}\n");

    fprintf(isofil,"triangle {\n");
    fprintf(isofil,"  <%f,%f,%f>,\n",
    cx[0]+mesh->xtra,cy[0]+mesh->ytra,cz[0]+mesh->ztra);
    fprintf(isofil,"  <%f,%f,%f>,\n",
    cx[2]+mesh->xtra,cy[2]+mesh->ytra,cz[2]+mesh->ztra);
    fprintf(isofil,"  <%f,%f,%f>\n" ,
    cx[3]+mesh->xtra,cy[3]+mesh->ytra,cz[3]+mesh->ztra);
          fprintf(isofil,"}\n");
        }
        else if ( !nbnul ) {
          for (l=0; l<3; l++) {
            k1 = nbneg == 3 ? neg[l] : pos[l];
            k2 = nbneg == 3 ? pos[0] : neg[0];
            p0 = &mesh->point[pt->v[k1]];
            p1 = &mesh->point[pt->v[k2]];
            ps0 = &mesh->sol[pt->v[k1]];
            ps1 = &mesh->sol[pt->v[k2]];
            cc = 0.0f;
            if ( fabs(ps1->bb-ps0->bb) > 0.0f ) 
              cc = (iso-ps0->bb) / (ps1->bb-ps0->bb);
            cx[l] = p0->c[0]+cc*(p1->c[0]-p0->c[0]);
            cy[l] = p0->c[1]+cc*(p1->c[1]-p0->c[1]);
            cz[l] = p0->c[2]+cc*(p1->c[2]-p0->c[2]);
          }
    fprintf(isofil,"triangle {\n");
    fprintf(isofil,"  <%f,%f,%f>,\n",
    cx[0]+mesh->xtra,cy[0]+mesh->ytra,cz[0]+mesh->ztra);
    fprintf(isofil,"  <%f,%f,%f>,\n",
    cx[1]+mesh->xtra,cy[1]+mesh->ytra,cz[1]+mesh->ztra);
    fprintf(isofil,"  <%f,%f,%f>\n",
    cx[2]+mesh->xtra,cy[2]+mesh->ytra,cz[2]+mesh->ztra);
          fprintf(isofil,"}\n");
        }
        k = pt->nxt;
      }
    }
    fprintf(isofil,"}\n");
  }

  fclose(isofil);
  return(1);
}
