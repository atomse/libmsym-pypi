#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

#include "msym.h"

void characterTableToTex(FILE *fp, msym_point_group_t *pg, const msym_character_table_t *ct);
void pointGroupToTex(FILE *fp, msym_point_group_t *pg, int l, char buf[l]);
void symmetryOperationToTex(FILE *fp, msym_symmetry_operation_t *sop, int l, char buf[l]);
void symmetrySpeciesToTex(FILE *fp, char *name);
void characterToTex(FILE *fp,int n, const msym_character_table_t *ct, int i, int j, int mode);

int main(int argc, const char * argv[]) {
    msym_error_t ret = MSYM_SUCCESS;
    const char *error = NULL;
    msym_context ctx = msymCreateContext();
    if(argc == 3){
        FILE *fp = fopen(argv[2],"w");
        if(!fp) {
            fprintf(stderr,"unable to open file %s for writing\n",argv[2]);
            return 1;
        }
        const msym_character_table_t *ct = NULL;
        msym_point_group_t *pg = NULL;
        if(MSYM_SUCCESS != (ret = msymSetPointGroupByName(ctx, argv[1]))) goto err;
        if(MSYM_SUCCESS != (ret = msymGetPointGroup(ctx, &pg))) goto err;
        if(MSYM_SUCCESS != (ret = msymGetCharacterTable(ctx, &ct))) goto err;
        characterTableToTex(fp,pg,ct);
    } else {
        fprintf(stderr, "usage tex_character_table <point group name> <filename>");
    }
    msymReleaseContext(ctx);
    return 0;
err:
    error = msymErrorString(ret);
    fprintf(stderr,"Error %s: ",error);
    error = msymGetErrorDetails();
    fprintf(stderr,"%s\n",error);
    msymReleaseContext(ctx);
    return ret;
}

void characterTableToTex(FILE *fp, msym_point_group_t *pg, const msym_character_table_t *ct){
    char buf[256];
    pointGroupToTex(fp,pg,sizeof(buf),buf);
    fprintf(fp,"\\documentclass{article}\n\
\\usepackage{tabu}\n\
\\usepackage{adjustbox}\n\
\\begin{document}\n\
\\begin{table}[h]\n\
\\caption{$%s$ character table, where $\\theta = \\frac{2\\pi}{%d}$}\n\
\\label{tab:%s_character_table}\n\
\\begin{center}\n\
\\begin{adjustbox}{max width=\\textwidth}",buf,pg->n,pg->name);

    fprintf(fp,"\\begin{tabular}{ l | ");
    for(int i = 0;i < ct->d;i++) fprintf(fp,"r ");
    fprintf(fp,"}\n$%s$ ",buf);
    for(int i = 0;i < ct->d;i++){
        symmetryOperationToTex(fp,ct->sops[i],sizeof(buf),buf);
        if(ct->classc[i] > 1) fprintf(fp,"& $%d %s$ ", ct->classc[i],buf);
        else fprintf(fp,"& $%s$ ", buf);
    }
    fprintf(fp,"\\\\ \\hline\n");
    
    for(int i = 0;i < ct->d;i++){
        symmetrySpeciesToTex(fp,ct->s[i].name);
        for(int j = 0;j < ct->d;j++){
            characterToTex(fp,pg->n,ct,i,j,2);
        }
        fprintf(fp,"\\\\\n");
    }
    
    fprintf(fp,"\\end{tabular}\n\
\\end{adjustbox}\n\
\\end{center}\n\
\\end{table}\n\
\\end{document}\n");
    
}

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288419716939937510582
#endif

#define CHARACTER_EQUAL 1.0e-9

void characterToTex(FILE *fp,int n, const msym_character_table_t *ct, int i, int j, int mode){
    double (*table)[ct->d] = (double (*)[ct->d]) ct->table;
    double c = table[i][j];
    if(mode == 0){
        fprintf(fp,"& $%.3lf$ ",c);
        return;
    }
    if(mode >= 1){
        for(int k = 0;k < 5;k++){
            if(fabs(round(c) - c) < CHARACTER_EQUAL){
                fprintf(fp,"& $%d$ ",(int) round(c));
                return;
            }
        }
    }
        
    double theta = (2*M_PI)/n;
    
    if(mode == 2){
        for(int k = 1;k < n;k++){
            if(fabs(c - 2*cos(k*theta)) < CHARACTER_EQUAL){
                if(k == 1) fprintf(fp,"& $2\\cos(\\theta)$ ");
                else fprintf(fp,"& $2\\cos(%d \\theta)$ ",k);
                return;
            } else if(fabs(c + 2*cos(k*theta)) < CHARACTER_EQUAL){
                if(k == 1) fprintf(fp,"& $-2\\cos(\\theta)$ ");
                else fprintf(fp,"& $-2\\cos(%d \\theta)$ ",k);
                return;
            }
        }
    }
    
    fprintf(fp,"& $%.3lf$ ",c);
}

void symmetrySpeciesToTex(FILE *fp, char *name){
    int prim = 0, start = 0;
    fprintf(fp,"$");
    if(name[0] == '*'){
        fprintf(fp,"^{*}");
        start = 1;
    }
    fprintf(fp,"%c",name[start]);
    int sub = name[start+1] != '\0' && name[start+1] != '\'';
    if(sub) fprintf(fp,"_{");
    for(int i = start+1;name[i] != '\0';i++){
        if(name[i] == '\'') prim++;
        else fprintf(fp,"%c",name[i]);
    }
    if(sub) fprintf(fp,"}");
    if(prim) {
        fprintf(fp,"^{");
        for(int i = 0;i < prim;i++) fprintf(fp,"\\prime");
        fprintf(fp,"}");
    }
    fprintf(fp,"$");
}


void pointGroupToTex(FILE *fp, msym_point_group_t *pg, int l, char buf[l]){
    switch(pg->type){
        case MSYM_POINT_GROUP_TYPE_Ci : snprintf(buf,l,"C_{i}"); break;
        case MSYM_POINT_GROUP_TYPE_Cs : snprintf(buf,l,"C_{s}"); break;
        case MSYM_POINT_GROUP_TYPE_Cn : snprintf(buf,l,"C_{%d}",pg->n); break;
        case MSYM_POINT_GROUP_TYPE_Cnh : snprintf(buf,l,"C_{%dh}",pg->n); break;
        case MSYM_POINT_GROUP_TYPE_Cnv : snprintf(buf,l,"C_{%dv}",pg->n); break;
        case MSYM_POINT_GROUP_TYPE_Dn : snprintf(buf,l,"D_{%d}",pg->n); break;
        case MSYM_POINT_GROUP_TYPE_Dnh : snprintf(buf,l,"D_{%dh}",pg->n); break;
        case MSYM_POINT_GROUP_TYPE_Dnd : snprintf(buf,l,"D_{%dd}",pg->n); break;
        case MSYM_POINT_GROUP_TYPE_Sn : snprintf(buf,l,"S_{%d}",pg->n); break;
        case MSYM_POINT_GROUP_TYPE_T : snprintf(buf,l,"T"); break;
        case MSYM_POINT_GROUP_TYPE_Td : snprintf(buf,l,"T_{d}"); break;
        case MSYM_POINT_GROUP_TYPE_Th : snprintf(buf,l,"T_{h}"); break;
        case MSYM_POINT_GROUP_TYPE_O : snprintf(buf,l,"O"); break;
        case MSYM_POINT_GROUP_TYPE_Oh : snprintf(buf,l,"O_{h}"); break;
        case MSYM_POINT_GROUP_TYPE_I : snprintf(buf,l,"I"); break;
        case MSYM_POINT_GROUP_TYPE_Ih : snprintf(buf,l,"I_{h}"); break;
        default : snprintf(buf,l,"?");
    }
}

void symmetryOperationToTex(FILE *fp, msym_symmetry_operation_t *sop, int l, char buf[l]){
    switch(sop->type){
        case MSYM_SYMMETRY_OPERATION_TYPE_IDENTITY : snprintf(buf,l,"\\hat{E}"); break;
        case MSYM_SYMMETRY_OPERATION_TYPE_PROPER_ROTATION :
            if(sop->order == 2){
                char *sup = NULL;
                switch(sop->orientation){
                    case MSYM_SYMMETRY_OPERATION_ORIENTATION_VERTICAL : sup = "^{\\prime}"; break;
                    case MSYM_SYMMETRY_OPERATION_ORIENTATION_DIHEDRAL : sup = "^{\\prime\\prime}"; break;
                    default : sup = ""; break;
                }
                snprintf(buf,l,"\\hat{C}_{%d}%s",sop->order,sup);
            } else if(sop->power == 1) {
                snprintf(buf,l,"\\hat{C}_{%d}",sop->order);
            } else {
                snprintf(buf,l,"\\hat{C}_{%d}^{%d}",sop->order,sop->power);
            }
            break;
        case MSYM_SYMMETRY_OPERATION_TYPE_IMPROPER_ROTATION :
            if(sop->power == 1) {
                snprintf(buf,l,"\\hat{S}_{%d}",sop->order);
            } else {
                snprintf(buf,l,"\\hat{S}_{%d}^{%d}",sop->order,sop->power);
            }
            break;
        case MSYM_SYMMETRY_OPERATION_TYPE_INVERSION : {snprintf(buf,l,"\\hat{\\imath}"); break;}
        case MSYM_SYMMETRY_OPERATION_TYPE_REFLECTION : {
            char *sub = NULL;
            switch(sop->orientation){
                case MSYM_SYMMETRY_OPERATION_ORIENTATION_HORIZONTAL : sub = "_{h}"; break;
                case MSYM_SYMMETRY_OPERATION_ORIENTATION_VERTICAL : sub = "_{v}"; break;
                case MSYM_SYMMETRY_OPERATION_ORIENTATION_DIHEDRAL : sub = "_{d}"; break;
                default : sub = ""; break;
            }
            snprintf(buf,l,"\\hat{\\sigma}%s",sub); break;
        }
    }
}
