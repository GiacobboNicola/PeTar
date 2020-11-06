// Generated by PIKG and then be modified to fit PeTar
#pragma once
#ifdef USE_FUGAKU
#include<cmath>
#include<limits>
#include<chrono>
#include <arm_sve.h>
#define PIKG_USE_FDPS_VECTOR
#include"pikg_vector.hpp"
#include"soft_ptcl.hpp"

struct EPI32{
    PIKG::F32vec pos;
    PIKG::F32 rs;
};

struct EPJ32{
    PIKG::F32vec pos;
    PIKG::F32 mass,rs;
};

struct FORCE32{
    PIKG::F32vec acc;
    PIKG::F32 pot;
    PIKG::S32 nnb;
};

struct CalcForceEpEpWithLinearCutoffFugaku{
    PIKG::F32 eps2;
    PIKG::F32 rcut2;
    PIKG::F64 G;

    CalcForceEpEpWithLinearCutoffFugaku(){}

    CalcForceEpEpWithLinearCutoffFugaku(PIKG::F32 eps2_, PIKG::F32 rcut2_, PIKG::F64 G_): eps2(eps2_), rcut2(rcut2_), G(G_) {}

    void initialize(PIKG::F32 eps2_, PIKG::F32 rcut2_, PIKG::F64 G_){
        eps2 = eps2_;
        rcut2 = rcut2_;
        G = G_;
    }

    int kernel_id = 0;

    void operator()(const EPISoft* __restrict__ epi, const int ni, const EPJSoft* __restrict__ epj, const int nj, ForceSoft* __restrict__ force, const int kernel_select = 1){

        static_assert(sizeof(EPI32) == 16,"check consistency of EPI member variable definition between PIKG source and original source");
        static_assert(sizeof(EPJ32) == 20,"check consistency of EPJ member variable definition between PIKG source and original source");
        static_assert(sizeof(FORCE32) == 20,"check consistency of FORCE member variable definition between PIKG source and original source");
        if(kernel_select>=0) kernel_id = kernel_select;
        if(kernel_id == 0){
            std::cout << "ni: " << ni << " nj:" << nj << std::endl;
            ForceSoft* force_tmp = new ForceSoft[ni];
            std::chrono::system_clock::time_point  start, end;
            double min_time = std::numeric_limits<double>::max();
            { // test Kernel_I16_J1
                for(int i=0;i<ni;i++) force_tmp[i] = force[i];
                start = std::chrono::system_clock::now();
                Kernel_I16_J1(epi,ni,epj,nj,force_tmp);
                end = std::chrono::system_clock::now();
                double elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
                std::cerr << "kerel 1: " << elapsed << " ns" << std::endl;
                if(min_time > elapsed){
                    min_time = elapsed;
                    kernel_id = 1;
                }
            }
            { // test Kernel_I1_J16
                for(int i=0;i<ni;i++) force_tmp[i] = force[i];
                start = std::chrono::system_clock::now();
                Kernel_I1_J16(epi,ni,epj,nj,force_tmp);
                end = std::chrono::system_clock::now();
                double elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
                std::cerr << "kerel 2: " << elapsed << " ns" << std::endl;
                if(min_time > elapsed){
                    min_time = elapsed;
                    kernel_id = 2;
                }
            }
            delete[] force_tmp;
        } // if(kernel_id == 0)
        if(kernel_id == 1) Kernel_I16_J1(epi,ni,epj,nj,force);
        if(kernel_id == 2) Kernel_I1_J16(epi,ni,epj,nj,force);
    } // operator() definition 

    void Kernel_I16_J1(const EPISoft* __restrict__ epi, const PIKG::S32 ni, const EPJSoft* __restrict__ epj, const PIKG::S32 nj, ForceSoft* __restrict__ force){

        //const PIKG::F32 eps2 = EPISoft::eps * EPISoft::eps;
        //const PIKG::F32 rcut2 = EPISoft::r_out*EPISoft::r_out;
        //const PIKG::F64 G = ForceSoft::grav_const;
        
        PIKG::S32 i;
        PIKG::S32 j;

        PIKG::S32 ni_act;
        PIKG::S32 nj_act;
        PIKG::S32 epi_list[ni];
        EPI32 epi_loc[ni];
        EPJ32 epj_loc[nj];
        FORCE32 force_loc[ni];

        ni_act=0;
        for(i=0; i<ni; ++i){
            if (epi[i].type==1) {
                epi_list[ni_act] = i;
                epi_loc[ni_act].pos.x = epi[i].pos.x;
                epi_loc[ni_act].pos.y = epi[i].pos.y;
                epi_loc[ni_act].pos.z = epi[i].pos.z;
                epi_loc[ni_act].rs = epi[i].r_search;

		force_loc[ni_act].acc = 0.f;
		force_loc[ni_act].pot = 0.0;
		force_loc[ni_act].nnb = 0;
                ni_act++;
            }
        }

        nj_act=0;
        for(j=0; j<nj; ++j) {
            if (epj[j].mass>0) {
                epj_loc[nj_act].pos.x = epj[j].pos.x;
                epj_loc[nj_act].pos.y = epj[j].pos.y;
                epj_loc[nj_act].pos.z = epj[j].pos.z;
                epj_loc[nj_act].mass = epj[j].mass;
                epj_loc[nj_act].rs = epj[j].r_search;
                nj_act++;
            }
        }
	
        for(i = 0;i < ((ni_act + 16 - 1)/16)*16;i += 16){
            svbool_t pg0 = svwhilelt_b32_s32(i,ni_act);
            svfloat32_t rsi;

            uint32_t index_gather_load0[16] = {0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60};
            svuint32_t vindex_gather_load0 = svld1_u32(pg0,index_gather_load0);
            rsi = svld1_gather_u32index_f32(pg0,((float*)&epi_loc[i+0].rs),vindex_gather_load0);
            svfloat32x3_t xi;

            uint32_t index_gather_load1[16] = {0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60};
            svuint32_t vindex_gather_load1 = svld1_u32(pg0,index_gather_load1);
            xi.v0 = svld1_gather_u32index_f32(pg0,((float*)&epi_loc[i+0].pos.x),vindex_gather_load1);
            uint32_t index_gather_load2[16] = {0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60};
            svuint32_t vindex_gather_load2 = svld1_u32(pg0,index_gather_load2);
            xi.v1 = svld1_gather_u32index_f32(pg0,((float*)&epi_loc[i+0].pos.y),vindex_gather_load2);
            uint32_t index_gather_load3[16] = {0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60};
            svuint32_t vindex_gather_load3 = svld1_u32(pg0,index_gather_load3);
            xi.v2 = svld1_gather_u32index_f32(pg0,((float*)&epi_loc[i+0].pos.z),vindex_gather_load3);
            svfloat32x3_t acc;

            acc.v0 = svdup_n_f32(0.0f);
            acc.v1 = svdup_n_f32(0.0f);
            acc.v2 = svdup_n_f32(0.0f);
            svint32_t nnbi;

            nnbi = svdup_n_s32(0);
            svfloat32_t pot;

            pot = svdup_n_f32(0.0f);
            for(j = 0;j < ((nj_act + 1 - 1)/1)*1;++j){
                svfloat32_t mj;

                mj = svdup_n_f32(epj_loc[j+0].mass);

                svfloat32_t rsj;

                rsj = svdup_n_f32(epj_loc[j+0].rs);

                svfloat32x3_t xj;

                xj.v0 = svdup_n_f32(epj_loc[j+0].pos.x);

                xj.v1 = svdup_n_f32(epj_loc[j+0].pos.y);

                xj.v2 = svdup_n_f32(epj_loc[j+0].pos.z);

                svfloat32x3_t rij;

                svfloat32_t __fkg_tmp2;

                svfloat32_t __fkg_tmp1;

                svfloat32_t r2;

                svfloat32_t r2_cut;

                svfloat32_t r_inv;

                svfloat32_t r2_inv;

                svfloat32_t mr_inv;

                svfloat32_t mr3_inv;

                svint32_t __fkg_tmp0;

                rij.v0 = svsub_f32_z(pg0,xi.v0,xj.v0);
                rij.v1 = svsub_f32_z(pg0,xi.v1,xj.v1);
                rij.v2 = svsub_f32_z(pg0,xi.v2,xj.v2);
		__fkg_tmp2 = svmad_f32_z(pg0,rij.v0,rij.v0,svdup_n_f32(eps2));
		__fkg_tmp1 = svmad_f32_z(pg0,rij.v1,rij.v1,__fkg_tmp2);
		r2 = svmad_f32_z(pg0,rij.v2,rij.v2,__fkg_tmp1);
		r2_cut = max(pg0,r2,svdup_n_f32(rcut2));
                r_inv = rsqrt(pg0,r2_cut);
                r2_inv = svmul_f32_z(pg0,r_inv,r_inv);
                mr_inv = svmul_f32_z(pg0,mj,r_inv);
                mr3_inv = svmul_f32_z(pg0,r2_inv,mr_inv);
                {
                    svbool_t pg2;
                    pg2 = svcmplt_f32(pg0,r2,max(pg0,svmul_f32_z(pg0,rsi,rsi),svmul_f32_z(pg0,rsj,rsj)));

                    __fkg_tmp0 = svadd_s32_z(pg2,nnbi,svdup_n_s32(1));
                    nnbi = svsel_s32(pg2,__fkg_tmp0,nnbi);;
                }

                acc.v0 = svmsb_f32_z(svptrue_b32(),mr3_inv,rij.v0,acc.v0);
                acc.v1 = svmsb_f32_z(svptrue_b32(),mr3_inv,rij.v1,acc.v1);
                acc.v2 = svmsb_f32_z(svptrue_b32(),mr3_inv,rij.v2,acc.v2);
                pot = svsub_f32_z(svptrue_b32(),pot,mr_inv);
            } // loop of j

            {
                svfloat32_t __fkg_tmp_accum;
                uint32_t index_gather_load4[16] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75};
                svuint32_t vindex_gather_load4 = svld1_u32(svptrue_b32(),index_gather_load4);
                __fkg_tmp_accum = svld1_gather_u32index_f32(svptrue_b32(),((float*)&force_loc[i+0].acc.x),vindex_gather_load4);
                __fkg_tmp_accum = svadd_f32_z(svptrue_b32(),__fkg_tmp_accum,acc.v0);
                uint32_t index_scatter_store0[16] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75};
                svuint32_t vindex_scatter_store0 = svld1_u32(svptrue_b32(),index_scatter_store0);
                svst1_scatter_u32index_f32(svptrue_b32(),((float*)&force_loc[i+0].acc.x),vindex_scatter_store0,__fkg_tmp_accum);}

            {
                svfloat32_t __fkg_tmp_accum;
                uint32_t index_gather_load5[16] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75};
                svuint32_t vindex_gather_load5 = svld1_u32(svptrue_b32(),index_gather_load5);
                __fkg_tmp_accum = svld1_gather_u32index_f32(svptrue_b32(),((float*)&force_loc[i+0].acc.y),vindex_gather_load5);
                __fkg_tmp_accum = svadd_f32_z(svptrue_b32(),__fkg_tmp_accum,acc.v1);
                uint32_t index_scatter_store1[16] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75};
                svuint32_t vindex_scatter_store1 = svld1_u32(svptrue_b32(),index_scatter_store1);
                svst1_scatter_u32index_f32(svptrue_b32(),((float*)&force_loc[i+0].acc.y),vindex_scatter_store1,__fkg_tmp_accum);}

            {
                svfloat32_t __fkg_tmp_accum;
                uint32_t index_gather_load6[16] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75};
                svuint32_t vindex_gather_load6 = svld1_u32(svptrue_b32(),index_gather_load6);
                __fkg_tmp_accum = svld1_gather_u32index_f32(svptrue_b32(),((float*)&force_loc[i+0].acc.z),vindex_gather_load6);
                __fkg_tmp_accum = svadd_f32_z(svptrue_b32(),__fkg_tmp_accum,acc.v2);
                uint32_t index_scatter_store2[16] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75};
                svuint32_t vindex_scatter_store2 = svld1_u32(svptrue_b32(),index_scatter_store2);
                svst1_scatter_u32index_f32(svptrue_b32(),((float*)&force_loc[i+0].acc.z),vindex_scatter_store2,__fkg_tmp_accum);}

            {
                svint32_t __fkg_tmp_accum;
                uint32_t index_gather_load7[16] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75};
                svuint32_t vindex_gather_load7 = svld1_u32(svptrue_b32(),index_gather_load7);
                __fkg_tmp_accum = svld1_gather_u32index_s32(svptrue_b32(),((int*)&force_loc[i+0].nnb),vindex_gather_load7);
                __fkg_tmp_accum = svadd_s32_z(svptrue_b32(),__fkg_tmp_accum,nnbi);
                uint32_t index_scatter_store3[16] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75};
                svuint32_t vindex_scatter_store3 = svld1_u32(svptrue_b32(),index_scatter_store3);
                svst1_scatter_u32index_s32(svptrue_b32(),((int*)&force_loc[i+0].nnb),vindex_scatter_store3,__fkg_tmp_accum);}

            {
                svfloat32_t __fkg_tmp_accum;
                uint32_t index_gather_load8[16] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75};
                svuint32_t vindex_gather_load8 = svld1_u32(svptrue_b32(),index_gather_load8);
                __fkg_tmp_accum = svld1_gather_u32index_f32(svptrue_b32(),((float*)&force_loc[i+0].pot),vindex_gather_load8);
                __fkg_tmp_accum = svadd_f32_z(svptrue_b32(),__fkg_tmp_accum,pot);
                uint32_t index_scatter_store4[16] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75};
                svuint32_t vindex_scatter_store4 = svld1_u32(svptrue_b32(),index_scatter_store4);
                svst1_scatter_u32index_f32(svptrue_b32(),((float*)&force_loc[i+0].pot),vindex_scatter_store4,__fkg_tmp_accum);}

        } // loop of i

        // copy back
        for(i=0; i<ni_act; ++i){
            j = epi_list[i];
            assert(epi[j].type==1);
	    force[j].acc.x += G*force_loc[i].acc.x;
	    force[j].acc.y += G*force_loc[i].acc.y;
	    force[j].acc.z += G*force_loc[i].acc.z;
	    force[j].pot += G*force_loc[i].pot;
	    force[j].n_ngb = force_loc[i].nnb;
        }

    } // Kernel_I16_J1 definition 

    void Kernel_I1_J16(const EPISoft* __restrict__ epi, const PIKG::S32 ni, const EPJSoft* __restrict__ epj, const PIKG::S32 nj, ForceSoft* __restrict__ force){
        PIKG::S32 i;
        PIKG::S32 j;

        PIKG::S32 ni_act;
        PIKG::S32 nj_act;
        PIKG::S32 epi_list[ni];
        EPI32 epi_loc[ni];
        EPJ32 epj_loc[nj];
        FORCE32 force_loc[ni];

        ni_act=0;
        for(i=0; i<ni; ++i){
            if (epi[i].type==1) {
                epi_list[ni_act] = i;
                epi_loc[ni_act].pos.x = epi[i].pos.x;
                epi_loc[ni_act].pos.y = epi[i].pos.y;
                epi_loc[ni_act].pos.z = epi[i].pos.z;
                epi_loc[ni_act].rs = epi[i].r_search;

		force_loc[ni_act].acc = 0.f;
		force_loc[ni_act].pot = 0.0;
		force_loc[ni_act].nnb = 0;
                ni_act++;
            }
        }

        nj_act=0;
        for(j=0; j<nj; ++j) {
            if (epj[j].mass>0) {
                epj_loc[nj_act].pos.x = epj[j].pos.x;
                epj_loc[nj_act].pos.y = epj[j].pos.y;
                epj_loc[nj_act].pos.z = epj[j].pos.z;
                epj_loc[nj_act].mass = epj[j].mass;
                epj_loc[nj_act].rs = epj[j].r_search;
                nj_act++;
            }
        }

        for(i = 0;i < ((ni_act + 1 - 1)/1)*1;++i){
            svfloat32_t rsi;

            rsi = svdup_n_f32(epi_loc[i+0].rs);

            svfloat32x3_t xi;

            xi.v0 = svdup_n_f32(epi_loc[i+0].pos.x);

            xi.v1 = svdup_n_f32(epi_loc[i+0].pos.y);

            xi.v2 = svdup_n_f32(epi_loc[i+0].pos.z);

            svfloat32x3_t acc;

            acc.v0 = svdup_n_f32(0.0f);
            acc.v1 = svdup_n_f32(0.0f);
            acc.v2 = svdup_n_f32(0.0f);
            svint32_t nnbi;

            nnbi = svdup_n_s32(0);
            svfloat32_t pot;

            pot = svdup_n_f32(0.0f);
            for(j = 0;j < ((nj_act + 16 - 1)/16)*16;j += 16){
                svbool_t pg0 = svwhilelt_b32_s32(j,nj_act);
                svfloat32_t mj;

                uint32_t index_gather_load9[16] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75};
                svuint32_t vindex_gather_load9 = svld1_u32(pg0,index_gather_load9);
                mj = svld1_gather_u32index_f32(pg0,((float*)&epj_loc[j+0].mass),vindex_gather_load9);
                svfloat32_t rsj;

                uint32_t index_gather_load10[16] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75};
                svuint32_t vindex_gather_load10 = svld1_u32(pg0,index_gather_load10);
                rsj = svld1_gather_u32index_f32(pg0,((float*)&epj_loc[j+0].rs),vindex_gather_load10);
                svfloat32x3_t xj;

                uint32_t index_gather_load11[16] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75};
                svuint32_t vindex_gather_load11 = svld1_u32(pg0,index_gather_load11);
                xj.v0 = svld1_gather_u32index_f32(pg0,((float*)&epj_loc[j+0].pos.x),vindex_gather_load11);
                uint32_t index_gather_load12[16] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75};
                svuint32_t vindex_gather_load12 = svld1_u32(pg0,index_gather_load12);
                xj.v1 = svld1_gather_u32index_f32(pg0,((float*)&epj_loc[j+0].pos.y),vindex_gather_load12);
                uint32_t index_gather_load13[16] = {0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75};
                svuint32_t vindex_gather_load13 = svld1_u32(pg0,index_gather_load13);
                xj.v2 = svld1_gather_u32index_f32(pg0,((float*)&epj_loc[j+0].pos.z),vindex_gather_load13);
                svfloat32x3_t rij;

                svfloat32_t __fkg_tmp2;

                svfloat32_t __fkg_tmp1;

                svfloat32_t r2;

                svfloat32_t r2_cut;

                svfloat32_t r_inv;

                svfloat32_t r2_inv;

                svfloat32_t mr_inv;

                svfloat32_t mr3_inv;

                svint32_t __fkg_tmp0;

                rij.v0 = svsub_f32_z(pg0,xi.v0,xj.v0);
                rij.v1 = svsub_f32_z(pg0,xi.v1,xj.v1);
                rij.v2 = svsub_f32_z(pg0,xi.v2,xj.v2);
                __fkg_tmp2 = svmad_f32_z(pg0,rij.v0,rij.v0,svdup_n_f32(eps2));
                __fkg_tmp1 = svmad_f32_z(pg0,rij.v1,rij.v1,__fkg_tmp2);
                r2 = svmad_f32_z(pg0,rij.v2,rij.v2,__fkg_tmp1);
                r2_cut = max(pg0,r2,svdup_n_f32(rcut2));
                r_inv = rsqrt(pg0,r2_cut);
                r2_inv = svmul_f32_z(pg0,r_inv,r_inv);
                mr_inv = svmul_f32_z(pg0,mj,r_inv);
                mr3_inv = svmul_f32_z(pg0,r2_inv,mr_inv);
                {
                    svbool_t pg2;
                    pg2 = svcmplt_f32(pg0,r2,max(pg0,svmul_f32_z(pg0,rsi,rsi),svmul_f32_z(pg0,rsj,rsj)));

                    __fkg_tmp0 = svadd_s32_z(pg2,nnbi,svdup_n_s32(1));
                    nnbi = svsel_s32(pg2,__fkg_tmp0,nnbi);;
                }

                acc.v0 = svmsb_f32_z(svptrue_b32(),mr3_inv,rij.v0,acc.v0);
                acc.v1 = svmsb_f32_z(svptrue_b32(),mr3_inv,rij.v1,acc.v1);
                acc.v2 = svmsb_f32_z(svptrue_b32(),mr3_inv,rij.v2,acc.v2);
                pot = svsub_f32_z(svptrue_b32(),pot,mr_inv);
            } // loop of j

            ((float*)&force_loc[i+0].acc.x)[0] += svaddv_f32(svptrue_b32(),acc.v0);

            ((float*)&force_loc[i+0].acc.y)[0] += svaddv_f32(svptrue_b32(),acc.v1);

            ((float*)&force_loc[i+0].acc.z)[0] += svaddv_f32(svptrue_b32(),acc.v2);

            ((int*)&force_loc[i+0].nnb)[0] += svaddv_s32(svptrue_b32(),nnbi);

            ((float*)&force_loc[i+0].pot)[0] += svaddv_f32(svptrue_b32(),pot);

        } // loop of i

        // copy back
        for(i=0; i<ni_act; ++i){
            j = epi_list[i];
            if (epi[j].type==1) {
                force[j].acc.x += G*force_loc[i].acc.x;
                force[j].acc.y += G*force_loc[i].acc.y;
                force[j].acc.z += G*force_loc[i].acc.z;
                force[j].pot += G*force_loc[i].pot;
                force[j].n_ngb = force_loc[i].nnb;
            }
        }

    } // Kernel_I1_J16 definition 

    PIKG::F64 rsqrt(PIKG::F64 op){ return 1.0/std::sqrt(op); }
    PIKG::F64 sqrt(PIKG::F64 op){ return std::sqrt(op); }
    PIKG::F64 inv(PIKG::F64 op){ return 1.0/op; }
    PIKG::F64 max(PIKG::F64 a,PIKG::F64 b){ return std::max(a,b);}
    PIKG::F64 min(PIKG::F64 a,PIKG::F64 b){ return std::min(a,b);}
    PIKG::F32 rsqrt(PIKG::F32 op){ return 1.f/std::sqrt(op); }
    PIKG::F32 sqrt(PIKG::F32 op){ return std::sqrt(op); }
    PIKG::F32 inv(PIKG::F32 op){ return 1.f/op; }
    PIKG::S64 max(PIKG::S64 a,PIKG::S64 b){ return std::max(a,b);}
    PIKG::S64 min(PIKG::S64 a,PIKG::S64 b){ return std::min(a,b);}
    PIKG::S32 max(PIKG::S32 a,PIKG::S32 b){ return std::max(a,b);}
    PIKG::S32 min(PIKG::S32 a,PIKG::S32 b){ return std::min(a,b);}
    PIKG::F64 table(PIKG::F64 tab[],PIKG::S64 i){ return tab[i]; }
    PIKG::F32 table(PIKG::F32 tab[],PIKG::S32 i){ return tab[i]; }
    PIKG::F64 to_float(PIKG::U64 op){return (PIKG::F64)op;}
    PIKG::F32 to_float(PIKG::U32 op){return (PIKG::F32)op;}
    PIKG::F64 to_float(PIKG::S64 op){return (PIKG::F64)op;}
    PIKG::F32 to_float(PIKG::S32 op){return (PIKG::F32)op;}
  //PIKG::S64   to_int(PIKG::F64 op){return (PIKG::S64)op;}
  //PIKG::S32   to_int(PIKG::F32 op){return (PIKG::S32)op;}
    PIKG::U64  to_uint(PIKG::F64 op){return (PIKG::U64)op;}
    PIKG::U32  to_uint(PIKG::F32 op){return (PIKG::U32)op;}
    template<typename T> PIKG::F64 to_f64(const T& op){return (PIKG::F64)op;}
    template<typename T> PIKG::F32 to_f32(const T& op){return (PIKG::F32)op;}
    template<typename T> PIKG::S64 to_s64(const T& op){return (PIKG::S64)op;}
    template<typename T> PIKG::S32 to_s32(const T& op){return (PIKG::S32)op;}
    template<typename T> PIKG::U64 to_u64(const T& op){return (PIKG::U64)op;}
    template<typename T> PIKG::U32 to_u32(const T& op){return (PIKG::U32)op;}
    svfloat32_t rsqrt(svbool_t pg,svfloat32_t op){
        svfloat32_t rinv = svrsqrte_f32(op);
        svfloat32_t h = svmul_f32_z(pg,op,rinv);
        h = svmsb_n_f32_z(pg,h,rinv,1.f);
        svfloat32_t poly = svmad_n_f32_z(pg,h,svdup_f32(0.375f),0.5f);
        poly = svmul_f32_z(pg,poly,h);
        rinv = svmad_f32_z(pg,rinv,poly,rinv);
        return rinv;
    }
    svfloat64_t rsqrt(svbool_t pg,svfloat64_t op){
        svfloat64_t rinv = svrsqrte_f64(op);
        svfloat64_t h = svmul_f64_z(pg,op,rinv);
        h = svmsb_n_f64_z(pg,h,rinv,1.f);
        svfloat64_t poly = svmad_n_f64_z(pg,h,svdup_f64(0.375f),0.5f);
        poly = svmul_f64_z(pg,poly,h);
        rinv = svmad_f64_z(pg,rinv,poly,rinv);
        return rinv;
    }
    svfloat32x2_t svdup_n_f32x3(PIKG::F32vec2 v){
        svfloat32x2_t ret;
        ret.v0 = svdup_n_f32(v.x);
        ret.v1 = svdup_n_f32(v.y);
        return ret;
    }
    svfloat32x3_t svdup_n_f32x3(PIKG::F32vec v){
        svfloat32x3_t ret;
        ret.v0 = svdup_n_f32(v.x);
        ret.v1 = svdup_n_f32(v.y);
        ret.v2 = svdup_n_f32(v.z);
        return ret;
    }
    svfloat32x4_t svdup_n_f32x4(PIKG::F32vec4 v){
        svfloat32x4_t ret;
        ret.v0 = svdup_n_f32(v.x);
        ret.v1 = svdup_n_f32(v.y);
        ret.v2 = svdup_n_f32(v.z);
        ret.v3 = svdup_n_f32(v.w);
        return ret;
    }
    svfloat64x2_t svdup_n_f64x3(PIKG::F64vec2 v){
        svfloat64x2_t ret;
        ret.v0 = svdup_n_f64(v.x);
        ret.v1 = svdup_n_f64(v.y);
        return ret;
    }
    svfloat64x3_t svdup_n_f64x3(PIKG::F64vec v){
        svfloat64x3_t ret;
        ret.v0 = svdup_n_f64(v.x);
        ret.v1 = svdup_n_f64(v.y);
        ret.v2 = svdup_n_f64(v.z);
        return ret;
    }
    svfloat64x4_t svdup_n_f64x4(PIKG::F64vec4 v){
        svfloat64x4_t ret;
        ret.v0 = svdup_n_f64(v.x);
        ret.v1 = svdup_n_f64(v.y);
        ret.v2 = svdup_n_f64(v.z);
        ret.v3 = svdup_n_f64(v.w);
        return ret;
    }
    svfloat32_t sqrt(svbool_t pg,svfloat32_t op){ return svsqrt_f32_z(pg,op); }
    svfloat64_t sqrt(svbool_t pg,svfloat64_t op){ return svsqrt_f64_z(pg,op); }
    svfloat32_t inv(svbool_t pg,svfloat32_t op){
        svfloat32_t x1 = svrecpe_f32(op);
        svfloat32_t x2 = svmsb_n_f32_z(pg,op,x1,2.f);
        x2 = svmul_f32_z(pg,x2,x1);
        svfloat32_t ret = svmsb_n_f32_z(pg,op,x2,2.f);
        ret = svmul_f32_z(pg,ret,x2);
        return ret;
    }
    svfloat64_t inv(svbool_t pg,svfloat64_t op){
        svfloat64_t x1 = svrecpe_f64(op);
        svfloat64_t x2 = svmsb_n_f64_z(pg,op,x1,2.f);
        x2 = svmul_f64_z(pg,x2,x1);
        svfloat64_t ret = svmsb_n_f64_z(pg,op,x2,2.f);
        ret = svmul_f64_z(pg,ret,x2);
        return ret;
    }
    svfloat64_t max(svbool_t pg,svfloat64_t a,svfloat64_t b){ return svmax_f64_z(pg,a,b);}
    svfloat64_t min(svbool_t pg,svfloat64_t a,svfloat64_t b){ return svmin_f64_z(pg,a,b);}
    svuint64_t max(svbool_t pg,svuint64_t a,svuint64_t b){ return svmax_u64_z(pg,a,b);}
    svuint64_t min(svbool_t pg,svuint64_t a,svuint64_t b){ return svmin_u64_z(pg,a,b);}
    svint64_t max(svbool_t pg,svint64_t a,svint64_t b){ return svmax_s64_z(pg,a,b);}
    svint64_t min(svbool_t pg,svint64_t a,svint64_t b){ return svmin_s64_z(pg,a,b);}
    svfloat32_t max(svbool_t pg,svfloat32_t a,svfloat32_t b){ return svmax_f32_z(pg,a,b);}
    svfloat32_t min(svbool_t pg,svfloat32_t a,svfloat32_t b){ return svmin_f32_z(pg,a,b);}
    svuint32_t max(svbool_t pg,svuint32_t a,svuint32_t b){ return svmax_u32_z(pg,a,b);}
    svuint32_t min(svbool_t pg,svuint32_t a,svuint32_t b){ return svmin_u32_z(pg,a,b);}
    svint32_t max(svbool_t pg,svint32_t a,svint32_t b){ return svmax_s32_z(pg,a,b);}
    svint32_t min(svbool_t pg,svint32_t a,svint32_t b){ return svmin_s32_z(pg,a,b);}
    svfloat64_t table(svfloat64_t tab,svuint64_t index){ return svtbl_f64(tab,index);}
    svfloat32_t table(svfloat32_t tab,svuint32_t index){ return svtbl_f32(tab,index);}
    svfloat64_t to_float(svbool_t pg, svint64_t op){ return svcvt_f64_s64_z(pg,op);}
    svfloat32_t to_float(svbool_t pg, svint32_t op){ return svcvt_f32_s32_z(pg,op);}
    svfloat64_t to_float(svbool_t pg,svuint64_t op){ return svcvt_f64_u64_z(pg,op);}
    svfloat32_t to_float(svbool_t pg,svuint32_t op){ return svcvt_f32_u32_z(pg,op);}
  //svint64_t  to_int(svbool_t pg,svfloat64_t op){ return svcvt_s64_f64_z(pg,op);}
  //svint32_t  to_int(svbool_t pg,svfloat32_t op){ return svcvt_s32_f32_z(pg,op);}
    svuint64_t to_uint(svbool_t pg,svfloat64_t op){ return svcvt_u64_f64_z(pg,op);}
    svuint32_t to_uint(svbool_t pg,svfloat32_t op){ return svcvt_u32_f32_z(pg,op);}
};// kernel functor definition 

#endif

