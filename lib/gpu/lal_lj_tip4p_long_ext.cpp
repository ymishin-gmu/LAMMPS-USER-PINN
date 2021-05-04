/***************************************************************************
                            lj_tip4p_long_ext.cpp
                             -------------------
                              V. Nikolskiy (HSE)

  Functions for LAMMPS access to lj/tip4p/long acceleration functions

 __________________________________________________________________________
    This file is part of the LAMMPS Accelerator Library (LAMMPS_AL)
 __________________________________________________________________________

    begin                :
    email                : thevsevak@gmail.com
 ***************************************************************************/

#include <iostream>
#include <cassert>
#include <cmath>

#include "lal_lj_tip4p_long.h"

using namespace std;
using namespace LAMMPS_AL;

static LJ_TIP4PLong<PRECISION,ACC_PRECISION> LJTIP4PLMF;

// ---------------------------------------------------------------------------
// Allocate memory on host and device and copy constants to device
// ---------------------------------------------------------------------------
int ljtip4p_long_gpu_init(const int ntypes, double **cutsq, double **host_lj1,
    double **host_lj2, double **host_lj3, double **host_lj4,
    double **offset, double *special_lj, const int inum,
    const int tH, const int tO,
    const double alpha, const double qdist,
    const int nall, const int max_nbors,  const int maxspecial,
    const double cell_size, int &gpu_mode, FILE *screen,
    double **host_cut_ljsq,
    const double host_cut_coulsq, const double host_cut_coulsqplus,
    double *host_special_coul, const double qqrd2e,
    const double g_ewald, int map_size,int max_same) {
  LJTIP4PLMF.clear();
  gpu_mode=LJTIP4PLMF.device->gpu_mode();
  double gpu_split=LJTIP4PLMF.device->particle_split();
  int first_gpu=LJTIP4PLMF.device->first_device();
  int last_gpu=LJTIP4PLMF.device->last_device();
  int world_me=LJTIP4PLMF.device->world_me();
  int gpu_rank=LJTIP4PLMF.device->gpu_rank();
  int procs_per_gpu=LJTIP4PLMF.device->procs_per_gpu();

  LJTIP4PLMF.device->init_message(screen,"lj/cut/tip4p/long/gpu",first_gpu,last_gpu);

  bool message=false;
  if (LJTIP4PLMF.device->replica_me()==0 && screen)
    message=true;

  if (message) {
    fprintf(screen,"Initializing Device and compiling on process 0...");
    fflush(screen);
  }

  int init_ok=0;
  if (world_me==0)
    init_ok=LJTIP4PLMF.init(ntypes, cutsq, host_lj1, host_lj2, host_lj3,
        host_lj4, offset, special_lj, inum,
        tH, tO, alpha, qdist, nall, max_nbors,
        maxspecial, cell_size, gpu_split, screen,
        host_cut_ljsq, host_cut_coulsq, host_cut_coulsqplus,
        host_special_coul, qqrd2e, g_ewald, map_size, max_same);

  LJTIP4PLMF.device->world_barrier();
  if (message)
    fprintf(screen,"Done.\n");

  for (int i=0; i<procs_per_gpu; i++) {
    if (message) {
      if (last_gpu-first_gpu==0)
        fprintf(screen,"Initializing Device %d on core %d...",first_gpu,i);
      else
        fprintf(screen,"Initializing Devices %d-%d on core %d...",first_gpu,
                last_gpu,i);
      fflush(screen);
    }
    if (gpu_rank==i && world_me!=0)
      init_ok=LJTIP4PLMF.init(ntypes, cutsq, host_lj1, host_lj2, host_lj3, host_lj4,
          offset, special_lj, inum,
          tH, tO, alpha, qdist, nall, max_nbors, maxspecial,
          cell_size, gpu_split, screen, host_cut_ljsq,
          host_cut_coulsq, host_cut_coulsqplus,
          host_special_coul, qqrd2e,
          g_ewald, map_size, max_same);

    LJTIP4PLMF.device->gpu_barrier();
    if (message)
      fprintf(screen,"Done.\n");
  }
  if (message)
    fprintf(screen,"\n");

  if (init_ok==0)
    LJTIP4PLMF.estimate_gpu_overhead(2);
  return init_ok;
}

void ljtip4p_long_gpu_clear() {
  LJTIP4PLMF.clear();
}

int ** ljtip4p_long_gpu_compute_n(const int ago, const int inum_full,
                        const int nall, double **host_x, int *host_type,
                        double *sublo, double *subhi,
                        tagint *tag, int *map_array, int map_size,
                        int *sametag, int max_same,
                        int **nspecial,
                        tagint **special, const bool eflag, const bool vflag,
                        const bool eatom, const bool vatom, int &host_start,
                        int **ilist, int **jnum, const double cpu_time,
                        bool &success, double *host_q, double *boxlo,
                        double *prd) {
  return LJTIP4PLMF.compute(ago, inum_full, nall, host_x, host_type, sublo,
                       subhi, tag, map_array, map_size, sametag, max_same,
                       nspecial, special, eflag, vflag, eatom,
                       vatom, host_start, ilist, jnum, cpu_time, success,
                       host_q,boxlo, prd);
}

void ljtip4p_long_gpu_compute(const int ago, const int inum_full, const int nall,
                     double **host_x, int *host_type, int *ilist, int *numj,
                     int **firstneigh, const bool eflag, const bool vflag,
                     const bool eatom, const bool vatom, int &host_start,
                     const double cpu_time, bool &success,double *host_q,
                     const int nlocal, double *boxlo, double *prd) {
  LJTIP4PLMF.compute(ago,inum_full,nall,host_x,host_type,ilist,numj,
      firstneigh,eflag,vflag,eatom,vatom,host_start,cpu_time,success,host_q,
      nlocal,boxlo,prd);
}

double ljtip4p_long_gpu_bytes() {
  return LJTIP4PLMF.host_memory_usage();
}

void ljtip4p_long_copy_molecule_data(int n, tagint* tag,
                                     int *map_array, int map_size,
                                     int *sametag, int max_same, int ago) {
  LJTIP4PLMF.copy_relations_data(n, tag, map_array, map_size, sametag, max_same, ago);
}

