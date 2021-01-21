#define RE
#include <iostream>
#include <memory>
#include <complex>
typedef std::complex<double> cplxd;
typedef std::unique_ptr<cplxd[]> cplxd_ptr;
#include <grid.h>
#include <fluid.h>
#include <hamop.h>
#include <wavefunction.h>
// #include <parameter.hh>
// #include <smallHelpers.hh>
#include <powers.hh>
#include "potentials.hh"
#include <commonvars.hh>
#include "config.hh"
#ifdef MOMENTA_DATA
#include <tsurffSpectrum.hh>
#endif

int main(int argc, char **argv)
{
  clock_t t = clock();
  print_banner("re: Real time propagation");
  int ind = processOptions(argc, argv);
  //OVERRIDE VARS WITH CUSTOM INIT (defined in config.hh)
  //Passing only unprocessed options
  processCustomOptions(argc - ind, &argv[ind]);
  configVars();
  configCustomVars();
  if (re_log_interval)
    file_log = fopen_with_check(dir_name + re_fname_log, "w", verbose);
  configPulse();
  correctGridSize();
  configPotentials();
  logConfig();

  grid g_prop, g_load;
  wavefunction staticpot, wf, wf_load;
  g_prop.set_dim(qprop_dim);
  g_prop.set_ngps(long(radial_grid_size / delta_r), re_l_grid_size, 1);
  g_prop.set_delt(delta_r);
  g_prop.set_offs(0, 0, 0);
#ifdef MOMENTA_DATA
  tsurffSaveWF tsurff_save_wf(g_prop);
#endif

  hamop hamilton;
  hamilton.init(g_prop, std::ref(*vecpot_x), std::ref(*vecpot_y), std::ref(*vecpot_z), std::ref(*scalarpotx), std::ref(*imaginarypot));
  staticpot.init(g_prop.size());
  staticpot.calculate_staticpot(g_prop, hamilton);
  wf.init(g_prop.size());
  if (re_superpositions)
  {
    logAdd("Loading %d superpositions\n", super_weights.size());
    for (int i = 0; i < super_positions.size(); i += 3)
    {
      long nn = super_positions[i];
      long ll = super_positions[i + 1];
      long mm = super_positions[i + 2];
      double weight = super_weights[i / 3];
      string str_stat_state = imMakeFileName(nn, ll, mm) + string("-wf.dat");
      FILE *file_wf_ini = fopen_with_check(dir_name + str_stat_state, "r", verbose);
      g_load.set_dim(qprop_dim);
      g_load.set_ngps(long(im_radial_grid_size / delta_r), ll + 1, 1);
      g_load.set_delt(delta_r, 0.0, 0.0);
      g_load.set_offs(0, 0, 0);
      wf_load.init(g_load.size());
      wf_load.init(g_load, file_wf_ini, 0, verbose);
      wf.regrid_adding(g_prop, g_load, wf_load, weight);
      fclose(file_wf_ini);
    }
  }
  else
  {
    FILE *file_wf_ini = fopen_with_check(dir_name + re_fname_wf, "r", verbose);
    g_load.set_dim(qprop_dim);
    g_load.set_ngps(long(im_radial_grid_size / delta_r), l_qnumber + 1, 1);
    g_load.set_delt(delta_r, 0.0, 0.0);
    g_load.set_offs(0, 0, 0);
    wf_load.init(g_load.size());
    wf_load.init(g_load, file_wf_ini, 0, verbose);
    wf.regrid(g_prop, g_load, wf_load);
    fclose(file_wf_ini);
  }
  long lno_of_ts = long(duration / dt);
  I_p = real(wf.energy(0.0, g_prop, hamilton, 0, staticpot, nuclear_charge));
  gamma_K = sqrt(I_p / 2.0 / U_p);

  logSilent(g_prop);

  logAdd("Wavefunction norm on load: %le\n", wf.norm(g_prop));
  wf.normalize(g_prop);
  logAdd("Wavefunction upon normalization: %le\n", wf.norm(g_prop));

#ifdef PROJ_DATA
// TODO: Is m serviced properly?
#ifdef PROJ_DATA_CONST_L
  wavefunction states_stationary[max_n_number - l_qnumber]; //array holding stationary states
  grid g_stationary[max_n_number - l_qnumber];
  cplxd P[max_n_number - l_qnumber]; //Array to hold projections
#else
  wavefunction states_stationary[max_n_number * (max_n_number + 1) / 2]; //array holding stationary states
  grid g_stationary[max_n_number * (max_n_number + 1) / 2];
  cplxd P[max_n_number * (max_n_number + 1) / 2]; //Array to hold projections
#endif
  int indexx;
  for (int nr1 = 1; nr1 <= max_n_number; nr1++)
  {
#ifdef PROJ_DATA_CONST_L
    int lr1 = l_qnumber;
#else
    for (int lr1 = 0; lr1 < nr1; lr1++)
#endif
    {
      indexx = (nr1 - 1) * (nr1) / 2 + lr1;
      logAdd("Loading stationary state n=%d, l=%d m=%d\n", nr1, lr1, re_initial_m);
      g_stationary[indexx].set_dim(qprop_dim);
      g_stationary[indexx].set_ngps(long(im_radial_grid_size / delta_r), lr1 + 1, 1);
      g_stationary[indexx].set_delt(delta_r, 0.0, 0.0);
      g_stationary[indexx].set_offs(0, 0, 0);
      string str_stat_state =
          imMakeFileName(nr1, lr1, re_initial_m) + string("-wf.dat");
      //  str_nuclear_charge +
      // string("-n") + to_string(nr1) + str_ell + string("-m") + to_string(0);
      FILE *file_wf_stat = fopen_with_check(dir_name + str_stat_state, "r", verbose);
      states_stationary[indexx].init(g_stationary[indexx].size());
      states_stationary[indexx].init(g_stationary[indexx], file_wf_stat, 0, 0);
      fclose(file_wf_stat);
    }
  }
#endif
  if (vpotential_log)
    file_vpotential = fopen_with_check(dir_name + re_fname_vpotential, "w", verbose);
  logVecpot(vecpot_y, vecpot_y, vecpot_z, lno_of_ts);
  if (obser_log_interval)
    file_observer = fopen_with_check(dir_name + re_fname_observer, "w", verbose);
  // ********************************************************
  // ***** real time propagation ****************************
  // ********************************************************
  cplxd timestep = cplxd(dt, 0.0);
  double N, E_tot, z_expect; //Norm and Total Energy
  string fname_wf;
  if (re_log_interval)
  {
    if (qprop_dim == 34)
      logAdd("%7s/%7s%15s%15s%15s%13s\n", "STEP", "TOTAL", "NORM", "ENERGY", "vectpot_z", "ETA [h:m:s]");
    else
      logAdd("%7s/%7s%15s%15s%15s%15s%13s\n", "STEP", "TOTAL", "NORM", "ENERGY", "vectpot_x", "vectpot_y", "ETA [h:m:s]");
  }

  for (long ts = 0; ts < lno_of_ts; ts++)
  {
    const double time = dt * double(ts);
#ifdef MOMENTA_DATA
    tsurff_save_wf(wf); // save the orbitals \varphi_{\ell}(\RI) and the derivative \partial_r\varphi_{\ell}(r)|_{r=\RI}
#endif
    if (obser_log_interval && ts % obser_log_interval == 0)
    {
      // calculate total energy, norm
      fprintf(file_observer, "%15.17le ", time);
#ifdef ENERGY_DATA
      E_tot = real(wf.energy(0.0, g_prop, hamilton, 0, staticpot, nuclear_charge));
      fprintf(file_observer, "%15.17le ", E_tot);
#endif
#ifdef NORM_DATA
      N = wf.norm(g_prop);
      fprintf(file_observer, "%15.17le ", N);
#endif
#ifdef ZEXP_DATA
      z_expect = real(wf.expect_z(g_prop));
      fprintf(file_observer, "%15.17le ", z_expect);
#endif
#ifdef HHG_DATA
      cplxd accel;
      if (qprop_dim == 34)
      {
        accel = wf.accel_z(g_prop, re_initial_m, hamilton, time, 0, dt); //d^2<z>/dt^2
        fprintf(file_observer, "%15.10le %15.10le ", real(accel), imag(accel));
      }
      else if (qprop_dim == 44)
      {
        accel = wf.accel_cycl_pol_plus(g_prop, hamilton, time, 0, dt); // d^2<x+iy>/dt^2
        cplxd imagi(0.0, 1.0);
        cplxd A_p = vecpot_x->operator()(time + 0.5 * dt, 0) + imagi * vecpot_y->operator()(time + 0.5 * dt, 0);
        cplxd A_m = vecpot_x->operator()(time - 0.5 * dt, 0) + imagi * vecpot_y->operator()(time - 0.5 * dt, 0);
        cplxd E = -(A_p - A_m) / dt;
        fprintf(file_observer, "%15.10le %15.10le %15.10le %15.10le ", real(accel), imag(accel), real(E), imag(E));
      };
#endif
#ifdef PROJ_DATA
      for (int nr1 = 1; nr1 <= max_n_number; nr1++)
      {
#ifdef PROJ_DATA_CONST_L
        int lr1 = l_qnumber;
#else
        for (int lr1 = 0; lr1 < nr1; lr1++)
#endif
        {
          indexx = (nr1 - 1) * (nr1) / 2 + lr1;
          P[((nr1 - 1) * (nr1) / 2 + lr1)] = wf.project(g_prop, g_stationary[indexx], states_stationary[indexx], 0);
#ifdef PROJ_DATA_ABS
          fprintf(file_observer, "%15.17le ", real(conj(P[indexx]) * P[indexx]));
#else
          fprintf(file_observer, "%15.17le %15.10le ", real(P[indexx]), imag(P[indexx]));
#endif
        }
      }
#endif
      fprintf(file_observer, "\n");

      if (re_log_interval && (ts % (re_log_interval) == 0)) //Terminal output
      {
#ifndef ENERGY_DATA
        E_tot = real(wf.energy(0.0, g_prop, hamilton, 0, staticpot, nuclear_charge));
#endif
#ifndef NORM_DATA
        N = wf.norm(g_prop);
#endif
        if (qprop_dim == 34)
          logAdd("%7ld/%7ld%15.3e%15.3e%15.3e", ts, lno_of_ts, N, E_tot, vecpot_z->operator()(time, 0));
        else
          logAdd("%7ld/%7ld%15.3e%15.3e%15.3e%15.3e", ts, lno_of_ts, N, E_tot, vecpot_x->operator()(time, 0), vecpot_y->operator()(time, 0));

        if (ts == 0)
          logAdd("\n");
        else
        {
          float rem = (1.0 * lno_of_ts / ts - 1.0) * ((float)(clock() - t)) / CLOCKS_PER_SEC;
          logAdd(" %6d:%02d:%02d\n", long(rem) / 3600, long(rem) % 3600 / 60, (long(rem * 100) % 6000) / 100);
        }
        fflush(file_observer);
      };
    }

    if (wf_log_interval && ts % (wf_log_interval) == 0)
    {
      fname_wf = re_fname_wfsnap + string("-wf[") + to_string(ts) + string("].dat");
      logAdd("%7ld/%7ld -> saving wf snapshot to %s\n", ts, lno_of_ts, fname_wf.c_str());
      FILE *file_wf = fopen_with_check(dir_name + fname_wf, "w", verbose);
      wf.dump_to_file_sh(g_prop, file_wf, 1, verbose); // wf at timestep ts is saved
      fclose(file_wf);
    }
    // propagate one step in (real) time
    wf.propagate(timestep, time, g_prop, hamilton, 0, staticpot, re_initial_m, nuclear_charge);
  };

  if (finalwf_log)
  {
    logAdd("%7ld/%7ld -> saving final wf to %s\n", lno_of_ts, lno_of_ts, re_fname_wffinal.c_str());
    FILE *file_wf = fopen_with_check(dir_name + re_fname_wffinal, "w", verbose);
    wf.dump_to_file_sh(g_prop, file_wf, 1, verbose); // final wf is saved
    fclose(file_wf);
  }

  float sec = ((float)(clock() - t)) / CLOCKS_PER_SEC;
  logAdd("Execution time: %4dh%3dmin%3dsec\n", long(sec) / 3600, long(sec) % 3600 / 60, (long(sec * 100) % 6000) / 100);

  stop_log();
  return 0;
}